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

/**
  this file implements the sal printer interface (SalPrinter, SalInfoPrinter
  and some printer relevant methods of SalInstance and SalGraphicsData)

  as underlying library the printer features of psprint are used.

  The query methods of a SalInfoPrinter are implemented by querying psprint

  The job methods of a SalPrinter are implemented by calling psprint
  printer job functions.
 */

// For spawning PDF and FAX generation
#if defined( UNX )
#  include <unistd.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#endif

#include <comphelper/fileurl.hxx>
#include "rtl/ustring.hxx"

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/idle.hxx>
#include <vcl/svapp.hxx>
#include <vcl/print.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/printerinfomanager.hxx>
#include <vcl/settings.hxx>
#include "svids.hrc"
#include "saldatabasic.hxx"
#include "unx/genprn.h"
#include "unx/geninst.h"
#include "unx/genpspgraphics.h"

#include "jobset.h"
#include "print.h"
#include "prtsetup.hxx"
#include "salptype.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace psp;
using namespace com::sun::star;

/*
 *  static helpers
 */
static OUString getPdfDir( const PrinterInfo& rInfo )
{
    OUString aDir;
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( aToken.startsWith( "pdf=" ) )
        {
            sal_Int32 nPos = 0;
            aDir = aToken.getToken( 1, '=', nPos );
            if( aDir.isEmpty() && getenv( "HOME" ) )
                aDir = OUString( getenv( "HOME" ), strlen( getenv( "HOME" ) ), osl_getThreadTextEncoding() );
            break;
        }
    }
    return aDir;
}

namespace
{
    class QueryString : public ModalDialog
    {
    private:
        VclPtr<OKButton>    m_pOKButton;
        VclPtr<FixedText>   m_pFixedText;
        VclPtr<Edit>        m_pEdit;
        OUString&           m_rReturnValue;

        DECL_LINK_TYPED( ClickBtnHdl, Button*, void );

    public:
        // parent window, Query text, initial value
        QueryString(vcl::Window*, OUString &, OUString &);
        virtual ~QueryString() { disposeOnce(); }
        virtual void dispose() override
        {
            m_pOKButton.clear();
            m_pFixedText.clear();
            m_pEdit.clear();
            ModalDialog::dispose();
        }
    };

    /*
     *  QueryString
     */
    QueryString::QueryString(vcl::Window* pParent, OUString& rQuery, OUString& rRet)
        : ModalDialog(pParent, "QueryDialog",
            "vcl/ui/querydialog.ui" )
        , m_rReturnValue( rRet )
    {
        get(m_pOKButton, "ok");
        get(m_pFixedText, "label");
        get(m_pEdit, "entry");

        m_pOKButton->SetClickHdl(LINK(this, QueryString, ClickBtnHdl));
        m_pFixedText->SetText(rQuery);
        m_pEdit->SetText(m_rReturnValue);
        SetText(rQuery);
    }

    IMPL_LINK_TYPED( QueryString, ClickBtnHdl, Button*, pButton, void )
    {
        if (pButton == m_pOKButton)
        {
            m_rReturnValue = m_pEdit->GetText();
            EndDialog( 1 );
        }
        else
            EndDialog();
    }

    int QueryFaxNumber(OUString& rNumber)
    {
        OUString aTmpString(VclResId(SV_PRINT_QUERYFAXNUMBER_TXT));
        ScopedVclPtrInstance< QueryString > aQuery( nullptr, aTmpString, rNumber );
        return aQuery->Execute();
    }
}

inline int PtTo10Mu( int nPoints ) { return (int)((((double)nPoints)*35.27777778)+0.5); }

inline int TenMuToPt( int nUnits ) { return (int)((((double)nUnits)/35.27777778)+0.5); }

