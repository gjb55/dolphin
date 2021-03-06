// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt/NetPlay/ChunkedProgressDialog.h"

#include <algorithm>
#include <cmath>
#include <functional>

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

#include "Common/StringUtil.h"

#include "Core/NetPlayClient.h"
#include "Core/NetPlayServer.h"

#include "DolphinQt/Settings.h"

static QString GetPlayerNameFromPID(int pid)
{
  QString player_name = QObject::tr("Invalid Player ID");
  auto client = Settings::Instance().GetNetPlayClient();
  if (!client)
    return player_name;

  for (const auto* player : client->GetPlayers())
  {
    if (player->pid == pid)
    {
      player_name = QString::fromStdString(player->name);
      break;
    }
  }
  return player_name;
}

ChunkedProgressDialog::ChunkedProgressDialog(QWidget* parent) : QDialog(parent)
{
  CreateWidgets();
  ConnectWidgets();
  setWindowTitle(tr("Data Transfer"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void ChunkedProgressDialog::CreateWidgets()
{
  m_main_layout = new QVBoxLayout;
  m_progress_box = new QGroupBox;
  m_progress_layout = new QVBoxLayout;

  m_progress_box->setLayout(m_progress_layout);

  m_main_layout->addWidget(m_progress_box);
  setLayout(m_main_layout);
}

void ChunkedProgressDialog::ConnectWidgets()
{
}

void ChunkedProgressDialog::show(const QString& title, const u64 data_size,
                                 const std::vector<int>& players)
{
  m_progress_box->setTitle(title);
  m_data_size = data_size;

  for (auto& pair : m_progress_bars)
  {
    m_progress_layout->removeWidget(pair.second);
    pair.second->deleteLater();
  }

  for (auto& pair : m_status_labels)
  {
    m_progress_layout->removeWidget(pair.second);
    pair.second->deleteLater();
  }

  m_progress_bars.clear();
  m_status_labels.clear();

  auto client = Settings::Instance().GetNetPlayClient();
  if (!client)
    return;

  for (const auto* player : client->GetPlayers())
  {
    if (std::find(players.begin(), players.end(), player->pid) == players.end())
      continue;

    m_progress_bars[player->pid] = new QProgressBar;
    m_status_labels[player->pid] = new QLabel;

    m_progress_layout->addWidget(m_progress_bars[player->pid]);
    m_progress_layout->addWidget(m_status_labels[player->pid]);
  }

  QDialog::show();
}

void ChunkedProgressDialog::SetProgress(const int pid, const u64 progress)
{
  QString player_name = GetPlayerNameFromPID(pid);

  if (!m_status_labels.count(pid))
    return;

  const float acquired = progress / 1024.0f / 1024.0f;
  const float total = m_data_size / 1024.0f / 1024.0f;
  const int prog = std::lround((static_cast<float>(progress) / m_data_size) * 100.0f);

  m_status_labels[pid]->setText(tr("%1[%2]: %3/%4 MiB")
                                    .arg(player_name, QString::number(pid),
                                         QString::fromStdString(StringFromFormat("%.2f", acquired)),
                                         QString::fromStdString(StringFromFormat("%.2f", total))));
  m_progress_bars[pid]->setValue(prog);
}
