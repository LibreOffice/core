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

#include <sal/config.h>

#include <string_view>

// For spawning PDF and FAX generation
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <comphelper/fileurl.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <osl/file.hxx>

#include <utility>
#include <vcl/gdimtf.hxx>
#include <vcl/idle.hxx>
#include <vcl/printer/Options.hxx>
#include <vcl/print.hxx>
#include <vcl/QueueInfo.hxx>
#include <vcl/pdfwriter.hxx>
#include <printerinfomanager.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <strings.hrc>
#include <unx/genprn.h>
#include <unx/geninst.h>
#include <unx/genpspgraphics.h>

#include <jobset.h>
#include <print.h>
#include "prtsetup.hxx"
#include <salptype.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace psp;
using namespace com::sun::star;

static bool getPdfDir( const PrinterInfo& rInfo, OUString &rDir )
{
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( aToken.startsWith( "pdf=" ) )
        {
            sal_Int32 nPos = 0;
            rDir = aToken.getToken( 1, '=', nPos );
            if( rDir.isEmpty() && getenv( "HOME" ) )
                rDir = OUString( getenv( "HOME" ), strlen( getenv( "HOME" ) ), osl_getThreadTextEncoding() );
            return true;
        }
    }
    return false;
}

namespace
{
    class QueryString : public weld::GenericDialogController
    {
    private:
        OUString&           m_rReturnValue;

        std::unique_ptr<weld::Button> m_xOKButton;
        std::unique_ptr<weld::Label> m_xFixedText;
        std::unique_ptr<weld::Entry> m_xEdit;

        DECL_LINK( ClickBtnHdl, weld::Button&, void );

    public:
        // parent window, Query text, initial value
        QueryString(weld::Window*, OUString const &, OUString &);
    };

    /*
     *  QueryString
     */
    QueryString::QueryString(weld::Window* pParent, OUString const & rQuery, OUString& rRet)
        : GenericDialogController(pParent, "vcl/ui/querydialog.ui", "QueryDialog")
        , m_rReturnValue( rRet )
        , m_xOKButton(m_xBuilder->weld_button("ok"))
        , m_xFixedText(m_xBuilder->weld_label("label"))
        , m_xEdit(m_xBuilder->weld_entry("entry"))
    {
        m_xOKButton->connect_clicked(LINK(this, QueryString, ClickBtnHdl));
        m_xFixedText->set_label(rQuery);
        m_xEdit->set_text(m_rReturnValue);
        m_xDialog->set_title(rQuery);
    }

    IMPL_LINK(QueryString, ClickBtnHdl, weld::Button&, rButton, void)
    {
        if (&rButton == m_xOKButton.get())
        {
            m_rReturnValue = m_xEdit->get_text();
            m_xDialog->response(RET_OK);
        }
        else
            m_xDialog->response(RET_CANCEL);
    }

    int QueryFaxNumber(OUString& rNumber)
    {
        QueryString aQuery(Application::GetDefDialogParent(), VclResId(SV_PRINT_QUERYFAXNUMBER_TXT), rNumber);
        return aQuery.run();
    }
}

static int PtTo10Mu( int nPoints ) { return static_cast<int>((static_cast<double>(nPoints)*35.27777778)+0.5); }

static int TenMuToPt( int nUnits ) { return static_cast<int>((static_cast<double>(nUnits)/35.27777778)+0.5); }

