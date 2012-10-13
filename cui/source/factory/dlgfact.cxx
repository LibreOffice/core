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

#include "align.hxx" //add for SvxAlignmentTabPage
#include "dlgfact.hxx"

#include <sfx2/basedlgs.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <cuires.hrc>
#include <svx/dialogs.hrc>
#include "numfmt.hxx"
#include "splitcelldlg.hxx"
#include "gallery.hrc"
#include "dstribut.hxx"
#include "cuiimapwnd.hxx"
#include "hlmarkwn.hxx"
#include "cuicharmap.hxx"
#include "srchxtra.hxx"
#include "textanim.hxx"
#include "autocdlg.hxx"
#include "treeopt.hxx"
#include "internationaloptions.hxx"
#include "labdlg.hxx"
#include "hangulhanjadlg.hxx" //add for HangulHanjaConversionDialog
#include "showcols.hxx" //add for FmShowColsDialog
#include "zoom.hxx" //add for SvxZoomDialog
#include "cuigaldlg.hxx"        //add for Gallery 6 Dialogs and 1 TabPage
#include "cuiimapwnd.hxx"       //add for URLDlg
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
#include "optasian.hxx"
#include "insdlg.hxx"
#include "pastedlg.hxx"
#include "linkdlg.hxx"
#include "SpellDialog.hxx"
#include "cfg.hxx"    //add for SvxConfigDialog
#include "numpages.hxx" // add for
#include "paragrph.hxx" //add for
#include "tabstpge.hxx" // add  for
#include "textattr.hxx" // add for SvxTextAttrPage
#include "backgrnd.hxx" //add for SvxBackgroundTabPage
#include "border.hxx" //add for SvxBorderTabPage
#include "chardlg.hxx" //add for SvxCharNamePage,SvxCharEffectsPage,SvxCharPositionPage,SvxCharTwoLinesPage
#include "page.hxx" //add for SvxPageDescPage
#include "postdlg.hxx" //add for SvxPostItDialog
#include "grfpage.hxx" //add for SvxGrfCropPage
#include "scriptdlg.hxx" // for ScriptOrgDialog
#include "selector.hxx" // for SvxScriptSelectorDialog
#include "macropg.hxx" // for SvxMacroAssignDlg
#include "sdrcelldlg.hxx"
#include "newtabledlg.hxx"
#include "macroass.hxx"
#include "acccfg.hxx"
#include "insrc.hxx"
#include "passwdomdlg.hxx"
#include "hyphen.hxx"
#include "thesdlg.hxx"
#include "about.hxx"
#include "dialmgr.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

using ::com::sun::star::uno::Reference;

using namespace svx;
// AbstractTabDialog implementations just forwards everything to the dialog
IMPL_ABSTDLG_BASE(CuiAbstractSfxDialog_Impl)
IMPL_ABSTDLG_BASE(CuiVclAbstractDialog_Impl)
IMPL_ABSTDLG_BASE(VclAbstractRefreshableDialog_Impl);
IMPL_ABSTDLG_BASE(CuiAbstractTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxDistributeDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractHangulHanjaConversionDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractFmShowColsDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractHyphenWordDialog_Impl)
IMPL_ABSTDLG_BASE(AbstractThesaurusDialog_Impl)

AbstractSvxZoomDialog_Impl::~AbstractSvxZoomDialog_Impl()                                       \
{
    delete pDlg;
}
short AbstractSvxZoomDialog_Impl::Execute()
{
    return pDlg->Execute();
}

IMPL_ABSTDLG_BASE(AbstractSearchProgress_Impl);
IMPL_ABSTDLG_BASE(AbstractTakeProgress_Impl);
IMPL_ABSTDLG_BASE(AbstractTitleDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractScriptSelectorDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractGalleryIdDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractURLDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxHlinkDlgMarkWnd_Impl);
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
IMPL_ABSTDLG_BASE(AbstractPasteDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractInsertObjectDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractLinksDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSpellDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSvxPostItDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractPasswordToOpenModifyDialog_Impl);

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
        SAL_WARN( "cui.factory", "VclAbstractDialog2_Impl::EndDialogHdl(): wrong dialog" );
    }

    m_aEndDlgHdl.Call( this );
    m_aEndDlgHdl = Link();

    return 0L;
}

//////////////////////////////////////////////////////////////////////////

void CuiAbstractTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

