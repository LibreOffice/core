/*************************************************************************
 *
 *  $RCSfile: dlgfact.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:15:21 $
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
#include "dlgfact.hxx"

// class ResId
#include <tools/rc.hxx>
#include <sfx2/basedlgs.hxx>

//#include "dialmgr.hxx"
#include "dialogs.hrc"
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
#include "cuioptgenrl.hxx"  //add for SvxGeneralTabPage
#include "cfg.hxx"    //add for SvxConfigDialog
#define ITEMID_TABSTOP  0 //add for #include "tabstpge.hxx"
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
IMPL_ABSTDLG_BASE(AbstractSvxMessDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxMultiPathDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxMultiFileDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxHpLinkDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractFmSearchDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractGraphicFilterDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxAreaTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxPostItDialog_Impl);

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

void AbstractHangulHanjaConversionDialog_Impl::SetConversionFormat( HangulHanjaConversion::ConversionFormat _eType )
{
     pDlg->SetConversionFormat(_eType);
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

// for HangulHanjaConversionDialog end
// for FmShowColsDialog begin
void  AbstractFmShowColsDialog_Impl::SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols)
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

String AbstractURLDlg_Impl::GetDescription() const
{
    return pDlg->GetDescription();
}

String AbstractURLDlg_Impl::GetTarget() const
{
    return pDlg-> GetTarget();
}

String AbstractURLDlg_Impl::GetName() const
{
    return pDlg-> GetName();
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
    return pDlg->GetChar();
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
void AbstractFmInputRecordNoDialog_Impl::SetValue(double dNew)
{
    pDlg->SetValue(dNew);
}

long AbstractFmInputRecordNoDialog_Impl::GetValue() const
{
    return pDlg->GetValue();
}
//for FmInputRecordNoDialog end

//for SvxNewDictionaryDialog begin
::com::sun::star::uno::Reference<
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
IMPL_LINK( AbstractSvxNameDialog_Impl, CheckNameHdl, Window*, pWin )
{
    if( aCheckNameHdl.IsSet() )
        return aCheckNameHdl.Call(this);
    return 0;
}
//for SvxNameDialog end

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
IMPL_LINK( AbstractSvxPostItDialog_Impl, NextHdl, Window*, pWin )
{
    if( aNextHdl.IsSet() )
        aNextHdl.Call(this);
    return 0;
}
IMPL_LINK( AbstractSvxPostItDialog_Impl, PrevHdl, Window*, pWin )
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
VclAbstractDialog* AbstractDialogFactory_Impl::CreateVclDialog( Window* pParent, const ResId& rResId )
{
    Dialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case SID_OPTIONS_TREEDIALOG :
            pDlg = new  OfaTreeOptionsDialog( pParent );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

// dialogs that use SfxBindings
VclAbstractDialog* AbstractDialogFactory_Impl::CreateSfxDialog( Window* pParent, const SfxBindings& rBindings, const ResId& rResId )
{
    return 0;
}

// TabDialog outside the drawing layer
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTabDialog( const ResId& rResId,
                                                Window* pParent,
                                                const SfxItemSet* pAttrSet,
                                                SfxViewFrame* pViewFrame,
                                                bool bEditFmt,
                                                const String *pUserButtonText )
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case RID_OFA_AUTOCORR_DLG :
            pDlg = new OfaAutoCorrDlg( pParent, pAttrSet );
            break;
        case RID_SVXDLG_CUSTOMIZE :
            pDlg = new SvxConfigDialog( pParent, pAttrSet, pViewFrame );
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
                                            const ResId& rResId,
                                            SdrView* pView,
                                            SdrModel* pModel )
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId,
                                            USHORT nAnchorTypes )
{
    SvxCaptionTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId,
                                            SvxDistributeHorizontal eHor ,
                                            SvxDistributeVertical eVer)
{
    SvxDistributeDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                                                    const ResId& rResId)
{
    HangulHanjaConversionDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                             const ResId& rResId)
{
   FmShowColsDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
   SvxZoomDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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

//CHINA001 GalleryDialog start
VclAbstractRefreshableDialog * AbstractDialogFactory_Impl::CreateActualizeProgressDialog( Window* pParent,  //add for ActualizeProgress
                                            GalleryTheme* pThm,
                                            const ResId& rResId)
{
   Dialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
   SearchProgress* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
   TakeProgress* pDlg=NULL;
    switch ( rResId.GetId() )
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
    Window* pParent, BOOL bShowSlots )
{
    SvxScriptSelectorDialog* pDlg = NULL;

    pDlg = new SvxScriptSelectorDialog( pParent, bShowSlots );

    if (pDlg)
    {
        return new AbstractScriptSelectorDialog_Impl( pDlg );
    }
    return 0;
}

String AbstractScriptSelectorDialog_Impl::GetScriptURL()
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
    OSL_TRACE("in ADF_Impl::CreateSvxScriptOrgDialog");
    Dialog* pDlg=NULL;
    rtl::OUString aResName;
    ResMgr* pBasResMgr = NULL;
    OSL_TRACE("creating dialog");
    aResName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "basctl" ));
    aResName += rtl::OUString::valueOf( sal_Int32( SUPD ));

    pBasResMgr = ResMgr::CreateResMgr( rtl::OUStringToOString( aResName, RTL_TEXTENCODING_ASCII_US ));

    pDlg = new SvxScriptOrgDialog( pParent, pBasResMgr, rLanguage);

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

AbstractTitleDialog * AbstractDialogFactory_Impl::CreateTitleDialog( Window* pParent,  //add for TitleDialog
                                            const String& rOldText,
                                            const ResId& rResId)
{
   TitleDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
   GalleryIdDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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

VclAbstractDialog * AbstractDialogFactory_Impl::CreateGalleryThemePropertiesDialog( Window* pParent,  //add for GalleryThemeProperties
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet,
                                            const ResId& rResId)
{
    Dialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case RID_SVXTABDLG_GALLERYTHEME :
            pDlg = new GalleryThemeProperties( pParent, pData, pItemSet);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}
//CHINA001 GalleryDialog end

AbstractURLDlg * AbstractDialogFactory_Impl::CreateURLDialog( Window* pParent,  //add for URLDlg
                                            const String& rURL, const String& rDescription,
                                            const String& rTarget, const String& rName,
                                            TargetList& rTargetList,
                                            const ResId& rResId)
{
    URLDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case RID_SVXDLG_IMAPURL :
            pDlg = new URLDlg( pParent, rURL, rDescription, rTarget, rName, rTargetList);
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
AbstractSvxHlinkDlgMarkWnd* AbstractDialogFactory_Impl::CreateSvxHlinkDlgMarkWndDialog( SvxHyperlinkTabPageBase* pParent, const ResId& rResId )
{
    SvxHlinkDlgMarkWnd* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            const ResId& rResId,
                                                            BOOL bOne  )
{
    SvxCharacterMap* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId ) //add for SvxSearchAttributeDialog
{
    Dialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            USHORT nLonger,
                                                            const ResId& rResId)
{
    SvxSearchSimilarityDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case RID_SVXDLG_SEARCHSIMILARITY :
            pDlg = new SvxSearchSimilarityDialog( pParent, bRelax, nOther, nShorter, nLonger );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxSearchSimilarityDialog_Impl( pDlg );
    return 0;
}
//CHINA001  SvxSearchSimilarityDialog end

//CHINA001  SvxBorderBackgroundDlg begin
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxBorderBackgroundDlg( Window* pParent,
                                            const SfxItemSet& rCoreSet,
                                            const ResId& rResId,
                                            BOOL bEnableSelector)
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                                const ResId& rResId,
                                                                                USHORT nAnchorTypes )
{
    SvxTransformTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                const ResId& rResId,
                                                                bool bSizeTabPage
                                                                )
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            const ResId& rResId)
{
    SvxJSearchOptionsDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            const ResId& rResId)
{
    FmInputRecordNoDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl,
                                            const ResId& rResId )
{
    SvxNewDictionaryDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> &xSpl,
                                            const ResId& rResId )
{
    Dialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                    const ResId& rResId )
{
    SvxNameDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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

//CHINA001  SvxMessDialog begin
AbstractSvxMessDialog * AbstractDialogFactory_Impl::CreateSvxMessDialog( Window* pParent, const ResId& rResId,
                                    const String& rText, const String& rDesc, Image* pImg )
{
    SvxMessDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
AbstractSvxMultiPathDialog * AbstractDialogFactory_Impl::CreateSvxMultiPathDialog( Window* pParent, const ResId& rResId,
                                                BOOL bEmptyAllowed )
{
    SvxMultiPathDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
AbstractSvxMultiFileDialog * AbstractDialogFactory_Impl::CreateSvxMultiFileDialog( Window* pParent, const ResId& rResId,
                                                BOOL bEmptyAllowed )
{
    SvxMultiFileDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
    SvxHpLinkDlg* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                        const String& strContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link& lnkContextSupplier,
                                                        const ResId& rResId,
                                                        FMSEARCH_MODE eMode )
{
    FmSearchDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case RID_SVXDLG_SEARCHFORM :
            pDlg = new FmSearchDialog( pParent, strInitialText,strContexts,nInitialContext,lnkContextSupplier,eMode );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractFmSearchDialog_Impl( pDlg );
    return 0;
}

//CHINA001 FmSearchDialog end

//CHINA001  GraphicFilterEmboss begin
AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterEmboss (Window* pParent,  //add for GraphicFilterEmboss
                                            const Graphic& rGraphic,
                                            RECT_POINT eLightSource,
                                            const ResId& rResId)
{
    GraphicFilterDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
    GraphicFilterDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
    GraphicFilterDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                            const ResId& rResId)
{
    GraphicFilterDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            const ResId& rResId,
                                                            const SdrView* pSdrView )
{
    SvxAreaTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                 const ResId& rResId,
                                                                 const SdrObject* pObj ,
                                                                BOOL bHasObj)
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                        const ResId& rResId
                                                                        )
{
    SfxSingleTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case RID_SVXPAGE_MEASURE :
            pDlg = new SvxMeasureDialog( pParent, rAttr, pView );
            break;
        case RID_SVXPAGE_CONNECTION :
            pDlg = new SvxConnectionDialog( pParent, rAttr, pView );
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
                                                                        const ResId& rResId,
                                                                        BOOL bPrevNext, BOOL bRedline )
{
    SvxPostItDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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

VclAbstractDialog * AbstractDialogFactory_Impl::CreateSvxMacroAssignDlg( Window* pParent,  SfxItemSet& rSet, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xNameReplace, sal_uInt16 nSelectedIndex )
{
    OSL_TRACE("in ADF_Impl::CreateSvxMacroAssignDlg");
    Dialog* pDlg=NULL;

    pDlg = new SvxMacroAssignDlg( pParent, rSet, xNameReplace, nSelectedIndex );

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

//STRIP001 AbstractSvxSpellCheckDialog * AbstractDialogFactory_Impl::CreateSvxSpellCheckDialog( Window* pParent,  //add for SvxSpellCheckDialog
//STRIP001 ::com::sun::star::uno::Reference<
//STRIP001 ::com::sun::star::linguistic2::XSpellChecker1 >  &xChecker,
//STRIP001 SvxSpellWrapper* pWrapper,
//STRIP001 const ResId& rResId)
//STRIP001 {
//STRIP001 SvxSpellCheckDialog* pDlg=NULL;
//STRIP001 switch ( rResId.GetId() )
//STRIP001 {
//STRIP001      case RID_SVXDLG_SPELLCHECK :
//STRIP001 pDlg = new SvxSpellCheckDialog( pParent, xChecker,pWrapper);
//STRIP001 break;
//STRIP001      default:
//STRIP001 break;
//STRIP001  }
//STRIP001
//STRIP001 if ( pDlg )
//STRIP001 return new AbstractSvxSpellCheckDialog_Impl( pDlg );
//STRIP001 return 0;
//STRIP001 }

// Factories for TabPages
CreateTabPage AbstractDialogFactory_Impl::GetTabPageCreatorFunc( USHORT nId )
{
    switch ( nId )
    {
        case RID_OFA_TP_INTERNATIONAL:
            return ::offapp::InternationalOptionsPage::CreateSc;
            break;
        case RID_OFA_TP_INTERNATIONAL_SD:
        case RID_OFA_TP_INTERNATIONAL_IMPR:
            return offapp::InternationalOptionsPage::CreateSd;
            break;
        case RID_SVXPAGE_TEXTANIMATION :
            return SvxTextAnimationPage::Create;
            break;
//CHINA001      case RID_SVXPAGE_CAPTION :
//CHINA001          return SvxCaptionTabPage::Create;
//CHINA001          break;
        case RID_SVXTABPAGE_GALLERY_GENERAL :
            return TPGalleryThemeGeneral::Create;
            break;
        case RID_SVXPAGE_TRANSPARENCE :
            return SvxTransparenceTabPage::Create;
            break;
        case RID_SVXPAGE_AREA :
            return SvxAreaTabPage::Create;
            break;
        case RID_SVXPAGE_SHADOW :
            return SvxShadowTabPage::Create;
            break;
        case RID_SVXPAGE_LINE :
            return SvxLineTabPage::Create;
            break;
        case RID_SVXPAGE_CONNECTION :
            return SvxConnectionPage::Create;
            break;
        case RID_SVXPAGE_MEASURE :
            return SvxMeasurePage::Create;
            break;
        case RID_SFXPAGE_GENERAL :
            return SvxGeneralTabPage::Create;
            break;
        case RID_SVXPAGE_PICK_SINGLE_NUM :
            return SvxSingleNumPickTabPage::Create;
            break;
        case RID_SVXPAGE_PICK_BMP :
            return SvxBitmapPickTabPage::Create;
            break;
        case RID_SVXPAGE_PICK_BULLET :
            return SvxBulletPickTabPage::Create;
            break;
        case RID_SVXPAGE_NUM_OPTIONS :
            return SvxNumOptionsTabPage::Create;
            break;
        case RID_SVXPAGE_PICK_NUM :
            return SvxNumPickTabPage::Create;
            break;
        case RID_SVXPAGE_NUM_POSITION :
            return SvxNumPositionTabPage::Create;
            break;
        case RID_SVXPAGE_PARA_ASIAN :
            return SvxAsianTabPage::Create;
            break;
        case RID_SVXPAGE_EXT_PARAGRAPH :
            return SvxExtParagraphTabPage::Create;
            break;
        case RID_SVXPAGE_ALIGN_PARAGRAPH :
            return SvxParaAlignTabPage::Create;
            break;
        case RID_SVXPAGE_STD_PARAGRAPH :
            return SvxStdParagraphTabPage::Create;
            break;
        case RID_SVXPAGE_TABULATOR :
            return SvxTabulatorTabPage::Create;
            break;
        case RID_SVXPAGE_TEXTATTR :
            return SvxTextAttrPage::Create;
            break;
        case RID_SVXPAGE_ALIGNMENT :
            return SvxAlignmentTabPage::Create;
            break;
        case RID_SW_TP_BACKGROUND :
        case RID_SVXPAGE_BACKGROUND :
            return SvxBackgroundTabPage::Create;
            break;
        case RID_SVXPAGE_BORDER :
            return SvxBorderTabPage::Create;
            break;
        case RID_SVXPAGE_CHAR_NAME :
            return SvxCharNamePage::Create;
            break;
        case RID_SVXPAGE_CHAR_EFFECTS :
            return SvxCharEffectsPage::Create;
            break;
        case RID_SVXPAGE_CHAR_POSITION :
            return SvxCharPositionPage::Create;
            break;
        case RID_SVXPAGE_CHAR_TWOLINES :
            return SvxCharTwoLinesPage::Create;
            break;
        case RID_SVXPAGE_NUMBERFORMAT :
            return SvxNumberFormatTabPage::Create;
            break;
        case RID_SVXPAGE_PAGE :
            return SvxPageDescPage::Create;
            break;
        case RID_SVXPAGE_GRFCROP :
            return SvxGrfCropPage::Create;
            break;

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
            break;
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
            break;
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
            break;
//CHINA001      case RID_SVXPAGE_CAPTION :
//CHINA001          return SvxCaptionTabPage::GetRanges;
//CHINA001          break;
        case RID_SVXPAGE_DISTRIBUTE:
            return SvxDistributePage::GetRanges;
            break;
        case RID_SVXPAGE_TRANSPARENCE :
            return SvxTransparenceTabPage::GetRanges;
            break;
        case RID_SVXPAGE_AREA :
            return SvxAreaTabPage::GetRanges;
            break;
        case RID_SVXPAGE_SHADOW :
            return SvxShadowTabPage::GetRanges;
            break;
        case RID_SVXPAGE_LINE :
            return SvxLineTabPage::GetRanges;
            break;
        case RID_SVXPAGE_CONNECTION :
            return SvxConnectionPage::GetRanges;
            break;
        case RID_SVXPAGE_MEASURE :
            return SvxMeasurePage::GetRanges;
            break;
        case RID_SVXPAGE_PARA_ASIAN :
            return SvxAsianTabPage::GetRanges;
            break;
        case RID_SVXPAGE_EXT_PARAGRAPH :
            return SvxExtParagraphTabPage::GetRanges;
            break;
        case RID_SVXPAGE_ALIGN_PARAGRAPH :
            return SvxParaAlignTabPage::GetRanges;
            break;
        case RID_SVXPAGE_STD_PARAGRAPH :
            return SvxStdParagraphTabPage::GetRanges;
            break;
        case RID_SVXPAGE_TABULATOR :
            return SvxTabulatorTabPage::GetRanges;
            break;
        case RID_SVXPAGE_TEXTATTR :
            return SvxTextAttrPage::GetRanges;
            break;
        case RID_SVXPAGE_ALIGNMENT :
            return SvxAlignmentTabPage::GetRanges;
            break;
        case RID_SW_TP_BACKGROUND :
        case RID_SVXPAGE_BACKGROUND :
            return SvxBackgroundTabPage::GetRanges;
            break;
        case RID_SVXPAGE_BORDER :
            return SvxBorderTabPage::GetRanges;
            break;
        case RID_SVXPAGE_CHAR_NAME :
            return SvxCharNamePage::GetRanges;
            break;
        case RID_SVXPAGE_CHAR_EFFECTS :
            return SvxCharEffectsPage::GetRanges;
            break;
        case RID_SVXPAGE_CHAR_POSITION :
            return SvxCharPositionPage::GetRanges;
            break;
        case RID_SVXPAGE_CHAR_TWOLINES :
            return SvxCharTwoLinesPage::GetRanges;
            break;
        case RID_SVXPAGE_NUMBERFORMAT :
            return SvxNumberFormatTabPage::GetRanges;
            break;
        case RID_SVXPAGE_PAGE :
            return SvxPageDescPage::GetRanges;
            break;

        default:
            break;
    }

    return 0;
}
