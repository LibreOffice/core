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

#ifndef INCLUDED_VCL_PRNTYPES_HXX
#define INCLUDED_VCL_PRNTYPES_HXX

#include <tools/solar.h>
#include <i18nutil/paper.hxx>


// - Duplex Mode -


enum DuplexMode { DUPLEX_UNKNOWN, DUPLEX_OFF, DUPLEX_LONGEDGE, DUPLEX_SHORTEDGE };


// - Orientation -


enum Orientation { ORIENTATION_PORTRAIT, ORIENTATION_LANDSCAPE };


// - QueueInfo-Types -


#define QUEUE_STATUS_READY              ((sal_uLong)0x00000001)
#define QUEUE_STATUS_PAUSED             ((sal_uLong)0x00000002)
#define QUEUE_STATUS_PENDING_DELETION   ((sal_uLong)0x00000004)
#define QUEUE_STATUS_BUSY               ((sal_uLong)0x00000008)
#define QUEUE_STATUS_INITIALIZING       ((sal_uLong)0x00000010)
#define QUEUE_STATUS_WAITING            ((sal_uLong)0x00000020)
#define QUEUE_STATUS_WARMING_UP         ((sal_uLong)0x00000040)
#define QUEUE_STATUS_PROCESSING         ((sal_uLong)0x00000080)
#define QUEUE_STATUS_PRINTING           ((sal_uLong)0x00000100)
#define QUEUE_STATUS_OFFLINE            ((sal_uLong)0x00000200)
#define QUEUE_STATUS_ERROR              ((sal_uLong)0x00000400)
#define QUEUE_STATUS_SERVER_UNKNOWN     ((sal_uLong)0x00000800)
#define QUEUE_STATUS_PAPER_JAM          ((sal_uLong)0x00001000)
#define QUEUE_STATUS_PAPER_OUT          ((sal_uLong)0x00002000)
#define QUEUE_STATUS_MANUAL_FEED        ((sal_uLong)0x00004000)
#define QUEUE_STATUS_PAPER_PROBLEM      ((sal_uLong)0x00008000)
#define QUEUE_STATUS_IO_ACTIVE          ((sal_uLong)0x00010000)
#define QUEUE_STATUS_OUTPUT_BIN_FULL    ((sal_uLong)0x00020000)
#define QUEUE_STATUS_TONER_LOW          ((sal_uLong)0x00040000)
#define QUEUE_STATUS_NO_TONER           ((sal_uLong)0x00080000)
#define QUEUE_STATUS_PAGE_PUNT          ((sal_uLong)0x00100000)
#define QUEUE_STATUS_USER_INTERVENTION  ((sal_uLong)0x00200000)
#define QUEUE_STATUS_OUT_OF_MEMORY      ((sal_uLong)0x00400000)
#define QUEUE_STATUS_DOOR_OPEN          ((sal_uLong)0x00800000)
#define QUEUE_STATUS_POWER_SAVE         ((sal_uLong)0x01000000)

#define QUEUE_JOBS_DONTKNOW             ((sal_uLong)0xFFFFFFFF)


// - Printer-Types -


#define PRINTER_CAPABILITIES_SUPPORTDIALOG      ((sal_uInt16)1)
#define PRINTER_CAPABILITIES_COPIES             ((sal_uInt16)2)
#define PRINTER_CAPABILITIES_COLLATECOPIES      ((sal_uInt16)3)
#define PRINTER_CAPABILITIES_SETORIENTATION     ((sal_uInt16)4)
#define PRINTER_CAPABILITIES_SETPAPERBIN        ((sal_uInt16)5)
#define PRINTER_CAPABILITIES_SETPAPERSIZE       ((sal_uInt16)6)
#define PRINTER_CAPABILITIES_SETPAPER           ((sal_uInt16)7)
#define PRINTER_CAPABILITIES_FAX                ((sal_uInt16)8)
#define PRINTER_CAPABILITIES_PDF                ((sal_uInt16)9)
#define PRINTER_CAPABILITIES_EXTERNALDIALOG     ((sal_uInt16)10)
#define PRINTER_CAPABILITIES_SETDUPLEX          ((sal_uInt16)11)
#define PRINTER_CAPABILITIES_USEPULLMODEL       ((sal_uInt16)12)

#endif // INCLUDED_VCL_PRNTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
