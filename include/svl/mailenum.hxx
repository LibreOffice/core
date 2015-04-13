/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVL_MAILENUM_HXX
#define INCLUDED_SVL_MAILENUM_HXX

enum MailState
{
    MAIL_STATE_SUCCESS = 0,
    MAIL_STATE_FAILURE,
    MAIL_STATE_ATTACHED_NOT_FOUND,
    MAIL_STATE_NO_MEMORY,
    MAIL_STATE_LOGIN_FAILURE,
    MAIL_STATE_RECEIVER_NOT_FOUND,
    MAIL_STATE_TOO_MANY_FILES,
    MAIL_STATE_TOO_MANY_RECEIVERS,
    MAIL_STATE_NO_RECEIVERS,
    MAIL_STATE_USER_CANCEL,
    MAIL_STATE_DRIVER_NOT_AVAILABLE
};

enum MailDriver
{
    MAIL_DRIVER_DETECT = 0,
    MAIL_DRIVER_BEGIN,
    MAIL_DRIVER_VIM = MAIL_DRIVER_BEGIN,
    MAIL_DRIVER_MAPI,
    MAIL_DRIVER_CMC,
    MAIL_DRIVER_SMP,
    MAIL_DRIVER_UNIX,
    MAIL_DRIVER_SMTP,
    MAIL_DRIVER_END
};

enum MailPriority
{
    MAIL_PRIORITY_LOW = 0,
    MAIL_PRIORITY_NORMAL,
    MAIL_PRIORITY_URGENT
};

enum MailReceiverRole
{
    MAIL_RECEIVER_TO = 0,
    MAIL_RECEIVER_CC,
    MAIL_RECEIVER_BCC,
    MAIL_RECEIVER_NEWSGROUP
};

enum MailAction
{
    MAIL_ACTION_DYING,      // server dies
    MAIL_ACTION_SEND,       // mail was sent
    MAIL_ACTION_READ,       // mail was marked as read
    MAIL_ACTION_REMOVED,    // mail was deleted
    MAIL_ACTION_UPDATED,    // all mails were updated
    MAIL_ACTION_NEXT,       // jump to next mail
    MAIL_ACTION_PREV        // jump to previous mail
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
