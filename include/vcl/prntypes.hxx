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

#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>


// appears to be a copy of css::view::DuplexMode
enum class DuplexMode { Unknown, Off, LongEdge, ShortEdge };


enum class Orientation { Portrait, Landscape };


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

constexpr inline sal_uInt32 QUEUE_JOBS_DONTKNOW = 0xFFFFFFFF;


enum class PrinterCapType
{
    SupportDialog      = 1,
    Copies             = 2,
    CollateCopies      = 3,
    SetOrientation     = 4,
    SetPaperSize       = 6,
    SetPaper           = 7,
    Fax                = 8,
    PDF                = 9,
    ExternalDialog     = 10,
    UsePullModel       = 12,
};

enum class PrinterSetupMode
{
    SingleJob = 0,
    DocumentGlobal = 1
};

#endif // INCLUDED_VCL_PRNTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
