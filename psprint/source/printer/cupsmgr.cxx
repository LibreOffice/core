/*************************************************************************
 *
 *  $RCSfile: cupsmgr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:08:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef ENABLE_CUPS
#include <cups/cups.h>
#include <cups/ppd.h>
#else
typedef void ppd_file_t;
typedef void cups_dest_t;
typedef void cups_option_t;
#endif

#include <osl/thread.h>
#include <osl/diagnose.h>
#include <osl/conditn.hxx>
#include <rtl/ustrbuf.hxx>
#include <cupsmgr.hxx>

namespace psp
{
class CUPSWrapper
{
    oslModule       m_pLib;
    osl::Mutex      m_aGetPPDMutex;

    int             (*m_pcupsPrintFile)(const char*, const char*, const char*, int, cups_option_t*);
    int             (*m_pcupsGetDests)(cups_dest_t**);
    void            (*m_pcupsSetDests)(int,cups_dest_t*);
    void            (*m_pcupsFreeDests)(int,cups_dest_t*);
    const char*     (*m_pcupsGetPPD)(const char*);
    int             (*m_pcupsMarkOptions)(ppd_file_t*,int,cups_option_t*);
    int             (*m_pcupsAddOption)(const char*,const char*,int,cups_option_t**);
    void            (*m_pcupsFreeOptions)(int,cups_option_t*);
    ppd_file_t*     (*m_pppdOpenFile)(const char* pFile);
    void            (*m_pppdClose)(ppd_file_t*);
    const char*     (*m_pcupsServer)();
    void            (*m_pcupsSetPasswordCB)(const char*(cb)(const char*));
    const char*     (*m_pcupsUser)();
    void            (*m_pcupsSetUser)(const char*);

    void* loadSymbol( const char* );
public:
    CUPSWrapper();
    ~CUPSWrapper();

    bool isValid();

    int cupsGetDests(cups_dest_t** pDests)
    { return m_pcupsGetDests(pDests); }

    void cupsSetDests( int nDests, cups_dest_t* pDests )
    { m_pcupsSetDests( nDests, pDests ); }

    void cupsFreeDests(int nDests, cups_dest_t* pDests)
    { m_pcupsFreeDests(nDests, pDests); }

    int cupsPrintFile( const char* pPrinter,
                       const char* pFileName,
                       const char* pTitle,
                       int nOptions,
                   cups_option_t* pOptions )
    { return m_pcupsPrintFile( pPrinter, pFileName, pTitle, nOptions, pOptions ); }

    const char* cupsGetPPD( const char* pPrinter );

    int cupsMarkOptions(ppd_file_t* pPPD, int nOptions, cups_option_t* pOptions )
    { return m_pcupsMarkOptions(pPPD, nOptions, pOptions); }

    int cupsAddOption( const char* pName, const char* pValue, int nOptions, cups_option_t** pOptions )
    { return m_pcupsAddOption( pName, pValue, nOptions, pOptions ); }

    void cupsFreeOptions( int nOptions, cups_option_t* pOptions )
    { m_pcupsFreeOptions( nOptions, pOptions ); }

    ppd_file_t* ppdOpenFile( const char* pFileName )
    { return m_pppdOpenFile( pFileName ); }

    void ppdClose( ppd_file_t* pPPD )
    { m_pppdClose( pPPD ); }

    const char  *cupsServer(void)
    { return m_pcupsServer(); }

    const char  *cupsUser(void)
    { return m_pcupsUser(); }

    void cupsSetPasswordCB(const char *(*cb)(const char *))
    { m_pcupsSetPasswordCB( cb ); }

    void cupsSetUser(const char *user)
    { m_pcupsSetUser( user ); }

};
}

using namespace psp;
using namespace osl;
using namespace rtl;

/*
 *  CUPSWrapper class
 */

void* CUPSWrapper::loadSymbol( const char* pSymbol )
{
    OUString aSym( OUString::createFromAscii( pSymbol ) );
    void* pSym = osl_getSymbol( m_pLib, aSym.pData );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "%s %s\n", pSymbol, pSym ? "found" : "not found" );
