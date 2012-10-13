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

#include "xlroot.hxx"
#include <rtl/strbuf.hxx>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <svl/stritem.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/font.hxx>
#include <editeng/editstat.hxx>
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include "document.hxx"
#include "docpool.hxx"
#include "docuno.hxx"
#include "editutil.hxx"
#include "drwlayer.hxx"
#include "scextopt.hxx"
#include "patattr.hxx"
#include "fapihelper.hxx"
#include "xlconst.hxx"
#include "xlstyle.hxx"
#include "xlchart.hxx"
#include "xltracer.hxx"
#include <unotools/useroptions.hxx>
#include "root.hxx"

namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;

using ::rtl::OUString;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::DeviceInfo;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XFramesSupplier;
using ::com::sun::star::lang::XMultiServiceFactory;

using namespace ::com::sun::star;

// Global data ================================================================

#ifdef DBG_UTIL
XclDebugObjCounter::~XclDebugObjCounter()
{
    OSL_ENSURE( mnObjCnt == 0, "XclDebugObjCounter::~XclDebugObjCounter - wrong root object count" );
}
#endif

// ----------------------------------------------------------------------------

XclRootData::XclRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc, bool bExport ) :
    meBiff( eBiff ),
    meOutput( EXC_OUTPUT_BINARY ),
    mrMedium( rMedium ),
    mxRootStrg( xRootStrg ),
    mrDoc( rDoc ),
    maDefPassword( CREATE_STRING( "VelvetSweatshop" ) ),
    meTextEnc( eTextEnc ),
    meSysLang( Application::GetSettings().GetLanguage() ),
    meDocLang( Application::GetSettings().GetLanguage() ),
    meUILang( Application::GetSettings().GetUILanguage() ),
    mnDefApiScript( ApiScriptType::LATIN ),
    maScMaxPos( MAXCOL, MAXROW, MAXTAB ),
    maXclMaxPos( EXC_MAXCOL2, EXC_MAXROW2, EXC_MAXTAB2 ),
    maMaxPos( EXC_MAXCOL2, EXC_MAXROW2, EXC_MAXTAB2 ),
    mxFontPropSetHlp( new XclFontPropSetHelper ),
    mxChPropSetHlp( new XclChPropSetHelper ),
    mxRD( new RootData ),//!
    mfScreenPixelX( 50.0 ),
    mfScreenPixelY( 50.0 ),
    mnCharWidth( 110 ),
    mnScTab( 0 ),
    mbExport( bExport )
{
 maUserName = SvtUserOptions().GetLastName();
    if( maUserName.Len() == 0 )
        maUserName = CREATE_STRING( "Calc" );

    switch( ScGlobal::GetDefaultScriptType() )
    {
        case SCRIPTTYPE_LATIN:      mnDefApiScript = ApiScriptType::LATIN;      break;
        case SCRIPTTYPE_ASIAN:      mnDefApiScript = ApiScriptType::ASIAN;      break;
        case SCRIPTTYPE_COMPLEX:    mnDefApiScript = ApiScriptType::COMPLEX;    break;
        default:    OSL_FAIL( "XclRootData::XclRootData - unknown script type" );
    }

    // maximum cell position
    switch( meBiff )
    {
        case EXC_BIFF2: maXclMaxPos.Set( EXC_MAXCOL2, EXC_MAXROW2, EXC_MAXTAB2 );   break;
        case EXC_BIFF3: maXclMaxPos.Set( EXC_MAXCOL3, EXC_MAXROW3, EXC_MAXTAB3 );   break;
        case EXC_BIFF4: maXclMaxPos.Set( EXC_MAXCOL4, EXC_MAXROW4, EXC_MAXTAB4 );   break;
        case EXC_BIFF5: maXclMaxPos.Set( EXC_MAXCOL5, EXC_MAXROW5, EXC_MAXTAB5 );   break;
        case EXC_BIFF8: maXclMaxPos.Set( EXC_MAXCOL8, EXC_MAXROW8, EXC_MAXTAB8 );   break;
        default:        DBG_ERROR_BIFF();
    }
    maMaxPos.SetCol( ::std::min( maScMaxPos.Col(), maXclMaxPos.Col() ) );
    maMaxPos.SetRow( ::std::min( maScMaxPos.Row(), maXclMaxPos.Row() ) );
    maMaxPos.SetTab( ::std::min( maScMaxPos.Tab(), maXclMaxPos.Tab() ) );

    // document URL and path
    if( const SfxItemSet* pItemSet = mrMedium.GetItemSet() )
        if( const SfxStringItem* pItem = static_cast< const SfxStringItem* >( pItemSet->GetItem( SID_FILE_NAME ) ) )
            maDocUrl = pItem->GetValue();
    maBasePath = maDocUrl.Copy( 0, maDocUrl.SearchBackward( '/' ) + 1 );

    // extended document options - always own object, try to copy existing data from document
    if( const ScExtDocOptions* pOldDocOpt = mrDoc.GetExtDocOptions() )
        mxExtDocOpt.reset( new ScExtDocOptions( *pOldDocOpt ) );
    else
        mxExtDocOpt.reset( new ScExtDocOptions );

    // screen pixel size
    try
    {
        Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory(), UNO_SET_THROW );
        Reference< XFramesSupplier > xFramesSupp( xFactory->createInstance( "com.sun.star.frame.Desktop" ), UNO_QUERY_THROW );
        Reference< XFrame > xFrame( xFramesSupp->getActiveFrame(), UNO_SET_THROW );
        Reference< XDevice > xDevice( xFrame->getContainerWindow(), UNO_QUERY_THROW );
        DeviceInfo aDeviceInfo = xDevice->getInfo();
        mfScreenPixelX = (aDeviceInfo.PixelPerMeterX > 0) ? (100000.0 / aDeviceInfo.PixelPerMeterX) : 50.0;
        mfScreenPixelY = (aDeviceInfo.PixelPerMeterY > 0) ? (100000.0 / aDeviceInfo.PixelPerMeterY) : 50.0;
    }
    catch( const Exception& e)
    {
#if OSL_DEBUG_LEVEL > 0
        OSL_FAIL( rtl::OStringBuffer("XclRootData::XclRootData - cannot get output device info: ").append(rtl::OUStringToOString(e.Message,osl_getThreadTextEncoding())).getStr() );
#else
        (void)e;
#endif
    }
}

