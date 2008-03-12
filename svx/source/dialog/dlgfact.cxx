/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgfact.cxx,v $
 *
 *  $Revision: 1.46 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:37:26 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include "dlgfact.hxx"

// class ResId
#include <tools/rc.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/app.hxx>

#include "splitcelldlg.hxx"
#include <svx/dialogs.hrc>
#include "fmresids.hrc"
#include "gallery.hrc"
#include "imapdlg.hrc"
// dialogs and tabpages
#include "dstribut.hxx"
#include "cuigaldlg.hxx"
#include "cuiimapwnd.hxx"
#include "hlmarkwn.hxx"
#include "cuicharmap.hxx"
#include "srchxtra.hxx"
#include "textanim.hxx"
#include "autocdlg.hxx"
#include "treeopt.hxx"
#include "internationaloptions.hxx"
#include "labdlg.hxx"
#include "dstribut.hxx"
#ifndef SVX_HANGUL_HANJA_DLG_HXX
#include "hangulhanjadlg.hxx" //add for HangulHanjaConversionDialog
#endif
#ifndef _SVX_SHOWCOLS_HXX
#include "showcols.hxx" //add for FmShowColsDialog
#endif
#include "zoom.hxx" //add for SvxZoomDialog
#ifndef _CUI_GALDLG_HXX_
#include "cuigaldlg.hxx"        //add for Gallery 6 Dialogs and 1 TabPage
#endif
#ifndef _CUI_IMAPWND_HXX
#include "cuiimapwnd.hxx"       //add for URLDlg
#endif
#include "hlmarkwn.hxx"     //add for SvxHlinkDlgMarkWnd
#include "srchxtra.hxx"     //add for SvxSearchFormatDialog
#include "transfrm.hxx" //add for SvxTransformTabDialog
#include "bbdlg.hxx" //add for SvxBorderBackgroundDlg
#include "cuisrchdlg.hxx" //add for SvxJSearchOptionsDialog
#include "cuitbxform.hxx" //add for FmInputRecordNoDialog
#include "optdict.hxx" //add for SvxNewDictionaryDialog
#include "dlgname.hxx" //add for SvxNameDialog & SvxMessDialog
#include "multipat.hxx" //add for SvxMultiPathDialog
#include "multifil.hxx" //add for SvxMultiFileDialog
#include "cuihyperdlg.hxx" //add for SvxHpLinkDlg
#include "cuifmsearch.hxx" //add  for FmSearchDialog
#include "cuigrfflt.hxx"    //add for GraphicFilterDialog
#include "cuitabarea.hxx" //add for SvxAreaTabDialog
#include "cuitabline.hxx" //add for SvxLineTabDialog
#include "measure.hxx" //add for SvxMeasureDialog
#include "connect.hxx" //add for SvxConnectionDialog
#include "dbregister.hxx" // add for DatabaseRegistrationDialog
#include "cuioptgenrl.hxx"  //add for SvxGeneralTabPage
#include <optasian.hxx>
#include "insdlg.hxx"
#include "pastedlg.hxx"
#include "linkdlg.hxx"
#include "SpellDialog.hxx"
#include "cfg.hxx"    //add for SvxConfigDialog

#include "numpages.hxx" // add for
#include "paragrph.hxx" //add for
#include "tabstpge.hxx" // add  for
#include "textattr.hxx" // add for SvxTextAttrPage
#include "align.hxx" //add for SvxAlignmentTabPage
#include "backgrnd.hxx" //add for SvxBackgroundTabPage
#include "border.hxx" //add for SvxBorderTabPage
#include "chardlg.hxx" //add for SvxCharNamePage,SvxCharEffectsPage,SvxCharPositionPage,SvxCharTwoLinesPage
#include "numfmt.hxx" //add for SvxNumberFormatTabPage
#include "page.hxx" //add for SvxPageDescPage
#include "postdlg.hxx" //add for SvxPostItDialog
#include "grfpage.hxx" //add for SvxGrfCropPage
#include "scriptdlg.hxx" // for ScriptOrgDialog
#include "selector.hxx" // for SvxScriptSelectorDialog
#include "macropg.hxx" // for SvxMacroAssignDlg
#include "sdrcelldlg.hxx"
#include "newtabledlg.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

using ::com::sun::star::uno::Reference;

using namespace svx;
// AbstractTabDialog implementations just forwards everything to the dialog
IMPL_ABSTDLG_BASE(VclAbstractDialog_Impl)
IMPL_ABSTDLG_BASE(VclAbstractRefreshableDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxDistributeDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractHangulHanjaConversionDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractFmShowColsDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxZoomDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSearchProgress_Impl);
IMPL_ABSTDLG_BASE(AbstractTakeProgress_Impl);
IMPL_ABSTDLG_BASE(AbstractTitleDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractScriptSelectorDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractGalleryIdDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractURLDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxHlinkDlgMarkWnd_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxCharacterMap_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxSearchSimilarityDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxTransformTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxCaptionDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxJSearchOptionsDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractFmInputRecordNoDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxNewDictionaryDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxNameDialog_Impl);

// #i68101#
IMPL_ABSTDLG_BASE(AbstractSvxObjectNameDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxObjectTitleDescDialog_Impl);

IMPL_ABSTDLG_BASE(AbstractSvxMessDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxMultiPathDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxMultiFileDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxHpLinkDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractFmSearchDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractGraphicFilterDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxAreaTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractPasteDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractInsertObjectDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractLinksDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSpellDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxPostItDialog_Impl);

//////////////////////////////////////////////////////////////////////////
// VclAbstractDialog2_Impl
//////////////////////////////////////////////////////////////////////////

// virtual
VclAbstractDialog2_Impl::~VclAbstractDialog2_Impl()
{
    delete m_pDlg;
}

// virtual
void  VclAbstractDialog2_Impl::StartExecuteModal( const Link& rEndDialogHdl )
{
    m_aEndDlgHdl = rEndDialogHdl;
    m_pDlg->StartExecuteModal(
        LINK( this, VclAbstractDialog2_Impl, EndDialogHdl ) );
}

// virtual
long VclAbstractDialog2_Impl::GetResult()
{
    return m_pDlg->GetResult();
}

IMPL_LINK( VclAbstractDialog2_Impl, EndDialogHdl, Dialog*, pDlg )
{
    if ( pDlg != m_pDlg )
    {
        DBG_ERRORFILE( "VclAbstractDialog2_Impl::EndDialogHdl(): wrong dialog" );
    }

    m_aEndDlgHdl.Call( this );
    m_aEndDlgHdl = Link();

    return 0L;
}

//////////////////////////////////////////////////////////////////////////

void AbstractTabDialog_Impl::SetCurPageId( USHORT nId )
{
    pDlg->SetCurPageId( nId );
}

const SfxItemSet* AbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//add by CHINA001
const USHORT* AbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}
//add by CHINA001
void AbstractTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractTabDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
String AbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

// AbstractTabDialog_Impl end

//for SvxDistributeDialog begin
SvxDistributeHorizontal AbstractSvxDistributeDialog_Impl::GetDistributeHor()const
{
    return pDlg->GetDistributeHor();
}
SvxDistributeVertical AbstractSvxDistributeDialog_Impl::GetDistributeVer()const
{
    return pDlg->GetDistributeVer();
}
//for SvxDistributeDialog end

// for HangulHanjaConversionDialog begin
void AbstractHangulHanjaConversionDialog_Impl::EndDialog(long nResult)
{
    pDlg->EndDialog(nResult);
}

void AbstractHangulHanjaConversionDialog_Impl::EnableRubySupport( sal_Bool _bVal )
{
    pDlg->EnableRubySupport(_bVal);
}

