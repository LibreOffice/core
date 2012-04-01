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
#ifndef _SV_GENERIC_INST_H
#define _SV_GENERIC_INST_H

#include <vcl/sv.h>
#include <osl/thread.hxx>
#include <vclpluginapi.h>
#include <vcl/solarmutex.hxx>
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

class VCL_DLLPUBLIC SalYieldMutex : public vcl::SolarMutexObject
{
protected:
    sal_uIntPtr         mnCount;
    oslThreadIdentifier mnThreadId;

public:
                        SalYieldMutex();
                        ~SalYieldMutex();

    virtual void        acquire();
    virtual void        release();
    virtual sal_Bool    tryToAcquire();

    virtual sal_uIntPtr GetAcquireCount() const { return mnCount; }
    oslThreadIdentifier GetThreadId() const { return mnThreadId; }
};

/*
 * Abstract generic class to build vclplugin's instance classes from
 */
class GenPspGraphics;
class ImplDevFontList;
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
    virtual osl::SolarMutex*    GetYieldMutex();
    virtual sal_uIntPtr         ReleaseYieldMutex();
    virtual void                AcquireYieldMutex( sal_uIntPtr nCount );
    virtual bool                CheckYieldMutex();

    // Printing
    virtual SalInfoPrinter*     CreateInfoPrinter      ( SalPrinterQueueInfo* pQueueInfo,
                                                         ImplJobSetup* pSetupData );
    virtual void                DestroyInfoPrinter     ( SalInfoPrinter* pPrinter );
    virtual SalPrinter*         CreatePrinter          ( SalInfoPrinter* pInfoPrinter );
    virtual void                DestroyPrinter         ( SalPrinter* pPrinter );
    virtual void                GetPrinterQueueInfo    ( ImplPrnQueueList* pList );
    virtual void                GetPrinterQueueState   ( SalPrinterQueueInfo* pInfo );
    virtual void                DeletePrinterQueueInfo ( SalPrinterQueueInfo* pInfo );
    virtual rtl::OUString       GetDefaultPrinter();
    virtual void                PostPrintersChanged() = 0;
    virtual void                updatePrinterUpdate();
    virtual void                jobStartedPrinterUpdate();
    virtual void                jobEndedPrinterUpdate();
    bool isPrinterInit() const { return mbPrinterInit; }
    virtual GenPspGraphics     *CreatePrintGraphics() = 0;

    // prolly belongs somewhere else ... just a font help
    static void RegisterFontSubstitutors( ImplDevFontList* pList );
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

#endif // _SV_GENERIC_INST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
