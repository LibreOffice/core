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

#include <cups/cups.h>
#include <cups/http.h>
#include <cups/ipp.h>
#include <cups/ppd.h>

#include <unistd.h>

#include <unx/cupsmgr.hxx>

#include <o3tl/string_view.hxx>
#include <osl/thread.h>
#include <osl/file.h>
#include <osl/conditn.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <officecfg/Office/Common.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>

#include <algorithm>
#include <cstddef>
#include <string_view>

using namespace psp;
using namespace osl;

namespace {

struct GetPPDAttribs
{
    osl::Condition      m_aCondition;
    OString             m_aParameter;
    OString             m_aResult;
    int                 m_nRefs;
    bool*               m_pResetRunning;
    osl::Mutex*         m_pSyncMutex;

    GetPPDAttribs( const char * m_pParameter,
                   bool* pResetRunning, osl::Mutex* pSyncMutex )
            : m_aParameter( m_pParameter ),
              m_pResetRunning( pResetRunning ),
              m_pSyncMutex( pSyncMutex )
    {
        m_nRefs = 2;
        m_aCondition.reset();
    }

    ~GetPPDAttribs()
    {
        if( !m_aResult.isEmpty() )
            unlink( m_aResult.getStr() );
    }

    void unref()
    {
        if( --m_nRefs == 0 )
        {
            *m_pResetRunning = false;
            delete this;
        }
    }

    void executeCall()
    {
        // This CUPS method is not at all thread-safe we need
        // to dup the pointer to a static buffer it returns ASAP
SAL_WNODEPRECATED_DECLARATIONS_PUSH
        const char* pResult = cupsGetPPD(m_aParameter.getStr());
        OString aResult = pResult ? OString(pResult) : OString();
SAL_WNODEPRECATED_DECLARATIONS_POP
        MutexGuard aGuard( *m_pSyncMutex );
        m_aResult = std::move(aResult);
        m_aCondition.set();
        unref();
    }

    OString waitResult( TimeValue const *pDelay )
    {
        m_pSyncMutex->release();

        if (m_aCondition.wait( pDelay ) != Condition::result_ok
            )
        {
            SAL_WARN("vcl.unx.print",
                    "cupsGetPPD " << m_aParameter << " timed out");
        }
        m_pSyncMutex->acquire();

        OString aRetval = m_aResult;
        m_aResult.clear();
        unref();

        return aRetval;
    }
};

}

extern "C" {
    static void getPPDWorker(void* pData)
    {
        osl_setThreadName("CUPSManager getPPDWorker");
        GetPPDAttribs* pAttribs = static_cast<GetPPDAttribs*>(pData);
        pAttribs->executeCall();
    }
}

OString CUPSManager::threadedCupsGetPPD( const char* pPrinter )
{
    OString aResult;

    m_aGetPPDMutex.acquire();
    // if one thread hangs in cupsGetPPD already, don't start another
    if( ! m_bPPDThreadRunning )
    {
        m_bPPDThreadRunning = true;
        GetPPDAttribs* pAttribs = new GetPPDAttribs( pPrinter,
                                                     &m_bPPDThreadRunning,
                                                     &m_aGetPPDMutex );

        oslThread aThread = osl_createThread( getPPDWorker, pAttribs );

        TimeValue aValue;
        aValue.Seconds = 5;
        aValue.Nanosec = 0;

        // NOTE: waitResult release and acquires the GetPPD mutex
        aResult = pAttribs->waitResult( &aValue );
        osl_destroyThread( aThread );
    }
    m_aGetPPDMutex.release();

    return aResult;
}

static const char* setPasswordCallback( const char* /*pIn*/ )
{
    const char* pRet = nullptr;

    PrinterInfoManager& rMgr = PrinterInfoManager::get();
    if( rMgr.getType() == PrinterInfoManager::Type::CUPS ) // sanity check
        pRet = static_cast<CUPSManager&>(rMgr).authenticateUser();
    return pRet;
}

/*
 *  CUPSManager class
 */

CUPSManager* CUPSManager::tryLoadCUPS()
{
    CUPSManager* pManager = nullptr;
    static const char* pEnv = getenv("SAL_DISABLE_CUPS");

    if (!pEnv || !*pEnv)
        pManager = new CUPSManager();
    return pManager;
}

