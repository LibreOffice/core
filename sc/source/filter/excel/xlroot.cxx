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

#include "xlroot.hxx"
#include <rtl/strbuf.hxx>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <svl/stritem.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/font.hxx>
#include <vcl/settings.hxx>

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

using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::DeviceInfo;
using ::com::sun::star::frame::XFrame;

using namespace ::com::sun::star;

// Global data ================================================================

#ifdef DBG_UTIL
XclDebugObjCounter::~XclDebugObjCounter()
{
    OSL_ENSURE( mnObjCnt == 0, "XclDebugObjCounter::~XclDebugObjCounter - wrong root object count" );
}
#endif

XclRootData::XclRootData( XclBiff eBiff, SfxMedium& rMedium,
        tools::SvRef<SotStorage> const & xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc, bool bExport ) :
    meBiff( eBiff ),
    meOutput( EXC_OUTPUT_BINARY ),
    mrMedium( rMedium ),
    mxRootStrg( xRootStrg ),
    mrDoc( rDoc ),
    maDefPassword( "VelvetSweatshop" ),
    meTextEnc( eTextEnc ),
    meSysLang( Application::GetSettings().GetLanguageTag().getLanguageType() ),
    meDocLang( Application::GetSettings().GetLanguageTag().getLanguageType() ),
    meUILang( Application::GetSettings().GetUILanguageTag().getLanguageType() ),
    mnDefApiScript( ApiScriptType::LATIN ),
    maScMaxPos( MAXCOL, MAXROW, MAXTAB ),
    maXclMaxPos( EXC_MAXCOL2, EXC_MAXROW2, EXC_MAXTAB2 ),
    maMaxPos( EXC_MAXCOL2, EXC_MAXROW2, EXC_MAXTAB2 ),
    mxFontPropSetHlp( new XclFontPropSetHelper ),
    mxChPropSetHlp( new XclChPropSetHelper ),
    mxRD( new RootData ),
    mfScreenPixelX( 50.0 ),
    mfScreenPixelY( 50.0 ),
    mnCharWidth( 110 ),
    mnScTab( 0 ),
    mbExport( bExport )
{
 maUserName = SvtUserOptions().GetLastName();
    if( maUserName.isEmpty() )
        maUserName = "Calc";

    switch( ScGlobal::GetDefaultScriptType() )
    {
        case SvtScriptType::LATIN:      mnDefApiScript = ApiScriptType::LATIN;      break;
        case SvtScriptType::ASIAN:      mnDefApiScript = ApiScriptType::ASIAN;      break;
        case SvtScriptType::COMPLEX:    mnDefApiScript = ApiScriptType::COMPLEX;    break;
        default:    SAL_WARN( "sc", "XclRootData::XclRootData - unknown script type" );
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
    maBasePath = maDocUrl.copy( 0, maDocUrl.lastIndexOf( '/' ) + 1 );

    // extended document options - always own object, try to copy existing data from document
    if( const ScExtDocOptions* pOldDocOpt = mrDoc.GetExtDocOptions() )
        mxExtDocOpt.reset( new ScExtDocOptions( *pOldDocOpt ) );
    else
        mxExtDocOpt.reset( new ScExtDocOptions );

    // screen pixel size
    try
    {
        Reference< frame::XDesktop2 > xFramesSupp = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        Reference< XFrame > xFrame( xFramesSupp->getActiveFrame(), UNO_SET_THROW );
        Reference< XDevice > xDevice( xFrame->getContainerWindow(), UNO_QUERY_THROW );
        DeviceInfo aDeviceInfo = xDevice->getInfo();
        mfScreenPixelX = (aDeviceInfo.PixelPerMeterX > 0) ? (100000.0 / aDeviceInfo.PixelPerMeterX) : 50.0;
        mfScreenPixelY = (aDeviceInfo.PixelPerMeterY > 0) ? (100000.0 / aDeviceInfo.PixelPerMeterY) : 50.0;
    }
    catch( const Exception& e)
    {
        SAL_WARN( "sc", "XclRootData::XclRootData - cannot get output device info: " << e.Message );
    }
}

XclRootData::~XclRootData()
{
}

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
        vcl::Font aFont( rFontData.maName, Size( 0, rFontData.mnHeight ) );
        aFont.SetFamily( rFontData.GetScFamily( GetTextEncoding() ) );
        aFont.SetCharSet( rFontData.GetFontEncoding() );
        aFont.SetWeight( rFontData.GetScWeight() );
        pPrinter->SetFont( aFont );
        // Usually digits have the same width, but in some fonts they don't ...
        // Match the import in sc/source/filter/oox/unitconverter.cxx
        // UnitConverter::finalizeImport()
        for (sal_Unicode cChar = '0'; cChar <= '9'; ++cChar)
            mrData.mnCharWidth = std::max( pPrinter->GetTextWidth( OUString(cChar)), mrData.mnCharWidth);
    }
    if( mrData.mnCharWidth <= 0 )
    {
        // #i48717# Win98 with HP LaserJet returns 0
        SAL_WARN( "sc", "XclRoot::SetCharWidth - invalid character width (no printer?)" );
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
    tools::SvRef<SotStorage> xRootStrg = GetRootStorage();
    return xRootStrg.is() && xRootStrg->IsContained( EXC_STORAGE_VBA_PROJECT );
}