const SfxItemSet* CuiAbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* CuiAbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void CuiAbstractTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void CuiAbstractTabDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
String CuiAbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}


const SfxItemSet* CuiAbstractSfxDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

void CuiAbstractSfxDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}

String CuiAbstractSfxDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

SvxDistributeHorizontal AbstractSvxDistributeDialog_Impl::GetDistributeHor()const
{
    return pDlg->GetDistributeHor();
}
SvxDistributeVertical AbstractSvxDistributeDialog_Impl::GetDistributeVer()const
{
    return pDlg->GetDistributeVer();
}

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

void AbstractHangulHanjaConversionDialog_Impl::SetConversionDirectionState( sal_Bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection )
{
    pDlg->SetConversionDirectionState(_bTryBothDirections, _ePrimaryConversionDirection);
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType )
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

editeng::HangulHanjaConversion::ConversionDirection AbstractHangulHanjaConversionDialog_Impl::GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const
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
editeng::HangulHanjaConversion::ConversionFormat     AbstractHangulHanjaConversionDialog_Impl::GetConversionFormat( ) const
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

String AbstractThesaurusDialog_Impl::GetWord()
{
    return pDlg->GetWord();
};

sal_uInt16 AbstractThesaurusDialog_Impl::GetLanguage() const
{
    return pDlg->GetLanguage();
};

Window* AbstractThesaurusDialog_Impl::GetWindow()
{
    return pDlg;
}

void AbstractHyphenWordDialog_Impl::SelLeft()
{
    pDlg->SelLeft();
}

void AbstractHyphenWordDialog_Impl::SelRight()
{
    pDlg->SelRight();
}

Window* AbstractHyphenWordDialog_Impl::GetWindow()
{
    return pDlg;
}

Reference < com::sun::star::embed::XEmbeddedObject > AbstractInsertObjectDialog_Impl::GetObject()
{
   return pDlg->GetObject();
}

sal_Bool AbstractInsertObjectDialog_Impl::IsCreateNew()
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

sal_uLong AbstractPasteDialog_Impl::GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats,
                        const TransferableObjectDescriptor* pDesc )
{
    return pDlg->GetFormat( aHelper, pFormats, pDesc );
}

void  AbstractFmShowColsDialog_Impl::SetColumns(const ::Reference< ::com::sun::star::container::XIndexContainer>& xCols)
{
     pDlg->SetColumns(xCols);
}

void AbstractSvxZoomDialog_Impl::SetLimits( sal_uInt16 nMin, sal_uInt16 nMax )
{
    pDlg->SetLimits( nMin, nMax );
}

void AbstractSvxZoomDialog_Impl::HideButton( sal_uInt16 nBtnId )
{
    pDlg->HideButton( nBtnId );
}

const SfxItemSet* AbstractSvxZoomDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

void AbstractSpellDialog_Impl::SetLanguage( sal_uInt16 nLang )
{
    pDlg->SetLanguage(nLang);
}

sal_Bool AbstractSpellDialog_Impl::Close()
{
    return pDlg->Close();
}

void  AbstractSpellDialog_Impl::Invalidate()
{
    pDlg->InvalidateDialog();
}

Window*     AbstractSpellDialog_Impl::GetWindow()
{
    return pDlg;
}

SfxBindings& AbstractSpellDialog_Impl::GetBindings()
{
    return pDlg->GetBindings();
}

void AbstractSearchProgress_Impl::Update()
{
     pDlg->Update();
}

