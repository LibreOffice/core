/*************************************************************************
 *
 *  $RCSfile: salprnpsp.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:46:26 $
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
#include <sys/stat.h>

#ifndef _SV_JOBSET_H
#include <jobset.h>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALPRN_H
#include <salprn.h>
#endif
#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_SALPTYPE_HXX
#include <salptype.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
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
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( ! aToken.compareToAscii( "pdf=", 4 ) )
        {
            sal_Int32 nPos = 0;
            aDir = aToken.getToken( 1, '=', nPos );
            if( ! aDir.Len() )
                aDir = String( ByteString( getenv( "HOME" ) ), osl_getThreadTextEncoding() );
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

inline PtTo10Mu( int nPoints ) { return (int)((((double)nPoints)*35.27777778)+0.5); }

inline TenMuToPt( int nUnits ) { return (int)((((double)nUnits)/35.27777778)+0.5); }

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

        if( rData.m_eOrientation == psp::orientation::Portrait )
        {
            pJobSetup->mnPaperWidth = width;
            pJobSetup->mnPaperHeight= height;
        }
        else
        {
            pJobSetup->mnPaperWidth = height;
            pJobSetup->mnPaperHeight= width;
        }
    }

    // copy input slot
    const PPDKey* pKey = NULL;
    const PPDValue* pValue = NULL;
    ::std::list< const PPDValue* > aValues;
    ::std::list< const PPDValue* >::iterator it;

    pJobSetup->mnPaperBin = 0xffff;
    pKey                        = rData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
    if( pKey )
        pValue                  = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        for( pJobSetup->mnPaperBin = 0;
             pValue != pKey->getValue( pJobSetup->mnPaperBin ) &&
                 pJobSetup->mnPaperBin < pKey->countValues();
             pJobSetup->mnPaperBin++ )
            ;
        if( pJobSetup->mnPaperBin >= pKey->countValues() || pValue == pKey->getDefaultValue() )
            pJobSetup->mnPaperBin = 0xffff;
    }


    // copy the whole context
    if( pJobSetup->mpDriverData )
        rtl_freeMemory( pJobSetup->mpDriverData );

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

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    ByteString aCmdLine( rCommandLine, aEncoding );
    ByteString aFilename( rFilename, aEncoding );

    bool bPipe = aCmdLine.Search( "(TMP)" ) != STRING_NOTFOUND ? false : true;

    // setup command line for exec
    if( ! bPipe )
        while( aCmdLine.SearchAndReplace( "(TMP)", aFilename ) != STRING_NOTFOUND )
            ;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "%s commandline: \"%s\"\n",
             bPipe ? "piping to" : "executing",
             aCmdLine.GetBuffer() );
    struct stat aStat;
    if( stat( aFilename.GetBuffer(), &aStat ) )
        fprintf( stderr, "stat( %s ) failed\n", aFilename.GetBuffer() );
    fprintf( stderr, "Tmp file %s has modes: %o\n", aFilename.GetBuffer(), aStat.st_mode );
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

    return bSuccess;
}

static bool sendAFax( const String& rFaxNumber, const String& rFileName, const String& rCommand )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
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

SalInfoPrinter* X11SalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                   ImplJobSetup*            pJobSetup )
{
    mbPrinterInit = true;
    // create and initialize SalInfoPrinter
    PspSalInfoPrinter* pPrinter = new PspSalInfoPrinter;

    if( pJobSetup )
    {
        PrinterInfoManager& rManager( PrinterInfoManager::get() );
        PrinterInfo aInfo( rManager.getPrinterInfo( pQueueInfo->maPrinterName ) );
        pPrinter->m_aJobData = aInfo;
        pPrinter->m_aPrinterGfx.Init( pPrinter->m_aJobData );

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

void X11SalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

SalPrinter* X11SalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    mbPrinterInit = true;
    // create and initialize SalPrinter
    PspSalPrinter* pPrinter = new PspSalPrinter;
    pPrinter->m_aJobData = static_cast<PspSalInfoPrinter*>(pInfoPrinter)->m_aJobData;

    return pPrinter;
}

// -----------------------------------------------------------------------

void X11SalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

void X11SalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    mbPrinterInit = true;
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

        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            String aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
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

void X11SalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

// -----------------------------------------------------------------------

void X11SalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
    mbPrinterInit = true;
}

// -----------------------------------------------------------------------

String X11SalInstance::GetDefaultPrinter()
{
    mbPrinterInit = true;
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    return rManager.getDefaultPrinter();
}

// =======================================================================

PspSalInfoPrinter::PspSalInfoPrinter()
{
    m_pGraphics = NULL;
    m_bPapersInit = false;
}

// -----------------------------------------------------------------------

PspSalInfoPrinter::~PspSalInfoPrinter()
{
    if( m_pGraphics )
    {
        delete m_pGraphics;
        m_pGraphics = NULL;
    }
}

// -----------------------------------------------------------------------

void PspSalInfoPrinter::InitPaperFormats( const ImplJobSetup* pSetupData )
{
    m_aPaperFormats.clear();
    m_bPapersInit = true;

    if( m_aJobData.m_pParser )
    {
        const PPDKey* pKey = m_aJobData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
        if( pKey )
        {
            int nValues = pKey->countValues();
            for( int i = 0; i < nValues; i++ )
            {
                const PPDValue* pValue = pKey->getValue( i );
                vcl::PaperInfo aInfo;
                aInfo.m_aPaperName = pValue->m_aOptionTranslation;
                if( ! aInfo.m_aPaperName.Len() )
                    aInfo.m_aPaperName = pValue->m_aOption;
                int nWidth = 0, nHeight = 0;
                m_aJobData.m_pParser->getPaperDimension( pValue->m_aOption, nWidth, nHeight );
                aInfo.m_nPaperWidth = (unsigned long)((PtTo10Mu( nWidth )+50)/100);
                aInfo.m_nPaperHeight = (unsigned long)((PtTo10Mu( nHeight )+50)/100);
                m_aPaperFormats.push_back( aInfo );
            }
        }
    }
}

// -----------------------------------------------------------------------

int PspSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* pSetupData )
{
    return 900;
}

// -----------------------------------------------------------------------

SalGraphics* PspSalInfoPrinter::GetGraphics()
{
    // return a valid pointer only once
    // the reasoning behind this is that we could have different
    // SalGraphics that can run in multiple threads
    // (future plans)
    SalGraphics* pRet = NULL;
    if( ! m_pGraphics )
    {
        m_pGraphics = new X11SalGraphics;
        m_pGraphics->SetLayout( 0 );
        m_pGraphics->m_pJobData = &m_aJobData;
        m_pGraphics->m_pPrinterGfx  = &m_aPrinterGfx;
        m_pGraphics->bPrinter_      = TRUE;
        pRet = m_pGraphics;
    }
    return pRet;
}

// -----------------------------------------------------------------------

void PspSalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
    if( pGraphics == m_pGraphics )
    {
        delete pGraphics;
        m_pGraphics = NULL;
    }
    return;
}

// -----------------------------------------------------------------------

BOOL PspSalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pJobSetup )
{
    if( ! pFrame || ! pJobSetup )
        return FALSE;

    getPaLib();

    if( ! pSetupFunction )
        return FALSE;

    PrinterInfoManager& rManager = PrinterInfoManager::get();

    PrinterInfo aInfo( rManager.getPrinterInfo( pJobSetup->maPrinterName ) );
    if ( pJobSetup->mpDriverData )
    {
        SetData( ~0, pJobSetup );
        JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aInfo );
    }

    if( pSetupFunction( aInfo ) )
    {
        rtl_freeMemory( pJobSetup->mpDriverData );
        pJobSetup->mpDriverData = NULL;

        int nBytes;
        void* pBuffer = NULL;
        aInfo.getStreamBuffer( pBuffer, nBytes );
        pJobSetup->mnDriverDataLen  = nBytes;
        pJobSetup->mpDriverData     = (BYTE*)pBuffer;

        // copy everything to job setup
        copyJobDataToJobSetup( pJobSetup, aInfo );
        return TRUE;
    }
    return FALSE;
}

// -----------------------------------------------------------------------

// This function gets the driver data and puts it into pJobSetup
// If pJobSetup->mpDriverData is NOT NULL, then the independend
// data should be merged into the driver data
// If pJobSetup->mpDriverData IS NULL, then the driver defaults
// should be merged into the independent data
BOOL PspSalInfoPrinter::SetPrinterData( ImplJobSetup* pJobSetup )
{
    if( pJobSetup->mpDriverData )
        return SetData( ~0, pJobSetup );

    copyJobDataToJobSetup( pJobSetup, m_aJobData );
    return TRUE;
}

// -----------------------------------------------------------------------

// This function merges the independ driver data
// and sets the new independ data in pJobSetup
// Only the data must be changed, where the bit
// in nGetDataFlags is set
BOOL PspSalInfoPrinter::SetData(
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
            int nWidth, nHeight;
            if( pJobSetup->meOrientation == ORIENTATION_PORTRAIT )
            {
                nWidth  = pJobSetup->mnPaperWidth;
                nHeight = pJobSetup->mnPaperHeight;
            }
            else
            {
                nWidth  = pJobSetup->mnPaperHeight;
                nHeight = pJobSetup->mnPaperWidth;
            }
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
                    pValue = pKey->getValue( pJobSetup->mnPaperBin );

                // may fail due to constraints;
                // real paper bin is copied back to jobsetup in that case
                aData.m_aContext.setValue( pKey, pValue );
            }
            // if printer has no InputSlot key simply ignore this setting
            // (e.g. SGENPRT has no InputSlot)
        }

        // merge orientation if necessary
        if( nSetDataFlags & SAL_JOBSET_ORIENTATION )
            aData.m_eOrientation = pJobSetup->meOrientation == ORIENTATION_LANDSCAPE ? orientation::Landscape : orientation::Portrait;

        m_aJobData = aData;
        copyJobDataToJobSetup( pJobSetup, aData );
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void PspSalInfoPrinter::GetPageInfo(
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
        int left = 0, top = 0, right = 0, bottom = 0;
        int nDPI = aData.m_aContext.getRenderResolution();


        if( aData.m_eOrientation == psp::orientation::Portrait )
        {
            aData.m_aContext.getPageSize( aPaper, width, height );
            aData.m_pParser->getMargins( aPaper, left, right, top, bottom );
        }
        else
        {
            aData.m_aContext.getPageSize( aPaper, height, width );
            aData.m_pParser->getMargins( aPaper, top, bottom, right, left );
        }

        rPageWidth  = width * nDPI / 72;
        rPageHeight = height * nDPI / 72;
        rPageOffX   = left * nDPI / 72;
        rPageOffY   = top * nDPI / 72;
        rOutWidth   = ( width  - left - right ) * nDPI / 72;
        rOutHeight  = ( height - top  - bottom ) * nDPI / 72;
    }
}

// -----------------------------------------------------------------------

ULONG PspSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pJobSetup )
{
    if( ! pJobSetup )
        return 0;

    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) ): NULL;
    return pKey ? pKey->countValues() : 0;
}

// -----------------------------------------------------------------------

String PspSalInfoPrinter::GetPaperBinName( const ImplJobSetup* pJobSetup, ULONG nPaperBin )
{
    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    String aRet;
    if( aData.m_pParser )
    {
        const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) ): NULL;
        if( nPaperBin == 0xffff || ! pKey )
            aRet = aData.m_pParser->getDefaultInputSlot();
        else
        {
            const PPDValue* pValue = pKey->getValue( nPaperBin );
            if( pValue )
                aRet = pValue->m_aOptionTranslation.Len() ? pValue->m_aOptionTranslation : pValue->m_aOption;
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

ULONG PspSalInfoPrinter::GetCapabilities( const ImplJobSetup* pJobSetup, USHORT nType )
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
                if( aFeatures.GetToken( i ).CompareToAscii( "fax", 3 ) == COMPARE_EQUAL )
                    return 1;
            }
            return 0;
        }
        case PRINTER_CAPABILITIES_PDF:
        {
            PrinterInfoManager& rManager = PrinterInfoManager::get();
            PrinterInfo aInfo( rManager.getPrinterInfo( pJobSetup->maPrinterName ) );
            String aFeatures( aInfo.m_aFeatures );
            int nTokenCount = aFeatures.GetTokenCount( ',' );
            for( int i = 0; i < nTokenCount; i++ )
            {
                if( aFeatures.GetToken( i ).CompareToAscii( "pdf=", 4 ) == COMPARE_EQUAL )
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

PspSalPrinter::PspSalPrinter()
{
}

// -----------------------------------------------------------------------

PspSalPrinter::~PspSalPrinter()
{
}

// -----------------------------------------------------------------------

static String getTmpName()
{
    rtl::OUString aTmp, aSys;
    osl_createTempFile( NULL, NULL, &aTmp.pData );
    osl_getSystemPathFromFileURL( aTmp.pData, &aSys.pData );

    return aSys;
}

BOOL PspSalPrinter::StartJob(
    const XubString* pFileName,
    const XubString& rJobName,
    const XubString& rAppName,
    ULONG nCopies, BOOL bCollate,
    ImplJobSetup* pJobSetup )
{
    vcl_sal::PrinterUpdate::jobStarted();

    m_bFax      = false;
    m_bPdf      = false;
    m_aFileName = pFileName ? *pFileName : String();
    m_aTmpFile  = String();
    m_nCopies       = nCopies;

    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, m_aJobData );
    if( m_nCopies > 1 )
        // in case user did not do anything (m_nCopies=1)
        // take the default from jobsetup
        m_aJobData.m_nCopies = m_nCopies;

    // check wether this printer is configured as fax
    int nMode = 0;
    const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( m_aJobData.m_aPrinterName ) );
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( ! aToken.compareToAscii( "fax", 3 ) )
        {
            m_bFax = true;
            m_aTmpFile = getTmpName();
            nMode = S_IRUSR | S_IWUSR;

            ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >::const_iterator it;
            it = pJobSetup->maValueMap.find( ::rtl::OUString::createFromAscii( "FAX#" ) );
            if( it != pJobSetup->maValueMap.end() )
                m_aFaxNr = it->second;

            sal_Int32 nPos = 0;
            m_bSwallowFaxNo = ! aToken.getToken( 1, '=', nPos ).compareToAscii( "swallow", 7 ) ? true : false;

            break;
        }
        if( ! aToken.compareToAscii( "pdf=", 4 ) )
        {
            m_bPdf = true;
            m_aTmpFile = getTmpName();
            nMode = S_IRUSR | S_IWUSR;

            if( ! m_aFileName.Len() )
            {
                m_aFileName = getPdfDir( rInfo );
                m_aFileName.Append( '/' );
                m_aFileName.Append( rJobName );
                m_aFileName.AppendAscii( ".pdf" );
            }
            break;
        }
    }
    m_aPrinterGfx.Init( m_aJobData );
    return m_aPrintJob.StartJob( m_aTmpFile.Len() ? m_aTmpFile : m_aFileName, nMode, rJobName, rAppName, m_aJobData, &m_aPrinterGfx ) ? TRUE : FALSE;
}

// -----------------------------------------------------------------------

BOOL PspSalPrinter::EndJob()
{
    BOOL bSuccess = m_aPrintJob.EndJob();

    if( bSuccess )
    {
        // check for fax
        if( m_bFax )
        {

            const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( m_aJobData.m_aPrinterName ) );
            // sendAFax removes the file after use
            bSuccess = sendAFax( m_aFaxNr, m_aTmpFile, rInfo.m_aCommand );
        }
        else if( m_bPdf )
        {
            const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( m_aJobData.m_aPrinterName ) );
            bSuccess = createPdf( m_aFileName, m_aTmpFile, rInfo.m_aCommand );
        }
    }
    vcl_sal::PrinterUpdate::jobEnded();
    return bSuccess;
}

// -----------------------------------------------------------------------

BOOL PspSalPrinter::AbortJob()
{
    BOOL bAbort = m_aPrintJob.AbortJob() ? TRUE : FALSE;
    vcl_sal::PrinterUpdate::jobEnded();
    return bAbort;
}

// -----------------------------------------------------------------------

SalGraphics* PspSalPrinter::StartPage( ImplJobSetup* pJobSetup, BOOL bNewJobData )
{
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, m_aJobData );
    m_pGraphics = new X11SalGraphics();
    m_pGraphics->SetLayout( 0 );
    m_pGraphics->m_pJobData     = &m_aJobData;
    m_pGraphics->m_pPrinterGfx      = &m_aPrinterGfx;
    m_pGraphics->bPrinter_          = true;
    m_pGraphics->m_pPhoneNr     = m_bFax ? &m_aFaxNr : NULL;
    m_pGraphics->m_bSwallowFaxNo    = m_bSwallowFaxNo;
    if( m_nCopies > 1 )
        // in case user did not do anything (m_nCopies=1)
        // take the default from jobsetup
        m_aJobData.m_nCopies = m_nCopies;

    m_aPrintJob.StartPage( m_aJobData, bNewJobData ? sal_True : sal_False );
    m_aPrinterGfx.Init( m_aPrintJob );

    return m_pGraphics;
}

// -----------------------------------------------------------------------

BOOL PspSalPrinter::EndPage()
{
    sal_Bool bResult = m_aPrintJob.EndPage();
    m_aPrinterGfx.Clear();
    return bResult ? TRUE : FALSE;
}

// -----------------------------------------------------------------------

ULONG PspSalPrinter::GetErrorCode()
{
    return 0;
}

/*
 *  vcl::PrinterUpdate
 */

