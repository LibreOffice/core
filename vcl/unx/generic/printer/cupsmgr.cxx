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


#include <cups/cups.h>
#include <cups/http.h>
#include <cups/ipp.h>
#include <cups/ppd.h>

#include <unistd.h>

#include "cupsmgr.hxx"

#include "osl/thread.h"
#include "osl/diagnose.h"
#include "osl/conditn.hxx"

#include "rtl/ustrbuf.hxx"

#include <algorithm>

using namespace psp;
using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringToOString;
using ::rtl::OStringToOUString;
using ::rtl::OUStringHash;
using ::rtl::OString;

struct GetPPDAttribs
{
    osl::Condition      m_aCondition;
    OString             m_aParameter;
    OString             m_aResult;
    oslThread           m_aThread;
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
        OString aResult = cupsGetPPD(m_aParameter.getStr());
        MutexGuard aGuard( *m_pSyncMutex );
        m_aResult = aResult;
        m_aCondition.set();
        unref();
    }

    OString waitResult( TimeValue *pDelay )
    {
        m_pSyncMutex->release();

        if (m_aCondition.wait( pDelay ) != Condition::result_ok
            )
        {
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "cupsGetPPD %s timed out\n", m_aParameter.getStr() );
            #endif
        }
        m_pSyncMutex->acquire();

        OString aRetval = m_aResult;
        m_aResult = OString();
        unref();

        return aRetval;
    }
};

extern "C" {
    static void getPPDWorker(void* pData)
    {
        GetPPDAttribs* pAttribs = (GetPPDAttribs*)pData;
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

static const char* setPasswordCallback( const char* pIn )
{
    const char* pRet = NULL;

    PrinterInfoManager& rMgr = PrinterInfoManager::get();
    if( rMgr.getType() == PrinterInfoManager::CUPS ) // sanity check
        pRet = static_cast<CUPSManager&>(rMgr).authenticateUser( pIn );
    return pRet;
}

/*
 *  CUPSManager class
 */

CUPSManager* CUPSManager::tryLoadCUPS()
{
    CUPSManager* pManager = NULL;
    static const char* pEnv = getenv("SAL_DISABLE_CUPS");

    if (!pEnv || !*pEnv)
        pManager = new CUPSManager();
    return pManager;
}

extern "C"
{
static void run_dest_thread_stub( void* pThis )
{
    CUPSManager::runDestThread( pThis );
}
}

CUPSManager::CUPSManager() :
        PrinterInfoManager( CUPS ),
        m_nDests( 0 ),
        m_pDests( NULL ),
        m_bNewDests( false ),
        m_bPPDThreadRunning( false )
{
    m_aDestThread = osl_createThread( run_dest_thread_stub, this );
}

CUPSManager::~CUPSManager()
{
    if( m_aDestThread )
    {
        // if the thread is still running here, then
        // cupsGetDests is hung; terminate the thread instead of joining
        osl_terminateThread( m_aDestThread );
        osl_destroyThread( m_aDestThread );
    }

    if (m_nDests && m_pDests)
        cupsFreeDests( m_nDests, (cups_dest_t*)m_pDests );
}

void CUPSManager::runDestThread( void* pThis )
{
    ((CUPSManager*)pThis)->runDests();
}

void CUPSManager::runDests()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "starting cupsGetDests\n" );
#endif
    cups_dest_t* pDests = NULL;

    // n#722902 - do a fast-failing check for cups working *at all* first
    http_t* p_http;
    if( (p_http=httpConnectEncrypt(
             cupsServer(),
             ippPort(),
             cupsEncryption())) != NULL )
    {
        // neat, cups is up, clean up the canary
        httpClose(p_http);

        int nDests = cupsGetDests( &pDests );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "came out of cupsGetDests\n" );
#endif

        osl::MutexGuard aGuard( m_aCUPSMutex );
        m_nDests = nDests;
        m_pDests = pDests;
        m_bNewDests = true;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "finished cupsGetDests\n" );
