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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "vcl/svapp.hxx"
#include "vcl/timer.hxx"
#include "vcl/printerinfomanager.hxx"

#include "jobset.h"
#include "print.h"
#include "salptype.hxx"

#include "svpprn.hxx"
#include "svppspgraphics.hxx"
#include "svpinst.hxx"

using namespace psp;
using namespace rtl;

/*
 *  static helpers
 */

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

inline int PtTo10Mu( int nPoints ) { return (int)((((double)nPoints)*35.27777778)+0.5); }

inline int TenMuToPt( int nUnits ) { return (int)((((double)nUnits)/35.27777778)+0.5); }

static void copyJobDataToJobSetup( ImplJobSetup* pJobSetup, JobData& rData )
{
    pJobSetup->meOrientation    = (Orientation)(rData.m_eOrientation == orientation::Landscape ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT);

    // copy page size
    String aPaper;
    int width, height;

    rData.m_aContext.getPageSize( aPaper, width, height );
    pJobSetup->mePaperFormat    = PaperInfo::fromPSName(OUStringToOString( aPaper, RTL_TEXTENCODING_ISO_8859_1 ));
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

    pJobSetup->mnPaperBin = 0xffff;
    if( rData.m_pParser )
        pKey                    = rData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
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

    // copy duplex
    pKey = NULL;
    pValue = NULL;

    pJobSetup->meDuplexMode = DUPLEX_UNKNOWN;
    if( rData.m_pParser )
        pKey = rData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) );
    if( pKey )
        pValue = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        if( pValue->m_aOption.EqualsIgnoreCaseAscii( "None" ) ||
            pValue->m_aOption.EqualsIgnoreCaseAscii( "Simplex", 0, 7 )
           )
        {
            pJobSetup->meDuplexMode = DUPLEX_OFF;
        }
        else if( pValue->m_aOption.EqualsIgnoreCaseAscii( "DuplexNoTumble" ) )
        {
            pJobSetup->meDuplexMode = DUPLEX_LONGEDGE;
        }
        else if( pValue->m_aOption.EqualsIgnoreCaseAscii( "DuplexTumble" ) )
        {
            pJobSetup->meDuplexMode = DUPLEX_SHORTEDGE;
        }
    }

    // copy the whole context
    if( pJobSetup->mpDriverData )
        rtl_freeMemory( pJobSetup->mpDriverData );

    int nBytes;
    void* pBuffer = NULL;
    if( rData.getStreamBuffer( pBuffer, nBytes ) )
    {
        pJobSetup->mnDriverDataLen = nBytes;
        pJobSetup->mpDriverData = (sal_uInt8*)pBuffer;
    }
    else
    {
        pJobSetup->mnDriverDataLen = 0;
        pJobSetup->mpDriverData = NULL;
    }
}

static bool passFileToCommandLine( const String& rFilename, const String& rCommandLine, bool bRemoveFile = true )
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
    fprintf( stderr, "Tmp file %s has modes: 0%03lo\n", aFilename.GetBuffer(), (long)aStat.st_mode );
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
    if( bRemoveFile )
        unlink( aFilename.GetBuffer() );

    return bSuccess;
}

static bool sendAFax( const String& rFaxNumber, const String& rFileName, const String& rCommand )
{
    std::list< OUString > aFaxNumbers;

    if( ! rFaxNumber.Len() )
        return false;

    sal_Int32 nIndex = 0;
    OUString aFaxes( rFaxNumber );
    OUString aBeginToken( RTL_CONSTASCII_USTRINGPARAM("<Fax#>") );
    OUString aEndToken( RTL_CONSTASCII_USTRINGPARAM("</Fax#>") );
    while( nIndex != -1 )
    {
        nIndex = aFaxes.indexOf( aBeginToken, nIndex );
        if( nIndex != -1 )
        {
            sal_Int32 nBegin = nIndex + aBeginToken.getLength();
            nIndex = aFaxes.indexOf( aEndToken, nIndex );
            if( nIndex != -1 )
            {
                aFaxNumbers.push_back( aFaxes.copy( nBegin, nIndex-nBegin ) );
                nIndex += aEndToken.getLength();
            }
        }
    }

    bool bSuccess = true;
    if( aFaxNumbers.begin() != aFaxNumbers.end() )
    {
        while( aFaxNumbers.begin() != aFaxNumbers.end() && bSuccess )
        {
            String aCmdLine( rCommand );
            String aFaxNumber( aFaxNumbers.front() );
            aFaxNumbers.pop_front();
            while( aCmdLine.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "(PHONE)" ) ), aFaxNumber ) != STRING_NOTFOUND )
                ;
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "sending fax to \"%s\"\n", OUStringToOString( aFaxNumber, osl_getThreadTextEncoding() ).getStr() );
#endif
            bSuccess = passFileToCommandLine( rFileName, aCmdLine, false );
        }
    }
    else
        bSuccess = false;

    // clean up temp file
    unlink( ByteString( rFileName, osl_getThreadTextEncoding() ).GetBuffer() );

    return bSuccess;
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

