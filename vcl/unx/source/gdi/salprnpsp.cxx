/*************************************************************************
 *
 *  $RCSfile: salprnpsp.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pl $ $Date: 2001-03-01 18:04:50 $
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

/**
  this file implements the sal printer interface ( SalPrinter, SalInfoPrinter
  and some printer relevant methods of SalInstance and SalGraphicsData )

  as aunderlying library the printer features of psprint are used.

  The query methods of a SalInfoPrinter are implemented by querying psprint

  The job methods of a SalPrinter are implemented by calling psprint
  printer job functions.
 */

#include <salunx.h>
#include <unistd.h>
#include <sys/wait.h>

#ifndef _SV_JOBSET_H
#include <jobset.h>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_SALPTYPE_HXX
#include <salptype.hxx>
#endif

#ifndef _PSPRINT_PRINTERINFOMANAGER_HXX_
#include <psprint/printerinfomanager.hxx>
#endif

using namespace psp;
using namespace rtl;

/*
 *  static helpers
 */

// NETBSD has no RTLD_GLOBAL
#ifndef RTLD_GLOBAL
#define DLOPEN_MODE (RTLD_LAZY)
#else
#define DLOPEN_MODE (RTLD_GLOBAL | RTLD_LAZY)
#endif
#include <dlfcn.h>
#include <rtsname.hxx>

static void* driverLib                      = NULL;
static int(*pSetupFunction)(PrinterInfo&)   = NULL;
static int(*pFaxNrFunction)(String&)        = NULL;

static String getPdfDir( const PrinterInfo& rInfo )
{
    String aDir;
    int nTokens = rInfo.m_aFeatures.getTokenCount( ',' );
    for( int i = 0; i < nTokens; i++ )
    {
        OUString aToken( rInfo.m_aFeatures.getToken( i, ',' ) );
        if( ! aToken.compareToAscii( "pdf=", 4 ) )
        {
            aDir = aToken.getToken( 1, '=' );
            if( ! aDir.Len() )
                aDir = String( ByteString( getenv( "HOME" ) ), gsl_getSystemTextEncoding() );
            break;
        }
    }
    return aDir;
}

static void getPaLib()
{
    const char* pErr = NULL;

    if( ! driverLib )
    {
        driverLib   = dlopen( _XSALSET_LIBNAME, DLOPEN_MODE );
        pErr        = dlerror();
        if ( !driverLib )
        {
            fprintf( stderr, "%s: when opening %s\n", pErr, _XSALSET_LIBNAME );
            return;
        }

        pSetupFunction  = (int(*)(PrinterInfo&))dlsym( driverLib, "Sal_SetupPrinterDriver" );
        pErr        = dlerror();
        if ( !pSetupFunction )
            fprintf( stderr, "%s: when getting Sal_SetupPrinterDriver\n", pErr );

        pFaxNrFunction = (int(*)(String&))dlsym( driverLib, "Sal_queryFaxNumber" );
        pErr        = dlerror();
        if ( !pFaxNrFunction )
            fprintf( stderr, "%s: when getting Sal_queryFaxNumber\n", pErr );
    }
}

inline PtTo10Mu( int nPoints ) { return (int)(((double)nPoints)*35.27777778); }

inline TenMuToPt( int nUnits ) { return (int)(((double)nUnits)/35.27777778); }

static struct
{
    int         width;
    int         height;
    const char* name;
    int         namelength;
    Paper       paper;
} aPaperTab[] =
{
    { 29700, 42000, "A3",           2,  PAPER_A3        },
    { 21000, 29700, "A4",           2,  PAPER_A4        },
    { 14800, 21000, "A5",           2,  PAPER_A5        },
    { 25000, 35300, "B4",           2,  PAPER_B4        },
    { 17600, 25000, "B5",           2,  PAPER_B5        },
    { 21600, 27900, "Letter",       6,  PAPER_LETTER    },
    { 21600, 35600, "Legal",        5,  PAPER_LEGAL     },
    { 27900, 43100, "Tabloid",      7,  PAPER_TABLOID   },
    { 0, 0,         "USER",         4,  PAPER_USER      }
};