static void copyJobDataToJobSetup( ImplJobSetup* pJobSetup, JobData& rData )
{
    pJobSetup->SetOrientation( rData.m_eOrientation == orientation::Landscape ?
        Orientation::Landscape : Orientation::Portrait );

    // copy page size
    OUString aPaper;
    int width, height;

    rData.m_aContext.getPageSize( aPaper, width, height );
    pJobSetup->SetPaperFormat( PaperInfo::fromPSName(
        OUStringToOString( aPaper, RTL_TEXTENCODING_ISO_8859_1 )));

    pJobSetup->SetPaperWidth( 0 );
    pJobSetup->SetPaperHeight( 0 );
    if( pJobSetup->GetPaperFormat() == PAPER_USER )
    {
        // transform to 100dth mm
        width               = PtTo10Mu( width );
        height              = PtTo10Mu( height );

        if( rData.m_eOrientation == psp::orientation::Portrait )
        {
            pJobSetup->SetPaperWidth( width );
            pJobSetup->SetPaperHeight( height );
        }
        else
        {
            pJobSetup->SetPaperWidth( height );
            pJobSetup->SetPaperHeight( width );
        }
    }

    // copy input slot
    const PPDKey* pKey = nullptr;
    const PPDValue* pValue = nullptr;

    pJobSetup->SetPaperBin( 0 );
    if( rData.m_pParser )
        pKey                    = rData.m_pParser->getKey( "InputSlot" );
    if( pKey )
        pValue                  = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        int nPaperBin;
        for( nPaperBin = 0;
             pValue != pKey->getValue( nPaperBin ) &&
                 nPaperBin < pKey->countValues();
             nPaperBin++);
        pJobSetup->SetPaperBin(
            nPaperBin == pKey->countValues() ? 0 : nPaperBin);
    }

    // copy duplex
    pKey = nullptr;
    pValue = nullptr;

    pJobSetup->SetDuplexMode( DuplexMode::Unknown );
    if( rData.m_pParser )
        pKey = rData.m_pParser->getKey( "Duplex" );
    if( pKey )
        pValue = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        if( pValue->m_aOption.equalsIgnoreAsciiCase( "None" ) ||
            pValue->m_aOption.startsWithIgnoreAsciiCase( "Simplex" )
           )
        {
            pJobSetup->SetDuplexMode( DuplexMode::Off);
        }
        else if( pValue->m_aOption.equalsIgnoreAsciiCase( "DuplexNoTumble" ) )
        {
            pJobSetup->SetDuplexMode( DuplexMode::LongEdge );
        }
        else if( pValue->m_aOption.equalsIgnoreAsciiCase( "DuplexTumble" ) )
        {
            pJobSetup->SetDuplexMode( DuplexMode::ShortEdge );
        }
    }

    // copy the whole context

    sal_uInt32 nBytes;
    std::unique_ptr<sal_uInt8[]> pBuffer;
    if( rData.getStreamBuffer( pBuffer, nBytes ) )
    {
        pJobSetup->SetDriverData( std::move(pBuffer), nBytes );
    }
    else
    {
        pJobSetup->SetDriverData( nullptr, 0 );
    }
    pJobSetup->SetPapersizeFromSetup( rData.m_bPapersizeFromSetup );
}

static std::vector<OUString> getFaxNumbers()
{
    std::vector<OUString> aFaxNumbers;

    OUString aNewNr;
    if (QueryFaxNumber(aNewNr))
    {
        for (sal_Int32 nIndex {0}; nIndex >= 0; )
            aFaxNumbers.push_back(aNewNr.getToken( 0, ';', nIndex ));
    }

    return aFaxNumbers;
}

/*
 *  SalInstance
 */

void SalGenericInstance::configurePspInfoPrinter(PspSalInfoPrinter *pPrinter,
    SalPrinterQueueInfo const * pQueueInfo, ImplJobSetup* pJobSetup)
{
    if( !pJobSetup )
        return;

    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    PrinterInfo aInfo( rManager.getPrinterInfo( pQueueInfo->maPrinterName ) );
    pPrinter->m_aJobData = aInfo;

    if( pJobSetup->GetDriverData() )
        JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(),
                                            pJobSetup->GetDriverDataLen(), aInfo );

    pJobSetup->SetSystem( JOBSETUP_SYSTEM_UNIX );
    pJobSetup->SetPrinterName( pQueueInfo->maPrinterName );
    pJobSetup->SetDriver( aInfo.m_aDriverName );
    copyJobDataToJobSetup( pJobSetup, aInfo );
}

SalInfoPrinter* SalGenericInstance::CreateInfoPrinter( SalPrinterQueueInfo*    pQueueInfo,
                                                       ImplJobSetup*           pJobSetup )
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

std::unique_ptr<SalPrinter> SalGenericInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    mbPrinterInit = true;
    // create and initialize SalPrinter
    PspSalPrinter* pPrinter = new PspSalPrinter( pInfoPrinter );
    pPrinter->m_aJobData = static_cast<PspSalInfoPrinter*>(pInfoPrinter)->m_aJobData;

    return std::unique_ptr<SalPrinter>(pPrinter);
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
    ::std::vector< OUString > aPrinters;
    rManager.listPrinters( aPrinters );

    for (auto const& printer : aPrinters)
    {
        const PrinterInfo& rInfo( rManager.getPrinterInfo(printer) );
        // create new entry
        std::unique_ptr<SalPrinterQueueInfo> pInfo(new SalPrinterQueueInfo);
        pInfo->maPrinterName    = printer;
        pInfo->maDriver         = rInfo.m_aDriverName;
        pInfo->maLocation       = rInfo.m_aLocation;
        pInfo->maComment        = rInfo.m_aComment;

        OUString sPdfDir;
        if (getPdfDir(rInfo, sPdfDir))
            pInfo->maLocation = sPdfDir;

        pList->Add( std::move(pInfo) );
    }
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
{
}