static void copyJobDataToJobSetup( ImplJobSetup* pJobSetup, JobData& rData )
{
    pJobSetup->meOrientation    = (Orientation)(rData.m_eOrientation == orientation::Landscape ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT);

    // copy page size
    OUString aPaper;
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
    const PPDKey* pKey = nullptr;
    const PPDValue* pValue = nullptr;

    pJobSetup->mnPaperBin = 0;
    if( rData.m_pParser )
        pKey                    = rData.m_pParser->getKey( OUString("InputSlot") );
    if( pKey )
        pValue                  = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        for( pJobSetup->mnPaperBin = 0;
             pValue != pKey->getValue( pJobSetup->mnPaperBin ) &&
                 pJobSetup->mnPaperBin < pKey->countValues();
             pJobSetup->mnPaperBin++ )
            ;
        if( pJobSetup->mnPaperBin >= pKey->countValues() )
            pJobSetup->mnPaperBin = 0;
    }

    // copy duplex
    pKey = nullptr;
    pValue = nullptr;

    pJobSetup->meDuplexMode = DUPLEX_UNKNOWN;
    if( rData.m_pParser )
        pKey = rData.m_pParser->getKey( OUString("Duplex") );
    if( pKey )
        pValue = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        if( pValue->m_aOption.equalsIgnoreAsciiCase( "None" ) ||
            pValue->m_aOption.startsWithIgnoreAsciiCase( "Simplex" )
           )
        {
            pJobSetup->meDuplexMode = DUPLEX_OFF;
        }
        else if( pValue->m_aOption.equalsIgnoreAsciiCase( "DuplexNoTumble" ) )
        {
            pJobSetup->meDuplexMode = DUPLEX_LONGEDGE;
        }
        else if( pValue->m_aOption.equalsIgnoreAsciiCase( "DuplexTumble" ) )
        {
            pJobSetup->meDuplexMode = DUPLEX_SHORTEDGE;
        }
    }

    // copy the whole context
    if( pJobSetup->mpDriverData )
        rtl_freeMemory( pJobSetup->mpDriverData );

    sal_uInt32 nBytes;
    void* pBuffer = nullptr;
    if( rData.getStreamBuffer( pBuffer, nBytes ) )
    {
        pJobSetup->mnDriverDataLen = nBytes;
        pJobSetup->mpDriverData = static_cast<sal_uInt8*>(pBuffer);
    }
    else
    {
        pJobSetup->mnDriverDataLen = 0;
        pJobSetup->mpDriverData = nullptr;
    }
}

// Needs a cleaner abstraction ...
#if defined( UNX )
static bool passFileToCommandLine( const OUString& rFilename, const OUString& rCommandLine )
{
    bool bSuccess = false;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OString aCmdLine(OUStringToOString(rCommandLine, aEncoding));
    OString aFilename(OUStringToOString(rFilename, aEncoding));

    bool bPipe = aCmdLine.indexOf( "(TMP)" ) == -1;

    // setup command line for exec
    if( ! bPipe )
        aCmdLine = aCmdLine.replaceAll(OString("(TMP)"), aFilename);

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "%s commandline: \"%s\"\n",
             bPipe ? "piping to" : "executing",
             aCmdLine.getStr() );
    struct stat aStat;
    if( stat( aFilename.getStr(), &aStat ) )
        fprintf( stderr, "stat( %s ) failed\n", aFilename.getStr() );
    fprintf( stderr, "Tmp file %s has modes: 0%03lo\n", aFilename.getStr(), (long)aStat.st_mode );
#endif
    const char* argv[4];
    if( ! ( argv[ 0 ] = getenv( "SHELL" ) ) )
        argv[ 0 ] = "/bin/sh";
    argv[ 1 ] = "-c";
    argv[ 2 ] = aCmdLine.getStr();
    argv[ 3 ] = nullptr;

    bool bHavePipes = false;
    int pid, fd[2];

    if( bPipe )
        bHavePipes = pipe( fd ) == 0;
    if( ( pid = fork() ) > 0 )
    {
        if( bPipe && bHavePipes )
        {
            close( fd[0] );
            char aBuffer[ 2048 ];
            FILE* fp = fopen( aFilename.getStr(), "r" );
            while (fp && !feof(fp))
            {
                size_t nBytesRead = fread(aBuffer, 1, sizeof( aBuffer ), fp);
                if (nBytesRead )
                {
                    size_t nBytesWritten = write(fd[1], aBuffer, nBytesRead);
                    OSL_ENSURE(nBytesWritten == nBytesRead, "short write");
                    if (nBytesWritten != nBytesRead)
                        break;
                }
            }
            fclose( fp );
            close( fd[ 1 ] );
        }
        int status = 0;
        if(waitpid( pid, &status, 0 ) != -1)
        {
            if( ! status )
                bSuccess = true;
        }
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
        fprintf( stderr, "failed to execute \"%s\"\n", aCmdLine.getStr() );
        _exit( 1 );
    }
    else
        fprintf( stderr, "failed to fork\n" );

    // clean up the mess
    unlink( aFilename.getStr() );

    return bSuccess;
}
#endif

static std::vector<OUString> getFaxNumbers()
{
    std::vector<OUString> aFaxNumbers;

    OUString aNewNr;
    if (QueryFaxNumber(aNewNr))
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString sToken = aNewNr.getToken( 0, ';', nIndex );
            aFaxNumbers.push_back(sToken);
        }
        while (nIndex >= 0);
    }

    return aFaxNumbers;
}

static bool createPdf( const OUString& rToFile, const OUString& rFromFile, const OUString& rCommandLine )
{
#if defined( UNX )
    OUString aCommandLine(
        rCommandLine.replaceAll("(OUTFILE)", rToFile));

    return passFileToCommandLine( rFromFile, aCommandLine );
#else
    (void)rToFile; (void)rFromFile; (void)rCommandLine;
    return false;
#endif
}