static Paper getPaperType( const String& rPaperName )
{
    ByteString aPaper( rPaperName, RTL_TEXTENCODING_ISO_8859_1 );
    for( int i = 0; i < sizeof( aPaperTab )/sizeof( aPaperTab[0] ); i++ )
    {
        if( ! strcmp( aPaper.GetBuffer(), aPaperTab[i].name ) )
            return aPaperTab[i].paper;
    }
    return PAPER_USER;
}

static void copyJobDataToJobSetup( ImplJobSetup* pJobSetup, JobData& rData )
{
    pJobSetup->meOrientation    = (Orientation)(rData.m_eOrientation == orientation::Landscape ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT);

    // copy page size
    String aPaper;
    int width, height;

    rData.m_aContext.getPageSize( aPaper, width, height );
    pJobSetup->mePaperFormat    = getPaperType( aPaper );
    pJobSetup->mnPaperWidth     = 0;
    pJobSetup->mnPaperHeight    = 0;
    if( pJobSetup->mePaperFormat == PAPER_USER )
    {
        // transform to 100dth mm
        width               = PtTo10Mu( width );
        height              = PtTo10Mu( height );

        pJobSetup->mnPaperWidth = width;
        pJobSetup->mnPaperHeight= height;
    }


    // copy input slot
    const PPDKey* pKey;
    const PPDValue* pValue;
    ::std::list< const PPDValue* > aValues;
    ::std::list< const PPDValue* >::iterator it;

    pKey                        = rData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
    pValue                      = rData.m_aContext.getValue( pKey );
    rData.m_aContext.getUnconstrainedValues( pKey, aValues );
    pJobSetup->mnPaperBin       = 0;
    for( it = aValues.begin(); it != aValues.end(); ++it, pJobSetup->mnPaperBin++ )
        if( *it == pValue )
            break;
    if( it == aValues.end() )
        pJobSetup->mnPaperBin = 0xffff;


    // copy the whole context
    if( pJobSetup->mpDriverData )
        delete pJobSetup->mpDriverData;

    int nBytes;
    void* pBuffer = NULL;
    if( rData.getStreamBuffer( pBuffer, nBytes ) )
    {
        pJobSetup->mnDriverDataLen = nBytes;
        pJobSetup->mpDriverData = (BYTE*)pBuffer;
    }
    else
    {
        pJobSetup->mnDriverDataLen = 0;
        pJobSetup->mpDriverData = NULL;
    }
}