extern "C"
{
static void run_dest_thread_stub( void* pThis )
{
    osl_setThreadName("CUPSManager cupsGetDests");
    CUPSManager::runDestThread( pThis );
}
}

CUPSManager::CUPSManager() :
        PrinterInfoManager( PrinterInfoManager::Type::CUPS ),
        m_nDests( 0 ),
        m_pDests( nullptr ),
        m_bNewDests( false ),
        m_bPPDThreadRunning( false )
{
    m_aDestThread = osl_createThread( run_dest_thread_stub, this );
}

CUPSManager::~CUPSManager()
{
    if( m_aDestThread )
    {
        osl_joinWithThread( m_aDestThread );
        osl_destroyThread( m_aDestThread );
    }

    if (m_nDests && m_pDests)
        cupsFreeDests( m_nDests, m_pDests );
}

void CUPSManager::runDestThread( void* pThis )
{
    static_cast<CUPSManager*>(pThis)->runDests();
}

void CUPSManager::runDests()
{
    SAL_INFO("vcl.unx.print", "starting cupsGetDests");
    cups_dest_t* pDests = nullptr;

    // n#722902 - do a fast-failing check for cups working *at all* first
    http_t* p_http;
SAL_WNODEPRECATED_DECLARATIONS_PUSH
    if( (p_http=httpConnectEncrypt(
             cupsServer(),
             ippPort(),
             cupsEncryption())) == nullptr )
        return;

    int nDests = cupsGetDests2(p_http,  &pDests);
    SAL_INFO("vcl.unx.print", "came out of cupsGetDests");

    osl::MutexGuard aGuard( m_aCUPSMutex );
    m_nDests = nDests;
    m_pDests = pDests;
    m_bNewDests = true;
    SAL_INFO("vcl.unx.print", "finished cupsGetDests");

    httpClose(p_http);
SAL_WNODEPRECATED_DECLARATIONS_POP
}

static void SetIfCustomOption(PPDContext& rContext, const cups_option_t& rOption, rtl_TextEncoding aEncoding)
{
    if (strncmp(rOption.value, RTL_CONSTASCII_STRINGPARAM("Custom.")) == 0)
    {
        const PPDParser* pParser = rContext.getParser();
        if (!pParser)
        {
            // normal for first sight of this printer
            return;
        }

        const PPDKey* pKey = pParser->getKey(OStringToOUString(rOption.name, aEncoding));
        if (!pKey)
        {
            SAL_WARN("vcl.unx.print", "Custom key " << rOption.name << " not found");
            return;
        }

        const PPDValue* pCustomValue = rContext.getValue(pKey);
        if (!pCustomValue)
        {
            SAL_WARN("vcl.unx.print", "Value for " << rOption.name << " not found");
            return;
        }

        if (!pCustomValue->m_bCustomOption)
        {
            SAL_WARN("vcl.unx.print", "Value for " << rOption.name << " not set to custom option");
            return;
        }

        // seems sensible to keep a value the user explicitly set even if lpoptions was used to set
        // another default
        if (pCustomValue->m_bCustomOptionSetViaApp)
            return;
        pCustomValue->m_aCustomOption = OStringToOUString(rOption.value, aEncoding);
    }
}

