/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _PSPRINT_CUPSMGR_HXX_
#define _PSPRINT_CUPSMGR_HXX_

#include "vcl/printerinfomanager.hxx"
#include "osl/module.h"
#include "osl/thread.h"
#include "osl/mutex.hxx"

namespace psp
{

class CUPSWrapper;
class PPDParser;

struct FPtrHash
{
    size_t operator()(const FILE* pPtr) const
    { return (size_t)pPtr; }
};

class CUPSManager : public PrinterInfoManager
{
    CUPSWrapper*                                                m_pCUPSWrapper;
    std::hash_map< FILE*, rtl::OString, FPtrHash >              m_aSpoolFiles;
    int                                                         m_nDests;
    void*                                                       m_pDests;
    bool                                                        m_bNewDests;
    std::hash_map< rtl::OUString, int, rtl::OUStringHash >      m_aCUPSDestMap;

    std::hash_map< rtl::OUString, PPDContext, rtl::OUStringHash > m_aDefaultContexts;

    rtl::OString                                                m_aUser;
    // this is a security risk, but the CUPS API demands
    // to deliver a pointer to a static buffer containing
    // the password, so this cannot be helped
    rtl::OString                                                m_aPassword;

    osl::Mutex                                                  m_aCUPSMutex;
    oslThread                                                   m_aDestThread;

    CUPSManager( CUPSWrapper* );
    virtual ~CUPSManager();

    virtual void initialize();

    void getOptionsFromDocumentSetup( const JobData& rJob, bool bBanner, int& rNumOptions, void** rOptions ) const;
    void runDests();
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