static bool passFileToCommandLine( const String& rFilename, const String& rCommandLine )
{
    bool bSuccess = false;

    rtl_TextEncoding aEncoding = gsl_getSystemTextEncoding();
    ByteString aCmdLine( rCommandLine, aEncoding );
    ByteString aFilename( rFilename, aEncoding );
    ByteString aPSFilename( aFilename );
    aPSFilename.Append( ".ps" );

    BOOL bPS = link( aFilename.GetBuffer(), aPSFilename.GetBuffer() ) ? FALSE : TRUE;

    const ByteString& rUseFilename( bPS ? aPSFilename : aFilename );
    bool bPipe = aCmdLine.Search( "(TMP)" ) != STRING_NOTFOUND ? false : true;

    // setup command line for exec
    if( ! bPipe )
        while( aCmdLine.SearchAndReplace( "(TMP)", rUseFilename ) != STRING_NOTFOUND )
            ;

#ifdef DEBUG
    fprintf( stderr, "%s commandline: \"%s\"\n",
             bPipe ? "piping to" : "executing",
             aCmdLine.GetBuffer() );
#endif
    const char* argv[4];
    if( ! ( argv[ 0 ] = getenv( "SHELL" ) ) )
        argv[ 0 ] = "/bin/sh";
    argv[ 1 ] = "-c";
    argv[ 2 ] = aCmdLine.GetBuffer();
    argv[ 3 ] = 0;

    bool bHavePipes = false;
    int pid, fd[2];

    if( bPipe )
        bHavePipes = pipe( fd ) ? false : true;
    if( ( pid = fork() ) > 0 )
    {
        if( bPipe && bHavePipes )
        {
            close( fd[0] );
            char aBuffer[ 2048 ];
            FILE* fp = fopen( aFilename.GetBuffer(), "r" );
            while( fp && ! feof( fp ) )
            {
                int nBytes = fread( aBuffer, 1, sizeof( aBuffer ), fp );
                if( nBytes )
                    write( fd[ 1 ], aBuffer, nBytes );
            }
            fclose( fp );
            close( fd[ 1 ] );
        }
        int status = 0;
        waitpid( pid, &status, 0 );
        if( ! status )
            bSuccess = true;
    }
    else if( ! pid )
    {
        if( bPipe && bHavePipes )
        {
            close( fd[1] );
            if( fd[0] != STDIN_FILENO ) // not probable, but who knows :)
                dup2( fd[0], STDIN_FILENO );
        }
        execv( argv[0], const_cast<char**>(argv) );
        fprintf( stderr, "failed to execute \"%s\"\n", aCmdLine.GetBuffer() );
        _exit( 1 );
    }
    else
        fprintf( stderr, "failed to fork\n" );

    // clean up the mess
    unlink( aFilename.GetBuffer() );
    if( bPS )
        unlink( aPSFilename.GetBuffer() );
    return bSuccess;
}

static bool sendAFax( const String& rFaxNumber, const String& rFileName, const String& rCommand )
{
    rtl_TextEncoding aEncoding = gsl_getSystemTextEncoding();
    String aFaxNumber( rFaxNumber );
    String aCmdLine( rCommand );
    if( ! aFaxNumber.Len() )
    {
        getPaLib();
        if( pFaxNrFunction )
        {
            String aNewNr;
            if( pFaxNrFunction( aNewNr ) )
                aFaxNumber = aNewNr, aEncoding;
        }
    }

    if( aFaxNumber.Len() )
    {
        while( aCmdLine.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "(PHONE)" ) ), aFaxNumber ) != STRING_NOTFOUND )
        ;
    }

    return passFileToCommandLine( rFileName, aCmdLine );
}

static bool createPdf( const String& rToFile, const String& rFromFile, const String& rCommandLine )
{
    String aCommandLine( rCommandLine );
    while( aCommandLine.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "(OUTFILE)" ) ), rToFile ) != STRING_NOTFOUND )
        ;
    return passFileToCommandLine( rFromFile, aCommandLine );
}

/*
 *  SalInstance
 */

// -----------------------------------------------------------------------

SalInfoPrinter* SalInstance::CreateInfoPrinter(
    SalPrinterQueueInfo*    pQueueInfo,
    ImplJobSetup*           pJobSetup )
{
    // create and initialize SalInfoPrinter
    SalInfoPrinter* pPrinter = new SalInfoPrinter;

    if( pJobSetup )
    {
        PrinterInfoManager& rManager( PrinterInfoManager::get() );
        PrinterInfo aInfo( rManager.getPrinterInfo( pQueueInfo->maPrinterName ) );
        pPrinter->maPrinterData.m_aJobData = aInfo;
        pPrinter->maPrinterData.m_aPrinterGfx.Init( pPrinter->maPrinterData.m_aJobData );

        if( pJobSetup->mpDriverData )
            JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aInfo );

        pJobSetup->mnSystem         = JOBSETUP_SYSTEM_UNIX;
        pJobSetup->maPrinterName    = pQueueInfo->maPrinterName;
        pJobSetup->maDriver         = aInfo.m_aDriverName;
        copyJobDataToJobSetup( pJobSetup, aInfo );
    }


    return pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