void AbstractSearchProgress_Impl::Sync()
{
     pDlg->Sync();
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

void AbstractTakeProgress_Impl::Update()
{
     pDlg->Update();
}

void AbstractTakeProgress_Impl::Sync()
{
     pDlg->Sync();
}

void AbstractTakeProgress_Impl::SetFile( const INetURLObject& rURL )
{
     pDlg->SetFile( rURL );
}

PLinkStub   AbstractTakeProgress_Impl::GetLinkStubCleanUpHdl()
{
    return TakeProgress::LinkStubCleanUpHdl;
}

String AbstractTitleDialog_Impl::GetTitle() const
{
    return pDlg->GetTitle();
}

sal_uLong AbstractGalleryIdDialog_Impl::GetId() const
{
    return pDlg->GetId();
}

void VclAbstractRefreshableDialog_Impl::Update()
{
    pDlg->Update();
}

void VclAbstractRefreshableDialog_Impl::Sync()
{
     pDlg->Sync();
}

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

void AbstractSvxHlinkDlgMarkWnd_Impl::Hide( sal_uInt16 nFlags )
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

sal_Bool AbstractSvxHlinkDlgMarkWnd_Impl::IsVisible( ) const
{
    return (( Window* )pDlg)->IsVisible();
}

void AbstractSvxHlinkDlgMarkWnd_Impl::Invalidate( sal_uInt16 nFlags )
{
    (( Window* )pDlg)->Invalidate(nFlags);
}

sal_Bool AbstractSvxHlinkDlgMarkWnd_Impl::MoveTo( Point aNewPos )const
{
    return pDlg->MoveTo(aNewPos);
}

sal_Bool AbstractSvxHlinkDlgMarkWnd_Impl::ConnectToDialog( sal_Bool bDoit  )const
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

sal_uInt16 AbstractSvxHlinkDlgMarkWnd_Impl::SetError( sal_uInt16 nError)
{
    return pDlg->SetError(nError);
}

sal_uInt16 AbstractSvxSearchSimilarityDialog_Impl::GetOther()
{
    return pDlg->GetOther();
}

sal_uInt16 AbstractSvxSearchSimilarityDialog_Impl::GetShorter()
{
    return pDlg->GetShorter();
}

sal_uInt16 AbstractSvxSearchSimilarityDialog_Impl::GetLonger()
{
    return pDlg->GetLonger();
}

sal_Bool AbstractSvxSearchSimilarityDialog_Impl::IsRelaxed()
{
    return pDlg->IsRelaxed();
}

// AbstractSvxTransformTabDialog implementations just forwards everything to the dialog
void AbstractSvxTransformTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}
const SfxItemSet* AbstractSvxTransformTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//
const sal_uInt16* AbstractSvxTransformTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
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

// AbstractSvxCaptionDialog implementations just forwards everything to the dialog
void AbstractSvxCaptionDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}
const SfxItemSet* AbstractSvxCaptionDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//
const sal_uInt16* AbstractSvxCaptionDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
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

sal_Int32 AbstractSvxJSearchOptionsDialog_Impl::GetTransliterationFlags() const
{
    return pDlg->GetTransliterationFlags();
}

void AbstractFmInputRecordNoDialog_Impl::SetValue(long nNew)
{
    pDlg->SetValue(nNew);
}

long AbstractFmInputRecordNoDialog_Impl::GetValue() const
{
    return pDlg->GetValue();
}

::Reference<
        ::com::sun::star::linguistic2::XDictionary > AbstractSvxNewDictionaryDialog_Impl::GetNewDictionary()
{
    return pDlg->GetNewDictionary();
}

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
void AbstractSvxNameDialog_Impl::SetEditHelpId(const rtl::OString& aHelpId)
{
    pDlg->SetEditHelpId( aHelpId );
}
void AbstractSvxNameDialog_Impl::SetHelpId( const rtl::OString& aHelpId )
{
    pDlg->SetHelpId( aHelpId );
}
void AbstractSvxNameDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
IMPL_LINK_NOARG(AbstractSvxNameDialog_Impl, CheckNameHdl)
{
    if( aCheckNameHdl.IsSet() )
        return aCheckNameHdl.Call(this);
    return 0;
}

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

IMPL_LINK_NOARG(AbstractSvxObjectNameDialog_Impl, CheckNameHdl)
{
    if(aCheckNameHdl.IsSet())
    {
        return aCheckNameHdl.Call(this);
    }

    return 0;
}

void AbstractSvxObjectTitleDescDialog_Impl::GetTitle(String& rTitle)
{
    pDlg->GetTitle(rTitle);
}

void AbstractSvxObjectTitleDescDialog_Impl::GetDescription(String& rDescription)
{
    pDlg->GetDescription(rDescription);
}

void AbstractSvxMessDialog_Impl::SetButtonText( sal_uInt16 nBtnId, const String& rNewTxt )
{
    pDlg->SetButtonText( nBtnId, rNewTxt );
}

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

void AbstractSvxMultiFileDialog_Impl::SetHelpId( const rtl::OString& aHelpId )
{
    pDlg->SetHelpId( aHelpId );
}

Window * AbstractSvxHpLinkDlg_Impl::GetWindow()
{
    return (Window *)pDlg;
}

sal_Bool AbstractSvxHpLinkDlg_Impl::QueryClose()
{
    return pDlg->QueryClose();
}


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