#endif
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
        m_aDestThread = NULL;
    }
    m_bNewDests = false;

    // clear old stuff
    m_aCUPSDestMap.clear();

    if( ! (m_nDests && m_pDests ) )
        return;

    if( isCUPSDisabled() )
        return;

    // check for CUPS server(?) > 1.2
    // since there is no API to query, check for options that were
    // introduced in dests with 1.2
    // this is needed to check for %%IncludeFeature support
    // (#i65684#, #i65491#)
    bool bUsePDF = false;
    cups_dest_t* pDest = ((cups_dest_t*)m_pDests);
    const char* pOpt = cupsGetOption( "printer-info",
                                                      pDest->num_options,
                                                      pDest->options );
    if( pOpt )
    {
        m_bUseIncludeFeature = true;
        bUsePDF = true;
        if( m_aGlobalDefaults.m_nPSLevel == 0 && m_aGlobalDefaults.m_nPDFDevice == 0 )
            m_aGlobalDefaults.m_nPDFDevice = 1;
    }
    // do not send include JobPatch; CUPS will insert that itself
    // TODO: currently unknown which versions of CUPS insert JobPatches
    // so currently it is assumed CUPS = don't insert JobPatch files
    m_bUseJobPatch = false;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    int nPrinter = m_nDests;

    // reset global default PPD options; these are queried on demand from CUPS
    m_aGlobalDefaults.m_pParser = NULL;
    m_aGlobalDefaults.m_aContext = PPDContext();

    // add CUPS printers, should there be a printer
    // with the same name as a CUPS printer, overwrite it
    while( nPrinter-- )
    {
        pDest = ((cups_dest_t*)m_pDests)+nPrinter;
        OUString aPrinterName = OStringToOUString( pDest->name, aEncoding );
        if( pDest->instance && *pDest->instance )
        {
            OUStringBuffer aBuf( 256 );
            aBuf.append( aPrinterName );
            aBuf.append( sal_Unicode( '/' ) );
            aBuf.append( OStringToOUString( pDest->instance, aEncoding ) );
            aPrinterName = aBuf.makeStringAndClear();
        }

        // initialize printer with possible configuration from psprint.conf
        bool bSetToGlobalDefaults = m_aPrinters.find( aPrinterName ) == m_aPrinters.end();
        Printer aPrinter = m_aPrinters[ aPrinterName ];
        if( bSetToGlobalDefaults )
            aPrinter.m_aInfo = m_aGlobalDefaults;
        aPrinter.m_aInfo.m_aPrinterName = aPrinterName;
        if( pDest->is_default )
            m_aDefaultPrinter = aPrinterName;

        for( int k = 0; k < pDest->num_options; k++ )
        {
            if(!strcmp(pDest->options[k].name, "printer-info"))
                aPrinter.m_aInfo.m_aComment=OStringToOUString(pDest->options[k].value, aEncoding);
            if(!strcmp(pDest->options[k].name, "printer-location"))
                aPrinter.m_aInfo.m_aLocation=OStringToOUString(pDest->options[k].value, aEncoding);
        }


        OUStringBuffer aBuf( 256 );
        aBuf.appendAscii( "CUPS:" );
        aBuf.append( aPrinterName );
        // note: the parser that goes with the PrinterInfo
        // is created implicitly by the JobData::operator=()
        // when it detects the NULL ptr m_pParser.
        // if we wanted to fill in the parser here this
        // would mean we'd have to download PPDs for each and
        // every printer - which would be really bad runtime
        // behaviour
        aPrinter.m_aInfo.m_pParser = NULL;
        aPrinter.m_aInfo.m_aContext.setParser( NULL );
        boost::unordered_map< OUString, PPDContext, OUStringHash >::const_iterator c_it = m_aDefaultContexts.find( aPrinterName );
        if( c_it != m_aDefaultContexts.end() )
        {
            aPrinter.m_aInfo.m_pParser = c_it->second.getParser();
            aPrinter.m_aInfo.m_aContext = c_it->second;
        }
        if( bUsePDF && aPrinter.m_aInfo.m_nPSLevel == 0 && aPrinter.m_aInfo.m_nPDFDevice == 0 )
            aPrinter.m_aInfo.m_nPDFDevice = 1;
        aPrinter.m_aInfo.m_aDriverName = aBuf.makeStringAndClear();
        aPrinter.m_bModified = false;

        m_aPrinters[ aPrinter.m_aInfo.m_aPrinterName ] = aPrinter;
        m_aCUPSDestMap[ aPrinter.m_aInfo.m_aPrinterName ] = nPrinter;
    }

    // remove everything that is not a CUPS printer and not
    // a special purpose printer (PDF, Fax)
    std::list< OUString > aRemovePrinters;
    for( boost::unordered_map< OUString, Printer, OUStringHash >::iterator it = m_aPrinters.begin();
         it != m_aPrinters.end(); ++it )
    {
        if( m_aCUPSDestMap.find( it->first ) != m_aCUPSDestMap.end() )
            continue;

        if( !it->second.m_aInfo.m_aFeatures.isEmpty() )
            continue;
        aRemovePrinters.push_back( it->first );
    }
    while( aRemovePrinters.begin() != aRemovePrinters.end() )
    {
        m_aPrinters.erase( aRemovePrinters.front() );
        aRemovePrinters.pop_front();
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
#if OSL_DEBUG_LEVEL > 1
                            fprintf( stderr, "key %s is set to %s\n", pOption->keyword, pChoice->choice );
#endif

                        }