#endif
    return pSym;
}

CUPSWrapper::CUPSWrapper()
        : m_pLib( NULL )
{
#ifdef ENABLE_CUPS
    OUString aLib( RTL_CONSTASCII_USTRINGPARAM( "libcups.so.2" ) );
    m_pLib = osl_loadModule( aLib.pData, SAL_LOADMODULE_LAZY );
    if( ! m_pLib )
    {
        aLib = OUString( RTL_CONSTASCII_USTRINGPARAM( "libcups.so" ) );
        m_pLib = osl_loadModule( aLib.pData, SAL_LOADMODULE_LAZY );
    }
#endif

    if( ! m_pLib )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "no cups library found\n" );
#endif
        return;
    }

    m_pcupsPrintFile        = (int(*)(const char*,const char*,const char*,int,cups_option_t*))
        loadSymbol( "cupsPrintFile" );
    m_pcupsGetDests         = (int(*)(cups_dest_t**))
        loadSymbol( "cupsGetDests" );
    m_pcupsSetDests         = (void(*)(int,cups_dest_t*))
        loadSymbol( "cupsSetDests" );
    m_pcupsFreeDests        = (void(*)(int,cups_dest_t*))
        loadSymbol( "cupsFreeDests" );
    m_pcupsGetPPD           = (const char*(*)(const char*))
        loadSymbol( "cupsGetPPD" );
    m_pcupsMarkOptions      = (int(*)(ppd_file_t*,int,cups_option_t*))
        loadSymbol( "cupsMarkOptions" );
    m_pcupsAddOption        = (int(*)(const char*,const char*,int,cups_option_t**))
        loadSymbol( "cupsAddOption" );
    m_pcupsFreeOptions      = (void(*)(int,cups_option_t*))
        loadSymbol( "cupsFreeOptions" );
    m_pppdOpenFile          = (ppd_file_t*(*)(const char*))
        loadSymbol( "ppdOpenFile" );
    m_pppdClose             = (void(*)(ppd_file_t*))
        loadSymbol( "ppdClose" );
    m_pcupsServer           = (const char*(*)())
        loadSymbol( "cupsServer" );
    m_pcupsUser             = (const char*(*)())
        loadSymbol( "cupsUser" );
    m_pcupsSetPasswordCB    = (void(*)(const char*(*)(const char*)))
        loadSymbol( "cupsSetPasswordCB" );
    m_pcupsSetUser          = (void(*)(const char*))
        loadSymbol( "cupsSetUser" );

    if( ! (
           m_pcupsPrintFile                 &&
           m_pcupsGetDests                  &&
           m_pcupsSetDests                  &&
           m_pcupsFreeDests                 &&
           m_pcupsGetPPD                    &&
           m_pcupsMarkOptions               &&
           m_pcupsAddOption                 &&
           m_pcupsServer                    &&
           m_pcupsUser                      &&
           m_pcupsSetPasswordCB             &&
           m_pcupsSetUser                   &&
           m_pcupsFreeOptions               &&
           m_pppdOpenFile                   &&
           m_pppdClose
           ) )
    {
        osl_unloadModule( m_pLib );
        m_pLib = NULL;
    }
}

CUPSWrapper::~CUPSWrapper()
{
    if( m_pLib )
        osl_unloadModule( m_pLib );
}

bool CUPSWrapper::isValid()
{
    return m_pLib != NULL;
}

static struct GetPPDAttribs
{
    const char* (*pFunction)(const char*);
    osl::Condition      m_aCondition;
    const char*         m_pParameter;
    const char*         m_pResult;
    oslThread           m_aThread;
} *pAttribs = NULL;

extern "C" {
    static void getPPDWorker(void*)
    {
        pAttribs->m_pResult = pAttribs->pFunction( pAttribs->m_pParameter );
        if( pAttribs->m_aCondition.check() )
        {
            // timed out, unlink file
            if( pAttribs->m_pResult )
                unlink( pAttribs->m_pResult );
            delete pAttribs;
            pAttribs = NULL;
        }
        else
            pAttribs->m_aCondition.set();
    }
}