Graphic AbstractGraphicFilterDialog_Impl::GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY )
{
    return pDlg->GetFilteredGraphic( rGraphic, fScaleX, fScaleY );
}

// AbstractSvxAreaTabDialog implementations just forwards everything to the dialog
void AbstractSvxAreaTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

const SfxItemSet* AbstractSvxAreaTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractSvxAreaTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

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

void AbstractSvxPostItDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
const SfxItemSet* AbstractSvxPostItDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
void AbstractSvxPostItDialog_Impl::EnableTravel(sal_Bool bNext, sal_Bool bPrev)
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
void AbstractSvxPostItDialog_Impl::SetReadonlyPostIt(sal_Bool bDisable)
{
    pDlg->SetReadonlyPostIt( bDisable );
}
sal_Bool AbstractSvxPostItDialog_Impl::IsOkEnabled() const
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
IMPL_LINK_NOARG(AbstractSvxPostItDialog_Impl, NextHdl)
{
    if( aNextHdl.IsSet() )
        aNextHdl.Call(this);
    return 0;
}
IMPL_LINK_NOARG(AbstractSvxPostItDialog_Impl, PrevHdl)
{
    if( aPrevHdl.IsSet() )
        aPrevHdl.Call(this);
    return 0;
}
Window * AbstractSvxPostItDialog_Impl::GetWindow()
{
    return (Window *)pDlg;
}

String AbstractPasswordToOpenModifyDialog_Impl::GetPasswordToOpen() const
{
    return pDlg->GetPasswordToOpen();
}
String AbstractPasswordToOpenModifyDialog_Impl::GetPasswordToModify() const
{
    return pDlg->GetPasswordToModify();
}
bool AbstractPasswordToOpenModifyDialog_Impl::IsRecommendToOpenReadonly() const
{
    return pDlg->IsRecommendToOpenReadonly();
}

// Create dialogs with simplest interface
VclAbstractDialog* AbstractDialogFactory_Impl::CreateVclDialog( Window* pParent, sal_uInt32 nResId )
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_DEFAULTABOUT:
        {
            pDlg = new AboutDialog( pParent, CUI_RES( (sal_uInt16) nResId ) );
            break;
        }
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
        return new CuiVclAbstractDialog_Impl( pDlg );
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
        return new CuiVclAbstractDialog_Impl( pDlg );
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
        return new CuiAbstractTabDialog_Impl( pDlg );
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
        return new CuiAbstractTabDialog_Impl( pDlg );
    return 0;
}

// TabDialog that use functionality of the drawing layer
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTextTabDialog( Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView,
                                            SdrModel* )
{
    SfxTabDialog* pDlg = new SvxTextTabDialog( pParent, pAttrSet, pView );
    return new CuiAbstractTabDialog_Impl( pDlg );
}

// TabDialog that use functionality of the drawing layer and add AnchorTypes -- for SvxCaptionTabDialog
AbstractSvxCaptionDialog*       AbstractDialogFactory_Impl::CreateCaptionDialog( Window* pParent,
                                            const SdrView* pView,
                                            sal_uInt16 nAnchorTypes )
{
    SvxCaptionTabDialog* pDlg = new SvxCaptionTabDialog( pParent, pView, nAnchorTypes );
    return new AbstractSvxCaptionDialog_Impl( pDlg );
}

AbstractSvxDistributeDialog*    AbstractDialogFactory_Impl::CreateSvxDistributeDialog(Window* pParent,
                                            const SfxItemSet& rAttr,
                                            SvxDistributeHorizontal eHor ,
                                            SvxDistributeVertical eVer)
{
    SvxDistributeDialog* pDlg = new SvxDistributeDialog( pParent, rAttr, eHor, eVer);
    return new AbstractSvxDistributeDialog_Impl( pDlg );
}

AbstractHangulHanjaConversionDialog* AbstractDialogFactory_Impl::CreateHangulHanjaConversionDialog(Window* pParent,
                                                                       editeng::HangulHanjaConversion::ConversionDirection _ePrimaryDirection )
{
    HangulHanjaConversionDialog* pDlg = new HangulHanjaConversionDialog( pParent, _ePrimaryDirection);
    return new AbstractHangulHanjaConversionDialog_Impl( pDlg );
}

AbstractThesaurusDialog* AbstractDialogFactory_Impl::CreateThesaurusDialog( Window* pParent,
                                ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XThesaurus >  xThesaurus,
                                const String &rWord, sal_Int16 nLanguage )
{
    SvxThesaurusDialog* pDlg = new SvxThesaurusDialog( pParent, xThesaurus, rWord, nLanguage );
    return new AbstractThesaurusDialog_Impl( pDlg );
}