SalPrinter* SalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    // create and initialize SalPrinter
    SalPrinter* pPrinter = new SalPrinter;
    pPrinter->maPrinterData.m_aJobData = pInfoPrinter->maPrinterData.m_aJobData;

    return pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    ::std::list< OUString > aPrinters;
    rManager.listPrinters( aPrinters );

    for( ::std::list< OUString >::iterator it = aPrinters.begin(); it != aPrinters.end(); ++it )
    {
        const PrinterInfo& rInfo( rManager.getPrinterInfo( *it ) );
        // Neuen Eintrag anlegen
        SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
        pInfo->maPrinterName    = *it;
        pInfo->maDriver         = rInfo.m_aDriverName;
        pInfo->maLocation       = rInfo.m_aLocation;
        pInfo->maComment        = rInfo.m_aComment;
        pInfo->mpSysData        = NULL;

        int nTokens = rInfo.m_aFeatures.getTokenCount( ',' );
        for( int i = 0; i < nTokens; i++ )
        {
            String aToken( rInfo.m_aFeatures.getToken( i, ',' ) );
            if( aToken.CompareToAscii( "pdf=", 4 ) == COMPARE_EQUAL )
            {
                pInfo->maLocation = getPdfDir( rInfo );
                break;
            }
        }

        pList->Add( pInfo );
    }
}

// -----------------------------------------------------------------------

void SalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
}

// -----------------------------------------------------------------------

String SalInstance::GetDefaultPrinter()
{
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    return rManager.getDefaultPrinter();
}

// =======================================================================

SalInfoPrinter::SalInfoPrinter()
{
    maPrinterData.m_pGraphics = NULL;
}

// -----------------------------------------------------------------------

SalInfoPrinter::~SalInfoPrinter()
{
    if( maPrinterData.m_pGraphics )
    {
        delete maPrinterData.m_pGraphics;
        maPrinterData.m_pGraphics = NULL;
    }
}

// -----------------------------------------------------------------------

SalGraphics* SalInfoPrinter::GetGraphics()
{
    // return a valid pointer only once
    // the reasoning behind this is that we could have different
    // SalGraphics that can run in multiple threads
    // (future plans)
    SalGraphics* pRet = NULL;
    if( ! maPrinterData.m_pGraphics )
    {
        maPrinterData.m_pGraphics = new SalGraphics;
        maPrinterData.m_pGraphics->maGraphicsData.m_pJobData    = &maPrinterData.m_aJobData;
        maPrinterData.m_pGraphics->maGraphicsData.m_pPrinterGfx = &maPrinterData.m_aPrinterGfx;
        maPrinterData.m_pGraphics->maGraphicsData.bPrinter_     = TRUE;
        pRet = maPrinterData.m_pGraphics;
    }
    return pRet;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
    if( pGraphics == maPrinterData.m_pGraphics )
    {
        delete pGraphics;
        maPrinterData.m_pGraphics = NULL;
    }
    return;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pJobSetup )
{
    if( ! pFrame || ! pJobSetup )
        return FALSE;

    getPaLib();

    if( ! pSetupFunction )
        return FALSE;

    PrinterInfoManager& rManager = PrinterInfoManager::get();

    PrinterInfo aInfo( rManager.getPrinterInfo( pJobSetup->maPrinterName ) );
    if ( pJobSetup->mpDriverData )
        JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aInfo );

    if( pSetupFunction( aInfo ) )
    {
        delete pJobSetup->mpDriverData;
        pJobSetup->mpDriverData = NULL;

        int nBytes;
        void* pBuffer = NULL;
        aInfo.getStreamBuffer( pBuffer, nBytes );
        pJobSetup->mnDriverDataLen  = nBytes;
        pJobSetup->mpDriverData     = (BYTE*)pBuffer;

        // copy everything to job setup
        copyJobDataToJobSetup( pJobSetup, aInfo );
    }

    return TRUE;
}