const char* CUPSWrapper::cupsGetPPD( const char* pPrinter )
{
    const char* pResult = NULL;

    // if one thread hangs in cupsGetPPD already, don't start another
    if( ! pAttribs )
    {
        pAttribs = new GetPPDAttribs();
        pAttribs->pFunction         = m_pcupsGetPPD;
        pAttribs->m_aCondition.reset();
        pAttribs->m_pParameter      = pPrinter;
        pAttribs->m_pResult         = NULL;
        pAttribs->m_aThread         = osl_createThread( getPPDWorker, NULL );

        TimeValue aValue;
        aValue.Seconds = 5;
        aValue.Nanosec = 0;
        if( pAttribs->m_aCondition.wait( &aValue ) == Condition::result_ok )
        {
            osl_destroyThread( pAttribs->m_aThread );
            pResult = pAttribs->m_pResult;
            delete pAttribs;
            pAttribs = NULL;
        }
        else
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "cupsGetPPD %s timed out\n", pPrinter );
#endif
            // should the thread awake again notify it to clean up itself
            pAttribs->m_aCondition.set();
            osl_destroyThread( pAttribs->m_aThread );
        }
    }

    return pResult;
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
#ifdef ENABLE_CUPS
    static const char* pEnv = getenv( "SAL_DISABLE_CUPS" );

    if( ! pEnv || ! *pEnv )
    {
        // try to load CUPS
        CUPSWrapper* pWrapper = new CUPSWrapper();
        if( pWrapper->isValid() )
            pManager = new CUPSManager( pWrapper );
        else
            delete pWrapper;
    }
#endif
    return pManager;
}

CUPSManager::CUPSManager( CUPSWrapper* pWrapper ) :
        PrinterInfoManager( CUPS ),
        m_pCUPSWrapper( pWrapper ),
        m_nDests( 0 ),
        m_pDests( NULL ),
        m_bNewDests( false )
{
    m_aDestThread = osl_createThread( runDestThread, this );
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

    if( m_nDests && m_pDests )
        m_pCUPSWrapper->cupsFreeDests( m_nDests, (cups_dest_t*)m_pDests );
    delete m_pCUPSWrapper;
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
    osl::MutexGuard aGuard( m_aCUPSMutex );

    m_nDests = m_pCUPSWrapper->cupsGetDests( (cups_dest_t**)&m_pDests );
    m_bNewDests = true;
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "finished cupsGetDests\n" );
#endif
}

