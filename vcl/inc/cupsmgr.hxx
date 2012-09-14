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

#ifndef _PSPRINT_CUPSMGR_HXX_
#define _PSPRINT_CUPSMGR_HXX_

#include "vcl/printerinfomanager.hxx"
#include "osl/module.h"
#include "osl/thread.h"
#include "osl/mutex.hxx"

namespace psp
{

class PPDParser;

struct FPtrHash
{
    size_t operator()(const FILE* pPtr) const
    { return (size_t)pPtr; }
};

class CUPSManager : public PrinterInfoManager
{
    boost::unordered_map< FILE*, rtl::OString, FPtrHash >               m_aSpoolFiles;
    int                                                         m_nDests;
    void*                                                       m_pDests;
    bool                                                        m_bNewDests;
    boost::unordered_map< rtl::OUString, int, rtl::OUStringHash >       m_aCUPSDestMap;

    boost::unordered_map< rtl::OUString, PPDContext, rtl::OUStringHash > m_aDefaultContexts;

    rtl::OString                                                m_aUser;
    // this is a security risk, but the CUPS API demands
    // to deliver a pointer to a static buffer containing
    // the password, so this cannot be helped
    rtl::OString                                                m_aPassword;

    osl::Mutex                                                  m_aCUPSMutex;
    oslThread                                                   m_aDestThread;

    osl::Mutex                                                  m_aGetPPDMutex;
    bool                                                        m_bPPDThreadRunning;

    CUPSManager();
    virtual ~CUPSManager();

    virtual void initialize();

    void getOptionsFromDocumentSetup( const JobData& rJob, bool bBanner, int& rNumOptions, void** rOptions ) const;
    void runDests();
    OString threadedCupsGetPPD(const char* pPrinter);
public:
    // public for stub
    static void runDestThread(void* pMgr);

    static CUPSManager* tryLoadCUPS();

    const PPDParser* createCUPSParser( const rtl::OUString& rPrinter );
    // wraps cupsGetPPD, so unlink after use !

    const char* authenticateUser( const char* );

    virtual FILE* startSpool( const rtl::OUString& rPrinterName, bool bQuickCommand );
    virtual int endSpool( const rtl::OUString& rPrinterName, const rtl::OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner );
    virtual void setupJobContextData( JobData& rData );

    // changes the info about a named printer
    virtual void changePrinterInfo( const ::rtl::OUString& rPrinter, const PrinterInfo& rNewInfo );

    // check if the printer configuration has changed
    virtual bool checkPrintersChanged( bool bWait );

    // members for administration (->padmin)
    // disable for CUPS
    virtual bool addPrinter( const rtl::OUString& rPrinterName, const ::rtl::OUString& rDriverName );
    virtual bool removePrinter( const rtl::OUString& rPrinterName, bool bCheckOnly = false );
    virtual bool writePrinterConfig();
    virtual bool setDefaultPrinter( const rtl::OUString& rPrinterName );

    virtual bool addOrRemovePossible() const;
};

} // namespace psp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
