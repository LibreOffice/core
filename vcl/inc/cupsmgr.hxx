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
    boost::unordered_map< FILE*, OString, FPtrHash >       m_aSpoolFiles;
    int                                                         m_nDests;
    void*                                                       m_pDests;
    bool                                                        m_bNewDests;
    boost::unordered_map< OUString, int, OUStringHash >       m_aCUPSDestMap;

    boost::unordered_map< OUString, PPDContext, OUStringHash > m_aDefaultContexts;

    OString                                                m_aUser;
    // this is a security risk, but the CUPS API demands
    // to deliver a pointer to a static buffer containing
    // the password, so this cannot be helped
    OString                                                m_aPassword;

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

    const PPDParser* createCUPSParser( const OUString& rPrinter );
    // wraps cupsGetPPD, so unlink after use !

    const char* authenticateUser( const char* );

    virtual FILE* startSpool( const OUString& rPrinterName, bool bQuickCommand );
    virtual int endSpool( const OUString& rPrinterName, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner );
    virtual void setupJobContextData( JobData& rData );

    // changes the info about a named printer
    virtual void changePrinterInfo( const OUString& rPrinter, const PrinterInfo& rNewInfo );

    // check if the printer configuration has changed
    virtual bool checkPrintersChanged( bool bWait );

    // members for administration (->padmin)
    // disable for CUPS
    virtual bool addPrinter( const OUString& rPrinterName, const OUString& rDriverName );
    virtual bool removePrinter( const OUString& rPrinterName, bool bCheckOnly = false );
    virtual bool writePrinterConfig();
    virtual bool setDefaultPrinter( const OUString& rPrinterName );

    virtual bool addOrRemovePossible() const;
};

} // namespace psp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