/*
 *  SalInstance
 */

void SalGenericInstance::configurePspInfoPrinter(PspSalInfoPrinter *pPrinter,
    SalPrinterQueueInfo* pQueueInfo, ImplJobSetup* pJobSetup)
{
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
}

SalInfoPrinter* SalGenericInstance::CreateInfoPrinter( SalPrinterQueueInfo*    pQueueInfo,
                                                   ImplJobSetup*            pJobSetup )
{
    mbPrinterInit = true;
    // create and initialize SalInfoPrinter
    PspSalInfoPrinter* pPrinter = new PspSalInfoPrinter();
    configurePspInfoPrinter(pPrinter, pQueueInfo, pJobSetup);
    return pPrinter;
}

void SalGenericInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

SalPrinter* SalGenericInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    mbPrinterInit = true;
    // create and initialize SalPrinter
    PspSalPrinter* pPrinter = new PspSalPrinter( pInfoPrinter );
    pPrinter->m_aJobData = static_cast<PspSalInfoPrinter*>(pInfoPrinter)->m_aJobData;

    return pPrinter;
}

void SalGenericInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

void SalGenericInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    mbPrinterInit = true;
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
        pInfo->mpSysData        = nullptr;

        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
            if( aToken.match( "pdf=" ) )
            {
                pInfo->maLocation = getPdfDir( rInfo );
                break;
            }
        }

        pList->Add( pInfo );
    }
}

void SalGenericInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

void SalGenericInstance::GetPrinterQueueState( SalPrinterQueueInfo* )
{
    mbPrinterInit = true;
}

OUString SalGenericInstance::GetDefaultPrinter()
{
    mbPrinterInit = true;
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    return rManager.getDefaultPrinter();
}

PspSalInfoPrinter::PspSalInfoPrinter()
    : m_pGraphics( nullptr )
{
}

PspSalInfoPrinter::~PspSalInfoPrinter()
{
    if( m_pGraphics )
    {
        delete m_pGraphics;
        m_pGraphics = nullptr;
    }
}

void PspSalInfoPrinter::InitPaperFormats( const ImplJobSetup* )
{
    m_aPaperFormats.clear();
    m_bPapersInit = true;

    if( m_aJobData.m_pParser )
    {
        const PPDKey* pKey = m_aJobData.m_pParser->getKey( OUString("PageSize") );
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

int PspSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* )
{
    return 900;
}

SalGraphics* PspSalInfoPrinter::AcquireGraphics()
{
    // return a valid pointer only once
    // the reasoning behind this is that we could have different
    // SalGraphics that can run in multiple threads
    // (future plans)
    SalGraphics* pRet = nullptr;
    if( ! m_pGraphics )
    {
        m_pGraphics = GetGenericInstance()->CreatePrintGraphics();
        m_pGraphics->Init(&m_aJobData, &m_aPrinterGfx, this);
        pRet = m_pGraphics;
    }
    return pRet;
}

void PspSalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
    if( pGraphics == m_pGraphics )
    {
        delete pGraphics;
        m_pGraphics = nullptr;
    }
    return;
}

bool PspSalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pJobSetup )
{
    if( ! pFrame || ! pJobSetup )
        return false;

    PrinterInfoManager& rManager = PrinterInfoManager::get();

    PrinterInfo aInfo( rManager.getPrinterInfo( pJobSetup->maPrinterName ) );
    if ( pJobSetup->mpDriverData )
    {
        SetData( JobSetFlags::ALL, pJobSetup );
        JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aInfo );
    }
    aInfo.m_bPapersizeFromSetup = pJobSetup->mbPapersizeFromSetup;

    if (SetupPrinterDriver(aInfo))
    {
        aInfo.resolveDefaultBackend();
        rtl_freeMemory( pJobSetup->mpDriverData );
        pJobSetup->mpDriverData = nullptr;

        sal_uInt32 nBytes;
        void* pBuffer = nullptr;
        aInfo.getStreamBuffer( pBuffer, nBytes );
        pJobSetup->mnDriverDataLen  = nBytes;
        pJobSetup->mpDriverData     = static_cast<sal_uInt8*>(pBuffer);

        // copy everything to job setup
        copyJobDataToJobSetup( pJobSetup, aInfo );
        JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, m_aJobData );
        return true;
    }
    return false;
}

// This function gets the driver data and puts it into pJobSetup
// If pJobSetup->mpDriverData is NOT NULL, then the independent
// data should be merged into the driver data
// If pJobSetup->mpDriverData IS NULL, then the driver defaults
// should be merged into the independent data
bool PspSalInfoPrinter::SetPrinterData( ImplJobSetup* pJobSetup )
{
    if( pJobSetup->mpDriverData )
        return SetData( JobSetFlags::ALL, pJobSetup );

    copyJobDataToJobSetup( pJobSetup, m_aJobData );

    return true;
}