// -----------------------------------------------------------------------

// This function gets the driver data and puts it into pJobSetup
// If pJobSetup->mpDriverData is NOT NULL, then the independend
// data should be merged into the driver data
// If pJobSetup->mpDriverData IS NULL, then the driver defaults
// should be merged into the independent data
BOOL SalInfoPrinter::SetPrinterData( ImplJobSetup* pJobSetup )
{
    if( pJobSetup->mpDriverData )
        return SetData( ~0, pJobSetup );

    copyJobDataToJobSetup( pJobSetup, maPrinterData.m_aJobData );
    return TRUE;
}

// -----------------------------------------------------------------------

// This function merges the independ driver data
// and sets the new independ data in pJobSetup
// Only the data must be changed, where the bit
// in nGetDataFlags is set
BOOL SalInfoPrinter::SetData(
    ULONG nSetDataFlags,
    ImplJobSetup* pJobSetup )
{
    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    if( aData.m_pParser )
    {
        const PPDKey* pKey;
        const PPDValue* pValue;

        // merge papersize if necessary
        if( nSetDataFlags & SAL_JOBSET_PAPERSIZE )
        {
            String aPaper;
            if( pJobSetup->mePaperFormat == PAPER_USER )
                aPaper = aData.m_pParser->matchPaper(
                    TenMuToPt( pJobSetup->mnPaperWidth ),
                    TenMuToPt( pJobSetup->mnPaperHeight ) );
            else
                aPaper = String( ByteString( aPaperTab[ pJobSetup->mePaperFormat ].name ), RTL_TEXTENCODING_ISO_8859_1 );
            pKey = aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
            pValue = pKey ? pKey->getValue( aPaper ) : NULL;
            if( ! ( pKey && pValue && aData.m_aContext.setValue( pKey, pValue, false ) == pValue ) )
                return FALSE;
        }

        // merge paperbin if necessary
        if( nSetDataFlags & SAL_JOBSET_PAPERBIN )
        {
            pKey = aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
            if( pKey )
            {
                int nPaperBin = pJobSetup->mnPaperBin;
                if( nPaperBin == 0xffff )
                    pValue = pKey->getDefaultValue();
                else
                {
                    ::std::list< const PPDValue* > aSlots;
                    aData.m_aContext.getUnconstrainedValues( pKey, aSlots );
                    ::std::list< const PPDValue* >::iterator it;
                    for( it = aSlots.begin(); it != aSlots.end() && nPaperBin; ++it, --nPaperBin )
                        ;
                    if( it == aSlots.end() )
                        return FALSE;
                    pValue = *it;
                }
                aData.m_aContext.setValue( pKey, pValue );
            }
            // if printer has no InputSlot key simply ignore this setting
            // (e.g. SGENPRT has no InputSlot)
        }

        // merge orientation if necessary
        if( nSetDataFlags & SAL_JOBSET_ORIENTATION )
            aData.m_eOrientation = pJobSetup->meOrientation == ORIENTATION_LANDSCAPE ? orientation::Landscape : orientation::Portrait;

        maPrinterData.m_aJobData = aData;
        copyJobDataToJobSetup( pJobSetup, aData );
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::GetPageInfo(
    const ImplJobSetup* pJobSetup,
    long& rOutWidth, long& rOutHeight,
    long& rPageOffX, long& rPageOffY,
    long& rPageWidth, long& rPageHeight )
{
    if( ! pJobSetup )
        return;

    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    // get the selected page size
    if( aData.m_pParser )
    {

        String aPaper;
        int width, height;

        aData.m_aContext.getPageSize( aPaper, width, height );

        int left = 0, top = 0, right = 0, bottom = 0;
        int nDPIx, nDPIy;
        aData.m_aContext.getResolution( nDPIx, nDPIy );
        aData.m_pParser->getMargins( aPaper, left, right, top, bottom );
        rPageWidth  = width * nDPIx / 72;
        rPageHeight = height * nDPIy / 72;
        rPageOffX   = left * nDPIx / 72;
        rPageOffY   = top * nDPIy / 72;
        rOutWidth   = ( width  - left - right ) * nDPIx / 72;
        rOutHeight  = ( height - top  - bottom ) * nDPIy / 72;
    }
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pJobSetup )
{
    if( ! pJobSetup )
        return 0;

    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    ::std::list< const PPDValue* > aValues;
    const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) ): NULL;
    aData.m_aContext.getUnconstrainedValues( pKey, aValues );

    return aValues.size();
}