#if OSL_DEBUG_LEVEL > 1
                        else
                            fprintf( stderr, "key %s is defaulted to %s\n", pOption->keyword, pChoice->choice );
#endif
                    }
#if OSL_DEBUG_LEVEL > 1
                    else
                        fprintf( stderr, "caution: value %s not found in key %s\n", pChoice->choice, pOption->keyword );
#endif
                }
#if OSL_DEBUG_LEVEL > 1
                else
                    fprintf( stderr, "caution: key %s not found in parser\n", pOption->keyword );
#endif
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
    const PPDParser* pNewParser = NULL;
    OUString aPrinter;

    if( rPrinter.compareToAscii( "CUPS:", 5 ) == 0 )
        aPrinter = rPrinter.copy( 5 );
    else
        aPrinter = rPrinter;

    if( m_aCUPSMutex.tryToAcquire() )
    {
        if( m_nDests && m_pDests && ! isCUPSDisabled() )
        {
            boost::unordered_map< OUString, int, OUStringHash >::iterator dest_it =
            m_aCUPSDestMap.find( aPrinter );
            if( dest_it != m_aCUPSDestMap.end() )
            {
                cups_dest_t* pDest = ((cups_dest_t*)m_pDests) + dest_it->second;
                OString aPPDFile = threadedCupsGetPPD( pDest->name );
                #if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "PPD for %s is %s\n", OUStringToOString( aPrinter, osl_getThreadTextEncoding() ).getStr(), aPPDFile.getStr() );
                #endif
                if( !aPPDFile.isEmpty() )
                {
                    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
                    OUString aFileName( OStringToOUString( aPPDFile, aEncoding ) );
                    // update the printer info with context information
                    ppd_file_t* pPPD = ppdOpenFile( aPPDFile.getStr() );
                    if( pPPD )
                    {
                        // create the new parser
                        PPDParser* pCUPSParser = new PPDParser( aFileName );
                        pCUPSParser->m_aFile = rPrinter;
                        pNewParser = pCUPSParser;

                        /*int nConflicts =*/ cupsMarkOptions( pPPD, pDest->num_options, pDest->options );
                        #if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "processing the following options for printer %s (instance %s):\n",
                        pDest->name, pDest->instance );
                        for( int k = 0; k < pDest->num_options; k++ )
                            fprintf( stderr, "   \"%s\" = \"%s\"\n",
                        pDest->options[k].name,
                        pDest->options[k].value );
                        #endif
                        PrinterInfo& rInfo = m_aPrinters[ aPrinter ].m_aInfo;

                        // remember the default context for later use
                        PPDContext& rContext = m_aDefaultContexts[ aPrinter ];
                        rContext.setParser( pNewParser );
                        // set system default paper; printer CUPS PPD options
                        // may overwrite it
                        setDefaultPaper( rContext );
                        for( int i = 0; i < pPPD->num_groups; i++ )
                            updatePrinterContextInfo( pPPD->groups + i, rContext );

                        rInfo.m_pParser = pNewParser;
                        rInfo.m_aContext = rContext;

                        // clean up the mess
                        ppdClose( pPPD );
                    }
                    #if OSL_DEBUG_LEVEL > 1
                    else
                        fprintf( stderr, "ppdOpenFile failed, falling back to generic driver\n" );
                    #endif

                    // remove temporary PPD file
                    unlink( aPPDFile.getStr() );
                }
                #if OSL_DEBUG_LEVEL > 1
                else
                    fprintf( stderr, "cupsGetPPD failed, falling back to generic driver\n" );
                #endif
            }
            #if OSL_DEBUG_LEVEL > 1
            else
                fprintf( stderr, "no dest found for printer %s\n", OUStringToOString( aPrinter, osl_getThreadTextEncoding() ).getStr() );
            #endif
        }
        m_aCUPSMutex.release();
    }
    #if OSL_DEBUG_LEVEL >1
    else
        fprintf( stderr, "could not acquire CUPS mutex !!!\n" );
    #endif

    if( ! pNewParser )
    {
        // get the default PPD
        pNewParser = PPDParser::getParser( String( RTL_CONSTASCII_USTRINGPARAM( "SGENPRT" ) ) );

        PrinterInfo& rInfo = m_aPrinters[ aPrinter ].m_aInfo;

        rInfo.m_pParser = pNewParser;
        rInfo.m_aContext.setParser( pNewParser );
    }

    return pNewParser;
}

