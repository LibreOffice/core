/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: prntypes.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SV_PRNTYPES_HXX
#define _SV_PRNTYPES_HXX

#include <tools/string.hxx>
#include <vcl/sv.h>

// ---------------
// - Duplex Mode -
// ---------------

enum DuplexMode { DUPLEX_UNKNOWN, DUPLEX_OFF, DUPLEX_ON };

// ---------------
// - Orientation -
// ---------------

enum Orientation { ORIENTATION_PORTRAIT, ORIENTATION_LANDSCAPE };

// ---------
// - Paper -
// ---------

typedef USHORT Paper;
#define PAPER_A3                    ((Paper)0)
#define PAPER_A4                    ((Paper)1)
#define PAPER_A5                    ((Paper)2)
#define PAPER_B4                    ((Paper)3)
#define PAPER_B5                    ((Paper)4)
#define PAPER_LETTER                ((Paper)5)
#define PAPER_LEGAL                 ((Paper)6)
#define PAPER_TABLOID               ((Paper)7)
#define PAPER_USER                  ((Paper)8)

namespace vcl
{
struct PaperInfo
{
    String              m_aPaperName;       // user readable name of paper
    unsigned long       m_nPaperWidth;      // width in mm
    unsigned long       m_nPaperHeight;     // height in mm

    PaperInfo() : m_nPaperWidth( 0 ), m_nPaperHeight( 0 ) {}
};
}

// -------------------
// - QueueInfo-Types -
// -------------------

#define QUEUE_STATUS_READY              ((ULONG)0x00000001)
#define QUEUE_STATUS_PAUSED             ((ULONG)0x00000002)
#define QUEUE_STATUS_PENDING_DELETION   ((ULONG)0x00000004)
#define QUEUE_STATUS_BUSY               ((ULONG)0x00000008)
#define QUEUE_STATUS_INITIALIZING       ((ULONG)0x00000010)
#define QUEUE_STATUS_WAITING            ((ULONG)0x00000020)
#define QUEUE_STATUS_WARMING_UP         ((ULONG)0x00000040)
#define QUEUE_STATUS_PROCESSING         ((ULONG)0x00000080)
#define QUEUE_STATUS_PRINTING           ((ULONG)0x00000100)
#define QUEUE_STATUS_OFFLINE            ((ULONG)0x00000200)
#define QUEUE_STATUS_ERROR              ((ULONG)0x00000400)
#define QUEUE_STATUS_SERVER_UNKNOWN     ((ULONG)0x00000800)
#define QUEUE_STATUS_PAPER_JAM          ((ULONG)0x00001000)
#define QUEUE_STATUS_PAPER_OUT          ((ULONG)0x00002000)
#define QUEUE_STATUS_MANUAL_FEED        ((ULONG)0x00004000)
#define QUEUE_STATUS_PAPER_PROBLEM      ((ULONG)0x00008000)
#define QUEUE_STATUS_IO_ACTIVE          ((ULONG)0x00010000)
#define QUEUE_STATUS_OUTPUT_BIN_FULL    ((ULONG)0x00020000)
#define QUEUE_STATUS_TONER_LOW          ((ULONG)0x00040000)
#define QUEUE_STATUS_NO_TONER           ((ULONG)0x00080000)
#define QUEUE_STATUS_PAGE_PUNT          ((ULONG)0x00100000)
#define QUEUE_STATUS_USER_INTERVENTION  ((ULONG)0x00200000)
#define QUEUE_STATUS_OUT_OF_MEMORY      ((ULONG)0x00400000)
#define QUEUE_STATUS_DOOR_OPEN          ((ULONG)0x00800000)
#define QUEUE_STATUS_POWER_SAVE         ((ULONG)0x01000000)

#define QUEUE_JOBS_DONTKNOW             ((ULONG)0xFFFFFFFF)

// -----------------
// - Printer-Types -
// -----------------

#define PRINTER_CAPABILITIES_SUPPORTDIALOG      ((USHORT)1)
#define PRINTER_CAPABILITIES_COPIES             ((USHORT)2)
#define PRINTER_CAPABILITIES_COLLATECOPIES      ((USHORT)3)
#define PRINTER_CAPABILITIES_SETORIENTATION     ((USHORT)4)
#define PRINTER_CAPABILITIES_SETPAPERBIN        ((USHORT)5)
#define PRINTER_CAPABILITIES_SETPAPERSIZE       ((USHORT)6)
#define PRINTER_CAPABILITIES_SETPAPER           ((USHORT)7)
#define PRINTER_CAPABILITIES_FAX                ((USHORT)8)
#define PRINTER_CAPABILITIES_PDF                ((USHORT)9)
#define PRINTER_CAPABILITIES_EXTERNALDIALOG     ((USHORT)10)

#endif  // _SV_PRNTYPES_HXX