void CUPSManager::initialize()
{
    // get normal printers, clear printer list
    PrinterInfoManager::initialize();

    // check whether thread has completed
    // if not behave like old printing system
    osl::MutexGuard aGuard( m_aCUPSMutex );

    if( ! m_bNewDests )
        return;

    // dest thread has run, clean up
    if( m_aDestThread )
    {
        osl_joinWithThread( m_aDestThread );
        osl_destroyThread( m_aDestThread );
        m_aDestThread = nullptr;
    }
    m_bNewDests = false;

    // clear old stuff
    m_aCUPSDestMap.clear();

    if( ! (m_nDests && m_pDests ) )
        return;

    // check for CUPS server(?) > 1.2
    // since there is no API to query, check for options that were
    // introduced in dests with 1.2
    // this is needed to check for %%IncludeFeature support
    // (#i65684#, #i65491#)
    cups_dest_t* pDest = m_pDests;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    int nPrinter = m_nDests;

    // reset global default PPD options; these are queried on demand from CUPS
    m_aGlobalDefaults.m_pParser = nullptr;
    m_aGlobalDefaults.m_aContext = PPDContext();

    // add CUPS printers, should there be a printer
    // with the same name as a CUPS printer, overwrite it
    while( nPrinter-- )
    {
        pDest = m_pDests+nPrinter;
        OUString aPrinterName = OStringToOUString( pDest->name, aEncoding );
        if( pDest->instance && *pDest->instance )
        {
            aPrinterName += "/" +
                OStringToOUString( pDest->instance, aEncoding );
        }

        // initialize printer with possible configuration from psprint.conf
        bool bSetToGlobalDefaults = m_aPrinters.find( aPrinterName ) == m_aPrinters.end();
        Printer aPrinter = m_aPrinters[ aPrinterName ];
        if( bSetToGlobalDefaults )
            aPrinter.m_aInfo = m_aGlobalDefaults;
        aPrinter.m_aInfo.m_aPrinterName = aPrinterName;
        if( pDest->is_default )
            m_aDefaultPrinter = aPrinterName;

        // note: the parser that goes with the PrinterInfo
        // is created implicitly by the JobData::operator=()
        // when it detects the NULL ptr m_pParser.
        // if we wanted to fill in the parser here this
        // would mean we'd have to download PPDs for each and
        // every printer - which would be really bad runtime
        // behaviour
        aPrinter.m_aInfo.m_pParser = nullptr;
        aPrinter.m_aInfo.m_aContext.setParser( nullptr );
        std::unordered_map< OUString, PPDContext >::const_iterator c_it = m_aDefaultContexts.find( aPrinterName );
        if( c_it != m_aDefaultContexts.end() )
        {
            aPrinter.m_aInfo.m_pParser = c_it->second.getParser();
            aPrinter.m_aInfo.m_aContext = c_it->second;
        }
        aPrinter.m_aInfo.m_aDriverName = "CUPS:" + aPrinterName;

        for( int k = 0; k < pDest->num_options; k++ )
        {
            if(!strcmp(pDest->options[k].name, "printer-info"))
                aPrinter.m_aInfo.m_aComment=OStringToOUString(pDest->options[k].value, aEncoding);
            if(!strcmp(pDest->options[k].name, "printer-location"))
                aPrinter.m_aInfo.m_aLocation=OStringToOUString(pDest->options[k].value, aEncoding);
            if(!strcmp(pDest->options[k].name, "auth-info-required"))
                aPrinter.m_aInfo.m_aAuthInfoRequired=OStringToOUString(pDest->options[k].value, aEncoding);
            // tdf#149439 Update Custom values that may have changed if this is not a newly discovered printer
            SetIfCustomOption(aPrinter.m_aInfo.m_aContext, pDest->options[k], aEncoding);
        }

        m_aPrinters[ aPrinter.m_aInfo.m_aPrinterName ] = aPrinter;
        m_aCUPSDestMap[ aPrinter.m_aInfo.m_aPrinterName ] = nPrinter;
    }

    // remove everything that is not a CUPS printer and not
    // a special purpose printer (PDF, Fax)
    std::unordered_map< OUString, Printer >::iterator it = m_aPrinters.begin();
    while(it != m_aPrinters.end())
    {
        if( m_aCUPSDestMap.find( it->first ) != m_aCUPSDestMap.end() )
        {
            ++it;
            continue;
        }

        if( !it->second.m_aInfo.m_aFeatures.isEmpty() )
        {
            ++it;
            continue;
        }
        it = m_aPrinters.erase(it);
    }

    cupsSetPasswordCB( setPasswordCallback );
}