PspSalInfoPrinter::~PspSalInfoPrinter()
{
}

void PspSalInfoPrinter::InitPaperFormats( const ImplJobSetup* )
{
    m_aPaperFormats.clear();
    m_bPapersInit = true;

    if( !m_aJobData.m_pParser )
        return;

    const PPDKey* pKey = m_aJobData.m_pParser->getKey( "PageSize" );
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
        m_pGraphics->Init(&m_aJobData);
        pRet = m_pGraphics.get();
    }
    return pRet;
}

void PspSalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
    if( m_pGraphics.get() == pGraphics )
    {
        m_pGraphics.reset();
    }
}

bool PspSalInfoPrinter::Setup( weld::Window* pFrame, ImplJobSetup* pJobSetup )
{
    if( ! pFrame || ! pJobSetup )
        return false;

    PrinterInfoManager& rManager = PrinterInfoManager::get();

    PrinterInfo aInfo( rManager.getPrinterInfo( pJobSetup->GetPrinterName() ) );
    if ( pJobSetup->GetDriverData() )
    {
        SetData( JobSetFlags::ALL, pJobSetup );
        JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(), pJobSetup->GetDriverDataLen(), aInfo );
    }
    aInfo.m_bPapersizeFromSetup = pJobSetup->GetPapersizeFromSetup();
    aInfo.meSetupMode = pJobSetup->GetPrinterSetupMode();

    if (SetupPrinterDriver(pFrame, aInfo))
    {
        pJobSetup->SetDriverData( nullptr, 0 );

        sal_uInt32 nBytes;
        std::unique_ptr<sal_uInt8[]> pBuffer;
        aInfo.getStreamBuffer( pBuffer, nBytes );
        pJobSetup->SetDriverData( std::move(pBuffer), nBytes );

        // copy everything to job setup
        copyJobDataToJobSetup( pJobSetup, aInfo );
        JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(), pJobSetup->GetDriverDataLen(), m_aJobData );
        return true;
    }
    return false;
}

// This function gets the driver data and puts it into pJobSetup
// If pJobSetup->GetDriverData() is NOT NULL, then the independent
// data should be merged into the driver data
// If pJobSetup->GetDriverData() IS NULL, then the driver defaults
// should be merged into the independent data
bool PspSalInfoPrinter::SetPrinterData( ImplJobSetup* pJobSetup )
{
    if( pJobSetup->GetDriverData() )
        return SetData( JobSetFlags::ALL, pJobSetup );

    copyJobDataToJobSetup( pJobSetup, m_aJobData );

    return true;
}