AbstractHyphenWordDialog* AbstractDialogFactory_Impl::CreateHyphenWordDialog( Window* pParent,
                                                const String &rWord, LanguageType nLang,
                                                ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper )
{
    SvxHyphenWordDialog* pDlg = new SvxHyphenWordDialog( rWord, nLang, pParent, xHyphen, pWrapper );
    return new AbstractHyphenWordDialog_Impl( pDlg );
}

AbstractFmShowColsDialog * AbstractDialogFactory_Impl::CreateFmShowColsDialog( Window* pParent )
{
    FmShowColsDialog* pDlg = new FmShowColsDialog( pParent);
    return new AbstractFmShowColsDialog_Impl( pDlg );
}
AbstractSvxZoomDialog * AbstractDialogFactory_Impl::CreateSvxZoomDialog( Window* pParent,  //add for SvxZoomDialog
                                            const SfxItemSet& rCoreSet)
{
    SvxZoomDialog* pDlg = new SvxZoomDialog( pParent, rCoreSet);
    return new AbstractSvxZoomDialog_Impl( pDlg );
}

AbstractSpellDialog *  AbstractDialogFactory_Impl::CreateSvxSpellDialog(
                        Window* pParent,
                        SfxBindings* pBindings,
                        svx::SpellDialogChildWindow* pSpellChildWindow )
{
    svx::SpellDialog* pDlg = new svx::SpellDialog(pSpellChildWindow, pParent, pBindings);
    return new AbstractSpellDialog_Impl(pDlg);
}

VclAbstractRefreshableDialog * AbstractDialogFactory_Impl::CreateActualizeProgressDialog( Window* pParent, GalleryTheme* pThm )
{
   Dialog* pDlg = new ActualizeProgress( pParent, pThm);
   return new VclAbstractRefreshableDialog_Impl( pDlg );
}

AbstractSearchProgress * AbstractDialogFactory_Impl::CreateSearchProgressDialog( Window* pParent,  //add for SearchProgress
                                            const INetURLObject& rStartURL )
{
    SearchProgress* pDlg = new SearchProgress( pParent, rStartURL);
    return new AbstractSearchProgress_Impl( pDlg );
}

AbstractTakeProgress * AbstractDialogFactory_Impl::CreateTakeProgressDialog( Window* pParent )
{
    TakeProgress* pDlg = new TakeProgress( pParent );
    return new AbstractTakeProgress_Impl( pDlg );
}

VclAbstractDialog*
AbstractDialogFactory_Impl::CreateScriptErrorDialog(
    Window* pParent, ::com::sun::star::uno::Any aException )
{
    return new SvxScriptErrorDialog( pParent, aException );
}

AbstractScriptSelectorDialog*
AbstractDialogFactory_Impl::CreateScriptSelectorDialog(
    Window* pParent, sal_Bool bShowSlots, const Reference< frame::XFrame >& _rxFrame )
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
        return new CuiVclAbstractDialog_Impl( pDlg );
    return 0;
}

AbstractTitleDialog * AbstractDialogFactory_Impl::CreateTitleDialog( Window* pParent,  //add for TitleDialog
                                            const String& rOldText)
{
   TitleDialog* pDlg = new TitleDialog( pParent, rOldText);
   return new AbstractTitleDialog_Impl( pDlg );
}

AbstractGalleryIdDialog * AbstractDialogFactory_Impl::CreateGalleryIdDialog( Window* pParent,  //add for SvxZoomDialog
                                            GalleryTheme* pThm )
{
   GalleryIdDialog* pDlg = new GalleryIdDialog( pParent, pThm);
   return new AbstractGalleryIdDialog_Impl( pDlg );
}

VclAbstractDialog2 * AbstractDialogFactory_Impl::CreateGalleryThemePropertiesDialog( Window* pParent,  //add for GalleryThemeProperties
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet)
{
    Dialog* pDlg = new GalleryThemeProperties( pParent, pData, pItemSet);
    return new VclAbstractDialog2_Impl( pDlg );
}