void CUPSManager::setupJobContextData( JobData& rData )
{
    boost::unordered_map< OUString, int, OUStringHash >::iterator dest_it =
        m_aCUPSDestMap.find( rData.m_aPrinterName );

    if( dest_it == m_aCUPSDestMap.end() )
        return PrinterInfoManager::setupJobContextData( rData );

    boost::unordered_map< OUString, Printer, OUStringHash >::iterator p_it =
        m_aPrinters.find( rData.m_aPrinterName );
    if( p_it == m_aPrinters.end() ) // huh ?
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "CUPS printer list in disorder, no dest for printer %s !\n", OUStringToOString( rData.m_aPrinterName, osl_getThreadTextEncoding() ).getStr() );
#endif
        return;
    }

    if( p_it->second.m_aInfo.m_pParser == NULL )
    {
        // in turn calls createCUPSParser
        // which updates the printer info
        p_it->second.m_aInfo.m_pParser = PPDParser::getParser( p_it->second.m_aInfo.m_aDriverName );
    }
    if( p_it->second.m_aInfo.m_aContext.getParser() == NULL )
    {
        OUString aPrinter;
        if( p_it->second.m_aInfo.m_aDriverName.compareToAscii( "CUPS:", 5 ) == 0 )
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
    OSL_TRACE( "endSpool: %s, %s",
               rtl::OUStringToOString( rPrintername, RTL_TEXTENCODING_UTF8 ).getStr(),
              bQuickCommand ? "true" : "false" );

    if( m_aCUPSDestMap.find( rPrintername ) == m_aCUPSDestMap.end() )
    {
        OSL_TRACE( "defer to PrinterInfoManager::startSpool" );
        return PrinterInfoManager::startSpool( rPrintername, bQuickCommand );
    }

    OUString aTmpURL, aTmpFile;
    osl_createTempFile( NULL, NULL, &aTmpURL.pData );
    osl_getSystemPathFromFileURL( aTmpURL.pData, &aTmpFile.pData );
    OString aSysFile = OUStringToOString( aTmpFile, osl_getThreadTextEncoding() );
    FILE* fp = fopen( aSysFile.getStr(), "w" );
    if( fp )
        m_aSpoolFiles[fp] = aSysFile;

    return fp;
}

struct less_ppd_key : public ::std::binary_function<double, double, bool>
{
    bool operator()(const PPDKey* left, const PPDKey* right)
    { return left->getOrderDependency() < right->getOrderDependency(); }
};

void CUPSManager::getOptionsFromDocumentSetup( const JobData& rJob, bool bBanner, int& rNumOptions, void** rOptions ) const
{
    rNumOptions = 0;
    *rOptions = NULL;

    // emit features ordered to OrderDependency
    // ignore features that are set to default

    // sanity check
    if( rJob.m_pParser == rJob.m_aContext.getParser() && rJob.m_pParser )
    {
        int i;
        int nKeys = rJob.m_aContext.countValuesModified();
        ::std::vector< const PPDKey* > aKeys( nKeys );
        for(  i = 0; i < nKeys; i++ )
            aKeys[i] = rJob.m_aContext.getModifiedKey( i );
        ::std::sort( aKeys.begin(), aKeys.end(), less_ppd_key() );

        for( i = 0; i < nKeys; i++ )
        {
            const PPDKey* pKey = aKeys[i];
            const PPDValue* pValue = rJob.m_aContext.getValue( pKey );
            if(pValue && pValue->m_eType == eInvocation && pValue->m_aValue.Len() )
            {
                OString aKey = OUStringToOString( pKey->getKey(), RTL_TEXTENCODING_ASCII_US );
                OString aValue = OUStringToOString( pValue->m_aOption, RTL_TEXTENCODING_ASCII_US );
                rNumOptions = cupsAddOption( aKey.getStr(), aValue.getStr(), rNumOptions, (cups_option_t**)rOptions );
            }
        }
    }

    if( rJob.m_nPDFDevice > 0 && rJob.m_nCopies > 1 )
    {
        rtl::OString aVal( rtl::OString::valueOf( sal_Int32( rJob.m_nCopies ) ) );
        rNumOptions = cupsAddOption( "copies", aVal.getStr(), rNumOptions, (cups_option_t**)rOptions );
    }
    if( ! bBanner )
    {
        rNumOptions = cupsAddOption( "job-sheets", "none", rNumOptions, (cups_option_t**)rOptions );
    }
}