// This function merges the independent driver data
// and sets the new independent data in pJobSetup
// Only the data must be changed, where the bit
// in nGetDataFlags is set
bool PspSalInfoPrinter::SetData(
    JobSetFlags nSetDataFlags,
    ImplJobSetup* pJobSetup )
{
    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(), pJobSetup->GetDriverDataLen(), aData );

    if( aData.m_pParser )
    {
        const PPDKey* pKey;
        const PPDValue* pValue;

        // merge orientation if necessary
        if( nSetDataFlags & JobSetFlags::ORIENTATION )
            aData.m_eOrientation = pJobSetup->GetOrientation() == Orientation::Landscape ? orientation::Landscape : orientation::Portrait;

        // merge papersize if necessary
        if( nSetDataFlags & JobSetFlags::PAPERSIZE )
        {
            OUString aPaper;

            if( pJobSetup->GetPaperFormat() == PAPER_USER )
                aPaper = aData.m_pParser->matchPaper(
                    TenMuToPt( pJobSetup->GetPaperWidth() ),
                    TenMuToPt( pJobSetup->GetPaperHeight() ),
                    &aData.m_eOrientation );
            else
                aPaper = OStringToOUString(PaperInfo::toPSName(pJobSetup->GetPaperFormat()), RTL_TEXTENCODING_ISO_8859_1);

            pKey = aData.m_pParser->getKey( "PageSize" );
            pValue = pKey ? pKey->getValueCaseInsensitive( aPaper ) : nullptr;

            // some PPD files do not specify the standard paper names (e.g. C5 instead of EnvC5)
            // try to find the correct paper anyway using the size
            if( pKey && ! pValue && pJobSetup->GetPaperFormat() != PAPER_USER )
            {
                PaperInfo aInfo( pJobSetup->GetPaperFormat() );
                aPaper = aData.m_pParser->matchPaper(
                    TenMuToPt( aInfo.getWidth() ),
                    TenMuToPt( aInfo.getHeight() ),
                    &aData.m_eOrientation );
                pValue = pKey->getValueCaseInsensitive( aPaper );
            }

            if( ! ( pKey && pValue && aData.m_aContext.setValue( pKey, pValue ) == pValue ) )
                return false;
        }

        // merge paperbin if necessary
        if( nSetDataFlags & JobSetFlags::PAPERBIN )
        {
            pKey = aData.m_pParser->getKey( "InputSlot" );
            if( pKey )
            {
                int nPaperBin = pJobSetup->GetPaperBin();
                if( nPaperBin >= pKey->countValues() )
                    pValue = pKey->getDefaultValue();
                else
                    pValue = pKey->getValue( pJobSetup->GetPaperBin() );

                // may fail due to constraints;
                // real paper bin is copied back to jobsetup in that case
                aData.m_aContext.setValue( pKey, pValue );
            }
            // if printer has no InputSlot key simply ignore this setting
            // (e.g. SGENPRT has no InputSlot)
        }

        // merge duplex if necessary
        if( nSetDataFlags & JobSetFlags::DUPLEXMODE )
        {
            pKey = aData.m_pParser->getKey( "Duplex" );
            if( pKey )
            {
                pValue = nullptr;
                switch( pJobSetup->GetDuplexMode() )
                {
                case DuplexMode::Off:
                    pValue = pKey->getValue( "None" );
                    if( pValue == nullptr )
                        pValue = pKey->getValue( "SimplexNoTumble" );
                    break;
                case DuplexMode::ShortEdge:
                    pValue = pKey->getValue( "DuplexTumble" );
                    break;
                case DuplexMode::LongEdge:
                    pValue = pKey->getValue( "DuplexNoTumble" );
                    break;
                case DuplexMode::Unknown:
                default:
                    pValue = nullptr;
                    break;
                }
                if( ! pValue )
                    pValue = pKey->getDefaultValue();
                aData.m_aContext.setValue( pKey, pValue );
            }
        }
        aData.m_bPapersizeFromSetup = pJobSetup->GetPapersizeFromSetup();

        m_aJobData = aData;
        copyJobDataToJobSetup( pJobSetup, aData );
        return true;
    }

    return false;
}

void PspSalInfoPrinter::GetPageInfo(
    const ImplJobSetup* pJobSetup,
    tools::Long& rOutWidth, tools::Long& rOutHeight,
    Point& rPageOffset,
    Size& rPaperSize )
{
    if( ! pJobSetup )
        return;

    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(), pJobSetup->GetDriverDataLen(), aData );

    // get the selected page size
    if( !aData.m_pParser )
        return;


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

    rPaperSize.setWidth( width * nDPI / 72 );
    rPaperSize.setHeight( height * nDPI / 72 );
    rPageOffset.setX( left * nDPI / 72 );
    rPageOffset.setY( top * nDPI / 72 );
    rOutWidth   = ( width  - left - right ) * nDPI / 72;
    rOutHeight  = ( height - top  - bottom ) * nDPI / 72;

}

sal_uInt16 PspSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pJobSetup )
{
    if( ! pJobSetup )
        return 0;

    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(), pJobSetup->GetDriverDataLen(), aData );

    const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( "InputSlot" ): nullptr;
    return pKey ? pKey->countValues() : 0;
}

OUString PspSalInfoPrinter::GetPaperBinName( const ImplJobSetup* pJobSetup, sal_uInt16 nPaperBin )
{
    JobData aData;
    JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(), pJobSetup->GetDriverDataLen(), aData );

    if( aData.m_pParser )
    {
        const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey( "InputSlot" ): nullptr;
        if( ! pKey || nPaperBin >= o3tl::make_unsigned(pKey->countValues()) )
            return aData.m_pParser->getDefaultInputSlot();
        const PPDValue* pValue = pKey->getValue( nPaperBin );
        if( pValue )
            return aData.m_pParser->translateOption( pKey->getKey(), pValue->m_aOption );
    }

    return OUString();
}