void CUPSManager::initialize()
{
    // get normal printers, clear printer list
    PrinterInfoManager::initialize();

#ifdef ENABLE_CUPS
    // check whether thread has completed
    // if not behave like old printing system
    osl::MutexGuard aGuard( m_aCUPSMutex );

    if( ! (m_nDests && m_pDests ) )
        return;

    // dest thread has run, clean up
    if( m_aDestThread )
    {
        osl_joinWithThread( m_aDestThread );
        osl_destroyThread( m_aDestThread );
        m_aDestThread = NULL;
    }

    // clear old stuff
    m_aCUPSDestMap.clear();

#if 0
    // update dests
    m_pCUPSWrapper->cupsFreeDests( m_nDests, (cups_dest_t*)m_pDests );
    m_nDests = m_pCUPSWrapper->cupsGetDests( (cups_dest_t**)&m_pDests );
#endif
    m_bNewDests = false;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    int nPrinter = m_nDests;

    // add CUPS printers, should there be a printer
    // with the same name as a CUPS printer, overwrite it
    while( nPrinter-- )
    {
        cups_dest_t* pDest = ((cups_dest_t*)m_pDests)+nPrinter;
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
        Printer aPrinter = m_aPrinters[ aPrinterName ];
        aPrinter.m_aInfo.m_aPrinterName = aPrinterName;
        if( pDest->is_default )
            m_aDefaultPrinter = aPrinterName;


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
        aPrinter.m_aInfo.m_aDriverName = aBuf.makeStringAndClear();
        aPrinter.m_bModified = false;

        m_aPrinters[ aPrinter.m_aInfo.m_aPrinterName ] = aPrinter;
        m_aCUPSDestMap[ aPrinter.m_aInfo.m_aPrinterName ] = nPrinter;
    }

    // remove everything that is not a CUPS printer and not
    // a special purpose printer (PDF, Fax)
    std::list< OUString > aRemovePrinters;
    for( std::hash_map< OUString, Printer, OUStringHash >::iterator it = m_aPrinters.begin();
         it != m_aPrinters.end(); ++it )
    {
        if( m_aCUPSDestMap.find( it->first ) != m_aCUPSDestMap.end() )
            continue;

        if( it->second.m_aInfo.m_aFeatures.getLength() > 0 )
            continue;
        aRemovePrinters.push_back( it->first );
    }
    while( aRemovePrinters.begin() != aRemovePrinters.end() )
    {
        m_aPrinters.erase( aRemovePrinters.front() );
        aRemovePrinters.pop_front();
    }

    m_pCUPSWrapper->cupsSetPasswordCB( setPasswordCallback );
#endif // ENABLE_CUPS
}

#ifdef ENABLE_CUPS
static void updatePrinterContextInfo( ppd_group_t* pPPDGroup, PrinterInfo& rInfo )
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
                const PPDKey* pKey = rInfo.m_pParser->getKey( OStringToOUString( pOption->keyword, aEncoding ) );
                if( pKey )
                {
                    const PPDValue* pValue = pKey->getValue( OStringToOUString( pChoice->choice, aEncoding ) );
                    if( pValue )
                    {
                        if( pValue != pKey->getDefaultValue() )
                        {
                            rInfo.m_aContext.setValue( pKey, pValue, true );
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
        updatePrinterContextInfo( pPPDGroup->subgroups + g, rInfo );
    }
}
#endif // ENABLE_CUPS

const PPDParser* CUPSManager::createCUPSParser( const OUString& rPrinter )
{
    const PPDParser* pNewParser = NULL;
    OUString aPrinter;

    if( rPrinter.compareToAscii( "CUPS:", 5 ) == 0 )
        aPrinter = rPrinter.copy( 5 );
    else
        aPrinter = rPrinter;

#ifdef ENABLE_CUPS
    if( m_aCUPSMutex.tryToAcquire() && m_nDests && m_pDests )
    {
        std::hash_map< OUString, int, OUStringHash >::iterator dest_it =
            m_aCUPSDestMap.find( aPrinter );
        if( dest_it != m_aCUPSDestMap.end() )
        {
            cups_dest_t* pDest = ((cups_dest_t*)m_pDests) + dest_it->second;
            const char* pPPDFile = m_pCUPSWrapper->cupsGetPPD( pDest->name );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "PPD for %s is %s\n", OUStringToOString( aPrinter, osl_getThreadTextEncoding() ).getStr(), pPPDFile );
#endif
            if( pPPDFile )
            {
                rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
                OUString aFileName( OStringToOUString( pPPDFile, aEncoding ) );
                // create the new parser
                PPDParser* pCUPSParser =  new PPDParser( aFileName );
                pCUPSParser->m_aFile = rPrinter;
                pNewParser = pCUPSParser;

                // update the printer info with context information
                ppd_file_t* pPPD = m_pCUPSWrapper->ppdOpenFile( pPPDFile );
                /*int nConflicts =*/ m_pCUPSWrapper->cupsMarkOptions( pPPD, pDest->num_options, pDest->options );
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "processing the following options for printer %s (instance %s):\n",
                         pDest->name, pDest->instance );
                for( int k = 0; k < pDest->num_options; k++ )
                    fprintf( stderr, "   \"%s\" = \"%s\"\n",
                             pDest->options[k].name,
                             pDest->options[k].value );
#endif
                PrinterInfo& rInfo = m_aPrinters[ aPrinter ].m_aInfo;

                rInfo.m_pParser = pNewParser;
                rInfo.m_aContext.setParser( pNewParser );
                for( int i = 0; i < pPPD->num_groups; i++ )
                    updatePrinterContextInfo( pPPD->groups + i, rInfo );

                // clean up the mess
                m_pCUPSWrapper->ppdClose( pPPD );

                // remove temporary PPD file
                unlink( pPPDFile );
            }
#if OSL_DEBUG_LEVEL > 1
            else
                fprintf( stderr, "no dest found for printer %s\n", OUStringToOString( aPrinter, osl_getThreadTextEncoding() ).getStr() );
#endif
        }
        m_aCUPSMutex.release();
    }