int CUPSManager::endSpool( const OUString& rPrintername, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner )
{
    OSL_TRACE( "endSpool: %s, %s, copy count = %d",
               rtl::OUStringToOString( rPrintername, RTL_TEXTENCODING_UTF8 ).getStr(),
               rtl::OUStringToOString( rJobTitle, RTL_TEXTENCODING_UTF8 ).getStr(),
               rDocumentJobData.m_nCopies
               );

    int nJobID = 0;

    osl::MutexGuard aGuard( m_aCUPSMutex );

    boost::unordered_map< OUString, int, OUStringHash >::iterator dest_it =
        m_aCUPSDestMap.find( rPrintername );
    if( dest_it == m_aCUPSDestMap.end() )
    {
        OSL_TRACE( "defer to PrinterInfoManager::endSpool" );
        return PrinterInfoManager::endSpool( rPrintername, rJobTitle, pFile, rDocumentJobData, bBanner );
    }

    boost::unordered_map< FILE*, OString, FPtrHash >::const_iterator it = m_aSpoolFiles.find( pFile );
    if( it != m_aSpoolFiles.end() )
    {
        fclose( pFile );
        rtl_TextEncoding aEnc = osl_getThreadTextEncoding();

        // setup cups options
        int nNumOptions = 0;
        cups_option_t* pOptions = NULL;
        getOptionsFromDocumentSetup( rDocumentJobData, bBanner, nNumOptions, (void**)&pOptions );

        cups_dest_t* pDest = ((cups_dest_t*)m_pDests) + dest_it->second;
        nJobID = cupsPrintFile( pDest->name,
        it->second.getStr(),
        OUStringToOString( rJobTitle, aEnc ).getStr(),
        nNumOptions, pOptions );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "cupsPrintFile( %s, %s, %s, %d, %p ) returns %d\n",
                    pDest->name,
                    it->second.getStr(),
                    OUStringToOString( rJobTitle, aEnc ).getStr(),
                    nNumOptions,
                    pOptions,
                    nJobID
                    );
        for( int n = 0; n < nNumOptions; n++ )
            fprintf( stderr, "    option %s=%s\n", pOptions[n].name, pOptions[n].value );
        OString aCmd( "cp " );
        aCmd = aCmd + it->second;
        aCmd = aCmd + OString( " $HOME/cupsprint.ps" );
        system( aCmd.getStr() );
#endif

        unlink( it->second.getStr() );
        m_aSpoolFiles.erase( pFile );
        if( pOptions )
            cupsFreeOptions( nNumOptions, pOptions );
    }

    return nJobID;
}


void CUPSManager::changePrinterInfo( const OUString& rPrinter, const PrinterInfo& rNewInfo )
{
    PrinterInfoManager::changePrinterInfo( rPrinter, rNewInfo );
}

