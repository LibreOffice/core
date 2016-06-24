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

class ImplJobSetup
{
private:
    sal_uInt16      mnSystem;           //< System - JOBSETUP_SYSTEM_xxxx
    OUString        maPrinterName;      //< Printer-Name
    OUString        maDriver;           //< Driver-Name
    Orientation     meOrientation;      //< Orientation
    DuplexMode      meDuplexMode;       //< Duplex
    sal_uInt16      mnPaperBin;         //< paper bin / in tray
    Paper           mePaperFormat;      //< paper format
    long            mnPaperWidth;       //< paper width (100th mm)
    long            mnPaperHeight;      //< paper height (100th mm)
    sal_uInt32      mnDriverDataLen;    //< length of system specific data
    sal_uInt8*      mpDriverData;       //< system specific data (will be streamed a byte block)
    bool            mbPapersizeFromSetup;
    std::unordered_map< OUString, OUString, OUStringHash > maValueMap;

public:
    ImplJobSetup();
    ImplJobSetup( const ImplJobSetup& rJobSetup );
    ~ImplJobSetup();

    bool operator==( const ImplJobSetup& rImplJobSetup ) const;

    sal_uInt16       GetSystem() const { return mnSystem; }
    void             SetSystem(sal_uInt16 nSystem);

    const OUString&  GetPrinterName() const { return maPrinterName; }
    void             SetPrinterName(const OUString& rPrinterName);

    const OUString&  GetDriver() const { return maDriver; }
    void             SetDriver(const OUString& rDriver);

    Orientation      GetOrientation() const { return meOrientation; }
    void             SetOrientation(Orientation eOrientation);

    DuplexMode       GetDuplexMode() const { return meDuplexMode; }
    void             SetDuplexMode(DuplexMode eDuplexMode);

    sal_uInt16       GetPaperBin() const { return mnPaperBin; }
    void             SetPaperBin(sal_uInt16 nPaperBin);

    Paper            GetPaperFormat() const { return mePaperFormat; }
    void             SetPaperFormat(Paper ePaperFormat);

    long             GetPaperWidth() const { return mnPaperWidth; }
    void             SetPaperWidth(long nWidth);

    long             GetPaperHeight() const { return mnPaperHeight; }
    void             SetPaperHeight(long nHeight);

    sal_uInt32       GetDriverDataLen() const { return mnDriverDataLen; }
    void             SetDriverDataLen(sal_uInt32 nDriverDataLen);

    const sal_uInt8* GetDriverData() const { return mpDriverData; }
    void             SetDriverData(sal_uInt8* pDriverData);

    bool             GetPapersizeFromSetup() const { return mbPapersizeFromSetup; }
    void             SetPapersizeFromSetup(bool bPapersizeFromSetup);

    const std::unordered_map< OUString, OUString, OUStringHash >& GetValueMap() const
                    { return maValueMap; }
    void            SetValueMap(const OUString& rKey, const OUString& rValue);
};

// If paper format is PAPER_USER, in the system-independent part it will
// automatically be computed from paper width/height.
// If paper width/height is 0, in the system-independent part it will
// automatically be computed from paper format, if the latter is not PAPER_USER.

#endif // INCLUDED_VCL_INC_JOBSET_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