#endif // ENABLE_CUPS

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
#ifdef ENABLE_CUPS
    std::hash_map< OUString, int, OUStringHash >::iterator dest_it =
        m_aCUPSDestMap.find( rData.m_aPrinterName );

    if( dest_it == m_aCUPSDestMap.end() )
        return PrinterInfoManager::setupJobContextData( rData );

    std::hash_map< OUString, Printer, OUStringHash >::iterator p_it =
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

    rData.m_pParser     = p_it->second.m_aInfo.m_pParser;
    rData.m_aContext    = p_it->second.m_aInfo.m_aContext;
#endif
}

FILE* CUPSManager::startSpool( const OUString& rPrintername )
{
#ifdef ENABLE_CUPS
    OUString aTmpURL, aTmpFile;
    osl_createTempFile( NULL, NULL, &aTmpURL.pData );
    osl_getSystemPathFromFileURL( aTmpURL.pData, &aTmpFile.pData );
    OString aSysFile = OUStringToOString( aTmpFile, osl_getThreadTextEncoding() );
    FILE* fp = fopen( aSysFile.getStr(), "w" );
    if( fp )
        m_aSpoolFiles[fp] = aSysFile;

    return fp;
#else
    return NULL;
#endif
}

int CUPSManager::endSpool( const OUString& rPrintername, const OUString& rJobTitle, FILE* pFile )
{
    int nJobID = 0;

#ifdef ENABLE_CUPS
    std::hash_map< FILE*, OString, FPtrHash >::const_iterator it = m_aSpoolFiles.find( pFile );
    if( it != m_aSpoolFiles.end() )
    {
        fclose( pFile );
        rtl_TextEncoding aEnc = osl_getThreadTextEncoding();

        osl::MutexGuard aGuard( m_aCUPSMutex );

        std::hash_map< OUString, int, OUStringHash >::iterator dest_it =
            m_aCUPSDestMap.find( rPrintername );
        if( dest_it != m_aCUPSDestMap.end() )
        {
            cups_dest_t* pDest = ((cups_dest_t*)m_pDests) + dest_it->second;
            nJobID = m_pCUPSWrapper->cupsPrintFile( pDest->name,
                                                    it->second.getStr(),
                                                    OUStringToOString( rJobTitle, aEnc ).getStr(),
                                                    0, NULL );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "cupsPrintFile( %s, %s, %s, 0, 0 ) returns %d\n",
                     pDest->name,
                     it->second.getStr(),
                     OUStringToOString( rJobTitle, aEnc ).getStr(),
                     nJobID );
            OString aCmd( "cp " );
            aCmd = aCmd + it->second;
            aCmd = aCmd + OString( " $HOME/cupsprint.ps" );
            system( aCmd.getStr() );
#endif
        }
#if OSL_DEBUG_LEVEL > 1
        else
            fprintf( stderr, "Error: no CUPS dest for %s found, discarding job\n", OUStringToOString( rPrintername, aEnc ).getStr() );
#endif

        unlink( it->second.getStr() );
        m_aSpoolFiles.erase( pFile );
    }
#endif // ENABLE_CUPS

    return nJobID;
}