bool CUPSManager::checkPrintersChanged( bool bWait )
{
    bool bChanged = false;
    if( bWait )
    {
        if(  m_aDestThread )
        {
            // initial asynchronous detection still running
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "syncing cups discovery thread\n" );
            #endif
            osl_joinWithThread( m_aDestThread );
            osl_destroyThread( m_aDestThread );
            m_aDestThread = NULL;
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "done: syncing cups discovery thread\n" );
            #endif
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
                cupsFreeDests( m_nDests, (cups_dest_t*)m_pDests );
            m_nDests = 0;
            m_pDests = NULL;
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

bool CUPSManager::addPrinter( const OUString& rName, const OUString& rDriver )
{
    // don't touch the CUPS printers
    if( m_aCUPSDestMap.find( rName ) != m_aCUPSDestMap.end() ||
        rDriver.compareToAscii( "CUPS:", 5 ) == 0
        )
        return false;
    return PrinterInfoManager::addPrinter( rName, rDriver );
}

bool CUPSManager::removePrinter( const OUString& rName, bool bCheck )
{
    // don't touch the CUPS printers
    if( m_aCUPSDestMap.find( rName ) != m_aCUPSDestMap.end() )
        return false;
    return PrinterInfoManager::removePrinter( rName, bCheck );
}

bool CUPSManager::setDefaultPrinter( const OUString& rName )
{
    bool bSuccess = false;
    boost::unordered_map< OUString, int, OUStringHash >::iterator nit =
        m_aCUPSDestMap.find( rName );
    if( nit != m_aCUPSDestMap.end() && m_aCUPSMutex.tryToAcquire() )
    {
        cups_dest_t* pDests = (cups_dest_t*)m_pDests;
        for( int i = 0; i < m_nDests; i++ )
            pDests[i].is_default = 0;
        pDests[ nit->second ].is_default = 1;
        cupsSetDests( m_nDests, (cups_dest_t*)m_pDests );
        m_aDefaultPrinter = rName;
        m_aCUPSMutex.release();
        bSuccess = true;
    }
    else
        bSuccess = PrinterInfoManager::setDefaultPrinter( rName );

    return bSuccess;
}

bool CUPSManager::writePrinterConfig()
{
    bool bDestModified = false;
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    for( boost::unordered_map< OUString, Printer, OUStringHash >::iterator prt =
             m_aPrinters.begin(); prt != m_aPrinters.end(); ++prt )
    {
        boost::unordered_map< OUString, int, OUStringHash >::iterator nit =
            m_aCUPSDestMap.find( prt->first );
        if( nit == m_aCUPSDestMap.end() )
            continue;

        if( ! prt->second.m_bModified )
            continue;

        if( m_aCUPSMutex.tryToAcquire() )
        {
            bDestModified = true;
            cups_dest_t* pDest = ((cups_dest_t*)m_pDests) + nit->second;
            PrinterInfo& rInfo = prt->second.m_aInfo;

            // create new option list
            int nNewOptions = 0;
            cups_option_t* pNewOptions = NULL;
            int nValues = rInfo.m_aContext.countValuesModified();
            for( int i = 0; i < nValues; i++ )
            {
                const PPDKey* pKey = rInfo.m_aContext.getModifiedKey( i );
                const PPDValue* pValue = rInfo.m_aContext.getValue( pKey );
                if( pKey && pValue ) // sanity check
                {
                    OString aName = OUStringToOString( pKey->getKey(), aEncoding );
                    OString aValue = OUStringToOString( pValue->m_aOption, aEncoding );
                    nNewOptions = cupsAddOption( aName.getStr(), aValue.getStr(), nNewOptions, &pNewOptions );
                }
            }
            // set PPD options on CUPS dest
            cupsFreeOptions( pDest->num_options, pDest->options );
            pDest->num_options = nNewOptions;
            pDest->options = pNewOptions;
            m_aCUPSMutex.release();
        }
    }
    if( bDestModified && m_aCUPSMutex.tryToAcquire() )
    {
        cupsSetDests( m_nDests, (cups_dest_t*)m_pDests );
        m_aCUPSMutex.release();
    }

    return PrinterInfoManager::writePrinterConfig();
}

bool CUPSManager::addOrRemovePossible() const
{
    return (m_nDests && m_pDests && ! isCUPSDisabled())? false : PrinterInfoManager::addOrRemovePossible();
}

const char* CUPSManager::authenticateUser( const char* /*pIn*/ )
{
    const char* pRet = NULL;

    OUString aLib(_XSALSET_LIBNAME );
    oslModule pLib = osl_loadModule( aLib.pData, SAL_LOADMODULE_LAZY );
    if( pLib )
    {
        OUString aSym( "Sal_authenticateQuery"  );
        bool (*getpw)( const OString& rServer, OString& rUser, OString& rPw) =
            (bool(*)(const OString&,OString&,OString&))osl_getFunctionSymbol( pLib, aSym.pData );
        if( getpw )
        {
            osl::MutexGuard aGuard( m_aCUPSMutex );

            OString aUser = cupsUser();
            OString aServer = cupsServer();
            OString aPassword;
            if( getpw( aServer, aUser, aPassword ) )
            {
                m_aPassword = aPassword;
                m_aUser = aUser;
                cupsSetUser( m_aUser.getStr() );
                pRet = m_aPassword.getStr();
            }
        }
        osl_unloadModule( pLib );
    }
#if OSL_DEBUG_LEVEL > 1
    else fprintf( stderr, "loading of module %s failed\n", OUStringToOString( aLib, osl_getThreadTextEncoding() ).getStr() );
#endif

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