XclRootData::~XclRootData()
{
}

// ----------------------------------------------------------------------------

XclRoot::XclRoot( XclRootData& rRootData ) :
    mrData( rRootData )
{
#if defined(DBG_UTIL) && OSL_DEBUG_LEVEL > 0
    ++mrData.mnObjCnt;
#endif

    // filter tracer
    mrData.mxTracer.reset( new XclTracer( GetDocUrl() ) );
}

XclRoot::XclRoot( const XclRoot& rRoot ) :
    mrData( rRoot.mrData )
{
#if defined(DBG_UTIL) && OSL_DEBUG_LEVEL > 0
    ++mrData.mnObjCnt;
#endif
}

XclRoot::~XclRoot()
{
#if defined(DBG_UTIL) && OSL_DEBUG_LEVEL > 0
    --mrData.mnObjCnt;
#endif
}

XclRoot& XclRoot::operator=( const XclRoot& rRoot )
{
    (void)rRoot;    // avoid compiler warning
    // allowed for assignment in derived classes - but test if the same root data is used
    OSL_ENSURE( &mrData == &rRoot.mrData, "XclRoot::operator= - incompatible root data" );
    return *this;
}

void XclRoot::SetTextEncoding( rtl_TextEncoding eTextEnc )
{
    if( eTextEnc != RTL_TEXTENCODING_DONTKNOW )
        mrData.meTextEnc = eTextEnc;
}