static void updatePrinterContextInfo( ppd_group_t* pPPDGroup, PPDContext& rContext )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    for( int i = 0; i < pPPDGroup->num_options; i++ )
    {
        ppd_option_t* pOption = pPPDGroup->options + i;
        for( int n = 0; n < pOption->num_choices; n++ )
        {
            ppd_choice_t* pChoice = pOption->choices + n;
            if( pChoice->marked )
            {
                const PPDKey* pKey = rContext.getParser()->getKey( OStringToOUString( pOption->keyword, aEncoding ) );
                if( pKey )
                {
                    const PPDValue* pValue = pKey->getValue( OStringToOUString( pChoice->choice, aEncoding ) );
                    if( pValue )
                    {
                        if( pValue != pKey->getDefaultValue() )
                        {
                            rContext.setValue( pKey, pValue, true );
                            SAL_INFO("vcl.unx.print", "key " << pOption->keyword << " is set to " << pChoice->choice);

                        }
                        else
                            SAL_INFO("vcl.unx.print", "key " << pOption->keyword << " is defaulted to " << pChoice->choice);
                    }
                    else
                        SAL_INFO("vcl.unx.print", "caution: value " << pChoice->choice << " not found in key " << pOption->keyword);
                }
                else
                    SAL_INFO("vcl.unx.print", "caution: key " << pOption->keyword << " not found in parser");
            }
        }
    }

    // recurse through subgroups
    for( int g = 0; g < pPPDGroup->num_subgroups; g++ )
    {
        updatePrinterContextInfo( pPPDGroup->subgroups + g, rContext );
    }
}

const PPDParser* CUPSManager::createCUPSParser( const OUString& rPrinter )
{
    const PPDParser* pNewParser = nullptr;
    OUString aPrinter;

    if( rPrinter.startsWith("CUPS:") )
        aPrinter = rPrinter.copy( 5 );
    else
        aPrinter = rPrinter;

    if( m_aCUPSMutex.tryToAcquire() )
    {
        if (m_nDests && m_pDests)
        {
            std::unordered_map< OUString, int >::iterator dest_it =
            m_aCUPSDestMap.find( aPrinter );
            if( dest_it != m_aCUPSDestMap.end() )
            {
                cups_dest_t* pDest = m_pDests + dest_it->second;
                OString aPPDFile = threadedCupsGetPPD( pDest->name );
                SAL_INFO("vcl.unx.print",
                        "PPD for " << aPrinter << " is " << aPPDFile);
                if( !aPPDFile.isEmpty() )
                {
                    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
                    OUString aFileName( OStringToOUString( aPPDFile, aEncoding ) );
                    // update the printer info with context information
SAL_WNODEPRECATED_DECLARATIONS_PUSH
                    ppd_file_t* pPPD = ppdOpenFile( aPPDFile.getStr() );
SAL_WNODEPRECATED_DECLARATIONS_POP
                    if( pPPD )
                    {
                        // create the new parser
                        PPDParser* pCUPSParser = new PPDParser( aFileName );
                        pCUPSParser->m_aFile = rPrinter;
                        pNewParser = pCUPSParser;

SAL_WNODEPRECATED_DECLARATIONS_PUSH
                        /*int nConflicts =*/ cupsMarkOptions( pPPD, pDest->num_options, pDest->options );
SAL_WNODEPRECATED_DECLARATIONS_POP
                        SAL_INFO("vcl.unx.print", "processing the following options for printer " << pDest->name << " (instance " << (pDest->instance == nullptr ? "null" : pDest->instance) << "):");
                        for( int k = 0; k < pDest->num_options; k++ )
                            SAL_INFO("vcl.unx.print",
                                "   \"" << pDest->options[k].name <<
                                "\" = \"" << pDest->options[k].value << "\"");
                        PrinterInfo& rInfo = m_aPrinters[ aPrinter ].m_aInfo;

                        // remember the default context for later use
                        PPDContext& rContext = m_aDefaultContexts[ aPrinter ];
                        rContext.setParser( pNewParser );
                        // set system default paper; printer CUPS PPD options
                        // may overwrite it
                        setDefaultPaper( rContext );
                        for( int i = 0; i < pPPD->num_groups; i++ )
                            updatePrinterContextInfo( pPPD->groups + i, rContext );

                        // tdf#149439 Set Custom values.
                        for (int k = 0; k < pDest->num_options; ++k)
                            SetIfCustomOption(rContext, pDest->options[k], aEncoding);

                        rInfo.m_pParser = pNewParser;
                        rInfo.m_aContext = rContext;

                        // clean up the mess
SAL_WNODEPRECATED_DECLARATIONS_PUSH
                        ppdClose( pPPD );
SAL_WNODEPRECATED_DECLARATIONS_POP

                    }
                    else
                        SAL_INFO("vcl.unx.print", "ppdOpenFile failed, falling back to generic driver");

                    // remove temporary PPD file
                    if (!getenv("SAL_CUPS_PPD_RETAIN_TMP"))
                        unlink( aPPDFile.getStr() );
                }
                else
                    SAL_INFO("vcl.unx.print", "cupsGetPPD failed, falling back to generic driver");
            }
            else
                SAL_INFO("vcl.unx.print", "no dest found for printer " << aPrinter);
        }
        m_aCUPSMutex.release();
    }
    else
        SAL_WARN("vcl.unx.print", "could not acquire CUPS mutex !!!" );

    if( ! pNewParser )
    {
        // get the default PPD
        pNewParser = PPDParser::getParser( u"SGENPRT"_ustr );
        SAL_INFO("vcl.unx.print", "Parsing default SGENPRT PPD" );

        PrinterInfo& rInfo = m_aPrinters[ aPrinter ].m_aInfo;

        rInfo.m_pParser = pNewParser;
        rInfo.m_aContext.setParser( pNewParser );
    }

    return pNewParser;
}