// -----------------------------------------------------------------------

String SalInfoPrinter::GetPaperBinName( const ImplJobSetup* pJobSetup, ULONG nPaperBin )
{
    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    String aRet;
    if( aData.m_pParser )
    {
        ::std::list< const PPDValue* > aValues;
        const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) ): NULL;
        aData.m_aContext.getUnconstrainedValues( pKey, aValues );
        if( nPaperBin == 0xffff )
            aRet = aData.m_pParser->getDefaultInputSlot();
        else
        {
            ::std::list< const PPDValue* >::iterator it;
            for( it = aValues.begin(); it != aValues.end(); ++it )
            {
                if( ! nPaperBin-- )
                {
                    aRet = String( (*it)->m_aOption );
                    break;
                }
            }
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetCapabilities( const ImplJobSetup* pJobSetup, USHORT nType )
{
    switch( nType )
    {
        case PRINTER_CAPABILITIES_SUPPORTDIALOG:
            return 1;
        case PRINTER_CAPABILITIES_COPIES:
            return 0xffff;
        case PRINTER_CAPABILITIES_COLLATECOPIES:
            return 0;
        case PRINTER_CAPABILITIES_SETORIENTATION:
            return 1;
        case PRINTER_CAPABILITIES_SETPAPERBIN:
            return 1;
        case PRINTER_CAPABILITIES_SETPAPERSIZE:
            return 1;
        case PRINTER_CAPABILITIES_SETPAPER:
            return 0;
        case PRINTER_CAPABILITIES_FAX:
        {
            PrinterInfoManager& rManager = PrinterInfoManager::get();
            PrinterInfo aInfo( rManager.getPrinterInfo( pJobSetup->maPrinterName ) );
            String aFeatures( aInfo.m_aFeatures );
            int nTokenCount = aFeatures.GetTokenCount( ',' );
            for( int i = 0; i < nTokenCount; i++ )
            {
                if( aFeatures.GetToken( i ).EqualsAscii( "fax" ) )
                    return 1;
            }
            return 0;
        }
        default: break;
    };
    return 0;
}

// =======================================================================

/*
 *  SalPrinter
 */

SalPrinter::SalPrinter()
{
}

// -----------------------------------------------------------------------

SalPrinter::~SalPrinter()
{
}

// -----------------------------------------------------------------------

static inline String getTmpName()
{
    char tmpNam[ L_tmpnam ];
    tmpnam_r( tmpNam );
    return String( ByteString( tmpNam ), gsl_getSystemTextEncoding() );
}

BOOL SalPrinter::StartJob(
    const XubString* pFileName,
    const XubString& rJobName,
    const XubString& rAppName,
    ULONG nCopies, BOOL bCollate,
    ImplJobSetup* pJobSetup )
{
    maPrinterData.m_bFax        = false;
    maPrinterData.m_bPdf        = false;
    maPrinterData.m_aFileName   = pFileName ? *pFileName : String();
    maPrinterData.m_aTmpFile    = String();

    // get the job data
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, maPrinterData.m_aJobData );

    // check wether this printer is configured as fax
    const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( maPrinterData.m_aJobData.m_aPrinterName ) );
    int nTokens = rInfo.m_aFeatures.getTokenCount( ',' );
    for( int i = 0; i < nTokens; i++ )
    {
        if( rInfo.m_aFeatures.getToken( i, ',' ).equalsAsciiL( "fax", 3 ) )
        {
            maPrinterData.m_bFax = true;
            maPrinterData.m_aTmpFile = getTmpName();

            ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >::const_iterator it;
            it = pJobSetup->maValueMap.find( ::rtl::OUString::createFromAscii( "FAX#" ) );
            if( it != pJobSetup->maValueMap.end() )
                maPrinterData.m_aFaxNr = it->second;
            break;
        }
        if( ! rInfo.m_aFeatures.getToken( i, ',' ).compareToAscii( "pdf=", 4 ) )
        {
            maPrinterData.m_bPdf = true;
            maPrinterData.m_aTmpFile = getTmpName();
            if( ! maPrinterData.m_aFileName.Len() )
            {
                maPrinterData.m_aFileName = getPdfDir( rInfo );
                maPrinterData.m_aFileName.Append( '/' );
                maPrinterData.m_aFileName.Append( rJobName );
                maPrinterData.m_aFileName.AppendAscii( ".pdf" );
            }
            break;
        }
    }

    return maPrinterData.m_aPrintJob.StartJob( maPrinterData.m_aTmpFile.Len() ? maPrinterData.m_aTmpFile : maPrinterData.m_aFileName, rJobName, rAppName, maPrinterData.m_aJobData ) ? TRUE : FALSE;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndJob()
{
    BOOL bSuccess = maPrinterData.m_aPrintJob.EndJob();

    if( bSuccess )
    {
        // check for fax
        if( maPrinterData.m_bFax )
        {

            const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( maPrinterData.m_aJobData.m_aPrinterName ) );
            // sendAFax removes the file after use
            bSuccess = sendAFax( maPrinterData.m_aFaxNr, maPrinterData.m_aTmpFile, rInfo.m_aCommand );
        }
        else if( maPrinterData.m_bPdf )
        {
            const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( maPrinterData.m_aJobData.m_aPrinterName ) );
            bSuccess = createPdf( maPrinterData.m_aFileName, maPrinterData.m_aTmpFile, rInfo.m_aCommand );
        }
    }
    return bSuccess;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::AbortJob()
{
    return maPrinterData.m_aPrintJob.AbortJob() ? TRUE : FALSE;
}

