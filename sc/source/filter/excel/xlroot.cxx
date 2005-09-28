/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlroot.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 11:53:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _EDITSTAT_HXX
#include <svx/editstat.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCDOCPOL_HXX
#include "docpool.hxx"
#endif
#ifndef SC_DOCUNO_HXX
#include "docuno.hxx"
#endif
#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif
#ifndef SC_SCEXTOPT_HXX
#include "scextopt.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif
#ifndef SC_XLCONST_HXX
#include "xlconst.hxx"
#endif
#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif

#include "root.hxx"

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }

namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XModel;

// Global data ================================================================

#ifdef DBG_UTIL
XclDebugObjCounter::~XclDebugObjCounter()
{
    DBG_ASSERT( mnObjCnt == 0, "XclDebugObjCounter::~XclDebugObjCounter - wrong root object count" );
}
#endif

// ----------------------------------------------------------------------------

XclRootData::XclRootData( XclBiff eBiff,
        SfxMedium& rMedium, SotStorageRef xRootStrg, SvStream& rBookStrm,
        ScDocument& rDoc, CharSet eCharSet, bool bExport ) :
    meBiff( eBiff ),
    mrMedium( rMedium ),
    mxRootStrg( xRootStrg ),
    mrBookStrm( rBookStrm ),
    mrDoc( rDoc ),
    meCharSet( eCharSet ),
    meSysLang( Application::GetSettings().GetLanguage() ),
    meDocLang( Application::GetSettings().GetLanguage() ),
    meUILang( Application::GetSettings().GetUILanguage() ),
    mnDefApiScript( ApiScriptType::LATIN ),
    maScMaxPos( MAXCOL, MAXROW, MAXTAB ),
    maXclMaxPos( EXC_MAXCOL2, EXC_MAXROW2, EXC_MAXTAB2 ),
    maMaxPos( EXC_MAXCOL2, EXC_MAXROW2, EXC_MAXTAB2 ),
    mnCharWidth( 110 ),
    mnScTab( 0 ),
    mbExport( bExport ),
    mbHasPassw( false ),
    mxRD( new RootData )//!
{
    // default script type, e.g. for empty cells
    switch( ScGlobal::GetDefaultScriptType() )
    {
        case SCRIPTTYPE_LATIN:      mnDefApiScript = ApiScriptType::LATIN;      break;
        case SCRIPTTYPE_ASIAN:      mnDefApiScript = ApiScriptType::ASIAN;      break;
        case SCRIPTTYPE_COMPLEX:    mnDefApiScript = ApiScriptType::COMPLEX;    break;
        default:    DBG_ERRORFILE( "XclRootData::XclRootData - unknown script type" );
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
}

XclRootData::~XclRootData()
{
}

// ----------------------------------------------------------------------------

XclRoot::XclRoot( XclRootData& rRootData ) :
    mrData( rRootData )
{
#ifdef DBG_UTIL
    ++mrData.mnObjCnt;
#endif

    // filter tracer
    // do not use CREATE_OUSTRING for conditional expression
    mrData.mxTracer.reset( new XclTracer( GetDocUrl(), OUString::createFromAscii(
        IsExport() ? "Office.Tracing/Export/Excel" : "Office.Tracing/Import/Excel" ) ) );
}

XclRoot::XclRoot( const XclRoot& rRoot ) :
    mrData( rRoot.mrData )
{
#ifdef DBG_UTIL
    ++mrData.mnObjCnt;
#endif
}

XclRoot::~XclRoot()
{
#ifdef DBG_UTIL
    --mrData.mnObjCnt;
#endif
}

XclRoot& XclRoot::operator=( const XclRoot& rRoot )
{
    // allowed for assignment in derived classes - but test if the same root data is used
    DBG_ASSERT( &mrData == &rRoot.mrData, "XclRoot::operator= - incompatible root data" );
    return *this;
}

void XclRoot::SetCharWidth( const XclFontData& rFontData )
{
    mrData.mnCharWidth = 0;
    if( SfxPrinter* pPrinter = GetPrinter() )
    {
        Font aFont( rFontData.maName, Size( 0, rFontData.mnHeight ) );
        aFont.SetFamily( rFontData.GetScFamily( GetCharSet() ) );
        aFont.SetCharSet( rFontData.GetScCharSet() );
        aFont.SetWeight( rFontData.GetScWeight() );
        pPrinter->SetFont( aFont );
        mrData.mnCharWidth = pPrinter->GetTextWidth( String( '0' ) );
    }
    if( mrData.mnCharWidth <= 0 )
    {
        // #i48717# Win98 with HP LaserJet returns 0
        DBG_ERRORFILE( "XclRoot::SetCharWidth - invalid character width (no printer?)" );
        mrData.mnCharWidth = 11 * rFontData.mnHeight / 20;
    }
}

const String& XclRoot::QueryPassword() const
{
    if( !mrData.mbHasPassw )
    {
        mrData.maPassw = ScfApiHelper::QueryPasswordForMedium( GetMedium() );
        // set to true, even if dialog has been cancelled (never ask twice)
        mrData.mbHasPassw = true;
    }
    return mrData.maPassw;
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
    return pDocShell ? ScModelObj::getImplementation( Reference< XModel >( pDocShell->GetModel() ) ) : 0;
}

SfxPrinter* XclRoot::GetPrinter() const
{
    return GetDoc().GetPrinter();
}

SvNumberFormatter& XclRoot::GetFormatter() const
{
    return *GetDoc().GetFormatTable();
}

ScStyleSheetPool& XclRoot::GetStyleSheetPool() const
{
    return *GetDoc().GetStyleSheetPool();
}

ScRangeName& XclRoot::GetNamedRanges() const
{
    return *GetDoc().GetRangeName();
}

ScDBCollection& XclRoot::GetDatabaseRanges() const
{
    return *GetDoc().GetDBCollection();
}

SdrPage* XclRoot::GetSdrPage( SCTAB nScTab ) const
{
    return GetDoc().GetDrawLayer() ?
        GetDoc().GetDrawLayer()->GetPage( static_cast< sal_uInt16 >( nScTab ) ) : 0;
}

ScEditEngineDefaulter& XclRoot::GetEditEngine() const
{
    if( !mrData.mxEditEngine.get() )
    {
        mrData.mxEditEngine.reset( new ScEditEngineDefaulter( GetDoc().GetEnginePool() ) );
        ScEditEngineDefaulter& rEE = *mrData.mxEditEngine;
        rEE.SetRefMapMode( MAP_100TH_MM );
        rEE.SetEditTextObjectPool( GetDoc().GetEditPool() );
        rEE.SetUpdateMode( FALSE );
        rEE.EnableUndo( FALSE );
        rEE.SetControlWord( rEE.GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS );
    }
    return *mrData.mxEditEngine;
}

ScHeaderEditEngine& XclRoot::GetHFEditEngine() const
{
    if( !mrData.mxHFEditEngine.get() )
    {
        mrData.mxHFEditEngine.reset( new ScHeaderEditEngine( EditEngine::CreatePool(), TRUE ) );
        ScHeaderEditEngine& rEE = *mrData.mxHFEditEngine;
        rEE.SetRefMapMode( MAP_TWIP );  // headers/footers use twips as default metric
        rEE.SetUpdateMode( FALSE );
        rEE.EnableUndo( FALSE );
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
        rEE.SetUpdateMode( FALSE );
        rEE.EnableUndo( FALSE );
        rEE.SetControlWord( rEE.GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS );
    }
    return *mrData.mxDrawEditEng;
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

