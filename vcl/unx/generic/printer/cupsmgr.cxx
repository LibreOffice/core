/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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

#include <officecfg/Office/Common.hxx>

#include <algorithm>

using namespace psp;
using namespace osl;


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
            SAL_WARN("vcl.unx.print",
                    "cupsGetPPD " << m_aParameter << " timed out");
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
    if( rMgr.getType() == PrinterInfoManager::CUPS ) 
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
    SAL_INFO("vcl.unx.print", "starting cupsGetDests");
    cups_dest_t* pDests = NULL;

    
    http_t* p_http;
    if( (p_http=httpConnectEncrypt(
             cupsServer(),
             ippPort(),
             cupsEncryption())) != NULL )
    {
        
        httpClose(p_http);

        int nDests = cupsGetDests( &pDests );
        SAL_INFO("vcl.unx.print", "came out of cupsGetDests");

        osl::MutexGuard aGuard( m_aCUPSMutex );
        m_nDests = nDests;
        m_pDests = pDests;
        m_bNewDests = true;
        SAL_INFO("vcl.unx.print", "finished cupsGetDests");
    }
}

void CUPSManager::initialize()
{
    
    PrinterInfoManager::initialize();

    
    
    osl::MutexGuard aGuard( m_aCUPSMutex );

    if( ! m_bNewDests )
        return;

    
    if( m_aDestThread )
    {
        osl_joinWithThread( m_aDestThread );
        osl_destroyThread( m_aDestThread );
        m_aDestThread = NULL;
    }
    m_bNewDests = false;

    
    m_aCUPSDestMap.clear();

    if( ! (m_nDests && m_pDests ) )
        return;

    if( isCUPSDisabled() )
        return;

    
    
    
    
    
    bool bUsePDF = false;
    cups_dest_t* pDest = ((cups_dest_t*)m_pDests);
    const char* pOpt = cupsGetOption( "printer-info",
                                                      pDest->num_options,
                                                      pDest->options );
    if( pOpt )
    {
        m_bUseIncludeFeature = true;
        bUsePDF = officecfg::Office::Common::Print::Option::Printer::PDFAsStandardPrintJobFormat::get();
    }

    m_aGlobalDefaults.setDefaultBackend(bUsePDF);

    
    
    
    m_bUseJobPatch = false;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    int nPrinter = m_nDests;

    
    m_aGlobalDefaults.m_pParser = NULL;
    m_aGlobalDefaults.m_aContext = PPDContext();

    
    
    while( nPrinter-- )
    {
        pDest = ((cups_dest_t*)m_pDests)+nPrinter;
        OUString aPrinterName = OStringToOUString( pDest->name, aEncoding );
        if( pDest->instance && *pDest->instance )
        {
            OUStringBuffer aBuf( 256 );
            aBuf.append( aPrinterName );
            aBuf.append( '/' );
            aBuf.append( OStringToOUString( pDest->instance, aEncoding ) );
            aPrinterName = aBuf.makeStringAndClear();
        }

        
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
        
        
        
        
        
        
        
        aPrinter.m_aInfo.m_pParser = NULL;
        aPrinter.m_aInfo.m_aContext.setParser( NULL );
        boost::unordered_map< OUString, PPDContext, OUStringHash >::const_iterator c_it = m_aDefaultContexts.find( aPrinterName );
        if( c_it != m_aDefaultContexts.end() )
        {
            aPrinter.m_aInfo.m_pParser = c_it->second.getParser();
            aPrinter.m_aInfo.m_aContext = c_it->second;
        }
        aPrinter.m_aInfo.setDefaultBackend(bUsePDF);
        aPrinter.m_aInfo.m_aDriverName = aBuf.makeStringAndClear();
        aPrinter.m_bModified = false;

        m_aPrinters[ aPrinter.m_aInfo.m_aPrinterName ] = aPrinter;
        m_aCUPSDestMap[ aPrinter.m_aInfo.m_aPrinterName ] = nPrinter;
    }

    
    
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

    
    for( int g = 0; g < pPPDGroup->num_subgroups; g++ )
    {
        updatePrinterContextInfo( pPPDGroup->subgroups + g, rContext );
    }
}