// -----------------------------------------------------------------------

SalGraphics* SalPrinter::StartPage( ImplJobSetup* pJobSetup, BOOL bNewJobData )
{
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, maPrinterData.m_aJobData );
    maPrinterData.m_pGraphics = new SalGraphics();
    maPrinterData.m_pGraphics->maGraphicsData.m_pJobData    = &maPrinterData.m_aJobData;
    maPrinterData.m_pGraphics->maGraphicsData.m_pPrinterGfx = &maPrinterData.m_aPrinterGfx;
    maPrinterData.m_pGraphics->maGraphicsData.bPrinter_     = true;
    maPrinterData.m_pGraphics->maGraphicsData.m_pPhoneNr    = maPrinterData.m_bFax ? &maPrinterData.m_aFaxNr : NULL;

    maPrinterData.m_aPrintJob.StartPage( maPrinterData.m_aJobData, bNewJobData ? sal_True : sal_False );
    maPrinterData.m_aPrinterGfx.Init( maPrinterData.m_aPrintJob );

    return maPrinterData.m_pGraphics;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndPage()
{
    maPrinterData.m_aPrinterGfx.OnEndPage();
    sal_Bool bResult = maPrinterData.m_aPrintJob.EndPage();
    maPrinterData.m_aPrinterGfx.Clear();
    return bResult ? TRUE : FALSE;
}

// -----------------------------------------------------------------------

ULONG SalPrinter::GetErrorCode()
{
    return 0;
}