SalInfoPrinter* SvpSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                   ImplJobSetup*            pJobSetup )
{
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

        // set/clear backwards compatibility flag
        bool bStrictSO52Compatibility = false;
        std::hash_map<rtl::OUString, rtl::OUString, rtl::OUStringHash >::const_iterator compat_it =
            pJobSetup->maValueMap.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StrictSO52Compatibility" ) ) );
        if( compat_it != pJobSetup->maValueMap.end() )
        {
            if( compat_it->second.equalsIgnoreAsciiCaseAscii( "true" ) )
                bStrictSO52Compatibility = true;
        }
        pPrinter->m_aPrinterGfx.setStrictSO52Compatibility( bStrictSO52Compatibility );
    }


    return pPrinter;
}

// -----------------------------------------------------------------------

void SvpSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

SalPrinter* SvpSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    // create and initialize SalPrinter
    PspSalPrinter* pPrinter = new PspSalPrinter( pInfoPrinter );
    pPrinter->m_aJobData = static_cast<PspSalInfoPrinter*>(pInfoPrinter)->m_aJobData;

    return pPrinter;
}

// -----------------------------------------------------------------------

void SvpSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

void SvpSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    static const char* pNoSyncDetection = getenv( "SAL_DISABLE_SYNCHRONOUS_PRINTER_DETECTION" );
    if( ! pNoSyncDetection || ! *pNoSyncDetection )
    {
        // #i62663# synchronize possible asynchronouse printer detection now
        rManager.checkPrintersChanged( true );
    }
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

void SvpSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

// -----------------------------------------------------------------------

void SvpSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* )
{
}

// -----------------------------------------------------------------------

String SvpSalInstance::GetDefaultPrinter()
{
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

void PspSalInfoPrinter::InitPaperFormats( const ImplJobSetup* )
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
                int nWidth = 0, nHeight = 0;
                m_aJobData.m_pParser->getPaperDimension( pValue->m_aOption, nWidth, nHeight );
                PaperInfo aInfo(PtTo10Mu( nWidth ), PtTo10Mu( nHeight ));
                m_aPaperFormats.push_back( aInfo );
            }
        }
    }
}

// -----------------------------------------------------------------------

int PspSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* )
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
        m_pGraphics = new PspGraphics( &m_aJobData, &m_aPrinterGfx, NULL, false, this );
        m_pGraphics->SetLayout( 0 );
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

sal_Bool PspSalInfoPrinter::Setup( SalFrame*, ImplJobSetup* )
{
    return sal_False;
}

// -----------------------------------------------------------------------

// This function gets the driver data and puts it into pJobSetup
// If pJobSetup->mpDriverData is NOT NULL, then the independend
// data should be merged into the driver data
// If pJobSetup->mpDriverData IS NULL, then the driver defaults
// should be merged into the independent data
sal_Bool PspSalInfoPrinter::SetPrinterData( ImplJobSetup* pJobSetup )
{
    if( pJobSetup->mpDriverData )
        return SetData( ~0, pJobSetup );

    copyJobDataToJobSetup( pJobSetup, m_aJobData );

    // set/clear backwards compatibility flag
    bool bStrictSO52Compatibility = false;
    std::hash_map<rtl::OUString, rtl::OUString, rtl::OUStringHash >::const_iterator compat_it =
        pJobSetup->maValueMap.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StrictSO52Compatibility" ) ) );
    if( compat_it != pJobSetup->maValueMap.end() )
    {
        if( compat_it->second.equalsIgnoreAsciiCaseAscii( "true" ) )
            bStrictSO52Compatibility = true;
    }
    m_aPrinterGfx.setStrictSO52Compatibility( bStrictSO52Compatibility );

    return sal_True;
}