void CUPSManager::changePrinterInfo( const OUString& rPrinter, const PrinterInfo& rNewInfo )
{
    PrinterInfoManager::changePrinterInfo( rPrinter, rNewInfo );
}

bool CUPSManager::checkPrintersChanged()
{
    bool bChanged = false;
    if( m_aCUPSMutex.tryToAcquire() )
    {
        bChanged = m_bNewDests;
        m_aCUPSMutex.release();
        initialize();
    }

    if( ! bChanged )
        bChanged = PrinterInfoManager::checkPrintersChanged();

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
#ifdef ENABLE_CUPS
    std::hash_map< OUString, int, OUStringHash >::iterator nit =
        m_aCUPSDestMap.find( rName );
    if( nit != m_aCUPSDestMap.end() && m_aCUPSMutex.tryToAcquire() )
    {
        cups_dest_t* pDests = (cups_dest_t*)m_pDests;
        for( int i = 0; i < m_nDests; i++ )
            pDests[i].is_default = 0;
        pDests[ nit->second ].is_default = 1;
        m_pCUPSWrapper->cupsSetDests( m_nDests, (cups_dest_t*)m_pDests );
        m_aDefaultPrinter = rName;
        m_aCUPSMutex.release();
        bSuccess = true;
    }
#endif
    return bSuccess;
}

bool CUPSManager::writePrinterConfig()
{
#ifdef ENABLE_CUPS
    bool bDestModified = false;
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    for( std::hash_map< OUString, Printer, OUStringHash >::iterator prt =
             m_aPrinters.begin(); prt != m_aPrinters.end(); ++prt )
    {
        std::hash_map< OUString, int, OUStringHash >::iterator nit =
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
                    nNewOptions = m_pCUPSWrapper->cupsAddOption( aName.getStr(), aValue.getStr(), nNewOptions, &pNewOptions );
                }
            }
            // set PPD options on CUPS dest
            m_pCUPSWrapper->cupsFreeOptions( pDest->num_options, pDest->options );
            pDest->num_options = nNewOptions;
            pDest->options = pNewOptions;
            m_aCUPSMutex.release();
        }
    }
    if( bDestModified && m_aCUPSMutex.tryToAcquire() )
    {
        m_pCUPSWrapper->cupsSetDests( m_nDests, (cups_dest_t*)m_pDests );
        m_aCUPSMutex.release();
    }
#endif // ENABLE_CUPS

    return PrinterInfoManager::writePrinterConfig();
}

#include <rtsname.hxx>

const char* CUPSManager::authenticateUser( const char* pIn )
{
    const char* pRet = NULL;

#ifdef ENABLE_CUPS
    OUString aLib = OUString::createFromAscii( _XSALSET_LIBNAME );
    oslModule pLib = osl_loadModule( aLib.pData, SAL_LOADMODULE_LAZY );
    if( pLib )
    {
        OUString aSym( RTL_CONSTASCII_USTRINGPARAM( "Sal_authenticateQuery" ) );
        bool (*getpw)( const OString& rServer, OString& rUser, OString& rPw) =
            (bool(*)(const OString&,OString&,OString&))osl_getSymbol( pLib, aSym.pData );
        if( getpw )
        {
            osl::MutexGuard aGuard( m_aCUPSMutex );

            OString aUser = m_pCUPSWrapper->cupsUser();
            OString aServer = m_pCUPSWrapper->cupsServer();
            OString aPassword;
            if( getpw( aServer, aUser, aPassword ) )
            {
                m_aPassword = aPassword;
                m_aUser = aUser;
                m_pCUPSWrapper->cupsSetUser( m_aUser.getStr() );
                pRet = m_aPassword.getStr();
            }
        }
        osl_unloadModule( pLib );
    }
#if OSL_DEBUG_LEVEL > 1
    else fprintf( stderr, "loading of module %s failed\n", OUStringToOString( aLib, osl_getThreadTextEncoding() ).getStr() );
#endif
#endif // ENABLE_CUPS

    return pRet;
}