void CUPSManager::setupJobContextData( JobData& rData )
{
    std::unordered_map< OUString, int >::iterator dest_it =
        m_aCUPSDestMap.find( rData.m_aPrinterName );

    if( dest_it == m_aCUPSDestMap.end() )
        return PrinterInfoManager::setupJobContextData( rData );

    std::unordered_map< OUString, Printer >::iterator p_it =
        m_aPrinters.find( rData.m_aPrinterName );
    if( p_it == m_aPrinters.end() ) // huh ?
    {
        SAL_WARN("vcl.unx.print", "CUPS printer list in disorder, "
                "no dest for printer " << rData.m_aPrinterName);
        return;
    }

    if( p_it->second.m_aInfo.m_pParser == nullptr )
    {
        // in turn calls createCUPSParser
        // which updates the printer info
        p_it->second.m_aInfo.m_pParser = PPDParser::getParser( p_it->second.m_aInfo.m_aDriverName );
    }
    if( p_it->second.m_aInfo.m_aContext.getParser() == nullptr )
    {
        OUString aPrinter;
        if( p_it->second.m_aInfo.m_aDriverName.startsWith("CUPS:") )
            aPrinter = p_it->second.m_aInfo.m_aDriverName.copy( 5 );
        else
            aPrinter = p_it->second.m_aInfo.m_aDriverName;

        p_it->second.m_aInfo.m_aContext = m_aDefaultContexts[ aPrinter ];
    }

    rData.m_pParser     = p_it->second.m_aInfo.m_pParser;
    rData.m_aContext    = p_it->second.m_aInfo.m_aContext;
}

FILE* CUPSManager::startSpool( const OUString& rPrintername, bool bQuickCommand )
{
    SAL_INFO( "vcl.unx.print", "startSpool: " << rPrintername << " " << (bQuickCommand ? "true" : "false") );

    if( m_aCUPSDestMap.find( rPrintername ) == m_aCUPSDestMap.end() )
    {
        SAL_INFO( "vcl.unx.print", "defer to PrinterInfoManager::startSpool" );
        return PrinterInfoManager::startSpool( rPrintername, bQuickCommand );
    }

    OUString aTmpURL, aTmpFile;
    osl_createTempFile( nullptr, nullptr, &aTmpURL.pData );
    osl_getSystemPathFromFileURL( aTmpURL.pData, &aTmpFile.pData );
    OString aSysFile = OUStringToOString( aTmpFile, osl_getThreadTextEncoding() );
    FILE* fp = fopen( aSysFile.getStr(), "w" );
    if( fp )
        m_aSpoolFiles[fp] = aSysFile;

    return fp;
}

namespace {

struct less_ppd_key
{
    bool operator()(const PPDKey* left, const PPDKey* right)
    { return left->getOrderDependency() < right->getOrderDependency(); }
};

}