sal_uInt16 PspSalInfoPrinter::GetPaperBinBySourceIndex( const ImplJobSetup*, sal_uInt16 )
{
    return 0xffff;
}

sal_uInt16  PspSalInfoPrinter::GetSourceIndexByPaperBin(const ImplJobSetup*, sal_uInt16)
{
    return 0;
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
        case PrinterCapType::SetPaperSize:
            return 1;
        case PrinterCapType::SetPaper:
            return 0;
        case PrinterCapType::Fax:
            {
                // see if the PPD contains the fax4CUPS "Dial" option and that it's not set
                // to "manually"
                JobData aData = PrinterInfoManager::get().getPrinterInfo(pJobSetup->GetPrinterName());
                if( pJobSetup->GetDriverData() )
                    JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(), pJobSetup->GetDriverDataLen(), aData );
                const PPDKey* pKey = aData.m_pParser ? aData.m_pParser->getKey("Dial") : nullptr;
                const PPDValue* pValue = pKey ? aData.m_aContext.getValue(pKey) : nullptr;
                if (pValue && !pValue->m_aOption.equalsIgnoreAsciiCase("Manually"))
                    return 1;
                return 0;
            }

        case PrinterCapType::PDF:
            return 1;
        case PrinterCapType::ExternalDialog:
            return PrinterInfoManager::get().checkFeatureToken( pJobSetup->GetPrinterName(), "external_dialog" ) ? 1 : 0;
        case PrinterCapType::UsePullModel:
            return 1;
        default: break;
    }
    return 0;
}

/*
 *  SalPrinter
 */
PspSalPrinter::PspSalPrinter( SalInfoPrinter* pInfoPrinter )
    : m_pInfoPrinter( pInfoPrinter )
{
}

PspSalPrinter::~PspSalPrinter()
{
}

bool PspSalPrinter::StartJob(
    const OUString* /*pFileName*/,
    const OUString& /*rJobName*/,
    const OUString& /*rAppName*/,
    sal_uInt32 /*nCopies*/,
    bool /*bCollate*/,
    bool /*bDirect*/,
    ImplJobSetup* /*pJobSetup*/ )
{
    OSL_FAIL( "should never be called" );
    return false;
}

bool PspSalPrinter::EndJob()
{
    GetSalInstance()->jobEndedPrinterUpdate();
    return true;
}

SalGraphics* PspSalPrinter::StartPage( ImplJobSetup*, bool )
{
    OSL_FAIL( "should never be called" );
    return nullptr;
}

void PspSalPrinter::EndPage()
{
    OSL_FAIL( "should never be called" );
}

namespace {

struct PDFNewJobParameters
{
    Size        maPageSize;
    sal_uInt16      mnPaperBin;

    PDFNewJobParameters( const Size& i_rSize = Size(),
                         sal_uInt16 i_nPaperBin = 0xffff )
    : maPageSize( i_rSize ), mnPaperBin( i_nPaperBin ) {}

    bool operator==(const PDFNewJobParameters& rComp ) const
    {
        const tools::Long nRotatedWidth = rComp.maPageSize.Height();
        const tools::Long nRotatedHeight = rComp.maPageSize.Width();
        Size aCompLSSize(nRotatedWidth, nRotatedHeight);
        return
            (maPageSize == rComp.maPageSize || maPageSize == aCompLSSize)
        &&  mnPaperBin == rComp.mnPaperBin
        ;
    }

    bool operator!=(const PDFNewJobParameters& rComp) const
    {
        return ! operator==(rComp);
    }
};

struct PDFPrintFile
{
    OUString       maTmpURL;
    PDFNewJobParameters maParameters;

    PDFPrintFile( OUString i_URL, const PDFNewJobParameters& i_rNewParameters )
    : maTmpURL(std::move( i_URL ))
    , maParameters( i_rNewParameters ) {}
};

}