// This function merges the independ driver data
// and sets the new independ data in pJobSetup
// Only the data must be changed, where the bit
// in nGetDataFlags is set
bool PspSalInfoPrinter::SetData(
    JobSetFlags nSetDataFlags,
    ImplJobSetup* pJobSetup )
{
    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    if( aData.m_pParser )
    {
        const PPDKey* pKey;
        const PPDValue* pValue;

        // merge papersize if necessary
        if( nSetDataFlags & JobSetFlags::PAPERSIZE )
        {
            OUString aPaper;

            if( pJobSetup->mePaperFormat == PAPER_USER )
                aPaper = aData.m_pParser->matchPaper(
                    TenMuToPt( pJobSetup->mnPaperWidth ),
                    TenMuToPt( pJobSetup->mnPaperHeight ) );
            else
                aPaper = OStringToOUString(PaperInfo::toPSName(pJobSetup->mePaperFormat), RTL_TEXTENCODING_ISO_8859_1);

            pKey = aData.m_pParser->getKey( OUString("PageSize") );
            pValue = pKey ? pKey->getValueCaseInsensitive( aPaper ) : nullptr;

            // some PPD files do not specify the standard paper names (e.g. C5 instead of EnvC5)
            // try to find the correct paper anyway using the size
            if( pKey && ! pValue && pJobSetup->mePaperFormat != PAPER_USER )
            {
                PaperInfo aInfo( pJobSetup->mePaperFormat );
                aPaper = aData.m_pParser->matchPaper(
                    TenMuToPt( aInfo.getWidth() ),
                    TenMuToPt( aInfo.getHeight() ) );
                pValue = pKey->getValueCaseInsensitive( aPaper );
            }

            if( ! ( pKey && pValue && aData.m_aContext.setValue( pKey, pValue ) == pValue ) )
                return false;
        }

        // merge paperbin if necessary
        if( nSetDataFlags & JobSetFlags::PAPERBIN )
        {
            pKey = aData.m_pParser->getKey( OUString("InputSlot") );
            if( pKey )
            {
                int nPaperBin = pJobSetup->mnPaperBin;
                if( nPaperBin >= pKey->countValues() )
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
        if( nSetDataFlags & JobSetFlags::ORIENTATION )
            aData.m_eOrientation = pJobSetup->meOrientation == ORIENTATION_LANDSCAPE ? orientation::Landscape : orientation::Portrait;

        // merge duplex if necessary
        if( nSetDataFlags & JobSetFlags::DUPLEXMODE )
        {
            pKey = aData.m_pParser->getKey( OUString("Duplex") );
            if( pKey )
            {
                pValue = nullptr;
                switch( pJobSetup->meDuplexMode )
                {
                case DUPLEX_OFF:
                    pValue = pKey->getValue( OUString("None") );
                    if( pValue == nullptr )
                        pValue = pKey->getValue( OUString("SimplexNoTumble") );
                    break;
                case DUPLEX_SHORTEDGE:
                    pValue = pKey->getValue( OUString("DuplexTumble") );
                    break;
                case DUPLEX_LONGEDGE:
                    pValue = pKey->getValue( OUString("DuplexNoTumble") );
                    break;
                case DUPLEX_UNKNOWN:
                default:
                    pValue = nullptr;
                    break;
                }
                if( ! pValue )
                    pValue = pKey->getDefaultValue();
                aData.m_aContext.setValue( pKey, pValue );
            }
        }

        m_aJobData = aData;
        copyJobDataToJobSetup( pJobSetup, aData );
        return true;
    }

    return false;
}

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

        OUString aPaper;
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

sal_uLong PspSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pJobSetup )
{
    if( ! pJobSetup )
        return 0;

    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( OUString("InputSlot") ): nullptr;
    return pKey ? pKey->countValues() : 0;
}

OUString PspSalInfoPrinter::GetPaperBinName( const ImplJobSetup* pJobSetup, sal_uLong nPaperBin )
{
    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );

    OUString aRet;
    if( aData.m_pParser )
    {
        const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( OUString("InputSlot") ): nullptr;
        if( ! pKey || nPaperBin >= (sal_uLong)pKey->countValues() )
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

sal_uInt32 PspSalInfoPrinter::GetCapabilities( const ImplJobSetup* pJobSetup, PrinterCapType nType )
{
    switch( nType )
    {
        case PrinterCapType::SupportDialog:
            return 1;
        case PrinterCapType::Copies:
            return 0xffff;
        case PrinterCapType::CollateCopies:
        {
            // PPDs don't mention the number of possible collated copies.
            // so let's guess as many as we want ?
            return 0xffff;
        }
        case PrinterCapType::SetOrientation:
            return 1;
        case PrinterCapType::SetDuplex:
            return 1;
        case PrinterCapType::SetPaperBin:
            return 1;
        case PrinterCapType::SetPaperSize:
            return 1;
        case PrinterCapType::SetPaper:
            return 0;
        case PrinterCapType::Fax:
            {
                // see if the PPD contains the fax4CUPS "Dial" option and that it's not set
                // to "manually"
                JobData aData = PrinterInfoManager::get().getPrinterInfo(pJobSetup->maPrinterName);
                if( pJobSetup->mpDriverData )
                    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );
                const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey(OUString("Dial")) : nullptr;
                const PPDValue* pValue = pKey ? aData.m_aContext.getValue(pKey) : nullptr;
                if (pValue && !pValue->m_aOption.equalsIgnoreAsciiCase("Manually"))
                    return 1;
                return 0;
            }

        case PrinterCapType::PDF:
            if( PrinterInfoManager::get().checkFeatureToken( pJobSetup->maPrinterName, "pdf" ) )
                return 1;
            else
            {
                // see if the PPD contains a value to set PDF device
                JobData aData = PrinterInfoManager::get().getPrinterInfo( pJobSetup->maPrinterName );
                if( pJobSetup->mpDriverData )
                    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );
                return aData.m_nPDFDevice > 0 ? 1 : 0;
            }
        case PrinterCapType::ExternalDialog:
            return PrinterInfoManager::get().checkFeatureToken( pJobSetup->maPrinterName, "external_dialog" ) ? 1 : 0;
        case PrinterCapType::UsePullModel:
        {
            // see if the PPD contains a value to set PDF device
            JobData aData = PrinterInfoManager::get().getPrinterInfo( pJobSetup->maPrinterName );
            if( pJobSetup->mpDriverData )
                JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aData );
            return aData.m_nPDFDevice > 0 ? 1 : 0;
        }
        default: break;
    }
    return 0;
}