AbstractURLDlg * AbstractDialogFactory_Impl::CreateURLDialog( Window* pParent,  //add for URLDlg
                                            const String& rURL, const String& rAltText, const String& rDescription,
                                            const String& rTarget, const String& rName,
                                            TargetList& rTargetList )
{
    URLDlg* pDlg = new URLDlg( pParent, rURL, rAltText, rDescription, rTarget, rName, rTargetList);
    return new AbstractURLDlg_Impl( pDlg );
}

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

SfxAbstractDialog* AbstractDialogFactory_Impl::CreateSfxDialog( sal_uInt32,
                                            Window* pParent,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& ,
                                            const SfxItemSet* pAttrSet )
{
    SfxModalDialog* pDlg = new SvxCharacterMap( pParent, sal_True, pAttrSet );
    return new CuiAbstractSfxDialog_Impl( pDlg );
}

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
        return new CuiAbstractTabDialog_Impl( pDlg );
    return 0;
}

VclAbstractDialog*      AbstractDialogFactory_Impl::CreateSvxSearchAttributeDialog( Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const sal_uInt16* pWhRanges )
{
    Dialog* pDlg = new SvxSearchAttributeDialog( pParent, rLst, pWhRanges);
    return new CuiVclAbstractDialog_Impl( pDlg );
}

AbstractSvxSearchSimilarityDialog * AbstractDialogFactory_Impl::CreateSvxSearchSimilarityDialog( Window* pParent,
                                                            sal_Bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger)
{
    SvxSearchSimilarityDialog* pDlg = new SvxSearchSimilarityDialog( pParent, bRelax, nOther, nShorter, nLonger );
    if ( pDlg )
        return new AbstractSvxSearchSimilarityDialog_Impl( pDlg );
    return NULL;
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxBorderBackgroundDlg( Window* pParent,
                                            const SfxItemSet& rCoreSet,
                                            sal_Bool bEnableSelector)
{
    SfxTabDialog* pDlg = new SvxBorderBackgroundDlg( pParent, rCoreSet, bEnableSelector);
    return new CuiAbstractTabDialog_Impl( pDlg );
}

AbstractSvxTransformTabDialog* AbstractDialogFactory_Impl::CreateSvxTransformTabDialog( Window* pParent,
                                                                              const SfxItemSet* pAttr,
                                                                                const SdrView* pView,
                                                                                sal_uInt16 nAnchorTypes )
{
    SvxTransformTabDialog* pDlg = new SvxTransformTabDialog( pParent, pAttr,pView, nAnchorTypes);
    return new AbstractSvxTransformTabDialog_Impl( pDlg );
}

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
        return new CuiAbstractTabDialog_Impl( pDlg );
    return 0;
}

AbstractSvxJSearchOptionsDialog * AbstractDialogFactory_Impl::CreateSvxJSearchOptionsDialog( Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            sal_Int32 nInitialFlags)
{
    SvxJSearchOptionsDialog* pDlg = new SvxJSearchOptionsDialog( pParent, rOptionsSet, nInitialFlags );
    return new AbstractSvxJSearchOptionsDialog_Impl( pDlg );
}

AbstractFmInputRecordNoDialog * AbstractDialogFactory_Impl::CreateFmInputRecordNoDialog( Window* pParent )
{
    FmInputRecordNoDialog* pDlg = new FmInputRecordNoDialog( pParent );
    return new AbstractFmInputRecordNoDialog_Impl( pDlg );
}

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
        return new CuiVclAbstractDialog_Impl( pDlg );
    return 0;
}

AbstractSvxNameDialog * AbstractDialogFactory_Impl::CreateSvxNameDialog( Window* pParent,
                                    const String& rName, const String& rDesc )
{
    SvxNameDialog* pDlg = new SvxNameDialog( pParent, rName, rDesc );
    return new AbstractSvxNameDialog_Impl( pDlg );
}

AbstractSvxObjectNameDialog* AbstractDialogFactory_Impl::CreateSvxObjectNameDialog(Window* pParent, const String& rName )
{
    return new AbstractSvxObjectNameDialog_Impl(new SvxObjectNameDialog(pParent, rName));
}

AbstractSvxObjectTitleDescDialog* AbstractDialogFactory_Impl::CreateSvxObjectTitleDescDialog(Window* pParent, const String& rTitle, const String& rDescription)
{
    return new AbstractSvxObjectTitleDescDialog_Impl(new SvxObjectTitleDescDialog(pParent, rTitle, rDescription));
}

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