void XclRoot::SetCharWidth( const XclFontData& rFontData )
{
    mrData.mnCharWidth = 0;
    if( OutputDevice* pPrinter = GetPrinter() )
    {
        Font aFont( rFontData.maName, Size( 0, rFontData.mnHeight ) );
        aFont.SetFamily( rFontData.GetScFamily( GetTextEncoding() ) );
        aFont.SetCharSet( rFontData.GetFontEncoding() );
        aFont.SetWeight( rFontData.GetScWeight() );
        pPrinter->SetFont( aFont );
        mrData.mnCharWidth = pPrinter->GetTextWidth( rtl::OUString('0') );
    }
    if( mrData.mnCharWidth <= 0 )
    {
        // #i48717# Win98 with HP LaserJet returns 0
        OSL_FAIL( "XclRoot::SetCharWidth - invalid character width (no printer?)" );
        mrData.mnCharWidth = 11 * rFontData.mnHeight / 20;
    }
}

sal_Int32 XclRoot::GetHmmFromPixelX( double fPixelX ) const
{
    return static_cast< sal_Int32 >( fPixelX * mrData.mfScreenPixelX + 0.5 );
}

sal_Int32 XclRoot::GetHmmFromPixelY( double fPixelY ) const
{
    return static_cast< sal_Int32 >( fPixelY * mrData.mfScreenPixelY + 0.5 );
}

uno::Sequence< beans::NamedValue > XclRoot::RequestEncryptionData( ::comphelper::IDocPasswordVerifier& rVerifier ) const
{
    ::std::vector< OUString > aDefaultPasswords;
    aDefaultPasswords.push_back( mrData.maDefPassword );
    return ScfApiHelper::QueryEncryptionDataForMedium( mrData.mrMedium, rVerifier, &aDefaultPasswords );
}

bool XclRoot::HasVbaStorage() const
{
    SotStorageRef xRootStrg = GetRootStorage();
    return xRootStrg.Is() && xRootStrg->IsContained( EXC_STORAGE_VBA_PROJECT );
}

SotStorageRef XclRoot::OpenStorage( SotStorageRef xStrg, const String& rStrgName ) const
{
    return mrData.mbExport ?
        ScfTools::OpenStorageWrite( xStrg, rStrgName ) :
        ScfTools::OpenStorageRead( xStrg, rStrgName );
}

SotStorageRef XclRoot::OpenStorage( const String& rStrgName ) const
{
    return OpenStorage( GetRootStorage(), rStrgName );
}

SotStorageStreamRef XclRoot::OpenStream( SotStorageRef xStrg, const String& rStrmName ) const
{
    return mrData.mbExport ?
        ScfTools::OpenStorageStreamWrite( xStrg, rStrmName ) :
        ScfTools::OpenStorageStreamRead( xStrg, rStrmName );
}

SotStorageStreamRef XclRoot::OpenStream( const String& rStrmName ) const
{
    return OpenStream( GetRootStorage(), rStrmName );
}

SfxObjectShell* XclRoot::GetDocShell() const
{
    return GetDoc().GetDocumentShell();
}

ScModelObj* XclRoot::GetDocModelObj() const
{
    SfxObjectShell* pDocShell = GetDocShell();
    return pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : 0;
}

OutputDevice* XclRoot::GetPrinter() const
{
    return GetDoc().GetRefDevice();
}

ScStyleSheetPool& XclRoot::GetStyleSheetPool() const
{
    return *GetDoc().GetStyleSheetPool();
}

ScRangeName& XclRoot::GetNamedRanges() const
{
    return *GetDoc().GetRangeName();
}

SdrPage* XclRoot::GetSdrPage( SCTAB nScTab ) const
{
    return ((nScTab >= 0) && GetDoc().GetDrawLayer()) ?
        GetDoc().GetDrawLayer()->GetPage( static_cast< sal_uInt16 >( nScTab ) ) : 0;
}

SvNumberFormatter& XclRoot::GetFormatter() const
{
    return *GetDoc().GetFormatTable();
}

DateTime XclRoot::GetNullDate() const
{
    return *GetFormatter().GetNullDate();
}

sal_uInt16 XclRoot::GetBaseYear() const
{
    // return 1904 for 1904-01-01, and 1900 for 1899-12-30
    return (GetNullDate().GetYear() == 1904) ? 1904 : 1900;
}

