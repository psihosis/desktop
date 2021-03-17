/*
 * Copyright (C) by Camila <hello@camila.codes>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "userstatus.h"
#include "account.h"
#include "accountstate.h"
#include "networkjobs.h"
#include "folderman.h"
#include "creds/abstractcredentials.h"
#include "theme.h"

#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

namespace OCC {

UserStatus::UserStatus(AccountState *accountState, QObject *parent)
    : QObject(parent)
    , _accountState(accountState)
    , _status("online")
    , _message("")
{
    connect(this, &UserStatus::fetchedCurrentUserStatus, _accountState, &AccountState::userStatusChanged);
}

void UserStatus::fetchStatus()
{
    if (_job) {
        _job->deleteLater();
    }

    AccountPtr account = _accountState->account();
    _job = new JsonApiJob(account, QStringLiteral("/ocs/v2.php/apps/user_status/api/v1/user_status"), this);
    connect(_job.data(), &JsonApiJob::jsonReceived, this, &UserStatus::slotFetchedCurrentStatus);
    _job->start();
}

void UserStatus::slotFetchedCurrentStatus(const QJsonDocument &json)
{
    const auto retrievedData = json.object().value("ocs").toObject().value("data").toObject();
    const auto icon = retrievedData.value("icon").toString();
    const auto message = retrievedData.value("message").toString();
    auto status = retrievedData.value("status").toString();
    _status = status;

    if(message.isEmpty()) {
        if(status == "dnd") {
            status = tr("Do not disturb");
        }
    } else {
        status = message;
    }

    _message = QString("%1 %2").arg(icon, status);
    emit fetchedCurrentUserStatus();
}

QString UserStatus::status() const
{
    return _status;
}

QString UserStatus::message() const
{
    return _message;
}

QUrl UserStatus::icon() const
{
    // online, away, dnd, invisible, offline
    if(_status == "online") {
        return Theme::instance()->statusOnlineImageSource();
    } else if (_status == "away") {
        return Theme::instance()->statusAwayImageSource();
    } else if (_status == "dnd") {
        return Theme::instance()->statusDoNotDisturbImageSource();
    } else if (_status == "invisible") {
        return Theme::instance()->statusInvisibleImageSource();
    } else if (_status == "offline") {
        return Theme::instance()->statusInvisibleImageSource();
    }

    return Theme::instance()->statusOnlineImageSource();
}

} // namespace OCC