const PPDParser* CUPSManager::createCUPSParser( const OUString& rPrinter )
{
    const PPDParser* pNewParser = NULL;
    OUString aPrinter;

    if( rPrinter.startsWith("CUPS:") )
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
                SAL_INFO("vcl.unx.print",
                        "PPD for " << aPrinter << " is " << aPPDFile);
                if( !aPPDFile.isEmpty() )
                {
                    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
                    OUString aFileName( OStringToOUString( aPPDFile, aEncoding ) );
                    
                    ppd_file_t* pPPD = ppdOpenFile( aPPDFile.getStr() );
                    if( pPPD )
                    {
                        
                        PPDParser* pCUPSParser = new PPDParser( aFileName );
                        pCUPSParser->m_aFile = rPrinter;
                        pNewParser = pCUPSParser;

                        /*int nConflicts =*/ cupsMarkOptions( pPPD, pDest->num_options, pDest->options );
                        SAL_INFO("vcl.unx.print", "processing the following options for printer " << pDest->name << " (instance " << pDest->instance << "):");
                        for( int k = 0; k < pDest->num_options; k++ )
                            SAL_INFO("vcl.unx.print",
                                "   \"" << pDest->options[k].name <<
                                "\" = \"" << pDest->options[k].value << "\"");
                        PrinterInfo& rInfo = m_aPrinters[ aPrinter ].m_aInfo;

                        
                        PPDContext& rContext = m_aDefaultContexts[ aPrinter ];
                        rContext.setParser( pNewParser );
                        
                        
                        setDefaultPaper( rContext );
                        for( int i = 0; i < pPPD->num_groups; i++ )
                            updatePrinterContextInfo( pPPD->groups + i, rContext );

                        rInfo.m_pParser = pNewParser;
                        rInfo.m_aContext = rContext;

                        
                        ppdClose( pPPD );
                    }
                    else
                        SAL_INFO("vcl.unx.print", "ppdOpenFile failed, falling back to generic driver");

                    
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
        
        pNewParser = PPDParser::getParser( OUString( "SGENPRT" ) );

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
    if( p_it == m_aPrinters.end() ) 
    {
        SAL_WARN("vcl.unx.print", "CUPS printer list in disorder, "
                "no dest for printer " << rData.m_aPrinterName);
        return;
    }

    if( p_it->second.m_aInfo.m_pParser == NULL )
    {
        
        
        p_it->second.m_aInfo.m_pParser = PPDParser::getParser( p_it->second.m_aInfo.m_aDriverName );
    }
    if( p_it->second.m_aInfo.m_aContext.getParser() == NULL )
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
    OSL_TRACE( "endSpool: %s, %s",
               OUStringToOString( rPrintername, RTL_TEXTENCODING_UTF8 ).getStr(),
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
            if(pValue && pValue->m_eType == eInvocation && !pValue->m_aOption.isEmpty() )
            {
                OString aKey = OUStringToOString( pKey->getKey(), RTL_TEXTENCODING_ASCII_US );
                OString aValue = OUStringToOString( pValue->m_aOption, RTL_TEXTENCODING_ASCII_US );
                rNumOptions = cupsAddOption( aKey.getStr(), aValue.getStr(), rNumOptions, (cups_option_t**)rOptions );
            }
        }
    }

    if( rJob.m_nPDFDevice > 0 && rJob.m_nCopies > 1 )
    {
        OString aVal( OString::number( rJob.m_nCopies ) );
        rNumOptions = cupsAddOption( "copies", aVal.getStr(), rNumOptions, (cups_option_t**)rOptions );
        aVal = OString::boolean(rJob.m_bCollate);
        rNumOptions = cupsAddOption( "collate", aVal.getStr(), rNumOptions, (cups_option_t**)rOptions );
    }
    if( ! bBanner )
    {
        rNumOptions = cupsAddOption( "job-sheets", "none", rNumOptions, (cups_option_t**)rOptions );
    }
}

bool CUPSManager::endSpool( const OUString& rPrintername, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner )
{
    OSL_TRACE( "endSpool: %s, %s, copy count = %d",
               OUStringToOString( rPrintername, RTL_TEXTENCODING_UTF8 ).getStr(),
               OUStringToOString( rJobTitle, RTL_TEXTENCODING_UTF8 ).getStr(),
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

        
        int nNumOptions = 0;
        cups_option_t* pOptions = NULL;
        getOptionsFromDocumentSetup( rDocumentJobData, bBanner, nNumOptions, (void**)&pOptions );

        cups_dest_t* pDest = ((cups_dest_t*)m_pDests) + dest_it->second;
        nJobID = cupsPrintFile( pDest->name,
        it->second.getStr(),
        OUStringToOString( rJobTitle, aEnc ).getStr(),
        nNumOptions, pOptions );
        SAL_INFO("vcl.unx.print", "cupsPrintFile( " << pDest->name << ", "
                << it->second << ", " << rJobTitle << ", " << nNumOptions
                << ", " << pOptions << " ) returns " << nJobID);
        for( int n = 0; n < nNumOptions; n++ )
            SAL_INFO("vcl.unx.print",
                "    option " << pOptions[n].name << "=" << pOptions[n].value);
#if OSL_DEBUG_LEVEL > 1
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

    return nJobID != 0;
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
            
            SAL_INFO("vcl.unx.print", "syncing cups discovery thread");
            osl_joinWithThread( m_aDestThread );
            osl_destroyThread( m_aDestThread );
            m_aDestThread = NULL;
            SAL_INFO("vcl.unx.print", "done: syncing cups discovery thread");
        }
        else
        {
            
            
            
            
            
            

            
            
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
        
        if( bChanged )
            m_bNewDests = true;
    }

    if( bChanged )
        initialize();

    return bChanged;
}

bool CUPSManager::addPrinter( const OUString& rName, const OUString& rDriver )
{
    
    if( m_aCUPSDestMap.find( rName ) != m_aCUPSDestMap.end() ||
        rDriver.startsWith("CUPS:")
        )
        return false;
    return PrinterInfoManager::addPrinter( rName, rDriver );
}

bool CUPSManager::removePrinter( const OUString& rName, bool bCheck )
{
    
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

            
            int nNewOptions = 0;
            cups_option_t* pNewOptions = NULL;
            int nValues = rInfo.m_aContext.countValuesModified();
            for( int i = 0; i < nValues; i++ )
            {
                const PPDKey* pKey = rInfo.m_aContext.getModifiedKey( i );
                const PPDValue* pValue = rInfo.m_aContext.getValue( pKey );
                if( pKey && pValue ) 
                {
                    OString aName = OUStringToOString( pKey->getKey(), aEncoding );
                    OString aValue = OUStringToOString( pValue->m_aOption, aEncoding );
                    nNewOptions = cupsAddOption( aName.getStr(), aValue.getStr(), nNewOptions, &pNewOptions );
                }
            }
            
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
    oslModule pLib = osl_loadModuleAscii( _XSALSET_LIBNAME, SAL_LOADMODULE_LAZY );
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
    else
    {
        SAL_WARN("vcl.unx.print",
            "loading of module " << _XSALSET_LIBNAME << " failed\n");
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