double XclRoot::GetDoubleFromDateTime( const DateTime& rDateTime ) const
{
    double fValue = rDateTime - GetNullDate();
    // adjust dates before 1900-03-01 to get correct time values in the range [0.0,1.0)
    if( rDateTime < DateTime( Date( 1, 3, 1900 ) ) )
        fValue -= 1.0;
    return fValue;
}

DateTime XclRoot::GetDateTimeFromDouble( double fValue ) const
{
    DateTime aDateTime = GetNullDate() + fValue;
    // adjust dates before 1900-03-01 to get correct time values
    if( aDateTime < DateTime( Date( 1, 3, 1900 ) ) )
        aDateTime += 1L;
    return aDateTime;
}

ScEditEngineDefaulter& XclRoot::GetEditEngine() const
{
    if( !mrData.mxEditEngine.get() )
    {
        mrData.mxEditEngine.reset( new ScEditEngineDefaulter( GetDoc().GetEnginePool() ) );
        ScEditEngineDefaulter& rEE = *mrData.mxEditEngine;
        rEE.SetRefMapMode( MAP_100TH_MM );
        rEE.SetEditTextObjectPool( GetDoc().GetEditPool() );
        rEE.SetUpdateMode( false );
        rEE.EnableUndo( false );
        rEE.SetControlWord( rEE.GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS );
    }
    return *mrData.mxEditEngine;
}

ScHeaderEditEngine& XclRoot::GetHFEditEngine() const
{
    if( !mrData.mxHFEditEngine.get() )
    {
        mrData.mxHFEditEngine.reset( new ScHeaderEditEngine( EditEngine::CreatePool(), sal_True ) );
        ScHeaderEditEngine& rEE = *mrData.mxHFEditEngine;
        rEE.SetRefMapMode( MAP_TWIP );  // headers/footers use twips as default metric
        rEE.SetUpdateMode( false );
        rEE.EnableUndo( false );
        rEE.SetControlWord( rEE.GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS );

        // set Calc header/footer defaults
        SfxItemSet* pEditSet = new SfxItemSet( rEE.GetEmptyItemSet() );
        SfxItemSet aItemSet( *GetDoc().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );
        ScPatternAttr::FillToEditItemSet( *pEditSet, aItemSet );
        // FillToEditItemSet() adjusts font height to 1/100th mm, we need twips
        pEditSet->Put( aItemSet.Get( ATTR_FONT_HEIGHT ), EE_CHAR_FONTHEIGHT );
        pEditSet->Put( aItemSet.Get( ATTR_CJK_FONT_HEIGHT ), EE_CHAR_FONTHEIGHT_CJK );
        pEditSet->Put( aItemSet.Get( ATTR_CTL_FONT_HEIGHT ), EE_CHAR_FONTHEIGHT_CTL );
        rEE.SetDefaults( pEditSet );    // takes ownership
   }
    return *mrData.mxHFEditEngine;
}

EditEngine& XclRoot::GetDrawEditEngine() const
{
    if( !mrData.mxDrawEditEng.get() )
    {
        mrData.mxDrawEditEng.reset( new EditEngine( &GetDoc().GetDrawLayer()->GetItemPool() ) );
        EditEngine& rEE = *mrData.mxDrawEditEng;
        rEE.SetRefMapMode( MAP_100TH_MM );
        rEE.SetUpdateMode( false );
        rEE.EnableUndo( false );
        rEE.SetControlWord( rEE.GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS );
    }
    return *mrData.mxDrawEditEng;
}

XclFontPropSetHelper& XclRoot::GetFontPropSetHelper() const
{
    return *mrData.mxFontPropSetHlp;
}

XclChPropSetHelper& XclRoot::GetChartPropSetHelper() const
{
    return *mrData.mxChPropSetHlp;
}

ScExtDocOptions& XclRoot::GetExtDocOptions() const
{
    return *mrData.mxExtDocOpt;
}

XclTracer& XclRoot::GetTracer() const
{
    return *mrData.mxTracer;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
