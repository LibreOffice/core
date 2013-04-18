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
#ifndef _MAILENUM_HXX
#define _MAILENUM_HXX

// enum ------------------------------------------------------------------

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
    MAIL_ACTION_DYING,      // Server stirbt
    MAIL_ACTION_SEND,       // Mail wurde versendet
    MAIL_ACTION_READ,       // Mail wurde als gelesen gekennzeichnet
    MAIL_ACTION_REMOVED,    // Mail wurde gel"oscht
    MAIL_ACTION_UPDATED,    // alle Mails wurden neu eingelesen
    MAIL_ACTION_NEXT,       // Sprung zur n"achsten Mail
    MAIL_ACTION_PREV        // Sprung zur vorherigen Mail
};

// Textformat zum Versenden von Nachrichten ------------------------------

#define TXTFORMAT_ASCII     ((sal_uInt8)0x01)
#define TXTFORMAT_HTML      ((sal_uInt8)0x02)
#define TXTFORMAT_RTF       ((sal_uInt8)0x04)
#define TXTFORMAT_OFFICE    ((sal_uInt8)0x08)


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
