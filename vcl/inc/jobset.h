/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_JOBSET_H
#define _SV_JOBSET_H

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/prntypes.hxx>
#include <boost/unordered_map.hpp>
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
    sal_uInt16          mnRefCount;         // RefCount (only independ data)
    sal_uInt16          mnSystem;           // Sytem - JOBSETUP_SYSTEM_xxxx
    String          maPrinterName;      // Printer-Name
    String          maDriver;           // Driver-Name
    Orientation     meOrientation;      // Orientation
    DuplexMode      meDuplexMode;       // Duplex
    sal_uInt16          mnPaperBin;         // paper bin / in tray
    Paper           mePaperFormat;      // paper format
    long            mnPaperWidth;       // paper width (100th mm)
    long            mnPaperHeight;      // paper height (100th mm)
    sal_uIntPtr         mnDriverDataLen;    // length of system specific data
    sal_uInt8*          mpDriverData;       // system specific data (will be streamed a byte block)
    ::boost::unordered_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >         maValueMap;

                    ImplJobSetup();
                    ImplJobSetup( const ImplJobSetup& rJobSetup );
                    ~ImplJobSetup();
};

// Papierformat wird wenn PAPER_USER im unabhaengigen Teil automatisch aus
// Papierbreite/hoehe berechnet
// Papierbreite/hoehe wird wenn 0 im unabhaengigen Teil automatisch aus
// Papierformat berechnet, wenn dieses ungleich PAPER_USER ist

#endif  // _SV_JOBSET_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