AbstractSvxMultiPathDialog * AbstractDialogFactory_Impl::CreateSvxMultiPathDialog( Window* pParent, sal_Bool bEmptyAllowed )
{
    SvxMultiPathDialog* pDlg = new SvxMultiPathDialog( pParent, bEmptyAllowed );
    return new AbstractSvxMultiPathDialog_Impl( pDlg );
}

AbstractSvxMultiFileDialog * AbstractDialogFactory_Impl::CreateSvxMultiFileDialog( Window* pParent, sal_Bool bEmptyAllowed )
{
    SvxMultiFileDialog* pDlg = new SvxMultiFileDialog( pParent, bEmptyAllowed );
    return new AbstractSvxMultiFileDialog_Impl( pDlg );
}

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

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterEmboss (Window* pParent,  //add for GraphicFilterEmboss
                                            const Graphic& rGraphic,
                                            RECT_POINT eLightSource,
                                            sal_uInt32)
{
    GraphicFilterDialog* pDlg = new GraphicFilterEmboss( pParent, rGraphic, eLightSource );
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterPosterSepia (Window* pParent,
                                            const Graphic& rGraphic,
                                            sal_uInt16 nCount,
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

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterSmooth ( Window* pParent,
                                            const Graphic& rGraphic, double nRadius, sal_uInt32)
{
    GraphicFilterDialog* pDlg = new GraphicFilterSmooth( pParent, rGraphic, nRadius );
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterSolarize (Window* pParent,  //add for GraphicFilterSolarize
                                            const Graphic& rGraphic, sal_uInt8 nGreyThreshold, sal_Bool bInvert, sal_uInt32 )
{
    GraphicFilterDialog* pDlg = new GraphicFilterSolarize( pParent, rGraphic, nGreyThreshold, bInvert );
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterMosaic (Window* pParent,  //add for GraphicFilterMosaic
                                            const Graphic& rGraphic, sal_uInt16 nTileWidth, sal_uInt16 nTileHeight,
                                            sal_Bool bEnhanceEdges, sal_uInt32 nResId)
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

AbstractSvxAreaTabDialog* AbstractDialogFactory_Impl::CreateSvxAreaTabDialog( Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            const SdrView* pSdrView )
{
    SvxAreaTabDialog* pDlg = new SvxAreaTabDialog( pParent, pAttr, pModel,pSdrView );
    return new AbstractSvxAreaTabDialog_Impl( pDlg );
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr, //add forSvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj ,
                                                                sal_Bool bHasObj)
{
    SfxTabDialog* pDlg = new SvxLineTabDialog( pParent, pAttr, pModel,pObj,bHasObj );
    return new CuiAbstractTabDialog_Impl( pDlg );
}

SfxAbstractDialog* AbstractDialogFactory_Impl::CreateSfxDialog( Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                                                        const Reference< XFrame >& _rxDocumentFrame,
                                                                        sal_uInt32 nResId
                                                                        )
{
    SfxModalDialog* pDlg=NULL;
    switch ( nResId )
    {
        case SID_EVENTCONFIG :
            pDlg = new SfxMacroAssignDlg( pParent, _rxDocumentFrame, rAttr );
            break;
        case RID_SVXPAGE_MACROASSIGN :
            pDlg = new SvxShortcutAssignDlg( pParent, _rxDocumentFrame, rAttr );
            break;
        case RID_SVXDLG_CHARMAP :
            pDlg = new SvxCharacterMap( pParent, sal_True, &rAttr );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new CuiAbstractSfxDialog_Impl( pDlg );
    return 0;
}

SfxAbstractDialog* AbstractDialogFactory_Impl::CreateSfxDialog( Window* pParent,
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
        return new CuiAbstractSfxDialog_Impl( pDlg );
    return 0;
}

AbstractSvxPostItDialog* AbstractDialogFactory_Impl::CreateSvxPostItDialog( Window* pParent,
                                                                        const SfxItemSet& rCoreSet,
                                                                        sal_Bool bPrevNext, sal_Bool bRedline )
{
    SvxPostItDialog* pDlg = new SvxPostItDialog( pParent, rCoreSet, bPrevNext, bRedline );
    return new AbstractSvxPostItDialog_Impl( pDlg );
}

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

    virtual short Execute();
    virtual ~SvxMacroAssignDialog();

private:
    SfxItemSet                              m_aItems;
    ::std::auto_ptr< SvxMacroAssignDlg >    m_pDialog;
};

short SvxMacroAssignDialog::Execute()
{
    return m_pDialog->Execute();
}

SvxMacroAssignDialog::~SvxMacroAssignDialog()
{
}

VclAbstractDialog * AbstractDialogFactory_Impl::CreateSvxMacroAssignDlg(
    Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
    const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
{
    return new SvxMacroAssignDialog( _pParent, _rxDocumentFrame, _bUnoDialogMode, _rxEvents, _nInitiallySelectedEvent );
}

// Factories for TabPages
CreateTabPage AbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
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
        case RID_SVXPAGE_MACROASSIGN :
            return SfxMacroTabPage::Create;
        default:
            break;
    }

    return 0;
}

CreateSvxDistributePage AbstractDialogFactory_Impl::GetSvxDistributePageCreatorFunc()
{
    return SvxDistributePage::Create;
}

DialogGetRanges AbstractDialogFactory_Impl::GetDialogGetRangesFunc( sal_uInt16 nId )
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
GetTabPageRanges AbstractDialogFactory_Impl::GetTabPageRangesFunc( sal_uInt16 nId )
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

SfxAbstractInsertObjectDialog* AbstractDialogFactory_Impl::CreateInsertObjectDialog( Window* pParent, const rtl::OUString& rCommand,
            const Reference < com::sun::star::embed::XStorage >& xStor,
            const SvObjectServerList* pList )
{
    InsertObjectDialog_Impl* pDlg=0;
    if ( rCommand == ".uno:InsertObject" )
        pDlg = new SvInsertOleDlg( pParent, xStor, pList );
    else if ( rCommand == ".uno:InsertPlugin" )
        pDlg = new SvInsertPlugInDialog( pParent, xStor );
    else if ( rCommand == ".uno:InsertObjectFloatingFrame" )
        pDlg = new SfxInsertFloatingFrameDialog( pParent, xStor );

    if ( pDlg )
    {
        pDlg->SetHelpId( rtl::OUStringToOString( rCommand, RTL_TEXTENCODING_UTF8 ) );
        return new AbstractInsertObjectDialog_Impl( pDlg );
    }
    return 0;
}

VclAbstractDialog* AbstractDialogFactory_Impl::CreateEditObjectDialog( Window* pParent,  const rtl::OUString& rCommand,
            const Reference < com::sun::star::embed::XEmbeddedObject >& xObj )
{
    InsertObjectDialog_Impl* pDlg=0;
    if ( rCommand == ".uno:InsertObjectFloatingFrame" )
    {
        pDlg = new SfxInsertFloatingFrameDialog( pParent, xObj );
        pDlg->SetHelpId( rtl::OUStringToOString( rCommand, RTL_TEXTENCODING_UTF8 ) );
        return new CuiVclAbstractDialog_Impl( pDlg );
    }
    return 0;
}



SfxAbstractPasteDialog* AbstractDialogFactory_Impl::CreatePasteDialog( Window* pParent )
{
    return new AbstractPasteDialog_Impl( new SvPasteObjectDialog( pParent ) );
}

SfxAbstractLinksDialog* AbstractDialogFactory_Impl::CreateLinksDialog( Window* pParent, sfx2::LinkManager* pMgr, sal_Bool bHTML, sfx2::SvBaseLink* p)
{
    SvBaseLinksDlg* pLinkDlg = new SvBaseLinksDlg( pParent, pMgr, bHTML );
    if ( p )
        pLinkDlg->SetActLink(p);
    return new AbstractLinksDialog_Impl( pLinkDlg );
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxFormatCellsDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, const SdrObject* /*pObj*/ )
{
    return new CuiAbstractTabDialog_Impl( new SvxFormatCellsDialog( pParent, pAttr, pModel ) );
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
    return new CuiVclAbstractDialog_Impl( new OfaTreeOptionsDialog( pParent, rExtensionId ) );
}

SvxAbstractInsRowColDlg* AbstractDialogFactory_Impl::CreateSvxInsRowColDlg( Window* pParent, bool bCol, const rtl::OString& sHelpId )
{
    return new SvxInsRowColDlg( pParent, bCol, sHelpId );
}

AbstractPasswordToOpenModifyDialog * AbstractDialogFactory_Impl::CreatePasswordToOpenModifyDialog(
    Window * pParent,
    sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify )
{
    PasswordToOpenModifyDialog * pDlg = new PasswordToOpenModifyDialog( pParent, nMinPasswdLen, nMaxPasswdLen, bIsPasswordToModify );
    return new AbstractPasswordToOpenModifyDialog_Impl( pDlg );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