/*
 *  SalPrinter
 */
PspSalPrinter::PspSalPrinter( SalInfoPrinter* pInfoPrinter )
    : m_pInfoPrinter( pInfoPrinter )
    , m_pGraphics( nullptr )
    , m_nCopies( 1 )
    , m_bCollate( false )
    , m_bPdf( false )
    , m_bIsPDFWriterJob( false )
{
}

PspSalPrinter::~PspSalPrinter()
{
}

static OUString getTmpName()
{
    OUString aTmp, aSys;
    osl_createTempFile( nullptr, nullptr, &aTmp.pData );
    osl_getSystemPathFromFileURL( aTmp.pData, &aSys.pData );

    return aSys;
}

bool PspSalPrinter::StartJob(
    const OUString* pFileName,
    const OUString& rJobName,
    const OUString& rAppName,
    sal_uInt32 nCopies,
    bool bCollate,
    bool bDirect,
    ImplJobSetup* pJobSetup )
{
    OSL_TRACE("PspSalPrinter::StartJob");
    GetSalData()->m_pInstance->jobStartedPrinterUpdate();
    m_bPdf      = false;
    if (pFileName)
        m_aFileName = *pFileName;
    else
        m_aFileName.clear();
    m_aTmpFile.clear();
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

    int nMode = 0;
#if defined( UNX )
    // check whether this printer is configured as fax
    sal_Int32 nIndex = 0;
    const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( m_aJobData.m_aPrinterName ) );
    while( nIndex != -1 )
    {
        OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( aToken.startsWith( "pdf=" ) )
        {
            m_bPdf = true;
            m_aTmpFile = getTmpName();
            nMode = S_IRUSR | S_IWUSR;

            if( m_aFileName.isEmpty() )
            {
                OUStringBuffer aFileName( getPdfDir( rInfo ) );
                aFileName.append( '/' );
                aFileName.append( rJobName );
                aFileName.append( ".pdf" );
                m_aFileName = aFileName.makeStringAndClear();
            }
            break;
        }
    }
#endif
    m_aPrinterGfx.Init( m_aJobData );

    return m_aPrintJob.StartJob( ! m_aTmpFile.isEmpty() ? m_aTmpFile : m_aFileName, nMode, rJobName, rAppName, m_aJobData, &m_aPrinterGfx, bDirect );
}

bool PspSalPrinter::EndJob()
{
    bool bSuccess = false;
    if( m_bIsPDFWriterJob )
        bSuccess = true;
    else
    {
        bSuccess = m_aPrintJob.EndJob();
        OSL_TRACE("PspSalPrinter::EndJob %d", bSuccess);

        if( bSuccess && m_bPdf )
        {
            const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( m_aJobData.m_aPrinterName ) );
            bSuccess = createPdf( m_aFileName, m_aTmpFile, rInfo.m_aCommand );
        }
    }
    GetSalData()->m_pInstance->jobEndedPrinterUpdate();
    return bSuccess;
}