// -----------------------------------------------------------------------

// This function merges the independ driver data
// and sets the new independ data in pJobSetup
// Only the data must be changed, where the bit
// in nGetDataFlags is set
sal_Bool PspSalInfoPrinter::SetData(
    sal_uLong nSetDataFlags,
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
                aPaper = rtl::OStringToOUString(PaperInfo::toPSName(pJobSetup->mePaperFormat), RTL_TEXTENCODING_ISO_8859_1);

            pKey = aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
            pValue = pKey ? pKey->getValue( aPaper ) : NULL;
            if( ! ( pKey && pValue && aData.m_aContext.setValue( pKey, pValue, false ) == pValue ) )
                return sal_False;
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

        // merge duplex if necessary
        if( nSetDataFlags & SAL_JOBSET_DUPLEXMODE )
        {
            pKey = aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) );
            if( pKey )
            {
                pValue = NULL;
                switch( pJobSetup->meDuplexMode )
                {
                case DUPLEX_OFF:
                    pValue = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
                    if( pValue == NULL )
                        pValue = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "SimplexNoTumble" ) ) );
                    break;
                case DUPLEX_SHORTEDGE:
                    pValue = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "DuplexTumble" ) ) );
                    break;
                case DUPLEX_LONGEDGE:
                    pValue = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "DuplexNoTumble" ) ) );
                    break;
                case DUPLEX_UNKNOWN:
                default:
                    pValue = 0;
                    break;
                }
                if( ! pValue )
                    pValue = pKey->getDefaultValue();
                aData.m_aContext.setValue( pKey, pValue );
            }
        }

        m_aJobData = aData;
        copyJobDataToJobSetup( pJobSetup, aData );
        return sal_True;
    }

    return sal_False;
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

sal_uLong PspSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pJobSetup )
{
    if( ! pJobSetup )
        return 0;

    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) ): NULL;
    return pKey ? pKey->countValues() : 0;
}

// -----------------------------------------------------------------------

String PspSalInfoPrinter::GetPaperBinName( const ImplJobSetup* pJobSetup, sal_uLong nPaperBin )
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
                aRet = aData.m_pParser->translateOption( pKey->getKey(), pValue->m_aOption );
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

sal_uLong PspSalInfoPrinter::GetCapabilities( const ImplJobSetup* pJobSetup, sal_uInt16 nType )
{
    switch( nType )
    {
        case PRINTER_CAPABILITIES_SUPPORTDIALOG:
            return 1;
        case PRINTER_CAPABILITIES_COPIES:
            return 0xffff;
        case PRINTER_CAPABILITIES_COLLATECOPIES:
        {
            // see if the PPD contains a value to set Collate to True
            JobData aData;
            JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

            const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Collate" ) ) ) : NULL;
            const PPDValue* pVal = pKey ? pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "True" ) ) ) : NULL;

            // PPDs don't mention the number of possible collated copies.
            // so let's guess as many as we want ?
            return pVal ? 0xffff : 0;
        }
        case PRINTER_CAPABILITIES_SETORIENTATION:
            return 1;
        case PRINTER_CAPABILITIES_SETDUPLEX:
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

PspSalPrinter::PspSalPrinter( SalInfoPrinter* pInfoPrinter )
 : m_bFax( false ),
   m_bPdf( false ),
   m_bSwallowFaxNo( false ),
   m_pGraphics( NULL ),
   m_nCopies( 1 ),
   m_bCollate( false ),
   m_pInfoPrinter( pInfoPrinter )
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

