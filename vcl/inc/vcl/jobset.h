/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jobset.h,v $
 * $Revision: 1.3 $
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

#ifndef _SV_JOBSET_H
#define _SV_JOBSET_H

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/prntypes.hxx>
#include <hash_map>
#include <rtl/ustring.hxx>

// ------------------
// - JobSetup-Types -
// ------------------

// see com.sun.star.portal.client.JobSetupSystem.idl:
#define JOBSETUP_SYSTEM_DONTKNOW    0
#define JOBSETUP_SYSTEM_WINDOWS     1
#define JOBSETUP_SYSTEM_OS2         2
#define JOBSETUP_SYSTEM_UNIX        3
#define JOBSETUP_SYSTEM_MAC         4
#define JOBSETUP_SYSTEM_JAVA        5

// ----------------
// - ImplJobSetup -
// ----------------

struct ImplJobSetup
{
    USHORT          mnRefCount;         // RefCount (only independ data)
    USHORT          mnSystem;           // Sytem - JOBSETUP_SYSTEM_xxxx
    String          maPrinterName;      // Printer-Name
    String          maDriver;           // Driver-Name
    Orientation     meOrientation;      // Orientation
    USHORT          mnPaperBin;         // Papierschacht
    Paper           mePaperFormat;      // Papierformat
    long            mnPaperWidth;       // Papierbreite in 100tel mm
    long            mnPaperHeight;      // Papierhoehe in 100tel mm
    ULONG           mnDriverDataLen;    // Laenge der systemabhaengigen Daten
    BYTE*           mpDriverData;       // Systemabhaengige Daten die als Byte-Block rausgeschrieben werden
    ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >            maValueMap;

                    ImplJobSetup();
                    ImplJobSetup( const ImplJobSetup& rJobSetup );
                    ~ImplJobSetup();
};

// Papierformat wird wenn PAPER_USER im unabhaengigen Teil automatisch aus
// Papierbreite/hoehe berechnet
// Papierbreite/hoehe wird wenn 0 im unabhaengigen Teil automatisch aus
// Papierformat berechnet, wenn dieses ungleich PAPER_USER ist

#endif  // _SV_JOBSET_H