SalGraphics* PspSalPrinter::StartPage( ImplJobSetup* pJobSetup, bool )
{
    OSL_TRACE("PspSalPrinter::StartPage");

    JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, m_aJobData );
    m_pGraphics = GetGenericInstance()->CreatePrintGraphics();
    m_pGraphics->Init(&m_aJobData, &m_aPrinterGfx, m_pInfoPrinter);

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

void PspSalPrinter::EndPage()
{
    m_aPrintJob.EndPage();
    m_aPrinterGfx.Clear();
    OSL_TRACE("PspSalPrinter::EndPage");
}

sal_uLong PspSalPrinter::GetErrorCode()
{
    return 0;
}

struct PDFNewJobParameters
{
    Size        maPageSize;
    sal_uInt16      mnPaperBin;

    PDFNewJobParameters( const Size& i_rSize = Size(),
                         sal_uInt16 i_nPaperBin = 0xffff )
    : maPageSize( i_rSize ), mnPaperBin( i_nPaperBin ) {}

    bool operator==(const PDFNewJobParameters& rComp ) const
    {
        const long nRotatedWidth = rComp.maPageSize.Height();
        const long nRotatedHeight = rComp.maPageSize.Width();
        Size aCompLSSize(nRotatedWidth, nRotatedHeight);
        return
            (maPageSize == rComp.maPageSize || maPageSize == aCompLSSize)
        &&  mnPaperBin == rComp.mnPaperBin
        ;
    }

    bool operator!=(const PDFNewJobParameters& rComp) const
    {
        return ! this->operator==(rComp);
    }
};

struct PDFPrintFile
{
    OUString       maTmpURL;
    PDFNewJobParameters maParameters;

    PDFPrintFile( const OUString& i_rURL, const PDFNewJobParameters& i_rNewParameters )
    : maTmpURL( i_rURL )
    , maParameters( i_rNewParameters ) {}
};

