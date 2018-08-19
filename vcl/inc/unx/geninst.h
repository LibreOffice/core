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

#ifndef INCLUDED_VCL_INC_GENERIC_GENINST_H
#define INCLUDED_VCL_INC_GENERIC_GENINST_H

#include <comphelper/solarmutex.hxx>
#include <tools/solar.h>
#include <osl/thread.hxx>
#include <vclpluginapi.h>
#include <salinst.hxx>
#include <saldatabasic.hxx>
#include <unx/genprn.h>

class VCL_DLLPUBLIC SalYieldMutex : public comphelper::SolarMutex
{
public:
    SalYieldMutex();
    virtual ~SalYieldMutex() override;
};

/*
 * Abstract generic class to build vclplugin's instance classes from
 */
class GenPspGraphics;
class PhysicalFontCollection;
class VCL_DLLPUBLIC SalGenericInstance : public SalInstance
{
protected:
    bool           mbPrinterInit;
    std::unique_ptr<SalYieldMutex> mpSalYieldMutex;

public:
    SalGenericInstance( std::unique_ptr<SalYieldMutex> pMutex )
        : mbPrinterInit( false ), mpSalYieldMutex( std::move(pMutex) ) {}
    virtual ~SalGenericInstance() override;

    // Yield mutex
    virtual comphelper::SolarMutex* GetYieldMutex() override;
    virtual sal_uInt32         ReleaseYieldMutexAll() override;
    virtual void               AcquireYieldMutex( sal_uInt32 nCount = 1 ) override;

    // Printing
    virtual SalInfoPrinter*     CreateInfoPrinter      ( SalPrinterQueueInfo* pQueueInfo,
                                                         ImplJobSetup* pSetupData ) override;
    virtual void                DestroyInfoPrinter     ( SalInfoPrinter* pPrinter ) override;
    virtual std::unique_ptr<SalPrinter> CreatePrinter  ( SalInfoPrinter* pInfoPrinter ) override;
    virtual void                GetPrinterQueueInfo    ( ImplPrnQueueList* pList ) override;
    virtual void                GetPrinterQueueState   ( SalPrinterQueueInfo* pInfo ) override;
    virtual void                DeletePrinterQueueInfo ( SalPrinterQueueInfo* pInfo ) override;
    virtual OUString            GetDefaultPrinter() override;
    virtual void                PostPrintersChanged() = 0;
    virtual void                updatePrinterUpdate() override;
    virtual void                jobStartedPrinterUpdate() override;
    virtual void                jobEndedPrinterUpdate() override;
    bool isPrinterInit() const { return mbPrinterInit; }
    virtual GenPspGraphics     *CreatePrintGraphics() = 0;

    virtual OUString            getOSVersion() override;

    // prolly belongs somewhere else ... just a font help
    static void RegisterFontSubstitutors( PhysicalFontCollection* pFontCollection );

protected:
    static void configurePspInfoPrinter( PspSalInfoPrinter* pInfoPrinter,
                                         SalPrinterQueueInfo const * pQueueInfo,
                                         ImplJobSetup* pSetupData );
};

inline SalGenericInstance *GetGenericInstance()
{
    return static_cast<SalGenericInstance *>(GetSalData()->m_pInstance);
}

#endif // INCLUDED_VCL_INC_GENERIC_GENINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