Timer* vcl_sal::PrinterUpdate::pPrinterUpdateTimer = NULL;
int vcl_sal::PrinterUpdate::nActiveJobs = 0;

struct PrnFrameCall
{
    X11SalFrame* pFrame;
    SalFrameDelData aDelData;

    PrnFrameCall() : pFrame( NULL ) {}
};

void vcl_sal::PrinterUpdate::doUpdate()
{
    ::psp::PrinterInfoManager& rManager( ::psp::PrinterInfoManager::get() );
    if( rManager.checkPrintersChanged() )
    {
        std::list< PrnFrameCall > aList;
        X11SalFrame* pFrame = GetSalData()->pFirstFrame_;
        while( pFrame )
        {
            aList.push_back( PrnFrameCall() );
            aList.back().pFrame = pFrame;
            pFrame->RegisterDeleteData( &aList.back().aDelData );
            pFrame = pFrame->GetNextFrame();
        }

        while( aList.begin() != aList.end() )
        {
            if( ! aList.front().aDelData.IsDeleted() )
            {
                aList.front().pFrame->CallCallback( SALEVENT_PRINTERCHANGED, NULL );
                if( !aList.front().aDelData.IsDeleted() )
                    aList.front().pFrame->UnregisterDeleteData( &aList.front().aDelData );
            }
            aList.pop_front();
        }
    }
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK( vcl_sal::PrinterUpdate, UpdateTimerHdl, void*, pDummy )
{
    if( nActiveJobs < 1 )
    {
        doUpdate();
        delete pPrinterUpdateTimer;
        pPrinterUpdateTimer = NULL;
    }
    else
        pPrinterUpdateTimer->Start();

    return 0;
}

// -----------------------------------------------------------------------

void vcl_sal::PrinterUpdate::update()
{
    if( nActiveJobs < 1 )
        doUpdate();
    else if( ! pPrinterUpdateTimer )
    {
        pPrinterUpdateTimer = new Timer();
        pPrinterUpdateTimer->SetTimeout( 500 );
        pPrinterUpdateTimer->SetTimeoutHdl( STATIC_LINK( NULL, vcl_sal::PrinterUpdate, UpdateTimerHdl ) );
        pPrinterUpdateTimer->Start();
    }
}

// -----------------------------------------------------------------------

void vcl_sal::PrinterUpdate::jobEnded()
{
    nActiveJobs--;
    if( nActiveJobs < 1 )
    {
        if( pPrinterUpdateTimer )
        {
            pPrinterUpdateTimer->Stop();
            delete pPrinterUpdateTimer;
            pPrinterUpdateTimer = NULL;
            doUpdate();
        }
    }
}
