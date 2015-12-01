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

#ifndef INCLUDED_VCL_INC_JOBSET_H
#define INCLUDED_VCL_INC_JOBSET_H

#include <rtl/ustring.hxx>
#include <vcl/prntypes.hxx>
#include <unordered_map>

// see com.sun.star.portal.client.JobSetupSystem.idl:
#define JOBSETUP_SYSTEM_WINDOWS     1
#define JOBSETUP_SYSTEM_UNIX        3
#define JOBSETUP_SYSTEM_MAC         4

struct ImplJobSetup
{
    sal_uInt16          mnRefCount;         // RefCount (only independ data)
    sal_uInt16          mnSystem;           // System - JOBSETUP_SYSTEM_xxxx
    OUString          maPrinterName;      // Printer-Name
    OUString          maDriver;           // Driver-Name
    Orientation     meOrientation;      // Orientation
    DuplexMode      meDuplexMode;       // Duplex
    sal_uInt16          mnPaperBin;         // paper bin / in tray
    Paper           mePaperFormat;      // paper format
    long            mnPaperWidth;       // paper width (100th mm)
    long            mnPaperHeight;      // paper height (100th mm)
    sal_uInt32      mnDriverDataLen;    // length of system specific data
    sal_uInt8*          mpDriverData;       // system specific data (will be streamed a byte block)
    bool            mbPapersizeFromSetup;
    std::unordered_map< OUString, OUString, OUStringHash >         maValueMap;

                    ImplJobSetup();
                    ImplJobSetup( const ImplJobSetup& rJobSetup );
                    ~ImplJobSetup();
};

// Papierformat wird wenn PAPER_USER im unabhaengigen Teil automatisch aus
// Papierbreite/hoehe berechnet
// Papierbreite/hoehe wird wenn 0 im unabhaengigen Teil automatisch aus
// Papierformat berechnet, wenn dieses ungleich PAPER_USER ist

#endif // INCLUDED_VCL_INC_JOBSET_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