void CUPSManager::getOptionsFromDocumentSetup( const JobData& rJob, bool bBanner, int& rNumOptions, void** rOptions )
{
    rNumOptions = 0;
    *rOptions = nullptr;

    // emit features ordered to OrderDependency
    // ignore features that are set to default

    // sanity check
    if( rJob.m_pParser == rJob.m_aContext.getParser() && rJob.m_pParser )
    {
        std::size_t i;
        std::size_t nKeys = rJob.m_aContext.countValuesModified();
        ::std::vector< const PPDKey* > aKeys( nKeys );
        for(  i = 0; i < nKeys; i++ )
            aKeys[i] = rJob.m_aContext.getModifiedKey( i );
        ::std::sort( aKeys.begin(), aKeys.end(), less_ppd_key() );

        for( i = 0; i < nKeys; i++ )
        {
            const PPDKey* pKey = aKeys[i];
            const PPDValue* pValue = rJob.m_aContext.getValue( pKey );
            OUString sPayLoad;
            if (pValue && pValue->m_eType == eInvocation)
            {
                sPayLoad = pValue->m_bCustomOption ? pValue->m_aCustomOption : pValue->m_aOption;
            }

            if (!sPayLoad.isEmpty())
            {
                OString aKey = OUStringToOString( pKey->getKey(), RTL_TEXTENCODING_ASCII_US );
                OString aValue = OUStringToOString( sPayLoad, RTL_TEXTENCODING_ASCII_US );
                rNumOptions = cupsAddOption( aKey.getStr(), aValue.getStr(), rNumOptions, reinterpret_cast<cups_option_t**>(rOptions) );
            }
        }
    }

    if( rJob.m_nCopies > 1 )
    {
        OString aVal( OString::number( rJob.m_nCopies ) );
        rNumOptions = cupsAddOption( "copies", aVal.getStr(), rNumOptions, reinterpret_cast<cups_option_t**>(rOptions) );
        aVal = OString::boolean(rJob.m_bCollate);
        rNumOptions = cupsAddOption( "collate", aVal.getStr(), rNumOptions, reinterpret_cast<cups_option_t**>(rOptions) );
    }
    if( ! bBanner )
    {
        rNumOptions = cupsAddOption( "job-sheets", "none", rNumOptions, reinterpret_cast<cups_option_t**>(rOptions) );
    }
}

namespace
{
    class RTSPWDialog : public weld::GenericDialogController
    {
        std::unique_ptr<weld::Label> m_xText;
        std::unique_ptr<weld::Label> m_xDomainLabel;
        std::unique_ptr<weld::Entry> m_xDomainEdit;
        std::unique_ptr<weld::Label> m_xUserLabel;
        std::unique_ptr<weld::Entry> m_xUserEdit;
        std::unique_ptr<weld::Label> m_xPassLabel;
        std::unique_ptr<weld::Entry> m_xPassEdit;

    public:
        RTSPWDialog(weld::Window* pParent, std::string_view rServer, std::string_view rUserName);

        OString getDomain() const
        {
            return OUStringToOString( m_xDomainEdit->get_text(), osl_getThreadTextEncoding() );
        }

        OString getUserName() const
        {
            return OUStringToOString( m_xUserEdit->get_text(), osl_getThreadTextEncoding() );
        }

        OString getPassword() const
        {
            return OUStringToOString( m_xPassEdit->get_text(), osl_getThreadTextEncoding() );
        }

        void SetDomainVisible(bool bShow)
        {
            m_xDomainLabel->set_visible(bShow);
            m_xDomainEdit->set_visible(bShow);
        }

        void SetUserVisible(bool bShow)
        {
            m_xUserLabel->set_visible(bShow);
            m_xUserEdit->set_visible(bShow);
        }

        void SetPassVisible(bool bShow)
        {
            m_xPassLabel->set_visible(bShow);
            m_xPassEdit->set_visible(bShow);
        }
    };