bool PspSalPrinter::StartJob( const OUString* i_pFileName, const OUString& i_rJobName, const OUString& i_rAppName,
                              ImplJobSetup* i_pSetupData, vcl::PrinterController& i_rController )
{
    OSL_TRACE( "StartJob with controller: pFilename = %s", i_pFileName ? OUStringToOString( *i_pFileName, RTL_TEXTENCODING_UTF8 ).getStr() : "<nil>" );
    // mark for endjob
    m_bIsPDFWriterJob = true;
    // reset IsLastPage
    i_rController.setLastPage( false );
    // is this a fax device
    bool bFax = m_pInfoPrinter->GetCapabilities(i_pSetupData, PrinterCapType::Fax) == 1;

    // update job data
    if( i_pSetupData )
        JobData::constructFromStreamBuffer( i_pSetupData->mpDriverData, i_pSetupData->mnDriverDataLen, m_aJobData );

    OSL_ASSERT( m_aJobData.m_nPDFDevice > 0 );
    m_aJobData.m_nPDFDevice = 1;

    // possibly create one job for collated output
    bool bSinglePrintJobs = false;
    beans::PropertyValue* pSingleValue = i_rController.getValue( OUString( "PrintCollateAsSingleJobs" ) );
    if( pSingleValue )
    {
        pSingleValue->Value >>= bSinglePrintJobs;
    }

    int nCopies = i_rController.getPrinter()->GetCopyCount();
    bool bCollate = i_rController.getPrinter()->IsCollateCopy();

    // notify start of real print job
    i_rController.jobStarted();

    // setup PDFWriter context
    vcl::PDFWriter::PDFWriterContext aContext;
    aContext.Version            = vcl::PDFWriter::PDF_1_4;
    aContext.Tagged             = false;
    aContext.DocumentLocale     = Application::GetSettings().GetLanguageTag().getLocale();
    aContext.ColorMode          = i_rController.getPrinter()->GetPrinterOptions().IsConvertToGreyscales()
    ? vcl::PDFWriter::DrawGreyscale : vcl::PDFWriter::DrawColor;

    // prepare doc info
    aContext.DocumentInfo.Title              = i_rJobName;
    aContext.DocumentInfo.Creator            = i_rAppName;
    aContext.DocumentInfo.Producer           = i_rAppName;

    // define how we handle metafiles in PDFWriter
    vcl::PDFWriter::PlayMetafileContext aMtfContext;
    aMtfContext.m_bOnlyLosslessCompression = true;

    std::shared_ptr<vcl::PDFWriter> xWriter;
    std::vector< PDFPrintFile > aPDFFiles;
    VclPtr<Printer> xPrinter( i_rController.getPrinter() );
    int nAllPages = i_rController.getFilteredPageCount();
    i_rController.createProgressDialog();
    bool bAborted = false;
    PDFNewJobParameters aLastParm;

    aContext.DPIx = xPrinter->GetDPIX();
    aContext.DPIy = xPrinter->GetDPIY();
    for( int nPage = 0; nPage < nAllPages && ! bAborted; nPage++ )
    {
        if( nPage == nAllPages-1 )
            i_rController.setLastPage( true );

        // get the page's metafile
        GDIMetaFile aPageFile;
        vcl::PrinterController::PageSize aPageSize = i_rController.getFilteredPageFile( nPage, aPageFile );
        if( i_rController.isProgressCanceled() )
        {
            bAborted = true;
            if( nPage != nAllPages-1 )
            {
                i_rController.createProgressDialog();
                i_rController.setLastPage( true );
                i_rController.getFilteredPageFile( nPage, aPageFile );
            }
        }
        else
        {
            xPrinter->SetMapMode( MapMode( MAP_100TH_MM ) );
            xPrinter->SetPaperSizeUser( aPageSize.aSize, true );
            PDFNewJobParameters aNewParm(xPrinter->GetPaperSize(), xPrinter->GetPaperBin());

            // create PDF writer on demand
            // either on first page
            // or on paper format change - cups does not support multiple paper formats per job (yet?)
            // so we need to start a new job to get a new paper format from the printer
            // orientation switches (that is switch of height and width) is handled transparently by CUPS
            if( ! xWriter ||
                (aNewParm != aLastParm && ! i_pFileName ) )
            {
                if( xWriter )
                {
                    xWriter->Emit();
                }
                // produce PDF file
                OUString aPDFUrl;
                if( i_pFileName )
                    aPDFUrl = *i_pFileName;
                else
                    osl_createTempFile( nullptr, nullptr, &aPDFUrl.pData );
                // normalize to file URL
                if( !comphelper::isFileUrl(aPDFUrl) )
                {
                    // this is not a file URL, but it should
                    // form it into a osl friendly file URL
                    OUString aTmp;
                    osl_getFileURLFromSystemPath( aPDFUrl.pData, &aTmp.pData );
                    aPDFUrl = aTmp;
                }
                // save current file and paper format
                aLastParm = aNewParm;
                aPDFFiles.push_back( PDFPrintFile( aPDFUrl, aNewParm ) );
                // update context
                aContext.URL = aPDFUrl;

                // create and initialize PDFWriter
                xWriter.reset( new vcl::PDFWriter( aContext, uno::Reference< beans::XMaterialHolder >() ) );
            }

            xWriter->NewPage( TenMuToPt( aNewParm.maPageSize.Width() ),
                              TenMuToPt( aNewParm.maPageSize.Height() ),
                              vcl::PDFWriter::Portrait );

            xWriter->PlayMetafile( aPageFile, aMtfContext );
        }
    }

    // emit the last file
    if( xWriter )
        xWriter->Emit();

    // handle collate, copy count and multiple jobs correctly
    int nOuterJobs = 1;
    if( bSinglePrintJobs )
    {
        nOuterJobs = nCopies;
        m_aJobData.m_nCopies = 1;
    }
    else
    {
        if( bCollate )
        {
            if (aPDFFiles.size() == 1 && xPrinter->HasSupport(SUPPORT_COLLATECOPY))
            {
                m_aJobData.setCollate( true );
                m_aJobData.m_nCopies = nCopies;
            }
            else
            {
                nOuterJobs = nCopies;
                m_aJobData.m_nCopies = 1;
            }
        }
        else
        {
            m_aJobData.setCollate( false );
            m_aJobData.m_nCopies = nCopies;
        }
    }

    std::vector<OUString> aFaxNumbers;

    // check for fax numbers
    if (!bAborted && bFax)
    {
        aFaxNumbers = getFaxNumbers();
        bAborted = aFaxNumbers.empty();
    }

    bool bSuccess(true);
    // spool files
    if( ! i_pFileName && ! bAborted )
    {
        do
        {
            OUString sFaxNumber;
            if (!aFaxNumbers.empty())
            {
                sFaxNumber = aFaxNumbers.back();
                aFaxNumbers.pop_back();
            }

            bool bFirstJob = true;
            for( int nCurJob = 0; nCurJob < nOuterJobs; nCurJob++ )
            {
                for( size_t i = 0; i < aPDFFiles.size(); i++ )
                {
                    oslFileHandle pFile = nullptr;
                    osl_openFile( aPDFFiles[i].maTmpURL.pData, &pFile, osl_File_OpenFlag_Read );
                    if (pFile && (osl_setFilePos(pFile, osl_Pos_Absolut, 0) == osl_File_E_None))
                    {
                        std::vector< char > buffer( 0x10000, 0 );
                        // update job data with current page size
                        Size aPageSize( aPDFFiles[i].maParameters.maPageSize );
                        m_aJobData.setPaper( TenMuToPt( aPageSize.Width() ), TenMuToPt( aPageSize.Height() ) );
                        // update job data with current paperbin
                        m_aJobData.setPaperBin( aPDFFiles[i].maParameters.mnPaperBin );

                        // spool current file
                        FILE* fp = PrinterInfoManager::get().startSpool(xPrinter->GetName(), i_rController.isDirectPrint());
                        if( fp )
                        {
                            sal_uInt64 nBytesRead = 0;
                            do
                            {
                                osl_readFile( pFile, &buffer[0], buffer.size(), &nBytesRead );
                                if( nBytesRead > 0 )
                                {
                                    size_t nBytesWritten = fwrite(&buffer[0], 1, nBytesRead, fp);
                                    OSL_ENSURE(nBytesRead == nBytesWritten, "short write");
                                    if (nBytesRead != nBytesWritten)
                                        break;
                                }
                            } while( nBytesRead == buffer.size() );
                            OUStringBuffer aBuf( i_rJobName.getLength() + 8 );
                            aBuf.append( i_rJobName );
                            if( i > 0 || nCurJob > 0 )
                            {
                                aBuf.append( ' ' );
                                aBuf.append( sal_Int32( i + nCurJob * aPDFFiles.size() ) );
                            }
                            bSuccess &=
                            PrinterInfoManager::get().endSpool(xPrinter->GetName(), aBuf.makeStringAndClear(), fp, m_aJobData, bFirstJob, sFaxNumber);
                            bFirstJob = false;
                        }
                    }
                    osl_closeFile( pFile );
                }
            }
        }
        while (!aFaxNumbers.empty());
    }

    // job has been spooled
    i_rController.setJobState( (bAborted)
            ? view::PrintableState_JOB_ABORTED
            : ((bSuccess) ? view::PrintableState_JOB_SPOOLED
                          : view::PrintableState_JOB_SPOOLING_FAILED));

    // clean up the temporary PDF files
    if( ! i_pFileName || bAborted )
    {
        for( size_t i = 0; i < aPDFFiles.size(); i++ )
        {
            osl_removeFile( aPDFFiles[i].maTmpURL.pData );
            OSL_TRACE( "removed print PDF file %s", OUStringToOString( aPDFFiles[i].maTmpURL, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }

    return true;
}

class PrinterUpdate
{
    static Idle*  pPrinterUpdateIdle;
    static int    nActiveJobs;

    static void doUpdate();
    DECL_STATIC_LINK_TYPED( PrinterUpdate, UpdateTimerHdl, Idle*, void );
public:
    static void update(SalGenericInstance &rInstance);
    static void jobStarted() { nActiveJobs++; }
    static void jobEnded();
};

Idle* PrinterUpdate::pPrinterUpdateIdle = nullptr;
int PrinterUpdate::nActiveJobs = 0;

void PrinterUpdate::doUpdate()
{
    ::psp::PrinterInfoManager& rManager( ::psp::PrinterInfoManager::get() );
    SalGenericInstance *pInst = static_cast<SalGenericInstance *>( GetSalData()->m_pInstance );
    if( pInst && rManager.checkPrintersChanged( false ) )
        pInst->PostPrintersChanged();
}

IMPL_STATIC_LINK_NOARG_TYPED( PrinterUpdate, UpdateTimerHdl, Idle*, void )
{
    if( nActiveJobs < 1 )
    {
        doUpdate();
        delete pPrinterUpdateIdle;
        pPrinterUpdateIdle = nullptr;
    }
    else
        pPrinterUpdateIdle->Start();
}

void PrinterUpdate::update(SalGenericInstance &rInstance)
{
    if( Application::GetSettings().GetMiscSettings().GetDisablePrinting() )
        return;

    if( ! rInstance.isPrinterInit() )
    {
        // #i45389# start background printer detection
        psp::PrinterInfoManager::get();
        return;
    }

    if( nActiveJobs < 1 )
        doUpdate();
    else if( ! pPrinterUpdateIdle )
    {
        pPrinterUpdateIdle = new Idle("PrinterUpdateTimer");
        pPrinterUpdateIdle->SetPriority( SchedulerPriority::LOWEST );
        pPrinterUpdateIdle->SetIdleHdl( LINK( nullptr, PrinterUpdate, UpdateTimerHdl ) );
        pPrinterUpdateIdle->Start();
    }
}

void SalGenericInstance::updatePrinterUpdate()
{
    PrinterUpdate::update(*this);
}

void SalGenericInstance::jobStartedPrinterUpdate()
{
    PrinterUpdate::jobStarted();
}

void PrinterUpdate::jobEnded()
{
    nActiveJobs--;
    if( nActiveJobs < 1 )
    {
        if( pPrinterUpdateIdle )
        {
            pPrinterUpdateIdle->Stop();
            delete pPrinterUpdateIdle;
            pPrinterUpdateIdle = nullptr;
            doUpdate();
        }
    }
}

void SalGenericInstance::jobEndedPrinterUpdate()
{
    PrinterUpdate::jobEnded();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