void AbstractHangulHanjaConversionDialog_Impl::SetByCharacter( sal_Bool _bByCharacter )
{
    pDlg->SetByCharacter(_bByCharacter);
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionDirectionState( sal_Bool _bTryBothDirections, HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection )
{
    pDlg->SetConversionDirectionState(_bTryBothDirections, _ePrimaryConversionDirection);
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionFormat( HangulHanjaConversion::ConversionFormat _eType )
{
     pDlg->SetConversionFormat(_eType);
}

void AbstractHangulHanjaConversionDialog_Impl::SetOptionsChangedHdl( const Link& _rHdl )
{
     pDlg->SetOptionsChangedHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetIgnoreHdl( const Link& _rHdl )
{
     pDlg->SetIgnoreHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetIgnoreAllHdl( const Link& _rHdl )
{
     pDlg->SetIgnoreAllHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetChangeHdl( const Link& _rHdl )
{
    pDlg->SetChangeHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetChangeAllHdl( const Link& _rHdl )
{
    pDlg->SetChangeAllHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetClickByCharacterHdl( const Link& _rHdl )
{
     pDlg->SetClickByCharacterHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionFormatChangedHdl( const Link& _rHdl )
{
    pDlg->SetConversionFormatChangedHdl(_rHdl );
}
void AbstractHangulHanjaConversionDialog_Impl::SetFindHdl( const Link& _rHdl )
{
     pDlg->SetFindHdl(_rHdl );
}

sal_Bool AbstractHangulHanjaConversionDialog_Impl::GetUseBothDirections( ) const
{
    return pDlg->GetUseBothDirections();
}

HangulHanjaConversion::ConversionDirection AbstractHangulHanjaConversionDialog_Impl::GetDirection( HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const
{
    return pDlg->GetDirection( _eDefaultDirection );
}

void AbstractHangulHanjaConversionDialog_Impl::SetCurrentString(
                    const String& _rNewString,
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument
                )
{
     pDlg->SetCurrentString(_rNewString,_rSuggestions,_bOriginatesFromDocument);
}
String   AbstractHangulHanjaConversionDialog_Impl::GetCurrentString( ) const
{
    return pDlg->GetCurrentString();
}
HangulHanjaConversion::ConversionFormat  AbstractHangulHanjaConversionDialog_Impl::GetConversionFormat( ) const
{
    return pDlg->GetConversionFormat();
}

void AbstractHangulHanjaConversionDialog_Impl::FocusSuggestion( )
{
    pDlg->FocusSuggestion();
}

String  AbstractHangulHanjaConversionDialog_Impl::GetCurrentSuggestion( ) const
{
    return pDlg->GetCurrentSuggestion();
}

Reference < com::sun::star::embed::XEmbeddedObject > AbstractInsertObjectDialog_Impl::GetObject()
{
   return pDlg->GetObject();
}

BOOL AbstractInsertObjectDialog_Impl::IsCreateNew()
{
    return pDlg->IsCreateNew();
}

::Reference< ::com::sun::star::io::XInputStream > AbstractInsertObjectDialog_Impl::GetIconIfIconified( ::rtl::OUString* pGraphicMediaType )
{
   return pDlg->GetIconIfIconified( pGraphicMediaType );
}

void AbstractPasteDialog_Impl::Insert( SotFormatStringId nFormat, const String & rFormatName )
{
    pDlg->Insert( nFormat, rFormatName );
}

void AbstractPasteDialog_Impl::SetObjName( const SvGlobalName & rClass, const String & rObjName )
{
    pDlg->SetObjName( rClass, rObjName );
}

ULONG AbstractPasteDialog_Impl::GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats,
                        const TransferableObjectDescriptor* pDesc )
{
    return pDlg->GetFormat( aHelper, pFormats, pDesc );
}

// for HangulHanjaConversionDialog end
// for FmShowColsDialog begin
void  AbstractFmShowColsDialog_Impl::SetColumns(const ::Reference< ::com::sun::star::container::XIndexContainer>& xCols)
{
     pDlg->SetColumns(xCols);
}

//for FmShowColsDialog end

//for SvxZoomDialog begin
void AbstractSvxZoomDialog_Impl::SetLimits( USHORT nMin, USHORT nMax )
{
    pDlg->SetLimits( nMin, nMax );
}

void AbstractSvxZoomDialog_Impl::HideButton( USHORT nBtnId )
{
    pDlg->HideButton( nBtnId );
}

const SfxItemSet* AbstractSvxZoomDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

//for SvxZoomDialog end
// svx::SvxSpellDialog
/*-- 04.05.2004 08:26:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void AbstractSpellDialog_Impl::SetLanguage( sal_uInt16 nLang )
{
    pDlg->SetLanguage(nLang);
}
/*-- 04.05.2004 08:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool AbstractSpellDialog_Impl::Close()
{
    return pDlg->Close();
}
/*-- 04.05.2004 08:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void  AbstractSpellDialog_Impl::Invalidate()
{
    pDlg->InvalidateDialog();
}
/*-- 04.05.2004 08:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
Window*     AbstractSpellDialog_Impl::GetWindow()
{
    return pDlg;
}
/*-- 04.05.2004 08:26:13---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxBindings& AbstractSpellDialog_Impl::GetBindings()
{
    return pDlg->GetBindings();
}
// end svx::SvxSpellDialog

//for SvxSpellCheckDialog begin
//STRIP001 void AbstractSvxSpellCheckDialog_Impl::SetNewEditWord( const String& _rNew )
//STRIP001 {
//STRIP001 return pDlg->SetNewEditWord(_rNew );
//STRIP001 }
//STRIP001
//STRIP001 void AbstractSvxSpellCheckDialog_Impl::SetLanguage( sal_uInt16 nLang )
//STRIP001 {
//STRIP001 return pDlg->SetLanguage( nLang );
//STRIP001 }
//STRIP001
//STRIP001 void AbstractSvxSpellCheckDialog_Impl::HideAutoCorrect()
//STRIP001 {
//STRIP001 return pDlg->HideAutoCorrect();
//STRIP001 }
//STRIP001
//STRIP001 String AbstractSvxSpellCheckDialog_Impl::GetNewEditWord()
//STRIP001 {
//STRIP001 return pDlg->GetNewEditWord();
//STRIP001 }
//STRIP001
//STRIP001 void AbstractSvxSpellCheckDialog_Impl::SetNewEditWord( const String& _rNew )
//STRIP001 {
//STRIP001 return pDlg->SetNewEditWord( _rNew );
//STRIP001 }
//STRIP001
//for SvxSpellCheckDialog end

//for SearchProgress begin
void AbstractSearchProgress_Impl::Update()
{
     pDlg->Update();
}

void AbstractSearchProgress_Impl::Sync()
{
     pDlg-> Sync();
}

void AbstractSearchProgress_Impl::SetFileType( const String& rType )
{
     pDlg->SetFileType( rType );
}

void AbstractSearchProgress_Impl::SetDirectory( const INetURLObject& rURL )
{
     pDlg->SetDirectory( rURL );
}
PLinkStub   AbstractSearchProgress_Impl::GetLinkStubCleanUpHdl()
{
    return SearchProgress::LinkStubCleanUpHdl;
}
//for SearchProgress end

//for TakeProgress begin
void AbstractTakeProgress_Impl::Update()
{
     pDlg->Update();
}

void AbstractTakeProgress_Impl::Sync()
{
     pDlg-> Sync();
}

void AbstractTakeProgress_Impl::SetFile( const INetURLObject& rURL )
{
     pDlg->SetFile( rURL );
}

PLinkStub   AbstractTakeProgress_Impl::GetLinkStubCleanUpHdl()
{
    return TakeProgress::LinkStubCleanUpHdl;
}
//for TakeProgress end

//for TitleDialog begin
String AbstractTitleDialog_Impl::GetTitle() const
{
    return pDlg->GetTitle();
}
//for TitleDialog end

//for GalleryIdDialog begin
ULONG AbstractGalleryIdDialog_Impl::GetId() const
{
    return pDlg->GetId();
}
//for GalleryIdDialog end

//for VclAbstracRefreshableDialog begin
void VclAbstractRefreshableDialog_Impl::Update()
{
    pDlg->Update();
}

void VclAbstractRefreshableDialog_Impl::Sync()
{
     pDlg-> Sync();
}
//for VclAbstracRefreshableDialog end

//for URLDlg begin
String AbstractURLDlg_Impl::GetURL() const
{
    return pDlg->GetURL();
}

String AbstractURLDlg_Impl::GetAltText() const
{
    return pDlg->GetAltText();
}

String AbstractURLDlg_Impl::GetDesc() const
{
    return pDlg->GetDesc();
}

String AbstractURLDlg_Impl::GetTarget() const
{
    return pDlg->GetTarget();
}

String AbstractURLDlg_Impl::GetName() const
{
    return pDlg->GetName();
}
//for URLDlg end

// AbstractSvxHlinkDlgMarkWnd_Impl begin

void AbstractSvxHlinkDlgMarkWnd_Impl::Hide( USHORT nFlags )
{
    ((Window*)pDlg)->Hide( nFlags );
}

void AbstractSvxHlinkDlgMarkWnd_Impl::SetSizePixel( const Size& rNewSize )
{
    pDlg->SetSizePixel( rNewSize );
}

Size AbstractSvxHlinkDlgMarkWnd_Impl::GetSizePixel() const
{
    return pDlg->GetSizePixel();
}

BOOL AbstractSvxHlinkDlgMarkWnd_Impl::IsVisible( ) const
{
    return (( Window* )pDlg)->IsVisible();
}

void AbstractSvxHlinkDlgMarkWnd_Impl::Invalidate( USHORT nFlags )
{
    (( Window* )pDlg)->Invalidate(nFlags);
}

BOOL AbstractSvxHlinkDlgMarkWnd_Impl::MoveTo( Point aNewPos )const
{
    return pDlg->MoveTo(aNewPos);
}

BOOL AbstractSvxHlinkDlgMarkWnd_Impl::ConnectToDialog( BOOL bDoit  )const
{
    return pDlg->ConnectToDialog(bDoit);
}

void AbstractSvxHlinkDlgMarkWnd_Impl::RefreshTree ( String aStrURL )
{
    pDlg->RefreshTree(aStrURL);
}

void AbstractSvxHlinkDlgMarkWnd_Impl::SelectEntry ( String aStrMark )
{
    pDlg->SelectEntry(aStrMark);
}

USHORT AbstractSvxHlinkDlgMarkWnd_Impl::SetError( USHORT nError)
{
    return pDlg->SetError(nError);
}

// AbstractSvxHlinkDlgMarkWnd_Impl end

// AbstractSvxCharacterMap_Impl begin
void AbstractSvxCharacterMap_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}

void AbstractSvxCharacterMap_Impl::DisableFontSelection()
{
    pDlg->DisableFontSelection();
}

const Font&  AbstractSvxCharacterMap_Impl::GetCharFont() const
{
    return pDlg->GetCharFont();
}

void AbstractSvxCharacterMap_Impl::SetCharFont( const Font& rFont )
{
    pDlg->SetCharFont(  rFont );
}

void AbstractSvxCharacterMap_Impl::SetFont( const Font& rFont )
{
    pDlg->SetFont(  rFont );
}

void AbstractSvxCharacterMap_Impl::SetChar( sal_Unicode c )
{
    pDlg->SetChar(  c );
}

sal_Unicode AbstractSvxCharacterMap_Impl::GetChar() const
{
    sal_UCS4 cUCS4 = pDlg->GetChar();
    // TODO: change return value to UCS4
    return static_cast<sal_Unicode>(cUCS4);
}

String AbstractSvxCharacterMap_Impl::GetCharacters() const
{
    return pDlg->GetCharacters();
}

// AbstractSvxCharacterMap_Impl end

//for SvxSearchSimilarityDialog begin
USHORT AbstractSvxSearchSimilarityDialog_Impl::GetOther()
{
    return pDlg->GetOther();
}

USHORT AbstractSvxSearchSimilarityDialog_Impl::GetShorter()
{
    return pDlg->GetShorter();
}

USHORT AbstractSvxSearchSimilarityDialog_Impl::GetLonger()
{
    return pDlg-> GetLonger();
}

BOOL AbstractSvxSearchSimilarityDialog_Impl::IsRelaxed()
{
    return pDlg-> IsRelaxed();
}
//for SvxSearchSimilarityDialog end

// AbstractSvxTransformTabDialog implementations just forwards everything to the dialog
void AbstractSvxTransformTabDialog_Impl::SetCurPageId( USHORT nId )
{
    pDlg->SetCurPageId( nId );
}
const SfxItemSet* AbstractSvxTransformTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//
const USHORT* AbstractSvxTransformTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}
//
void AbstractSvxTransformTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractSvxTransformTabDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
String AbstractSvxTransformTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}
void AbstractSvxTransformTabDialog_Impl::SetValidateFramePosLink( const Link& rLink )
{
    pDlg->SetValidateFramePosLink( rLink );
}
// AbstractSvxTransformTabDialog end

// AbstractSvxCaptionDialog implementations just forwards everything to the dialog
void AbstractSvxCaptionDialog_Impl::SetCurPageId( USHORT nId )
{
    pDlg->SetCurPageId( nId );
}
const SfxItemSet* AbstractSvxCaptionDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//
const USHORT* AbstractSvxCaptionDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}
//
void AbstractSvxCaptionDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractSvxCaptionDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
String AbstractSvxCaptionDialog_Impl::GetText() const
{
    return pDlg->GetText();
}
void AbstractSvxCaptionDialog_Impl::SetValidateFramePosLink( const Link& rLink )
{
    pDlg->SetValidateFramePosLink( rLink );
}
// SvxAbstractCaptionDialog end

//for SvxJSearchOptionsDialog begin
INT32 AbstractSvxJSearchOptionsDialog_Impl::GetTransliterationFlags() const
{
    return pDlg->GetTransliterationFlags();
}
//for SvxJSearchOptionsDialog end

//for FmInputRecordNoDialog begin
void AbstractFmInputRecordNoDialog_Impl::SetValue(long nNew)
{
    pDlg->SetValue(nNew);
}

long AbstractFmInputRecordNoDialog_Impl::GetValue() const
{
    return pDlg->GetValue();
}
//for FmInputRecordNoDialog end

//for SvxNewDictionaryDialog begin
::Reference<
        ::com::sun::star::linguistic2::XDictionary1 > AbstractSvxNewDictionaryDialog_Impl::GetNewDictionary()
{
    return pDlg->GetNewDictionary();
}
//for SvxNewDictionaryDialog end

//for SvxNameDialog begin
void AbstractSvxNameDialog_Impl::GetName( String& rName )
{
    pDlg->GetName( rName );
}
void AbstractSvxNameDialog_Impl::SetCheckNameHdl( const Link& rLink, bool bCheckImmediately )
{
    aCheckNameHdl = rLink;
    if( rLink.IsSet() )
        pDlg->SetCheckNameHdl( LINK(this, AbstractSvxNameDialog_Impl, CheckNameHdl), bCheckImmediately );
    else
        pDlg->SetCheckNameHdl( Link(), bCheckImmediately );
}
void AbstractSvxNameDialog_Impl::SetEditHelpId(ULONG nHelpId)
{
    pDlg->SetEditHelpId( nHelpId );
}
void AbstractSvxNameDialog_Impl::SetHelpId( ULONG nHelpId )
{
    pDlg->SetHelpId( nHelpId );
}
void AbstractSvxNameDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
IMPL_LINK( AbstractSvxNameDialog_Impl, CheckNameHdl, Window*, EMPTYARG )
{
    if( aCheckNameHdl.IsSet() )
        return aCheckNameHdl.Call(this);
    return 0;
}
//for SvxNameDialog end

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#

void AbstractSvxObjectNameDialog_Impl::GetName(String& rName)
{
    pDlg->GetName(rName);
}

void AbstractSvxObjectNameDialog_Impl::SetCheckNameHdl(const Link& rLink, bool bCheckImmediately)
{
    aCheckNameHdl = rLink;

    if(rLink.IsSet())
    {
        pDlg->SetCheckNameHdl(LINK(this, AbstractSvxObjectNameDialog_Impl, CheckNameHdl), bCheckImmediately);
    }
    else
    {
        pDlg->SetCheckNameHdl(Link(), bCheckImmediately);
    }
}

IMPL_LINK(AbstractSvxObjectNameDialog_Impl, CheckNameHdl, Window*, EMPTYARG)
{
    if(aCheckNameHdl.IsSet())
    {
        return aCheckNameHdl.Call(this);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#

void AbstractSvxObjectTitleDescDialog_Impl::GetTitle(String& rTitle)
{
    pDlg->GetTitle(rTitle);
}

void AbstractSvxObjectTitleDescDialog_Impl::GetDescription(String& rDescription)
{
    pDlg->GetDescription(rDescription);
}

///////////////////////////////////////////////////////////////////////////////////////////////

//for SvxMessDialog begin
void AbstractSvxMessDialog_Impl::SetButtonText( USHORT nBtnId, const String& rNewTxt )
{
    pDlg->SetButtonText( nBtnId, rNewTxt );
}
//for SvxMessDialog end

//for SvxMultiPathDialog begin
String AbstractSvxMultiPathDialog_Impl::GetPath() const
{
    return pDlg->GetPath();
}

void AbstractSvxMultiPathDialog_Impl::SetPath( const String& rPath )
{
    pDlg->SetPath( rPath );
}

void AbstractSvxMultiPathDialog_Impl::EnableRadioButtonMode()
{
    pDlg->EnableRadioButtonMode();
}

void AbstractSvxMultiPathDialog_Impl::SetTitle( const String& rNewTitle )
{
    pDlg->SetText( rNewTitle );
}
//for SvxMultiPathDialog end

//for SvxMultiFileDialog begin
String AbstractSvxMultiFileDialog_Impl::GetPath() const
{
    return pDlg->GetPath();
}

void AbstractSvxMultiFileDialog_Impl::SetPath( const String& rPath )
{
    pDlg->SetPath( rPath );
}

String AbstractSvxMultiFileDialog_Impl::GetFiles() const
{
    return pDlg->GetFiles();
}

void AbstractSvxMultiFileDialog_Impl::SetFiles( const String& rPath )
{
    pDlg->SetFiles( rPath );
}

void AbstractSvxMultiFileDialog_Impl::SetClassPathMode()
{
    pDlg->SetClassPathMode();
}

void AbstractSvxMultiFileDialog_Impl::EnableRadioButtonMode()
{
    pDlg->EnableRadioButtonMode();
}

void AbstractSvxMultiFileDialog_Impl::SetTitle( const String& rNewTitle )
{
    pDlg->SetText( rNewTitle );
}

void AbstractSvxMultiFileDialog_Impl::SetHelpId( ULONG nHelpId )
{
    pDlg->SetHelpId( nHelpId );
}
//for SvxMultiFileDialog end

//for SvxHpLinkDlg begin
Window * AbstractSvxHpLinkDlg_Impl::GetWindow()
{
    return (Window *)pDlg;
}

sal_Bool AbstractSvxHpLinkDlg_Impl::QueryClose()
{
    return pDlg->QueryClose();
}
//for SvxHpLinkDlg end


//for FmSearchDialog begin
void AbstractFmSearchDialog_Impl::SetFoundHandler(const Link& lnk)
{
    pDlg->SetFoundHandler(lnk);
}
void AbstractFmSearchDialog_Impl::SetCanceledNotFoundHdl(const Link& lnk)
{
    pDlg->SetCanceledNotFoundHdl(lnk);
}
void AbstractFmSearchDialog_Impl::SetActiveField(const String& strField)
{
    pDlg->SetActiveField(strField);
}
//for FmSearchDialog end

//for GraphicFilterDialog begin
Graphic AbstractGraphicFilterDialog_Impl::GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY )
{
    return pDlg->GetFilteredGraphic( rGraphic, fScaleX, fScaleY );
}
//for GraphicFilterDialog end

// AbstractSvxAreaTabDialog implementations just forwards everything to the dialog
void AbstractSvxAreaTabDialog_Impl::SetCurPageId( USHORT nId )
{
    pDlg->SetCurPageId( nId );
}

const SfxItemSet* AbstractSvxAreaTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//add by CHINA001
const USHORT* AbstractSvxAreaTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}
//add by CHINA001
void AbstractSvxAreaTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractSvxAreaTabDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
String AbstractSvxAreaTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

void AbstractSvxAreaTabDialog_Impl::DontDeleteColorTable()
{
    return pDlg->DontDeleteColorTable();
}
// AbstractSvxAreaTabDialog_Impl end

// add SvxMeasureDialog & SvxConnectionDialog begin
const SfxItemSet* AbstractSfxSingleTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
// SvxMeasureDialog & SvxConnectionDialog end

// add SvxPostItDialog begin
void AbstractSvxPostItDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
const SfxItemSet* AbstractSvxPostItDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
void AbstractSvxPostItDialog_Impl::EnableTravel(BOOL bNext, BOOL bPrev)
{
    pDlg->EnableTravel( bNext, bPrev );
}
String AbstractSvxPostItDialog_Impl::GetNote()
{
    return pDlg->GetNote();
}
void AbstractSvxPostItDialog_Impl::SetNote(const String& rTxt)
{
    pDlg->SetNote( rTxt );
}
void AbstractSvxPostItDialog_Impl::ShowLastAuthor(const String& rAuthor, const String& rDate)
{
    pDlg->ShowLastAuthor( rAuthor, rDate );
}
void AbstractSvxPostItDialog_Impl::DontChangeAuthor()
{
    pDlg->DontChangeAuthor();
}
void AbstractSvxPostItDialog_Impl::HideAuthor()
{
    pDlg->HideAuthor();
}
void AbstractSvxPostItDialog_Impl::SetReadonlyPostIt(BOOL bDisable)
{
    pDlg->SetReadonlyPostIt( bDisable );
}
BOOL AbstractSvxPostItDialog_Impl::IsOkEnabled() const
{
    return pDlg->IsOkEnabled();
}
void AbstractSvxPostItDialog_Impl::SetNextHdl( const Link& rLink )
{
    aNextHdl = rLink;
    if( rLink.IsSet() )
        pDlg->SetNextHdl( LINK(this, AbstractSvxPostItDialog_Impl, NextHdl ) );
    else
        pDlg->SetNextHdl( Link() );
}
void AbstractSvxPostItDialog_Impl::SetPrevHdl( const Link& rLink )
{
    aPrevHdl = rLink;
    if( rLink.IsSet() )
        pDlg->SetPrevHdl( LINK(this, AbstractSvxPostItDialog_Impl, PrevHdl ) );
    else
        pDlg->SetPrevHdl( Link() );
}
IMPL_LINK( AbstractSvxPostItDialog_Impl, NextHdl, Window*, EMPTYARG )
{
    if( aNextHdl.IsSet() )
        aNextHdl.Call(this);
    return 0;
}
IMPL_LINK( AbstractSvxPostItDialog_Impl, PrevHdl, Window*, EMPTYARG )
{
    if( aPrevHdl.IsSet() )
        aPrevHdl.Call(this);
    return 0;
}
Window * AbstractSvxPostItDialog_Impl::GetWindow()
{
    return (Window *)pDlg;
}

// SvxPostItDialog end

//--------------------------------------------------------------
// Create dialogs with simplest interface
VclAbstractDialog* AbstractDialogFactory_Impl::CreateVclDialog( Window* pParent, sal_uInt32 nResId )
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case SID_OPTIONS_TREEDIALOG :
        case SID_OPTIONS_DATABASES :
        case SID_LANGUAGE_OPTIONS :
        {
            bool bActivateLastSelection = false;
            if (nResId == SID_OPTIONS_TREEDIALOG)
                bActivateLastSelection = true;
            Reference< frame::XFrame > xFrame;
            OfaTreeOptionsDialog* pOptDlg = new OfaTreeOptionsDialog( pParent, xFrame, bActivateLastSelection );
            if (nResId == SID_OPTIONS_DATABASES)
            {
                pOptDlg->ActivatePage(SID_SB_DBREGISTEROPTIONS);
            }
            else if (nResId == SID_LANGUAGE_OPTIONS)
            {
                //open the tab page "tools/options/languages"
                pOptDlg->ActivatePage(OFA_TP_LANGUAGES_FOR_SET_DOCUMENT_LANGUAGE);
            }
            pDlg = pOptDlg;
        }
        break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

// dialogs that use SfxBindings
VclAbstractDialog* AbstractDialogFactory_Impl::CreateSfxDialog( Window* /*pParent*/, const SfxBindings&, sal_uInt32 )
{
    return 0;
}

VclAbstractDialog* AbstractDialogFactory_Impl::CreateFrameDialog(
    Window* pParent, const Reference< frame::XFrame >& rxFrame,
    sal_uInt32 nResId, const String& rParameter )
{
    Dialog* pDlg = NULL;
    if ( SID_OPTIONS_TREEDIALOG == nResId || SID_OPTIONS_DATABASES == nResId )
    {
        // only activate last page if we dont want to activate a special page
        bool bActivateLastSelection = ( nResId != SID_OPTIONS_DATABASES && rParameter.Len() == 0 );
        OfaTreeOptionsDialog* pOptDlg = new OfaTreeOptionsDialog( pParent, rxFrame, bActivateLastSelection );
        if ( nResId == SID_OPTIONS_DATABASES )
            pOptDlg->ActivatePage(SID_SB_DBREGISTEROPTIONS);
        else if ( rParameter.Len() > 0 )
            pOptDlg->ActivatePage( rParameter );
        pDlg = pOptDlg;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    else
        return NULL;
}

// TabDialog outside the drawing layer
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTabDialog( sal_uInt32 nResId,
                                                Window* pParent,
                                                const SfxItemSet* pAttrSet,
                                                SfxViewFrame* ,
                                                bool /*bEditFmt*/,
                                                const String * )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_OFA_AUTOCORR_DLG :
            pDlg = new OfaAutoCorrDlg( pParent, pAttrSet );
            break;
        case RID_SVXDLG_CUSTOMIZE :
            pDlg = new SvxConfigDialog( pParent, pAttrSet );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTabDialog( sal_uInt32 nResId,
                                                Window* pParent,
                                                const SfxItemSet* pAttrSet,
                                                const Reference< frame::XFrame >& xViewFrame,
                                                bool /*bEditFmt*/,
                                                const String * )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_OFA_AUTOCORR_DLG :
            pDlg = new OfaAutoCorrDlg( pParent, pAttrSet );
            break;
        case RID_SVXDLG_CUSTOMIZE :
            {
                SvxConfigDialog* pDlg1 = new SvxConfigDialog( pParent, pAttrSet );
                pDlg1->SetFrame(xViewFrame);
                pDlg = (SfxTabDialog*)pDlg1;
            }
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}

// TabDialog that use functionality of the drawing layer
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTextTabDialog( Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            sal_uInt32 nResId,
                                            SdrView* pView,
                                            SdrModel* )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_TEXT :
            pDlg = new SvxTextTabDialog( pParent, pAttrSet, pView );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}

//TabDialog that use functionality of the drawing layer and add AnchorTypes  -- for SvxCaptionTabDialog CHINA001
AbstractSvxCaptionDialog*       AbstractDialogFactory_Impl::CreateCaptionDialog( Window* pParent,
                                            const SdrView* pView,
                                            sal_uInt32 nResId,
                                            USHORT nAnchorTypes )
{
    SvxCaptionTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_CAPTION :
            pDlg = new SvxCaptionTabDialog( pParent, pView, nAnchorTypes );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxCaptionDialog_Impl( pDlg );
    return 0;
}

AbstractSvxDistributeDialog*    AbstractDialogFactory_Impl::CreateSvxDistributeDialog(Window* pParent,
                                            const SfxItemSet& rAttr,
                                            sal_uInt32 nResId,
                                            SvxDistributeHorizontal eHor ,
                                            SvxDistributeVertical eVer)
{
    SvxDistributeDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXPAGE_DISTRIBUTE :
            pDlg = new SvxDistributeDialog( pParent, rAttr, eHor, eVer);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxDistributeDialog_Impl( pDlg );
    return 0;
}

AbstractHangulHanjaConversionDialog* AbstractDialogFactory_Impl::CreateHangulHanjaConversionDialog(Window* pParent,
                                                                                                   HangulHanjaConversion::ConversionDirection _ePrimaryDirection,
                                                                                                    sal_uInt32 nResId)
{
    HangulHanjaConversionDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVX_MDLG_HANGULHANJA :
            pDlg = new HangulHanjaConversionDialog( pParent, _ePrimaryDirection);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractHangulHanjaConversionDialog_Impl( pDlg );
    return 0;
}

AbstractFmShowColsDialog * AbstractDialogFactory_Impl::CreateFmShowColsDialog( Window* pParent,  //add for FmShowColsDialog
                                             sal_uInt32 nResId)
{
   FmShowColsDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVX_DLG_SHOWGRIDCOLUMNS :
            pDlg = new FmShowColsDialog( pParent);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractFmShowColsDialog_Impl( pDlg );
    return 0;
}
AbstractSvxZoomDialog * AbstractDialogFactory_Impl::CreateSvxZoomDialog( Window* pParent,  //add for SvxZoomDialog
                                            const SfxItemSet& rCoreSet,
                                            sal_uInt32 nResId)
{
   SvxZoomDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_ZOOM :
            pDlg = new SvxZoomDialog( pParent, rCoreSet);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxZoomDialog_Impl( pDlg );
    return 0;
}

/*-- 04.05.2004 08:18:17---------------------------------------------------

  -----------------------------------------------------------------------*/
AbstractSpellDialog *  AbstractDialogFactory_Impl::CreateSvxSpellDialog(
                        Window* pParent,
                        SfxBindings* pBindings,
                        svx::SpellDialogChildWindow* pSpellChildWindow )
{
    svx::SpellDialog* pDlg = new svx::SpellDialog(pSpellChildWindow, pParent, pBindings);
    return new AbstractSpellDialog_Impl(pDlg);
}

//CHINA001 GalleryDialog start
VclAbstractRefreshableDialog * AbstractDialogFactory_Impl::CreateActualizeProgressDialog( Window* pParent,  //add for ActualizeProgress
                                            GalleryTheme* pThm,
                                            sal_uInt32 nResId)
{
   Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_GALLERY_ACTUALIZE_PROGRESS :
            pDlg = new ActualizeProgress( pParent, pThm);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractRefreshableDialog_Impl( pDlg );
    return 0;
}

AbstractSearchProgress * AbstractDialogFactory_Impl::CreateSearchProgressDialog( Window* pParent,  //add for SearchProgress
                                            const INetURLObject& rStartURL,
                                            sal_uInt32 nResId)
{
   SearchProgress* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_GALLERY_SEARCH_PROGRESS :
            pDlg = new SearchProgress( pParent, rStartURL);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSearchProgress_Impl( pDlg );
    return 0;
}

AbstractTakeProgress * AbstractDialogFactory_Impl::CreateTakeProgressDialog( Window* pParent,  //add for TakeProgress
                                            sal_uInt32 nResId)
{
   TakeProgress* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_GALLERY_TAKE_PROGRESS :
            pDlg = new TakeProgress( pParent );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTakeProgress_Impl( pDlg );
    return 0;
}

VclAbstractDialog*
AbstractDialogFactory_Impl::CreateScriptErrorDialog(
    Window* pParent, ::com::sun::star::uno::Any aException )
{
    return new SvxScriptErrorDialog( pParent, aException );
}

AbstractScriptSelectorDialog*
AbstractDialogFactory_Impl::CreateScriptSelectorDialog(
    Window* pParent, BOOL bShowSlots, const Reference< frame::XFrame >& _rxFrame )
{
    SvxScriptSelectorDialog* pDlg = NULL;

    pDlg = new SvxScriptSelectorDialog( pParent, bShowSlots, _rxFrame );

    if (pDlg)
    {
        return new AbstractScriptSelectorDialog_Impl( pDlg );
    }
    return 0;
}

String AbstractScriptSelectorDialog_Impl::GetScriptURL() const
{
    if (pDlg)
        return pDlg->GetScriptURL();
    return String();
}

void AbstractScriptSelectorDialog_Impl::SetRunLabel()
{
    if (pDlg)
        pDlg->SetRunLabel();
    return;
}

VclAbstractDialog * AbstractDialogFactory_Impl::CreateSvxScriptOrgDialog( Window* pParent,  //add for SvxScriptOrgDialog
                                            const String& rLanguage)
{
    Dialog* pDlg=NULL;

    pDlg = new SvxScriptOrgDialog( pParent, rLanguage);

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

AbstractTitleDialog * AbstractDialogFactory_Impl::CreateTitleDialog( Window* pParent,  //add for TitleDialog
                                            const String& rOldText,
                                            sal_uInt32 nResId)
{
   TitleDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_GALLERY_TITLE :
            pDlg = new TitleDialog( pParent, rOldText);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTitleDialog_Impl( pDlg );
    return 0;
}

AbstractGalleryIdDialog * AbstractDialogFactory_Impl::CreateGalleryIdDialog( Window* pParent,  //add for SvxZoomDialog
                                            GalleryTheme* pThm,
                                            sal_uInt32 nResId)
{
   GalleryIdDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_GALLERY_THEMEID :
            pDlg = new GalleryIdDialog( pParent, pThm);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractGalleryIdDialog_Impl( pDlg );
    return 0;
}

VclAbstractDialog2 * AbstractDialogFactory_Impl::CreateGalleryThemePropertiesDialog( Window* pParent,  //add for GalleryThemeProperties
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet,
                                            sal_uInt32 nResId)
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXTABDLG_GALLERYTHEME :
            pDlg = new GalleryThemeProperties( pParent, pData, pItemSet);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog2_Impl( pDlg );
    return 0;
}
//CHINA001 GalleryDialog end

AbstractURLDlg * AbstractDialogFactory_Impl::CreateURLDialog( Window* pParent,  //add for URLDlg
                                            const String& rURL, const String& rAltText, const String& rDescription,
                                            const String& rTarget, const String& rName,
                                            TargetList& rTargetList,
                                            sal_uInt32 nResId)
{
    URLDlg* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_IMAPURL :
            pDlg = new URLDlg( pParent, rURL, rAltText, rDescription, rTarget, rName, rTargetList);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractURLDlg_Impl( pDlg );
    return 0;
}
//CHINA001 URLDlg end

//CHINA001 SvxHlinkDlgMarkWnd begin
AbstractSvxHlinkDlgMarkWnd* AbstractDialogFactory_Impl::CreateSvxHlinkDlgMarkWndDialog( SvxHyperlinkTabPageBase* pParent, sal_uInt32 nResId )
{
    SvxHlinkDlgMarkWnd* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXFLOAT_HYPERLINK_MARKWND :
            pDlg = new  SvxHlinkDlgMarkWnd( pParent );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxHlinkDlgMarkWnd_Impl( pDlg );
    return 0;
}
//CHINA001 SvxHlinkDlgMarkWnd end

//CHINA001  SvxCharacterMap begin
AbstractSvxCharacterMap * AbstractDialogFactory_Impl::CreateSvxCharacterMap( Window* pParent,  //add for SvxCharacterMap
                                                            sal_uInt32 nResId,
                                                            BOOL bOne  )
{
    SvxCharacterMap* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_CHARMAP :
            pDlg = new  SvxCharacterMap( pParent,bOne );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxCharacterMap_Impl( pDlg );
    return 0;
}
//CHINA001  SvxCharacterMap end

//CHINA001  SvxSearchFormatDialog begin
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTabItemDialog( Window* pParent,
                                            const SfxItemSet& rSet,
                                            sal_uInt32 nResId)
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_SEARCHFORMAT :
            pDlg = new SvxSearchFormatDialog( pParent, rSet );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxSearchFormatDialog end

//CHINA001  SvxSearchAttributeDialog begin
VclAbstractDialog*      AbstractDialogFactory_Impl::CreateSvxSearchAttributeDialog( Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const USHORT* pWhRanges,
                                            sal_uInt32 nResId ) //add for SvxSearchAttributeDialog
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_SEARCHATTR :
            pDlg = new SvxSearchAttributeDialog( pParent, rLst, pWhRanges);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxSearchAttributeDialog end

//CHINA001  SvxSearchSimilarityDialog begin
AbstractSvxSearchSimilarityDialog * AbstractDialogFactory_Impl::CreateSvxSearchSimilarityDialog( Window* pParent,
                                                            BOOL bRelax,
                                                            USHORT nOther,
                                                            USHORT nShorter,
                                                            USHORT nLonger)
{
    SvxSearchSimilarityDialog* pDlg = new SvxSearchSimilarityDialog( pParent, bRelax, nOther, nShorter, nLonger );
    if ( pDlg )
        return new AbstractSvxSearchSimilarityDialog_Impl( pDlg );
    return NULL;
}
//CHINA001  SvxSearchSimilarityDialog end

//CHINA001  SvxBorderBackgroundDlg begin
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxBorderBackgroundDlg( Window* pParent,
                                            const SfxItemSet& rCoreSet,
                                            sal_uInt32 nResId,
                                            BOOL bEnableSelector)
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_BBDLG :
            pDlg = new SvxBorderBackgroundDlg( pParent, rCoreSet, bEnableSelector);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxBorderBackgroundDlg end

//CHINA001  SvxTransformTabDialog begin
AbstractSvxTransformTabDialog* AbstractDialogFactory_Impl::CreateSvxTransformTabDialog( Window* pParent,
                                                                              const SfxItemSet* pAttr,
                                                                                const SdrView* pView,
                                                                                sal_uInt32 nResId,
                                                                                USHORT nAnchorTypes )
{
    SvxTransformTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_TRANSFORM :
            pDlg = new SvxTransformTabDialog( pParent, pAttr,pView, nAnchorTypes);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxTransformTabDialog_Impl( pDlg );
    return 0;
}

//CHINA001  SvxTransformTabDialog end

//CHINA001  SchTransformTabDialog begin
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSchTransformTabDialog( Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                sal_uInt32 nResId,
                                                                bool bSizeTabPage
                                                                )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SCH_TransformTabDLG_SVXPAGE_ANGLE :
            {
            pDlg = new SvxTransformTabDialog( pParent, pAttr,pSdrView, bSizeTabPage ? SVX_OBJ_NOPROTECT :  SVX_OBJ_NOPROTECT|SVX_OBJ_NORESIZE);
            pDlg->RemoveTabPage( RID_SVXPAGE_ANGLE );
            pDlg->RemoveTabPage( RID_SVXPAGE_SLANT );
            }
            break;
        case RID_SCH_TransformTabDLG_SVXPAGE_SLANT:
            {
            pDlg = new  SvxTransformTabDialog( pParent, pAttr,pSdrView, bSizeTabPage ? SVX_OBJ_NOPROTECT :  SVX_OBJ_NOPROTECT|SVX_OBJ_NORESIZE);
            pDlg->RemoveTabPage( RID_SVXPAGE_SLANT );
            }
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SchTransformTabDialog end

//CHINA001  SvxJSearchOptionsDialog begin
AbstractSvxJSearchOptionsDialog * AbstractDialogFactory_Impl::CreateSvxJSearchOptionsDialog( Window* pParent,
                                                            const SfxItemSet& rOptionsSet, USHORT nUniqueId,
                                                            INT32 nInitialFlags,
                                                            sal_uInt32 nResId)
{
    SvxJSearchOptionsDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXPAGE_JSEARCH_OPTIONS :
            pDlg = new SvxJSearchOptionsDialog( pParent, rOptionsSet, nUniqueId, nInitialFlags );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxJSearchOptionsDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxJSearchOptionsDialog end

//CHINA001  FmInputRecordNoDialog begin
AbstractFmInputRecordNoDialog * AbstractDialogFactory_Impl::CreateFmInputRecordNoDialog( Window* pParent,
                                                            sal_uInt32 nResId)
{
    FmInputRecordNoDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVX_DLG_INPUTRECORDNO :
            pDlg = new FmInputRecordNoDialog( pParent );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractFmInputRecordNoDialog_Impl( pDlg );
    return 0;
}
//CHINA001  FmInputRecordNoDialog end

//CHINA001  SvxNewDictionaryDialog begin
AbstractSvxNewDictionaryDialog * AbstractDialogFactory_Impl::CreateSvxNewDictionaryDialog( Window* pParent,
                                            ::Reference< ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl,
                                            sal_uInt32 nResId )
{
    SvxNewDictionaryDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SFXDLG_NEWDICT :
            pDlg = new SvxNewDictionaryDialog( pParent, xSpl );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxNewDictionaryDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxNewDictionaryDialog end

//CHINA001  SvxEditDictionaryDialog begin
VclAbstractDialog*      AbstractDialogFactory_Impl::CreateSvxEditDictionaryDialog( Window* pParent,
                                            const String& rName,
                                            ::Reference< ::com::sun::star::linguistic2::XSpellChecker1> &xSpl,
                                            sal_uInt32 nResId )
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SFXDLG_EDITDICT :
            pDlg = new SvxEditDictionaryDialog( pParent, rName, xSpl );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxEditDictionaryDialog end

//CHINA001  SvxNameDialog begin
AbstractSvxNameDialog * AbstractDialogFactory_Impl::CreateSvxNameDialog( Window* pParent,
                                    const String& rName, const String& rDesc,
                                    sal_uInt32 nResId )
{
    SvxNameDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_NAME :
            pDlg = new SvxNameDialog( pParent, rName, rDesc );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxNameDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxNameDialog end

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#

AbstractSvxObjectNameDialog* AbstractDialogFactory_Impl::CreateSvxObjectNameDialog(Window* pParent, const String& rName, sal_uInt32 nResId)
{
    return ((RID_SVXDLG_OBJECT_NAME == nResId)
        ? new AbstractSvxObjectNameDialog_Impl(new SvxObjectNameDialog(pParent, rName))
        : NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#

AbstractSvxObjectTitleDescDialog* AbstractDialogFactory_Impl::CreateSvxObjectTitleDescDialog(Window* pParent, const String& rTitle, const String& rDescription, sal_uInt32 nResId)
{
    return ((RID_SVXDLG_OBJECT_TITLE_DESC == nResId)
        ? new AbstractSvxObjectTitleDescDialog_Impl(new SvxObjectTitleDescDialog(pParent, rTitle, rDescription))
        : NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////

//CHINA001  SvxMessDialog begin
AbstractSvxMessDialog * AbstractDialogFactory_Impl::CreateSvxMessDialog( Window* pParent, sal_uInt32 nResId,
                                    const String& rText, const String& rDesc, Image* pImg )
{
    SvxMessDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_MESSBOX :
            pDlg = new SvxMessDialog( pParent, rText, rDesc, pImg );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxMessDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxMessDialog end

//CHINA001  SvxMultiPathDialog begin
AbstractSvxMultiPathDialog * AbstractDialogFactory_Impl::CreateSvxMultiPathDialog( Window* pParent, sal_uInt32 nResId,
                                                BOOL bEmptyAllowed )
{
    SvxMultiPathDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_MULTIPATH :
            pDlg = new SvxMultiPathDialog( pParent, bEmptyAllowed );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxMultiPathDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxMultiPathDialog end

//CHINA001  SvxMultiFileDialog begin
AbstractSvxMultiFileDialog * AbstractDialogFactory_Impl::CreateSvxMultiFileDialog( Window* pParent, sal_uInt32 nResId,
                                                BOOL bEmptyAllowed )
{
    SvxMultiFileDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_MULTIPATH :
            pDlg = new SvxMultiFileDialog( pParent, bEmptyAllowed );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxMultiFileDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxMultiFileDialog end

//CHINA001  SvxHpLinkDlg begin
AbstractSvxHpLinkDlg * AbstractDialogFactory_Impl::CreateSvxHpLinkDlg (Window* pParent,  //add for SvxMultiFileDialog
                                            SfxBindings* pBindings,
                                            sal_uInt32 nResId)
{
    SvxHpLinkDlg* pDlg=NULL;
    switch ( nResId )
    {
        case SID_HYPERLINK_DIALOG :
            pDlg = new SvxHpLinkDlg( pParent, pBindings );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxHpLinkDlg_Impl( pDlg );
    return 0;
}
//CHINA001  SvxHpLinkDlg end

//CHINA001  FmSearchDialog begin
AbstractFmSearchDialog*  AbstractDialogFactory_Impl::CreateFmSearchDialog(Window* pParent, //add for FmSearchDialog
                                                        const String& strInitialText,
                                                        const ::std::vector< String >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link& lnkContextSupplier)
{
    FmSearchDialog* pDlg = new FmSearchDialog( pParent, strInitialText, _rContexts, nInitialContext, lnkContextSupplier );
    if ( pDlg )
        return new AbstractFmSearchDialog_Impl( pDlg );
    return 0;
}

//CHINA001 FmSearchDialog end

//CHINA001  GraphicFilterEmboss begin
AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterEmboss (Window* pParent,  //add for GraphicFilterEmboss
                                            const Graphic& rGraphic,
                                            RECT_POINT eLightSource,
                                            sal_uInt32 nResId)
{
    GraphicFilterDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVX_GRFFILTER_DLG_EMBOSS :
            pDlg = new GraphicFilterEmboss( pParent, rGraphic, eLightSource );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractGraphicFilterDialog_Impl( pDlg );
    return 0;
}
//CHINA001  GraphicFilterEmboss end

//CHINA001  GraphicFilterPoster & GraphicFilterSepia begin
AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterPosterSepia (Window* pParent,  //add for GraphicFilterPoster & GraphicFilterSepia
                                            const Graphic& rGraphic,
                                            USHORT nCount,
                                            sal_uInt32 nResId)
{
    GraphicFilterDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVX_GRFFILTER_DLG_POSTER :
            pDlg = new GraphicFilterPoster( pParent, rGraphic, nCount );
            break;
        case RID_SVX_GRFFILTER_DLG_SEPIA :
            pDlg = new GraphicFilterSepia( pParent, rGraphic, nCount );
            break;

        default:
            break;
    }

    if ( pDlg )
        return new AbstractGraphicFilterDialog_Impl( pDlg );
    return 0;
}
//CHINA001  GraphicFilterPoster & GraphicFilterSepia end

//CHINA001  GraphicFilterSolarize begin
AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterSolarize (Window* pParent,  //add for GraphicFilterSolarize
                                            const Graphic& rGraphic,
                                            BYTE nGreyThreshold, BOOL bInvert,
                                            sal_uInt32 nResId)
{
    GraphicFilterDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVX_GRFFILTER_DLG_SOLARIZE :
            pDlg = new GraphicFilterSolarize( pParent, rGraphic, nGreyThreshold, bInvert );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractGraphicFilterDialog_Impl( pDlg );
    return 0;
}
//CHINA001  GraphicFilterSolarize end

//CHINA001  GraphicFilterMosaic begin
AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterMosaic (Window* pParent,  //add for GraphicFilterMosaic
                                            const Graphic& rGraphic,
                                            USHORT nTileWidth, USHORT nTileHeight, BOOL bEnhanceEdges,
                                            sal_uInt32 nResId)
{
    GraphicFilterDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVX_GRFFILTER_DLG_MOSAIC :
            pDlg = new GraphicFilterMosaic( pParent, rGraphic, nTileWidth, nTileHeight, bEnhanceEdges );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractGraphicFilterDialog_Impl( pDlg );
    return 0;
}
//CHINA001  GraphicFilterMosaic end
//CHINA001  SvxAreaTabDialog begin
AbstractSvxAreaTabDialog* AbstractDialogFactory_Impl::CreateSvxAreaTabDialog( Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            sal_uInt32 nResId,
                                                            const SdrView* pSdrView )
{
    SvxAreaTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_AREA :
            pDlg = new SvxAreaTabDialog( pParent, pAttr, pModel,pSdrView );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxAreaTabDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxAreaTabDialog end
//CHINA001  SvxLineTabDialog begin
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr, //add forSvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 sal_uInt32 nResId,
                                                                 const SdrObject* pObj ,
                                                                BOOL bHasObj)
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_LINE :
            pDlg = new SvxLineTabDialog( pParent, pAttr, pModel,pObj,bHasObj );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxLineTabDialog end

//CHINA001  SvxMeasureDialog & SvxConnectionDialog begin
AbstractSfxSingleTabDialog* AbstractDialogFactory_Impl::CreateSfxSingleTabDialog( Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                                                        const SdrView* pView,
                                                                        sal_uInt32 nResId
                                                                        )
{
    SfxSingleTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXPAGE_MEASURE :
            pDlg = new SvxMeasureDialog( pParent, rAttr, pView );
            break;
        case RID_SVXPAGE_CONNECTION :
            pDlg = new SvxConnectionDialog( pParent, rAttr, pView );
            break;
        case RID_SFXPAGE_DBREGISTER :
            pDlg = new DatabaseRegistrationDialog( pParent, rAttr );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSfxSingleTabDialog_Impl( pDlg );
    return 0;
}

//CHINA001 SvxPostItDialog begin
AbstractSvxPostItDialog* AbstractDialogFactory_Impl::CreateSvxPostItDialog( Window* pParent,
                                                                        const SfxItemSet& rCoreSet,
                                                                        sal_uInt32 nResId,
                                                                        BOOL bPrevNext, BOOL bRedline )
{
    SvxPostItDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXDLG_POSTIT :
            pDlg = new SvxPostItDialog( pParent, rCoreSet, bPrevNext, bRedline );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxPostItDialog_Impl( pDlg );
    return 0;
}
//CHINA001   SvxPostItDialog end

class SvxMacroAssignDialog : public VclAbstractDialog
{
public:
    SvxMacroAssignDialog( Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
            const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
        :m_aItems( SFX_APP()->GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM )
    {
        m_aItems.Put( SfxBoolItem( SID_ATTR_MACROITEM, _bUnoDialogMode ) );
        m_pDialog.reset( new SvxMacroAssignDlg( _pParent, _rxDocumentFrame, m_aItems, _rxEvents, _nInitiallySelectedEvent ) );
    }

    virtual USHORT Execute()
    {
        return m_pDialog->Execute();
    }

    virtual ~SvxMacroAssignDialog()
    {
    }

private:
    SfxItemSet                              m_aItems;
    ::std::auto_ptr< SvxMacroAssignDlg >    m_pDialog;
};

VclAbstractDialog * AbstractDialogFactory_Impl::CreateSvxMacroAssignDlg(
    Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
    const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
{
    return new SvxMacroAssignDialog( _pParent, _rxDocumentFrame, _bUnoDialogMode, _rxEvents, _nInitiallySelectedEvent );
}

// Factories for TabPages
CreateTabPage AbstractDialogFactory_Impl::GetTabPageCreatorFunc( USHORT nId )
{
    switch ( nId )
    {
        case RID_OFA_TP_INTERNATIONAL:
            return ::offapp::InternationalOptionsPage::CreateSc;
        case RID_OFA_TP_INTERNATIONAL_SD:
        case RID_OFA_TP_INTERNATIONAL_IMPR:
            return offapp::InternationalOptionsPage::CreateSd;
        case RID_SVXPAGE_TEXTANIMATION :
            return SvxTextAnimationPage::Create;
        case RID_SVXTABPAGE_GALLERY_GENERAL :
            return TPGalleryThemeGeneral::Create;
        case RID_SVXPAGE_TRANSPARENCE :
            return SvxTransparenceTabPage::Create;
        case RID_SVXPAGE_AREA :
            return SvxAreaTabPage::Create;
        case RID_SVXPAGE_SHADOW :
            return SvxShadowTabPage::Create;
        case RID_SVXPAGE_LINE :
            return SvxLineTabPage::Create;
        case RID_SVXPAGE_CONNECTION :
            return SvxConnectionPage::Create;
        case RID_SVXPAGE_MEASURE :
            return SvxMeasurePage::Create;
        case RID_SFXPAGE_GENERAL :
            return SvxGeneralTabPage::Create;
        case RID_SVXPAGE_PICK_SINGLE_NUM :
            return SvxSingleNumPickTabPage::Create;
        case RID_SVXPAGE_PICK_BMP :
            return SvxBitmapPickTabPage::Create;
        case RID_SVXPAGE_PICK_BULLET :
            return SvxBulletPickTabPage::Create;
        case RID_SVXPAGE_NUM_OPTIONS :
            return SvxNumOptionsTabPage::Create;
        case RID_SVXPAGE_PICK_NUM :
            return SvxNumPickTabPage::Create;
        case RID_SVXPAGE_NUM_POSITION :
            return SvxNumPositionTabPage::Create;
        case RID_SVXPAGE_PARA_ASIAN :
            return SvxAsianTabPage::Create;
        case RID_SVXPAGE_EXT_PARAGRAPH :
            return SvxExtParagraphTabPage::Create;
        case RID_SVXPAGE_ALIGN_PARAGRAPH :
            return SvxParaAlignTabPage::Create;
        case RID_SVXPAGE_STD_PARAGRAPH :
            return SvxStdParagraphTabPage::Create;
        case RID_SVXPAGE_TABULATOR :
            return SvxTabulatorTabPage::Create;
        case RID_SVXPAGE_TEXTATTR :
            return SvxTextAttrPage::Create;
        case RID_SVXPAGE_ALIGNMENT :
            return svx::AlignmentTabPage::Create;
        case RID_SW_TP_BACKGROUND :
        case RID_SVXPAGE_BACKGROUND :
            return SvxBackgroundTabPage::Create;
        case RID_SVXPAGE_BORDER :
            return SvxBorderTabPage::Create;
        case RID_SVXPAGE_CHAR_NAME :
            return SvxCharNamePage::Create;
        case RID_SVXPAGE_CHAR_EFFECTS :
            return SvxCharEffectsPage::Create;
        case RID_SVXPAGE_CHAR_POSITION :
            return SvxCharPositionPage::Create;
        case RID_SVXPAGE_CHAR_TWOLINES :
            return SvxCharTwoLinesPage::Create;
        case RID_SVXPAGE_NUMBERFORMAT :
            return SvxNumberFormatTabPage::Create;
        case RID_SVXPAGE_PAGE :
            return SvxPageDescPage::Create;
        case RID_SVXPAGE_GRFCROP :
            return SvxGrfCropPage::Create;

        default:
            break;
    }

    return 0;
}
CreateSvxDistributePage AbstractDialogFactory_Impl::GetSvxDistributePageCreatorFunc(USHORT nId )
{
        switch ( nId )
    {
        case RID_SVXPAGE_DISTRIBUTE:
            return SvxDistributePage::Create;
        default:
            break;
    }

    return 0;
}
DialogGetRanges AbstractDialogFactory_Impl::GetDialogGetRangesFunc( USHORT nId )
{
        switch ( nId )
    {
        case RID_SVXDLG_POSTIT:
            return SvxPostItDialog::GetRanges;  //add for SvxPostItDialog
        default:
            break;
    }

    return 0;
}
GetTabPageRanges AbstractDialogFactory_Impl::GetTabPageRangesFunc( USHORT nId )
{
    switch ( nId )
    {
        case RID_SVXPAGE_TEXTANIMATION :
            return SvxTextAnimationPage::GetRanges;
        case RID_SVXPAGE_DISTRIBUTE:
            return SvxDistributePage::GetRanges;
        case RID_SVXPAGE_TRANSPARENCE :
            return SvxTransparenceTabPage::GetRanges;
        case RID_SVXPAGE_AREA :
            return SvxAreaTabPage::GetRanges;
        case RID_SVXPAGE_SHADOW :
            return SvxShadowTabPage::GetRanges;
        case RID_SVXPAGE_LINE :
            return SvxLineTabPage::GetRanges;
        case RID_SVXPAGE_CONNECTION :
            return SvxConnectionPage::GetRanges;
        case RID_SVXPAGE_MEASURE :
            return SvxMeasurePage::GetRanges;
        case RID_SVXPAGE_PARA_ASIAN :
            return SvxAsianTabPage::GetRanges;
        case RID_SVXPAGE_EXT_PARAGRAPH :
            return SvxExtParagraphTabPage::GetRanges;
        case RID_SVXPAGE_ALIGN_PARAGRAPH :
            return SvxParaAlignTabPage::GetRanges;
        case RID_SVXPAGE_STD_PARAGRAPH :
            return SvxStdParagraphTabPage::GetRanges;
        case RID_SVXPAGE_TABULATOR :
            return SvxTabulatorTabPage::GetRanges;
        case RID_SVXPAGE_TEXTATTR :
            return SvxTextAttrPage::GetRanges;
        case RID_SVXPAGE_ALIGNMENT :
            return svx::AlignmentTabPage::GetRanges;
        case RID_SW_TP_BACKGROUND :
        case RID_SVXPAGE_BACKGROUND :
            return SvxBackgroundTabPage::GetRanges;
        case RID_SVXPAGE_BORDER :
            return SvxBorderTabPage::GetRanges;
        case RID_SVXPAGE_CHAR_NAME :
            return SvxCharNamePage::GetRanges;
        case RID_SVXPAGE_CHAR_EFFECTS :
            return SvxCharEffectsPage::GetRanges;
        case RID_SVXPAGE_CHAR_POSITION :
            return SvxCharPositionPage::GetRanges;
        case RID_SVXPAGE_CHAR_TWOLINES :
            return SvxCharTwoLinesPage::GetRanges;
        case RID_SVXPAGE_NUMBERFORMAT :
            return SvxNumberFormatTabPage::GetRanges;
        case RID_SVXPAGE_PAGE :
            return SvxPageDescPage::GetRanges;
        case RID_SVXPAGE_ASIAN_LAYOUT:
            return SvxAsianLayoutPage::GetRanges;
        default:
            break;
    }

    return 0;
}

SfxAbstractInsertObjectDialog* AbstractDialogFactory_Impl::CreateInsertObjectDialog( Window* pParent, USHORT nSlotId,
            const Reference < com::sun::star::embed::XStorage >& xStor,
            const SvObjectServerList* pList )
{
    InsertObjectDialog_Impl* pDlg=0;
    switch ( nSlotId )
    {
        case SID_INSERT_OBJECT : pDlg = new SvInsertOleDlg( pParent, xStor, pList ); break;
        case SID_INSERT_PLUGIN : pDlg = new SvInsertPlugInDialog( pParent, xStor ); break;
        case SID_INSERT_APPLET : pDlg = new SvInsertAppletDialog( pParent, xStor ); break;
        case SID_INSERT_FLOATINGFRAME : pDlg = new SfxInsertFloatingFrameDialog( pParent, xStor ); break;
        default: break;
    }

    if ( pDlg )
    {
        pDlg->SetHelpId( nSlotId );
        return new AbstractInsertObjectDialog_Impl( pDlg );
    }
    return 0;
}

VclAbstractDialog* AbstractDialogFactory_Impl::CreateEditObjectDialog( Window* pParent, USHORT nSlotId,
            const Reference < com::sun::star::embed::XEmbeddedObject >& xObj )
{
    InsertObjectDialog_Impl* pDlg=0;
    switch ( nSlotId )
    {
        case SID_INSERT_APPLET : pDlg = new SvInsertAppletDialog( pParent, xObj ); break;
        case SID_INSERT_FLOATINGFRAME : pDlg = new SfxInsertFloatingFrameDialog( pParent, xObj ); break;
        default: break;
    }

    if ( pDlg )
    {
        pDlg->SetHelpId( nSlotId );
        return new VclAbstractDialog_Impl( pDlg );
    }
    return 0;
}

SfxAbstractPasteDialog* AbstractDialogFactory_Impl::CreatePasteDialog( Window* pParent )
{
    return new AbstractPasteDialog_Impl( new SvPasteObjectDialog( pParent ) );
}

SfxAbstractLinksDialog* AbstractDialogFactory_Impl::CreateLinksDialog( Window* pParent, sfx2::SvLinkManager* pMgr, BOOL bHTML, sfx2::SvBaseLink* p)
{
    SvBaseLinksDlg* pLinkDlg = new SvBaseLinksDlg( pParent, pMgr, bHTML );
    if ( p )
        pLinkDlg->SetActLink(p);
    return new AbstractLinksDialog_Impl( pLinkDlg );
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxFormatCellsDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, const SdrObject* /*pObj*/ )
{
    return new AbstractTabDialog_Impl( new SvxFormatCellsDialog( pParent, pAttr, pModel ) );
}

SvxAbstractSplittTableDialog* AbstractDialogFactory_Impl::CreateSvxSplittTableDialog( Window* pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal )
{
    return new SvxSplitTableDlg( pParent, bIsTableVertical, nMaxVertical, nMaxHorizontal );
}

SvxAbstractNewTableDialog* AbstractDialogFactory_Impl::CreateSvxNewTableDialog( Window* pParent )
{
    return new SvxNewTableDialog( pParent );
}

VclAbstractDialog* AbstractDialogFactory_Impl::CreateOptionsDialog(
    Window* pParent, const rtl::OUString& rExtensionId, const rtl::OUString& /*rApplicationContext*/ )
{
    return new VclAbstractDialog_Impl( new OfaTreeOptionsDialog( pParent, rExtensionId ) );
}