    RTSPWDialog::RTSPWDialog(weld::Window* pParent, std::string_view rServer, std::string_view rUserName)
        : GenericDialogController(pParent, u"vcl/ui/cupspassworddialog.ui"_ustr, u"CUPSPasswordDialog"_ustr)
        , m_xText(m_xBuilder->weld_label(u"text"_ustr))
        , m_xDomainLabel(m_xBuilder->weld_label(u"label3"_ustr))
        , m_xDomainEdit(m_xBuilder->weld_entry(u"domain"_ustr))
        , m_xUserLabel(m_xBuilder->weld_label(u"label1"_ustr))
        , m_xUserEdit(m_xBuilder->weld_entry(u"user"_ustr))
        , m_xPassLabel(m_xBuilder->weld_label(u"label2"_ustr))
        , m_xPassEdit(m_xBuilder->weld_entry(u"pass"_ustr))
    {
        OUString aText(m_xText->get_label());
        aText = aText.replaceFirst("%s", OStringToOUString(rServer, osl_getThreadTextEncoding()));
        m_xText->set_label(aText);
        m_xDomainEdit->set_text(u"WORKGROUP"_ustr);
        if (rUserName.empty())
            m_xUserEdit->grab_focus();
        else
        {
            m_xUserEdit->set_text(OStringToOUString(rUserName, osl_getThreadTextEncoding()));
            m_xPassEdit->grab_focus();
        }
    }

    bool AuthenticateQuery(std::string_view rServer, OString& rUserName, OString& rPassword)
    {
        bool bRet = false;

        RTSPWDialog aDialog(Application::GetDefDialogParent(), rServer, rUserName);
        if (aDialog.run() == RET_OK)
        {
            rUserName = aDialog.getUserName();
            rPassword = aDialog.getPassword();
            bRet = true;
        }

        return bRet;
    }
}

namespace
{
    OString EscapeCupsOption(const OString& rIn)
    {
        OStringBuffer sRet;
        sal_Int32 nLen = rIn.getLength();
        for (sal_Int32 i = 0; i < nLen; ++i)
        {
            switch(rIn[i])
            {
                case '\\':
                case '\'':
                case '\"':
                case ',':
                case ' ':
                case '\f':
                case '\n':
                case '\r':
                case '\t':
                case '\v':
                    sRet.append('\\');
                    break;
            }
            sRet.append(rIn[i]);
        }
        return sRet.makeStringAndClear();
    }
}

bool CUPSManager::endSpool( const OUString& rPrintername, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner, const OUString& rFaxNumber )
{
    SAL_INFO( "vcl.unx.print", "endSpool: " << rPrintername << "," << rJobTitle << " copy count = " << rDocumentJobData.m_nCopies );

    int nJobID = 0;

    osl::MutexGuard aGuard( m_aCUPSMutex );

    std::unordered_map< OUString, int >::iterator dest_it =
        m_aCUPSDestMap.find( rPrintername );
    if( dest_it == m_aCUPSDestMap.end() )
    {
        SAL_INFO( "vcl.unx.print", "defer to PrinterInfoManager::endSpool" );
        return PrinterInfoManager::endSpool( rPrintername, rJobTitle, pFile, rDocumentJobData, bBanner, rFaxNumber );
    }

    std::unordered_map< FILE*, OString, FPtrHash >::const_iterator it = m_aSpoolFiles.find( pFile );
    if( it != m_aSpoolFiles.end() )
    {
        fclose( pFile );
        rtl_TextEncoding aEnc = osl_getThreadTextEncoding();

        // setup cups options
        int nNumOptions = 0;
        cups_option_t* pOptions = nullptr;
        auto ppOptions = reinterpret_cast<void**>(&pOptions);
        getOptionsFromDocumentSetup( rDocumentJobData, bBanner, nNumOptions, ppOptions );

        PrinterInfo aInfo(getPrinterInfo(rPrintername));
        if (!aInfo.m_aAuthInfoRequired.isEmpty())
        {
            bool bDomain(false), bUser(false), bPass(false);
            sal_Int32 nIndex = 0;
            do
            {
                std::u16string_view aToken = o3tl::getToken(aInfo.m_aAuthInfoRequired, 0, ',', nIndex);
                if (aToken == u"domain")
                    bDomain = true;
                else if (aToken == u"username")
                    bUser = true;
                else if (aToken == u"password")
                    bPass = true;
            }
            while (nIndex >= 0);

            if (bDomain || bUser || bPass)
            {
                OString sPrinterName(OUStringToOString(rPrintername, RTL_TEXTENCODING_UTF8));
                OString sUser = cupsUser();
                RTSPWDialog aDialog(Application::GetDefDialogParent(), sPrinterName, sUser);
                aDialog.SetDomainVisible(bDomain);
                aDialog.SetUserVisible(bUser);
                aDialog.SetPassVisible(bPass);

                if (aDialog.run() == RET_OK)
                {
                    OString sAuth;
                    if (bDomain)
                        sAuth = EscapeCupsOption(aDialog.getDomain());
                    if (bUser)
                    {
                        if (bDomain)
                            sAuth += ",";
                        sAuth += EscapeCupsOption(aDialog.getUserName());
                    }
                    if (bPass)
                    {
                        if (bUser || bDomain)
                            sAuth += ",";
                        sAuth += EscapeCupsOption(aDialog.getPassword());
                    }
                    nNumOptions = cupsAddOption("auth-info", sAuth.getStr(), nNumOptions, &pOptions);
                }
            }
        }

        OString sJobName(OUStringToOString(rJobTitle, aEnc));

        //fax4CUPS, "the job name will be dialled for you"
        //so override the jobname with the desired number
        if (!rFaxNumber.isEmpty())
        {
            sJobName = OUStringToOString(rFaxNumber, aEnc);
        }

        cups_dest_t* pDest = m_pDests + dest_it->second;
        nJobID = cupsPrintFile(pDest->name,
            it->second.getStr(),
            sJobName.getStr(),
            nNumOptions, pOptions);
        SAL_INFO("vcl.unx.print", "cupsPrintFile( " << pDest->name << ", "
                << it->second << ", " << rJobTitle << ", " << nNumOptions
                << ", " << pOptions << " ) returns " << nJobID);
        for( int n = 0; n < nNumOptions; n++ )
            SAL_INFO("vcl.unx.print",
                "    option " << pOptions[n].name << "=" << pOptions[n].value);
#if OSL_DEBUG_LEVEL > 1
        OString aCmd( "cp " );
        aCmd += it->second.getStr();
        aCmd += OString( " $HOME/cupsprint.ps" );
        system( aCmd.getStr() );
#endif

        unlink( it->second.getStr() );
        m_aSpoolFiles.erase(it);
        if( pOptions )
            cupsFreeOptions( nNumOptions, pOptions );
    }

    return nJobID != 0;
}

