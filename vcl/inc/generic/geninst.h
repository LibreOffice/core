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
#include <generic/genprn.h>

class VCL_DLLPUBLIC SalYieldMutexReleaser
{
    sal_uLong m_nYieldCount;
public:
    inline SalYieldMutexReleaser();
    inline ~SalYieldMutexReleaser();
};

inline SalYieldMutexReleaser::SalYieldMutexReleaser()
{
    m_nYieldCount = GetSalData()->m_pInstance->ReleaseYieldMutex();
}

inline SalYieldMutexReleaser::~SalYieldMutexReleaser()
{
    GetSalData()->m_pInstance->AcquireYieldMutex( m_nYieldCount );
}

class VCL_DLLPUBLIC SalYieldMutex : public comphelper::SolarMutex
{
    osl::Mutex m_mutex;

protected:
    sal_uIntPtr         mnCount;
    oslThreadIdentifier mnThreadId;

public:
                        SalYieldMutex();
                        ~SalYieldMutex();

    virtual void        acquire();
    virtual void        release();
    virtual bool        tryToAcquire();

    sal_uIntPtr GetAcquireCount() const { return mnCount; }
    oslThreadIdentifier GetThreadId() const { return mnThreadId; }
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
    SalYieldMutex *mpSalYieldMutex;

public:
    SalGenericInstance( SalYieldMutex* pMutex )
        : mbPrinterInit( false ), mpSalYieldMutex( pMutex ) {}
    virtual ~SalGenericInstance();

    // Yield mutex
    virtual comphelper::SolarMutex* GetYieldMutex() SAL_OVERRIDE;
    virtual sal_uIntPtr         ReleaseYieldMutex() SAL_OVERRIDE;
    virtual void                AcquireYieldMutex( sal_uIntPtr nCount ) SAL_OVERRIDE;
    virtual bool                CheckYieldMutex() SAL_OVERRIDE;

    // Printing
    virtual SalInfoPrinter*     CreateInfoPrinter      ( SalPrinterQueueInfo* pQueueInfo,
                                                         ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual void                DestroyInfoPrinter     ( SalInfoPrinter* pPrinter ) SAL_OVERRIDE;
    virtual SalPrinter*         CreatePrinter          ( SalInfoPrinter* pInfoPrinter ) SAL_OVERRIDE;
    virtual void                DestroyPrinter         ( SalPrinter* pPrinter ) SAL_OVERRIDE;
    virtual void                GetPrinterQueueInfo    ( ImplPrnQueueList* pList ) SAL_OVERRIDE;
    virtual void                GetPrinterQueueState   ( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
    virtual void                DeletePrinterQueueInfo ( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
    virtual OUString            GetDefaultPrinter() SAL_OVERRIDE;
    virtual void                PostPrintersChanged() = 0;
    virtual void                updatePrinterUpdate() SAL_OVERRIDE;
    virtual void                jobStartedPrinterUpdate() SAL_OVERRIDE;
    virtual void                jobEndedPrinterUpdate() SAL_OVERRIDE;
    bool isPrinterInit() const { return mbPrinterInit; }
    virtual GenPspGraphics     *CreatePrintGraphics() = 0;

    // prolly belongs somewhere else ... just a font help
    static void RegisterFontSubstitutors( PhysicalFontCollection* pFontCollection );
    static int  FetchFontSubstitutionFlags();

protected:
    void configurePspInfoPrinter( PspSalInfoPrinter* pInfoPrinter,
                                  SalPrinterQueueInfo* pQueueInfo,
                                  ImplJobSetup* pSetupData );
};

inline SalGenericInstance *GetGenericInstance()
{
    return static_cast<SalGenericInstance *>(GetSalData()->m_pInstance);
}

#endif // INCLUDED_VCL_INC_GENERIC_GENINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