tools::SvRef<SotStorage> XclRoot::OpenStorage( tools::SvRef<SotStorage> const & xStrg, const OUString& rStrgName ) const
{
    return mrData.mbExport ?
        ScfTools::OpenStorageWrite( xStrg, rStrgName ) :
        ScfTools::OpenStorageRead( xStrg, rStrgName );
}

tools::SvRef<SotStorage> XclRoot::OpenStorage( const OUString& rStrgName ) const
{
    return OpenStorage( GetRootStorage(), rStrgName );
}

tools::SvRef<SotStorageStream> XclRoot::OpenStream( tools::SvRef<SotStorage> const & xStrg, const OUString& rStrmName ) const
{
    return mrData.mbExport ?
        ScfTools::OpenStorageStreamWrite( xStrg, rStrmName ) :
        ScfTools::OpenStorageStreamRead( xStrg, rStrmName );
}

tools::SvRef<SotStorageStream> XclRoot::OpenStream( const OUString& rStrmName ) const
{
    return OpenStream( GetRootStorage(), rStrmName );
}

ScDocument& XclRoot::GetDoc() const
{
    return mrData.mrDoc;
}

ScDocument& XclRoot::GetDocRef() const
{
    return mrData.mrDoc;
}

SfxObjectShell* XclRoot::GetDocShell() const
{
    return GetDoc().GetDocumentShell();
}

ScModelObj* XclRoot::GetDocModelObj() const
{
    SfxObjectShell* pDocShell = GetDocShell();
    return pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : nullptr;
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
        GetDoc().GetDrawLayer()->GetPage( static_cast< sal_uInt16 >( nScTab ) ) : nullptr;
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
        rEE.SetRefMapMode( MapUnit::Map100thMM );
        rEE.SetEditTextObjectPool( GetDoc().GetEditPool() );
        rEE.SetUpdateMode( false );
        rEE.EnableUndo( false );
        rEE.SetControlWord( rEE.GetControlWord() & ~EEControlBits::ALLOWBIGOBJS );
    }
    return *mrData.mxEditEngine;
}

ScHeaderEditEngine& XclRoot::GetHFEditEngine() const
{
    if( !mrData.mxHFEditEngine.get() )
    {
        mrData.mxHFEditEngine.reset( new ScHeaderEditEngine( EditEngine::CreatePool() ) );
        ScHeaderEditEngine& rEE = *mrData.mxHFEditEngine;
        rEE.SetRefMapMode( MapUnit::MapTwip );  // headers/footers use twips as default metric
        rEE.SetUpdateMode( false );
        rEE.EnableUndo( false );
        rEE.SetControlWord( rEE.GetControlWord() & ~EEControlBits::ALLOWBIGOBJS );

        // set Calc header/footer defaults
        SfxItemSet* pEditSet = new SfxItemSet( rEE.GetEmptyItemSet() );
        SfxItemSet aItemSet( *GetDoc().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );
        ScPatternAttr::FillToEditItemSet( *pEditSet, aItemSet );
        // FillToEditItemSet() adjusts font height to 1/100th mm, we need twips
        std::unique_ptr<SfxPoolItem> pNewItem( aItemSet.Get( ATTR_FONT_HEIGHT ).CloneSetWhich(EE_CHAR_FONTHEIGHT));
        pEditSet->Put( *pNewItem );
        pNewItem.reset( aItemSet.Get( ATTR_CJK_FONT_HEIGHT ).CloneSetWhich(EE_CHAR_FONTHEIGHT_CJK));
        pEditSet->Put( *pNewItem );
        pNewItem.reset( aItemSet.Get( ATTR_CTL_FONT_HEIGHT ).CloneSetWhich(EE_CHAR_FONTHEIGHT_CTL));
        pEditSet->Put( *pNewItem );
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
        rEE.SetRefMapMode( MapUnit::Map100thMM );
        rEE.SetUpdateMode( false );
        rEE.EnableUndo( false );
        rEE.SetControlWord( rEE.GetControlWord() & ~EEControlBits::ALLOWBIGOBJS );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