bool CUPSManager::checkPrintersChanged( bool bWait )
{
    bool bChanged = false;
    if( bWait )
    {
        if(  m_aDestThread )
        {
            // initial asynchronous detection still running
            SAL_INFO("vcl.unx.print", "syncing cups discovery thread");
            osl_joinWithThread( m_aDestThread );
            osl_destroyThread( m_aDestThread );
            m_aDestThread = nullptr;
            SAL_INFO("vcl.unx.print", "done: syncing cups discovery thread");
        }
        else
        {
            // #i82321# check for cups printer updates
            // with this change the whole asynchronous detection in a thread is
            // almost useless. The only relevance left is for some stalled systems
            // where the user can set SAL_DISABLE_SYNCHRONOUS_PRINTER_DETECTION
            // (see vcl/unx/source/gdi/salprnpsp.cxx)
            // so that checkPrintersChanged( true ) will never be called

            // there is no way to query CUPS whether the printer list has changed
            // so get the dest list anew
            if( m_nDests && m_pDests )
                cupsFreeDests( m_nDests, m_pDests );
            m_nDests = 0;
            m_pDests = nullptr;
            runDests();
        }
    }
    if( m_aCUPSMutex.tryToAcquire() )
    {
        bChanged = m_bNewDests;
        m_aCUPSMutex.release();
    }

    if( ! bChanged )
    {
        bChanged = PrinterInfoManager::checkPrintersChanged( bWait );
        // #i54375# ensure new merging with CUPS list in :initialize
        if( bChanged )
            m_bNewDests = true;
    }

    if( bChanged )
        initialize();

    return bChanged;
}

const char* CUPSManager::authenticateUser()
{
    const char* pRet = nullptr;

    osl::MutexGuard aGuard( m_aCUPSMutex );

    OString aUser = cupsUser();
    OString aServer = cupsServer();
    OString aPassword;
    if (AuthenticateQuery(aServer, aUser, aPassword))
    {
        m_aPassword = aPassword;
        m_aUser = aUser;
        cupsSetUser( m_aUser.getStr() );
        pRet = m_aPassword.getStr();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
