/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

