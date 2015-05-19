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
#include <o3tl/typed_flags_set.hxx>


// - Duplex Mode -


enum DuplexMode { DUPLEX_UNKNOWN, DUPLEX_OFF, DUPLEX_LONGEDGE, DUPLEX_SHORTEDGE };


// - Orientation -


enum Orientation { ORIENTATION_PORTRAIT, ORIENTATION_LANDSCAPE };


// - QueueInfo-Types -

enum class PrintQueueFlags
{
    NONE              = 0x00000000,
    Ready             = 0x00000001,
    Paused            = 0x00000002,
    PendingDeletion   = 0x00000004,
    Busy              = 0x00000008,
    Initializing      = 0x00000010,
    Waiting           = 0x00000020,
    WarmingUp         = 0x00000040,
    Processing        = 0x00000080,
    Printing          = 0x00000100,
    Offline           = 0x00000200,
    Error             = 0x00000400,
    StatusUnknown     = 0x00000800,
    PaperJam          = 0x00001000,
    PaperOut          = 0x00002000,
    ManualFeed        = 0x00004000,
    PaperProblem      = 0x00008000,
    IOActive          = 0x00010000,
    OutputBinFull     = 0x00020000,
    TonerLow          = 0x00040000,
    NoToner           = 0x00080000,
    PagePunt          = 0x00100000,
    UserIntervention  = 0x00200000,
    OutOfMemory       = 0x00400000,
    DoorOpen          = 0x00800000,
    PowerSave         = 0x01000000,
};
namespace o3tl
{
    template<> struct typed_flags<PrintQueueFlags> : is_typed_flags<PrintQueueFlags, 0x01ffffff> {};
}

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