bool PspSalPrinter::StartJob( const OUString* i_pFileName, const OUString& i_rJobName, const OUString& i_rAppName,
                              ImplJobSetup* i_pSetupData, vcl::PrinterController& i_rController )
{
    SAL_INFO( "vcl.unx.print", "StartJob with controller: pFilename = " << (i_pFileName ? *i_pFileName : "<nil>") );
    // reset IsLastPage
    i_rController.setLastPage( false );
    // is this a fax device
    bool bFax = m_pInfoPrinter->GetCapabilities(i_pSetupData, PrinterCapType::Fax) == 1;

    // update job data
    if( i_pSetupData )
        JobData::constructFromStreamBuffer( i_pSetupData->GetDriverData(), i_pSetupData->GetDriverDataLen(), m_aJobData );

    // possibly create one job for collated output
    int nCopies = i_rController.getPrinter()->GetCopyCount();
    bool bCollate = i_rController.getPrinter()->IsCollateCopy();
    bool bSinglePrintJobs = i_rController.getPrinter()->IsSinglePrintJobs();

    // notify start of real print job
    i_rController.jobStarted();

    // setup PDFWriter context
    vcl::PDFWriter::PDFWriterContext aContext;
    aContext.Version            = vcl::PDFWriter::PDFVersion::PDF_1_4;
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
            xPrinter->SetMapMode( MapMode( MapUnit::Map100thMM ) );
            xPrinter->SetPaperSizeUser( aPageSize.aSize );
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
                    // form it into an osl friendly file URL
                    OUString aTmp;
                    osl_getFileURLFromSystemPath( aPDFUrl.pData, &aTmp.pData );
                    aPDFUrl = aTmp;
                }
                // save current file and paper format
                aLastParm = aNewParm;
                aPDFFiles.emplace_back( aPDFUrl, aNewParm );
                // update context
                aContext.URL = aPDFUrl;

                // create and initialize PDFWriter
                xWriter = std::make_shared<vcl::PDFWriter>( aContext, uno::Reference< beans::XMaterialHolder >() );
            }

            xWriter->NewPage( TenMuToPt( aNewParm.maPageSize.Width() ),
                              TenMuToPt( aNewParm.maPageSize.Height() ),
                              vcl::PDFWriter::Orientation::Portrait );

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
            if (aPDFFiles.size() == 1 && xPrinter->HasSupport(PrinterSupport::CollateCopy))
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
                                osl_readFile( pFile, buffer.data(), buffer.size(), &nBytesRead );
                                if( nBytesRead > 0 )
                                {
                                    size_t nBytesWritten = fwrite(buffer.data(), 1, nBytesRead, fp);
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
    i_rController.setJobState( bAborted
            ? view::PrintableState_JOB_ABORTED
            : (bSuccess ? view::PrintableState_JOB_SPOOLED
                          : view::PrintableState_JOB_SPOOLING_FAILED));

    // clean up the temporary PDF files
    if( ! i_pFileName || bAborted )
    {
        for(PDFPrintFile & rPDFFile : aPDFFiles)
        {
            osl_removeFile( rPDFFile.maTmpURL.pData );
            SAL_INFO( "vcl.unx.print", "removed print PDF file " << rPDFFile.maTmpURL );
        }
    }

    return true;
}

namespace {

class PrinterUpdate
{
    static Idle*  pPrinterUpdateIdle;
    static int    nActiveJobs;

    static void doUpdate();
    DECL_STATIC_LINK( PrinterUpdate, UpdateTimerHdl, Timer*, void );
public:
    static void update(SalGenericInstance const &rInstance);
    static void jobEnded();
};

}

Idle* PrinterUpdate::pPrinterUpdateIdle = nullptr;
int PrinterUpdate::nActiveJobs = 0;

void PrinterUpdate::doUpdate()
{
    ::psp::PrinterInfoManager& rManager( ::psp::PrinterInfoManager::get() );
    SalGenericInstance *pInst = GetGenericInstance();
    if( pInst && rManager.checkPrintersChanged( false ) )
        pInst->PostPrintersChanged();
}

IMPL_STATIC_LINK_NOARG( PrinterUpdate, UpdateTimerHdl, Timer*, void )
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

void PrinterUpdate::update(SalGenericInstance const &rInstance)
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
        pPrinterUpdateIdle->SetPriority( TaskPriority::LOWEST );
        pPrinterUpdateIdle->SetInvokeHandler( LINK( nullptr, PrinterUpdate, UpdateTimerHdl ) );
        pPrinterUpdateIdle->Start();
    }
}

void SalGenericInstance::updatePrinterUpdate()
{
    PrinterUpdate::update(*this);
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