sal_Bool PspSalPrinter::StartJob(
    const XubString* pFileName,
    const XubString& rJobName,
    const XubString& rAppName,
    sal_uLong nCopies,
    bool bCollate,
    bool /*bDirect*/,
    ImplJobSetup* pJobSetup )
{
    vcl_sal::PrinterUpdate::jobStarted();

    m_bFax      = false;
    m_bPdf      = false;
    m_aFileName = pFileName ? *pFileName : String();
    m_aTmpFile  = String();
    m_nCopies   = nCopies;
    m_bCollate  = bCollate;

    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, m_aJobData );
    if( m_nCopies > 1 )
    {
        // in case user did not do anything (m_nCopies=1)
        // take the default from jobsetup
        m_aJobData.m_nCopies = m_nCopies;
        m_aJobData.setCollate( bCollate );
    }

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

    // set/clear backwards compatibility flag
    bool bStrictSO52Compatibility = false;
    std::hash_map<rtl::OUString, rtl::OUString, rtl::OUStringHash >::const_iterator compat_it =
        pJobSetup->maValueMap.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StrictSO52Compatibility" ) ) );
    if( compat_it != pJobSetup->maValueMap.end() )
    {
        if( compat_it->second.equalsIgnoreAsciiCaseAscii( "true" ) )
            bStrictSO52Compatibility = true;
    }
    m_aPrinterGfx.setStrictSO52Compatibility( bStrictSO52Compatibility );

    return m_aPrintJob.StartJob( m_aTmpFile.Len() ? m_aTmpFile : m_aFileName, nMode, rJobName, rAppName, m_aJobData, &m_aPrinterGfx, false ) ? sal_True : sal_False;
}

// -----------------------------------------------------------------------

sal_Bool PspSalPrinter::EndJob()
{
    sal_Bool bSuccess = m_aPrintJob.EndJob();

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

sal_Bool PspSalPrinter::AbortJob()
{
    sal_Bool bAbort = m_aPrintJob.AbortJob() ? sal_True : sal_False;
    vcl_sal::PrinterUpdate::jobEnded();
    return bAbort;
}

// -----------------------------------------------------------------------

SalGraphics* PspSalPrinter::StartPage( ImplJobSetup* pJobSetup, sal_Bool )
{
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, m_aJobData );
    m_pGraphics = new PspGraphics( &m_aJobData, &m_aPrinterGfx, m_bFax ? &m_aFaxNr : NULL, m_bSwallowFaxNo, m_pInfoPrinter  );
    m_pGraphics->SetLayout( 0 );
    if( m_nCopies > 1 )
    {
        // in case user did not do anything (m_nCopies=1)
        // take the default from jobsetup
        m_aJobData.m_nCopies = m_nCopies;
        m_aJobData.setCollate( m_nCopies > 1 && m_bCollate );
    }

    m_aPrintJob.StartPage( m_aJobData );
    m_aPrinterGfx.Init( m_aPrintJob );

    return m_pGraphics;
}

// -----------------------------------------------------------------------

sal_Bool PspSalPrinter::EndPage()
{
    sal_Bool bResult = m_aPrintJob.EndPage();
    m_aPrinterGfx.Clear();
    return bResult ? sal_True : sal_False;
}

// -----------------------------------------------------------------------

sal_uLong PspSalPrinter::GetErrorCode()
{
    return 0;
}

/*
 *  vcl::PrinterUpdate
 */

Timer* vcl_sal::PrinterUpdate::pPrinterUpdateTimer = NULL;
int vcl_sal::PrinterUpdate::nActiveJobs = 0;

void vcl_sal::PrinterUpdate::doUpdate()
{
    ::psp::PrinterInfoManager& rManager( ::psp::PrinterInfoManager::get() );
    if( rManager.checkPrintersChanged( false ) && SvpSalInstance::s_pDefaultInstance )
    {
        const std::list< SalFrame* >& rList = SvpSalInstance::s_pDefaultInstance->getFrames();
        for( std::list< SalFrame* >::const_iterator it = rList.begin();
             it != rList.end(); ++it )
             SvpSalInstance::s_pDefaultInstance->PostEvent( *it, NULL, SALEVENT_PRINTERCHANGED );
    }
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( vcl_sal::PrinterUpdate, UpdateTimerHdl, void*, )
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
    if( Application::GetSettings().GetMiscSettings().GetDisablePrinting() )
        return;

    static bool bOnce = false;
    if( ! bOnce )
    {
        bOnce = true;
        // start background printer detection
        psp::PrinterInfoManager::get();
        return;
    }

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
