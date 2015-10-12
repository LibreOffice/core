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

#ifndef INCLUDED_VCL_INC_CUPSMGR_HXX
#define INCLUDED_VCL_INC_CUPSMGR_HXX

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
    { return reinterpret_cast<size_t>(pPtr); }
};

class CUPSManager : public PrinterInfoManager
{
    std::unordered_map< FILE*, OString, FPtrHash >         m_aSpoolFiles;
    int                                                    m_nDests;
    void*                                                  m_pDests;
    bool                                                   m_bNewDests;
    std::unordered_map< OUString, int, OUStringHash >      m_aCUPSDestMap;

    std::unordered_map< OUString, PPDContext, OUStringHash > m_aDefaultContexts;

    OString                                                m_aUser;
    /** this is a security risk, but the CUPS API demands
        to deliver a pointer to a static buffer containing
        the password, so this cannot be helped*/
    OString                                                m_aPassword;

    osl::Mutex                                                  m_aCUPSMutex;
    oslThread                                                   m_aDestThread;

    osl::Mutex                                                  m_aGetPPDMutex;
    bool                                                        m_bPPDThreadRunning;

    struct PendingJob
    {
        OUString printerName;
        OUString jobTitle;
        JobData jobData;
        bool banner;
        OUString faxNumber;
        OString file;
        PendingJob( const OUString& printerName_, const OUString& jobTitle_, const JobData& jobData_,
            bool banner_, const OUString& faxNumber_, const OString& file_ )
            : printerName( printerName_ ), jobTitle( jobTitle_ ), jobData( jobData_ ), banner( banner_ ), faxNumber( faxNumber_ ), file( file_ )
            {}
        PendingJob() : banner( false ) {}
    };
    std::list< PendingJob > pendingJobs;
    bool batchMode;

    CUPSManager();
    virtual ~CUPSManager();

    virtual void initialize() override;

    static void getOptionsFromDocumentSetup( const JobData& rJob, bool bBanner, int& rNumOptions, void** rOptions );
    void runDests();
    OString threadedCupsGetPPD(const char* pPrinter);

    bool processPendingJobs();
    bool printJobs( const PendingJob& job, const std::vector< OString >& files );
public:
    static void runDestThread(void* pMgr);

    static CUPSManager* tryLoadCUPS();

    /// wraps cupsGetPPD, so unlink after use !
    const PPDParser* createCUPSParser( const OUString& rPrinter );

    const char* authenticateUser( const char* );

    virtual FILE* startSpool( const OUString& rPrinterName, bool bQuickCommand ) override;
    virtual bool endSpool( const OUString& rPrinterName, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner, const OUString& rFaxNumber ) override;
    virtual void setupJobContextData( JobData& rData ) override;

    virtual bool startBatchPrint() override;
    virtual bool flushBatchPrint() override;
    virtual bool supportsBatchPrint() const override;

    /// changes the info about a named printer
    virtual void changePrinterInfo( const OUString& rPrinter, const PrinterInfo& rNewInfo ) override;

    /// check if the printer configuration has changed
    virtual bool checkPrintersChanged( bool bWait ) override;

    // members for administration
    // disable for CUPS
    virtual bool addPrinter( const OUString& rPrinterName, const OUString& rDriverName ) override;
    virtual bool removePrinter( const OUString& rPrinterName, bool bCheckOnly = false ) override;
    virtual bool writePrinterConfig() override;
    virtual bool setDefaultPrinter( const OUString& rPrinterName ) override;
};

} // namespace psp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
