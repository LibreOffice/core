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

#include "align.hxx"
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
#include "labdlg.hxx"
#include "hangulhanjadlg.hxx"
#include "showcols.hxx"
#include "zoom.hxx"
#include "cuigaldlg.hxx"
#include "transfrm.hxx"
#include "bbdlg.hxx"
#include "cuisrchdlg.hxx"
#include "cuitbxform.hxx"
#include "optdict.hxx"
#include "dlgname.hxx"
#include "multipat.hxx"
#include "cuihyperdlg.hxx"
#include "cuifmsearch.hxx"
#include "cuigrfflt.hxx"
#include "cuitabarea.hxx"
#include "cuitabline.hxx"
#include "measure.hxx"
#include "connect.hxx"
#include "dbregister.hxx"
#include "cuioptgenrl.hxx"
#include "optasian.hxx"
#include "insdlg.hxx"
#include "pastedlg.hxx"
#include "linkdlg.hxx"
#include "SpellDialog.hxx"
#include "cfg.hxx"
#include "numpages.hxx"
#include "paragrph.hxx"
#include "tabstpge.hxx"
#include "textattr.hxx"
#include "backgrnd.hxx"
#include "border.hxx"
#include "chardlg.hxx"
#include "page.hxx"
#include "postdlg.hxx"
#include "grfpage.hxx"
#include "scriptdlg.hxx"
#include "selector.hxx"
#include "macropg.hxx"
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
IMPL_ABSTDLG_BASE(AbstractSvxZoomDialog_Impl)
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
IMPL_ABSTDLG_BASE(AbstractSvxPathSelectDialog_Impl);
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


// VclAbstractDialog2_Impl


// virtual
VclAbstractDialog2_Impl::~VclAbstractDialog2_Impl()
{
    m_pDlg.disposeAndClear();
}

// virtual
void  VclAbstractDialog2_Impl::StartExecuteModal( const Link<>& rEndDialogHdl )
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
    m_aEndDlgHdl = Link<>();

    return 0L;
}



void CuiAbstractTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

void CuiAbstractTabDialog_Impl::SetCurPageId( const OString& rName )
{
    pDlg->SetCurPageId( rName );
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
void CuiAbstractTabDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}
OUString CuiAbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}


const SfxItemSet* CuiAbstractSfxDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

void CuiAbstractSfxDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

OUString CuiAbstractSfxDialog_Impl::GetText() const
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

void AbstractHangulHanjaConversionDialog_Impl::EnableRubySupport( bool _bVal )
{
    pDlg->EnableRubySupport(_bVal);
}

void AbstractHangulHanjaConversionDialog_Impl::SetByCharacter( bool _bByCharacter )
{
    pDlg->SetByCharacter(_bByCharacter);
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionDirectionState( bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection )
{
    pDlg->SetConversionDirectionState(_bTryBothDirections, _ePrimaryConversionDirection);
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType )
{
     pDlg->SetConversionFormat(_eType);
}

void AbstractHangulHanjaConversionDialog_Impl::SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl )
{
     pDlg->SetOptionsChangedHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetIgnoreHdl( const Link<Button*,void>& _rHdl )
{
     pDlg->SetIgnoreHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetIgnoreAllHdl( const Link<Button*,void>& _rHdl )
{
     pDlg->SetIgnoreAllHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetChangeHdl( const Link<Button*,void>& _rHdl )
{
    pDlg->SetChangeHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetChangeAllHdl( const Link<Button*,void>& _rHdl )
{
    pDlg->SetChangeAllHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetClickByCharacterHdl( const Link<CheckBox*,void>& _rHdl )
{
     pDlg->SetClickByCharacterHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionFormatChangedHdl( const Link<Button*,void>& _rHdl )
{
    pDlg->SetConversionFormatChangedHdl(_rHdl );
}
void AbstractHangulHanjaConversionDialog_Impl::SetFindHdl( const Link<Button*,void>& _rHdl )
{
     pDlg->SetFindHdl(_rHdl );
}

bool AbstractHangulHanjaConversionDialog_Impl::GetUseBothDirections( ) const
{
    return pDlg->GetUseBothDirections();
}

editeng::HangulHanjaConversion::ConversionDirection AbstractHangulHanjaConversionDialog_Impl::GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const
{
    return pDlg->GetDirection( _eDefaultDirection );
}

void AbstractHangulHanjaConversionDialog_Impl::SetCurrentString(
                    const OUString& _rNewString,
                    const css::uno::Sequence< OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument
                )
{
     pDlg->SetCurrentString(_rNewString,_rSuggestions,_bOriginatesFromDocument);
}
OUString   AbstractHangulHanjaConversionDialog_Impl::GetCurrentString( ) const
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

OUString  AbstractHangulHanjaConversionDialog_Impl::GetCurrentSuggestion( ) const
{
    return pDlg->GetCurrentSuggestion();
}

OUString AbstractThesaurusDialog_Impl::GetWord()
{
    return pDlg->GetWord();
};

vcl::Window* AbstractHyphenWordDialog_Impl::GetWindow()
{
    return pDlg;
}

Reference < css::embed::XEmbeddedObject > AbstractInsertObjectDialog_Impl::GetObject()
{
   return pDlg->GetObject();
}

bool AbstractInsertObjectDialog_Impl::IsCreateNew()
{
    return pDlg->IsCreateNew();
}

::Reference< css::io::XInputStream > AbstractInsertObjectDialog_Impl::GetIconIfIconified( OUString* pGraphicMediaType )
{
   return pDlg->GetIconIfIconified( pGraphicMediaType );
}

void AbstractPasteDialog_Impl::Insert( SotClipboardFormatId nFormat, const OUString & rFormatName )
{
    pDlg->Insert( nFormat, rFormatName );
}

void AbstractPasteDialog_Impl::SetObjName( const SvGlobalName & rClass, const OUString & rObjName )
{
    pDlg->SetObjName( rClass, rObjName );
}

SotClipboardFormatId AbstractPasteDialog_Impl::GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats,
                        const TransferableObjectDescriptor* pDesc )
{
    return pDlg->GetFormat( aHelper, pFormats, pDesc );
}

void  AbstractFmShowColsDialog_Impl::SetColumns(const ::Reference< css::container::XIndexContainer>& xCols)
{
     pDlg->SetColumns(xCols);
}

void AbstractSvxZoomDialog_Impl::SetLimits( sal_uInt16 nMin, sal_uInt16 nMax )
{
    pDlg->SetLimits( nMin, nMax );
}

void AbstractSvxZoomDialog_Impl::HideButton( ZoomButtonId nBtnId )
{
    pDlg->HideButton( nBtnId );
}

const SfxItemSet* AbstractSvxZoomDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

void  AbstractSpellDialog_Impl::Invalidate()
{
    pDlg->InvalidateDialog();
}

vcl::Window*     AbstractSpellDialog_Impl::GetWindow()
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

void AbstractTakeProgress_Impl::Update()
{
     pDlg->Update();
}

OUString AbstractTitleDialog_Impl::GetTitle() const
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

OUString AbstractURLDlg_Impl::GetURL() const
{
    return pDlg->GetURL();
}

OUString AbstractURLDlg_Impl::GetAltText() const
{
    return pDlg->GetAltText();
}

OUString AbstractURLDlg_Impl::GetDesc() const
{
    return pDlg->GetDesc();
}

OUString AbstractURLDlg_Impl::GetTarget() const
{
    return pDlg->GetTarget();
}

OUString AbstractURLDlg_Impl::GetName() const
{
    return pDlg->GetName();
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

bool AbstractSvxSearchSimilarityDialog_Impl::IsRelaxed()
{
    return pDlg->IsRelaxed();
}

// AbstractSvxTransformTabDialog implementations just forwards everything to the dialog
void AbstractSvxTransformTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

void AbstractSvxTransformTabDialog_Impl::SetCurPageId( const OString& rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSvxTransformTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractSvxTransformTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void AbstractSvxTransformTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractSvxTransformTabDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}
OUString AbstractSvxTransformTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}
void AbstractSvxTransformTabDialog_Impl::SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink )
{
    pDlg->SetValidateFramePosLink( rLink );
}

// AbstractSvxCaptionDialog implementations just forwards everything to the dialog
void AbstractSvxCaptionDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

void AbstractSvxCaptionDialog_Impl::SetCurPageId( const OString& rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSvxCaptionDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractSvxCaptionDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void AbstractSvxCaptionDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractSvxCaptionDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}
OUString AbstractSvxCaptionDialog_Impl::GetText() const
{
    return pDlg->GetText();
}
void AbstractSvxCaptionDialog_Impl::SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink )
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

::Reference< css::linguistic2::XDictionary > AbstractSvxNewDictionaryDialog_Impl::GetNewDictionary()
{
    return pDlg->GetNewDictionary();
}

void AbstractSvxNameDialog_Impl::GetName(OUString& rName)
{
    pDlg->GetName( rName );
}

void AbstractSvxNameDialog_Impl::SetCheckNameHdl( const Link<AbstractSvxNameDialog&,bool>& rLink, bool bCheckImmediately )
{
    aCheckNameHdl = rLink;
    if( rLink.IsSet() )
        pDlg->SetCheckNameHdl( LINK(this, AbstractSvxNameDialog_Impl, CheckNameHdl), bCheckImmediately );
    else
        pDlg->SetCheckNameHdl( Link<SvxNameDialog&,bool>(), bCheckImmediately );
}
void AbstractSvxNameDialog_Impl::SetEditHelpId(const OString& aHelpId)
{
    pDlg->SetEditHelpId( aHelpId );
}
void AbstractSvxNameDialog_Impl::SetHelpId( const OString& aHelpId )
{
    pDlg->SetHelpId( aHelpId );
}
void AbstractSvxNameDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}
IMPL_LINK_NOARG_TYPED(AbstractSvxNameDialog_Impl, CheckNameHdl, SvxNameDialog&, bool)
{
    if( aCheckNameHdl.IsSet() )
        return aCheckNameHdl.Call(*this);
    return false;
}

void AbstractSvxObjectNameDialog_Impl::GetName(OUString& rName)
{
    pDlg->GetName(rName);
}

void AbstractSvxObjectNameDialog_Impl::SetCheckNameHdl(const Link<AbstractSvxObjectNameDialog&,bool>& rLink, bool bCheckImmediately)
{
    aCheckNameHdl = rLink;

    if(rLink.IsSet())
    {
        pDlg->SetCheckNameHdl(LINK(this, AbstractSvxObjectNameDialog_Impl, CheckNameHdl), bCheckImmediately);
    }
    else
    {
        pDlg->SetCheckNameHdl(Link<SvxObjectNameDialog&,bool>(), bCheckImmediately);
    }
}

IMPL_LINK_NOARG_TYPED(AbstractSvxObjectNameDialog_Impl, CheckNameHdl, SvxObjectNameDialog&, bool)
{
    if(aCheckNameHdl.IsSet())
    {
        return aCheckNameHdl.Call(*this);
    }

    return false;
}

void AbstractSvxObjectTitleDescDialog_Impl::GetTitle(OUString& rTitle)
{
    pDlg->GetTitle(rTitle);
}

void AbstractSvxObjectTitleDescDialog_Impl::GetDescription(OUString& rDescription)
{
    pDlg->GetDescription(rDescription);
}

OUString AbstractSvxMultiPathDialog_Impl::GetPath() const
{
    return pDlg->GetPath();
}

void AbstractSvxMultiPathDialog_Impl::SetPath( const OUString& rPath )
{
    pDlg->SetPath( rPath );
}

void AbstractSvxMultiPathDialog_Impl::SetTitle( const OUString& rNewTitle )
{
    pDlg->SetText( rNewTitle );
}

OUString AbstractSvxPathSelectDialog_Impl::GetPath() const
{
    return pDlg->GetPath();
}

void AbstractSvxPathSelectDialog_Impl::SetPath( const OUString& rPath )
{
    pDlg->SetPath( rPath );
}

void AbstractSvxPathSelectDialog_Impl::SetTitle( const OUString& rNewTitle )
{
    pDlg->SetText( rNewTitle );
}

vcl::Window * AbstractSvxHpLinkDlg_Impl::GetWindow()
{
    return static_cast<vcl::Window *>(pDlg);
}

bool AbstractSvxHpLinkDlg_Impl::QueryClose()
{
    return pDlg->QueryClose();
}


void AbstractFmSearchDialog_Impl::SetFoundHandler(const Link<FmFoundRecordInformation&,void>& lnk)
{
    pDlg->SetFoundHandler(lnk);
}
void AbstractFmSearchDialog_Impl::SetCanceledNotFoundHdl(const Link<>& lnk)
{
    pDlg->SetCanceledNotFoundHdl(lnk);
}
void AbstractFmSearchDialog_Impl::SetActiveField(const OUString& strField)
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

void AbstractSvxAreaTabDialog_Impl::SetCurPageId( const OString& rName )
{
    pDlg->SetCurPageId( rName );
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
void AbstractSvxAreaTabDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}
OUString AbstractSvxAreaTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

void AbstractSvxPostItDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}
const SfxItemSet* AbstractSvxPostItDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
void AbstractSvxPostItDialog_Impl::EnableTravel(bool bNext, bool bPrev)
{
    pDlg->EnableTravel( bNext, bPrev );
}
OUString AbstractSvxPostItDialog_Impl::GetNote()
{
    return pDlg->GetNote();
}
void AbstractSvxPostItDialog_Impl::SetNote(const OUString& rTxt)
{
    pDlg->SetNote( rTxt );
}
void AbstractSvxPostItDialog_Impl::ShowLastAuthor(const OUString& rAuthor, const OUString& rDate)
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
void AbstractSvxPostItDialog_Impl::SetNextHdl( const Link<AbstractSvxPostItDialog&,void>& rLink )
{
    aNextHdl = rLink;
    if( rLink.IsSet() )
        pDlg->SetNextHdl( LINK(this, AbstractSvxPostItDialog_Impl, NextHdl ) );
    else
        pDlg->SetNextHdl( Link<SvxPostItDialog&,void>() );
}
void AbstractSvxPostItDialog_Impl::SetPrevHdl( const Link<AbstractSvxPostItDialog&,void>& rLink )
{
    aPrevHdl = rLink;
    if( rLink.IsSet() )
        pDlg->SetPrevHdl( LINK(this, AbstractSvxPostItDialog_Impl, PrevHdl ) );
    else
        pDlg->SetPrevHdl( Link<SvxPostItDialog&,void>() );
}
IMPL_LINK_NOARG_TYPED(AbstractSvxPostItDialog_Impl, NextHdl, SvxPostItDialog&, void)
{
    if( aNextHdl.IsSet() )
        aNextHdl.Call(*this);
}
IMPL_LINK_NOARG_TYPED(AbstractSvxPostItDialog_Impl, PrevHdl, SvxPostItDialog&, void)
{
    if( aPrevHdl.IsSet() )
        aPrevHdl.Call(*this);
}
vcl::Window * AbstractSvxPostItDialog_Impl::GetWindow()
{
    return static_cast<vcl::Window *>(pDlg);
}

OUString AbstractPasswordToOpenModifyDialog_Impl::GetPasswordToOpen() const
{
    return pDlg->GetPasswordToOpen();
}
OUString AbstractPasswordToOpenModifyDialog_Impl::GetPasswordToModify() const
{
    return pDlg->GetPasswordToModify();
}
bool AbstractPasswordToOpenModifyDialog_Impl::IsRecommendToOpenReadonly() const
{
    return pDlg->IsRecommendToOpenReadonly();
}

// Create dialogs with simplest interface
VclAbstractDialog* AbstractDialogFactory_Impl::CreateVclDialog( vcl::Window* pParent, sal_uInt32 nResId )
{
    VclPtr<Dialog> pDlg;
    switch ( nResId )
    {
        case RID_DEFAULTABOUT:
        {
            pDlg = VclPtr<AboutDialog>::Create(pParent);
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
            VclPtrInstance<OfaTreeOptionsDialog> pOptDlg( pParent, xFrame, bActivateLastSelection );
            if (nResId == SID_OPTIONS_DATABASES)
            {
                pOptDlg->ActivatePage(SID_SB_DBREGISTEROPTIONS);
            }
            else if (nResId == SID_LANGUAGE_OPTIONS)
            {
                //open the tab page "tools/options/languages"
                pOptDlg->ActivatePage(OFA_TP_LANGUAGES_FOR_SET_DOCUMENT_LANGUAGE);
            }
            pDlg.reset(pOptDlg);
        }
        break;
        default:
            break;
    }

    if ( pDlg )
        return new CuiVclAbstractDialog_Impl( pDlg );
    return 0;
}

VclAbstractDialog* AbstractDialogFactory_Impl::CreateFrameDialog(
    vcl::Window* pParent, const Reference< frame::XFrame >& rxFrame,
    sal_uInt32 nResId, const OUString& rParameter )
{
    VclPtr<Dialog> pDlg;
    if ( SID_OPTIONS_TREEDIALOG == nResId || SID_OPTIONS_DATABASES == nResId )
    {
        // only activate last page if we dont want to activate a special page
        bool bActivateLastSelection = ( nResId != SID_OPTIONS_DATABASES && rParameter.isEmpty() );
        VclPtrInstance<OfaTreeOptionsDialog> pOptDlg( pParent, rxFrame, bActivateLastSelection );
        if ( nResId == SID_OPTIONS_DATABASES )
            pOptDlg->ActivatePage(SID_SB_DBREGISTEROPTIONS);
        else if ( !rParameter.isEmpty() )
            pOptDlg->ActivatePage( rParameter );
        pDlg.reset(pOptDlg);
    }

    if ( pDlg )
        return new CuiVclAbstractDialog_Impl( pDlg );
    else
        return NULL;
}

// TabDialog outside the drawing layer
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTabDialog( sal_uInt32 nResId,
                                                vcl::Window* pParent,
                                                const SfxItemSet* pAttrSet,
                                                SfxViewFrame* ,
                                                bool /*bEditFmt*/,
                                                const OUString * )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_OFA_AUTOCORR_DLG :
            pDlg = VclPtr<OfaAutoCorrDlg>::Create( pParent, pAttrSet );
            break;
        case RID_SVXDLG_CUSTOMIZE :
            pDlg = VclPtr<SvxConfigDialog>::Create( pParent, pAttrSet );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new CuiAbstractTabDialog_Impl( pDlg );
    return 0;
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTabDialog( sal_uInt32 nResId,
                                                vcl::Window* pParent,
                                                const SfxItemSet* pAttrSet,
                                                const Reference< frame::XFrame >& xViewFrame,
                                                bool /*bEditFmt*/,
                                                const OUString * )
{
    VclPtr<SfxTabDialog> pDlg;
    switch ( nResId )
    {
        case RID_OFA_AUTOCORR_DLG :
            pDlg = VclPtr<OfaAutoCorrDlg>::Create( pParent, pAttrSet );
            break;
        case RID_SVXDLG_CUSTOMIZE :
            {
                VclPtrInstance<SvxConfigDialog> pDlg1( pParent, pAttrSet );
                pDlg1->SetFrame(xViewFrame);
                pDlg.reset(pDlg1);
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
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTextTabDialog( vcl::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView,
                                            SdrModel* )
{
    VclPtrInstance<SvxTextTabDialog> pDlg( pParent, pAttrSet, pView );
    return new CuiAbstractTabDialog_Impl( pDlg );
}

// TabDialog that use functionality of the drawing layer and add AnchorTypes -- for SvxCaptionTabDialog
AbstractSvxCaptionDialog*       AbstractDialogFactory_Impl::CreateCaptionDialog( vcl::Window* pParent,
                                            const SdrView* pView,
                                            sal_uInt16 nAnchorTypes )
{
    VclPtrInstance<SvxCaptionTabDialog> pDlg( pParent, pView, nAnchorTypes );
    return new AbstractSvxCaptionDialog_Impl( pDlg );
}

AbstractSvxDistributeDialog*    AbstractDialogFactory_Impl::CreateSvxDistributeDialog(vcl::Window* pParent,
                                            const SfxItemSet& rAttr,
                                            SvxDistributeHorizontal eHor ,
                                            SvxDistributeVertical eVer)
{
    VclPtrInstance<SvxDistributeDialog> pDlg( pParent, rAttr, eHor, eVer);
    return new AbstractSvxDistributeDialog_Impl( pDlg );
}

AbstractHangulHanjaConversionDialog* AbstractDialogFactory_Impl::CreateHangulHanjaConversionDialog(vcl::Window* pParent,
                                                                       editeng::HangulHanjaConversion::ConversionDirection _ePrimaryDirection )
{
    VclPtrInstance<HangulHanjaConversionDialog> pDlg( pParent, _ePrimaryDirection);
    return new AbstractHangulHanjaConversionDialog_Impl( pDlg );
}

AbstractThesaurusDialog* AbstractDialogFactory_Impl::CreateThesaurusDialog( vcl::Window* pParent,
                                css::uno::Reference< css::linguistic2::XThesaurus >  xThesaurus,
                                const OUString &rWord, sal_Int16 nLanguage )
{
    VclPtrInstance<SvxThesaurusDialog> pDlg( pParent, xThesaurus, rWord, nLanguage );
    return new AbstractThesaurusDialog_Impl( pDlg );
}

AbstractHyphenWordDialog* AbstractDialogFactory_Impl::CreateHyphenWordDialog( vcl::Window* pParent,
                                                const OUString &rWord, LanguageType nLang,
                                                css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper )
{
    VclPtrInstance<SvxHyphenWordDialog> pDlg( rWord, nLang, pParent, xHyphen, pWrapper );
    return new AbstractHyphenWordDialog_Impl( pDlg );
}

AbstractFmShowColsDialog * AbstractDialogFactory_Impl::CreateFmShowColsDialog( vcl::Window* pParent )
{
    VclPtrInstance<FmShowColsDialog> pDlg( pParent);
    return new AbstractFmShowColsDialog_Impl( pDlg );
}
AbstractSvxZoomDialog * AbstractDialogFactory_Impl::CreateSvxZoomDialog( vcl::Window* pParent,
                                            const SfxItemSet& rCoreSet)
{
    VclPtrInstance<SvxZoomDialog> pDlg( pParent, rCoreSet);
    return new AbstractSvxZoomDialog_Impl( pDlg );
}

AbstractSpellDialog *  AbstractDialogFactory_Impl::CreateSvxSpellDialog(
                        vcl::Window* pParent,
                        SfxBindings* pBindings,
                        svx::SpellDialogChildWindow* pSpellChildWindow )
{
    VclPtrInstance<svx::SpellDialog> pDlg(pSpellChildWindow, pParent, pBindings);
    return new AbstractSpellDialog_Impl(pDlg);
}

VclAbstractRefreshableDialog * AbstractDialogFactory_Impl::CreateActualizeProgressDialog( vcl::Window* pParent, GalleryTheme* pThm )
{
   VclPtrInstance<ActualizeProgress> pDlg(pParent, pThm);
   return new VclAbstractRefreshableDialog_Impl( pDlg );
}

VclAbstractDialog*
AbstractDialogFactory_Impl::CreateScriptErrorDialog(
    vcl::Window* pParent, const css::uno::Any& rException)
{
    return new SvxScriptErrorDialog(pParent, rException);
}

AbstractScriptSelectorDialog*
AbstractDialogFactory_Impl::CreateScriptSelectorDialog(
    vcl::Window* pParent, bool bShowSlots, const Reference< frame::XFrame >& _rxFrame )
{
    VclPtrInstance<SvxScriptSelectorDialog> pDlg(pParent, bShowSlots, _rxFrame);
    return new AbstractScriptSelectorDialog_Impl(pDlg);
}

OUString AbstractScriptSelectorDialog_Impl::GetScriptURL() const
{
    if (pDlg)
        return pDlg->GetScriptURL();
    return OUString();
}

void AbstractScriptSelectorDialog_Impl::SetRunLabel()
{
    if (pDlg)
        pDlg->SetRunLabel();
    return;
}

VclAbstractDialog * AbstractDialogFactory_Impl::CreateSvxScriptOrgDialog( vcl::Window* pParent,
                                            const OUString& rLanguage)
{
    VclPtrInstance<SvxScriptOrgDialog> pDlg( pParent, rLanguage);
    return new CuiVclAbstractDialog_Impl( pDlg );
}

AbstractTitleDialog * AbstractDialogFactory_Impl::CreateTitleDialog( vcl::Window* pParent,
                                            const OUString& rOldText)
{
   VclPtrInstance<TitleDialog> pDlg( pParent, rOldText);
   return new AbstractTitleDialog_Impl( pDlg );
}

AbstractGalleryIdDialog * AbstractDialogFactory_Impl::CreateGalleryIdDialog( vcl::Window* pParent,
                                            GalleryTheme* pThm )
{
   VclPtrInstance<GalleryIdDialog> pDlg( pParent, pThm);
   return new AbstractGalleryIdDialog_Impl( pDlg );
}

VclAbstractDialog2 * AbstractDialogFactory_Impl::CreateGalleryThemePropertiesDialog( vcl::Window* pParent,
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet)
{
    VclPtrInstance<GalleryThemeProperties> pDlg( pParent, pData, pItemSet);
    return new VclAbstractDialog2_Impl( pDlg );
}

AbstractURLDlg * AbstractDialogFactory_Impl::CreateURLDialog( vcl::Window* pParent,
                                            const OUString& rURL, const OUString& rAltText, const OUString& rDescription,
                                            const OUString& rTarget, const OUString& rName,
                                            TargetList& rTargetList )
{
    VclPtrInstance<URLDlg> pDlg( pParent, rURL, rAltText, rDescription,
                                 rTarget, rName, rTargetList);
    return new AbstractURLDlg_Impl( pDlg );
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateTabItemDialog(vcl::Window* pParent,
    const SfxItemSet& rSet)
{
    VclPtrInstance<SvxSearchFormatDialog> pDlg(pParent, rSet);
    return new CuiAbstractTabDialog_Impl(pDlg);
}

VclAbstractDialog*      AbstractDialogFactory_Impl::CreateSvxSearchAttributeDialog( vcl::Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const sal_uInt16* pWhRanges )
{
    VclPtrInstance<SvxSearchAttributeDialog> pDlg( pParent, rLst, pWhRanges);
    return new CuiVclAbstractDialog_Impl( pDlg );
}

AbstractSvxSearchSimilarityDialog * AbstractDialogFactory_Impl::CreateSvxSearchSimilarityDialog( vcl::Window* pParent,
                                                            bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger)
{
    VclPtrInstance<SvxSearchSimilarityDialog> pDlg( pParent, bRelax, nOther, nShorter, nLonger );
    return new AbstractSvxSearchSimilarityDialog_Impl( pDlg );
}

//UUUU
SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxBorderBackgroundDlg(
    vcl::Window* pParent,
    const SfxItemSet& rCoreSet,
    bool bEnableSelector,
    bool bEnableDrawingLayerFillStyles)
{
    VclPtrInstance<SvxBorderBackgroundDlg> pDlg(
        pParent,
        rCoreSet,
        bEnableSelector,
        bEnableDrawingLayerFillStyles);

    return new CuiAbstractTabDialog_Impl(pDlg);
}

AbstractSvxTransformTabDialog* AbstractDialogFactory_Impl::CreateSvxTransformTabDialog( vcl::Window* pParent,
                                                                              const SfxItemSet* pAttr,
                                                                                const SdrView* pView,
                                                                                sal_uInt16 nAnchorTypes )
{
    VclPtrInstance<SvxTransformTabDialog> pDlg( pParent, pAttr,pView, nAnchorTypes);
    return new AbstractSvxTransformTabDialog_Impl( pDlg );
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSchTransformTabDialog( vcl::Window* pParent,
                                                                const SfxItemSet* pAttr,
                                                                const SdrView* pSdrView,
                                                                sal_uInt32 nResId,
                                                                bool bSizeTabPage
                                                                )
{
    VclPtr<SfxTabDialog> pDlg;
    switch ( nResId )
    {
        case RID_SCH_TransformTabDLG_SVXPAGE_ANGLE :
            {
            pDlg.reset( VclPtr<SvxTransformTabDialog>::Create( pParent, pAttr,pSdrView, bSizeTabPage ? SVX_OBJ_NOPROTECT :  SVX_OBJ_NOPROTECT|SVX_OBJ_NORESIZE) );
            pDlg->RemoveTabPage( "RID_SVXPAGE_ANGLE" );
            pDlg->RemoveTabPage( "RID_SVXPAGE_SLANT" );
            }
            break;
        case RID_SCH_TransformTabDLG_SVXPAGE_SLANT:
            {
            pDlg.reset(VclPtr<SvxTransformTabDialog>::Create ( pParent, pAttr,pSdrView, bSizeTabPage ? SVX_OBJ_NOPROTECT :  SVX_OBJ_NOPROTECT|SVX_OBJ_NORESIZE ) );
            pDlg->RemoveTabPage( "RID_SVXPAGE_SLANT" );
            }
            break;
        default:
            break;
    }

    if ( pDlg )
        return new CuiAbstractTabDialog_Impl( pDlg );
    return 0;
}

AbstractSvxJSearchOptionsDialog * AbstractDialogFactory_Impl::CreateSvxJSearchOptionsDialog( vcl::Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            sal_Int32 nInitialFlags)
{
    VclPtrInstance<SvxJSearchOptionsDialog> pDlg( pParent, rOptionsSet, nInitialFlags );
    return new AbstractSvxJSearchOptionsDialog_Impl( pDlg );
}

AbstractFmInputRecordNoDialog * AbstractDialogFactory_Impl::CreateFmInputRecordNoDialog( vcl::Window* pParent )
{
    VclPtrInstance<FmInputRecordNoDialog> pDlg( pParent );
    return new AbstractFmInputRecordNoDialog_Impl( pDlg );
}

AbstractSvxNewDictionaryDialog * AbstractDialogFactory_Impl::CreateSvxNewDictionaryDialog( vcl::Window* pParent,
                                            ::Reference< css::linguistic2::XSpellChecker1 >  &xSpl )
{
    VclPtrInstance<SvxNewDictionaryDialog> pDlg( pParent, xSpl );
    return new AbstractSvxNewDictionaryDialog_Impl( pDlg );
}

VclAbstractDialog*      AbstractDialogFactory_Impl::CreateSvxEditDictionaryDialog( vcl::Window* pParent,
                                            const OUString& rName,
                                            ::Reference< css::linguistic2::XSpellChecker1> &xSpl,
                                            sal_uInt32 nResId )
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SFXDLG_EDITDICT :
            pDlg = VclPtr<SvxEditDictionaryDialog>::Create( pParent, rName, xSpl );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new CuiVclAbstractDialog_Impl( pDlg );
    return 0;
}

AbstractSvxNameDialog * AbstractDialogFactory_Impl::CreateSvxNameDialog( vcl::Window* pParent,
                                    const OUString& rName, const OUString& rDesc )
{
    VclPtrInstance<SvxNameDialog> pDlg( pParent, rName, rDesc );
    return new AbstractSvxNameDialog_Impl( pDlg );
}

AbstractSvxObjectNameDialog* AbstractDialogFactory_Impl::CreateSvxObjectNameDialog(vcl::Window* pParent, const OUString& rName )
{
    return new AbstractSvxObjectNameDialog_Impl(VclPtr<SvxObjectNameDialog>::Create(pParent, rName));
}

AbstractSvxObjectTitleDescDialog* AbstractDialogFactory_Impl::CreateSvxObjectTitleDescDialog(vcl::Window* pParent, const OUString& rTitle, const OUString& rDescription)
{
    return new AbstractSvxObjectTitleDescDialog_Impl(VclPtr<SvxObjectTitleDescDialog>::Create(pParent, rTitle, rDescription));
}

AbstractSvxMultiPathDialog * AbstractDialogFactory_Impl::CreateSvxMultiPathDialog(vcl::Window* pParent)
{
    VclPtrInstance<SvxMultiPathDialog> pDlg(pParent);
    return new AbstractSvxMultiPathDialog_Impl( pDlg );
}

AbstractSvxMultiPathDialog * AbstractDialogFactory_Impl::CreateSvxPathSelectDialog(vcl::Window* pParent)
{
    VclPtrInstance<SvxPathSelectDialog> pDlg(pParent);
    return new AbstractSvxPathSelectDialog_Impl( pDlg );
}

AbstractSvxHpLinkDlg * AbstractDialogFactory_Impl::CreateSvxHpLinkDlg (vcl::Window* pParent,
                                            SfxBindings* pBindings,
                                            sal_uInt32 nResId)
{
    SvxHpLinkDlg* pDlg=NULL;
    switch ( nResId )
    {
        case SID_HYPERLINK_DIALOG :
            pDlg = VclPtr<SvxHpLinkDlg>::Create( pParent, pBindings );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSvxHpLinkDlg_Impl( pDlg );
    return 0;
}

AbstractFmSearchDialog*  AbstractDialogFactory_Impl::CreateFmSearchDialog(vcl::Window* pParent,
                                                        const OUString& strInitialText,
                                                        const ::std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier)
{
    VclPtrInstance<FmSearchDialog> pDlg( pParent, strInitialText, _rContexts,
                                         nInitialContext, lnkContextSupplier );
    return new AbstractFmSearchDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterEmboss (vcl::Window* pParent,
                                            const Graphic& rGraphic,
                                            RECT_POINT eLightSource)
{
    VclPtrInstance<GraphicFilterEmboss> pDlg( pParent, rGraphic, eLightSource );
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterPoster(vcl::Window* pParent,
                                            const Graphic& rGraphic,
                                            sal_uInt16 nCount)
{
    VclPtrInstance<GraphicFilterPoster> pDlg( pParent, rGraphic, nCount );
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterSepia (vcl::Window* pParent,
                                            const Graphic& rGraphic,
                                            sal_uInt16 nCount)
{
    VclPtrInstance<GraphicFilterSepia> pDlg( pParent, rGraphic, nCount );
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterSmooth(vcl::Window* pParent,
                                            const Graphic& rGraphic, double nRadius)
{
    VclPtrInstance<GraphicFilterSmooth> pDlg( pParent, rGraphic, nRadius );
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterSolarize (vcl::Window* pParent,
                                            const Graphic& rGraphic, sal_uInt8 nGreyThreshold, bool bInvert)
{
    VclPtrInstance<GraphicFilterSolarize> pDlg( pParent, rGraphic, nGreyThreshold, bInvert );
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractGraphicFilterDialog * AbstractDialogFactory_Impl::CreateGraphicFilterMosaic (vcl::Window* pParent,
                                            const Graphic& rGraphic, sal_uInt16 nTileWidth, sal_uInt16 nTileHeight,
                                            bool bEnhanceEdges)
{
    VclPtrInstance<GraphicFilterMosaic> pDlg(pParent, rGraphic, nTileWidth, nTileHeight, bEnhanceEdges);
    return new AbstractGraphicFilterDialog_Impl( pDlg );
}

AbstractSvxAreaTabDialog* AbstractDialogFactory_Impl::CreateSvxAreaTabDialog( vcl::Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            bool bShadow)
{
    VclPtrInstance<SvxAreaTabDialog> pDlg( pParent, pAttr, pModel, bShadow );
    return new AbstractSvxAreaTabDialog_Impl( pDlg );
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxLineTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr, //add forSvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj ,
                                                                 bool bHasObj)
{
    VclPtrInstance<SvxLineTabDialog> pDlg( pParent, pAttr, pModel,pObj,bHasObj );
    return new CuiAbstractTabDialog_Impl( pDlg );
}

SfxAbstractDialog* AbstractDialogFactory_Impl::CreateSfxDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                                                        const Reference< XFrame >& _rxDocumentFrame,
                                                                        sal_uInt32 nResId
                                                                        )
{
    SfxModalDialog* pDlg=NULL;
    switch ( nResId )
    {
        case SID_EVENTCONFIG :
            pDlg = VclPtr<SfxMacroAssignDlg>::Create( pParent, _rxDocumentFrame, rAttr );
            break;
        case RID_SVXDLG_CHARMAP :
            pDlg = VclPtr<SvxCharacterMap>::Create( pParent, true, &rAttr );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new CuiAbstractSfxDialog_Impl( pDlg );
    return 0;
}

SfxAbstractDialog* AbstractDialogFactory_Impl::CreateSfxDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                                                        const SdrView* pView,
                                                                        sal_uInt32 nResId
                                                                        )
{

    SfxSingleTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RID_SVXPAGE_MEASURE :
            pDlg = VclPtr<SvxMeasureDialog>::Create( pParent, rAttr, pView );
            break;
        case RID_SVXPAGE_CONNECTION :
            pDlg = VclPtr<SvxConnectionDialog>::Create( pParent, rAttr, pView );
            break;
        case RID_SFXPAGE_DBREGISTER :
            pDlg = VclPtr<DatabaseRegistrationDialog>::Create( pParent, rAttr );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new CuiAbstractSfxDialog_Impl( pDlg );

    return 0;
}

AbstractSvxPostItDialog* AbstractDialogFactory_Impl::CreateSvxPostItDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rCoreSet,
                                                                        bool bPrevNext )
{
    VclPtrInstance<SvxPostItDialog> pDlg( pParent, rCoreSet, bPrevNext );
    return new AbstractSvxPostItDialog_Impl( pDlg );
}

class SvxMacroAssignDialog : public VclAbstractDialog
{
public:
    SvxMacroAssignDialog( vcl::Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
            const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
        :m_aItems( SfxGetpApp()->GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM )
    {
        m_aItems.Put( SfxBoolItem( SID_ATTR_MACROITEM, _bUnoDialogMode ) );
        m_pDialog.reset( VclPtr<SvxMacroAssignDlg>::Create( _pParent, _rxDocumentFrame, m_aItems, _rxEvents, _nInitiallySelectedEvent ) );
    }

    virtual short Execute() SAL_OVERRIDE;
    virtual ~SvxMacroAssignDialog();

private:
    SfxItemSet                              m_aItems;
    VclPtr< SvxMacroAssignDlg >             m_pDialog;
};

short SvxMacroAssignDialog::Execute()
{
    return m_pDialog->Execute();
}

SvxMacroAssignDialog::~SvxMacroAssignDialog()
{
}

VclAbstractDialog * AbstractDialogFactory_Impl::CreateSvxMacroAssignDlg(
    vcl::Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
    const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
{
    return new SvxMacroAssignDialog( _pParent, _rxDocumentFrame, _bUnoDialogMode, _rxEvents, _nInitiallySelectedEvent );
}

// Factories for TabPages
CreateTabPage AbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
{
    switch ( nId )
    {
        case RID_SVXPAGE_TEXTANIMATION :
            return SvxTextAnimationPage::Create;
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

DialogGetRanges AbstractDialogFactory_Impl::GetDialogGetRangesFunc()
{
    return SvxPostItDialog::GetRanges;
}

GetTabPageRanges AbstractDialogFactory_Impl::GetTabPageRangesFunc( sal_uInt16 nId )
{
    switch ( nId )
    {
        case RID_SVXPAGE_TEXTANIMATION :
            return SvxTextAnimationPage::GetRanges;
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

SfxAbstractInsertObjectDialog* AbstractDialogFactory_Impl::CreateInsertObjectDialog( vcl::Window* pParent, const OUString& rCommand,
            const Reference < css::embed::XStorage >& xStor,
            const SvObjectServerList* pList )
{
    InsertObjectDialog_Impl* pDlg=0;
    if ( rCommand == ".uno:InsertObject" )
        pDlg = VclPtr<SvInsertOleDlg>::Create( pParent, xStor, pList );
    else if ( rCommand == ".uno:InsertPlugin" )
        pDlg = VclPtr<SvInsertPlugInDialog>::Create( pParent, xStor );
    else if ( rCommand == ".uno:InsertObjectFloatingFrame" )
        pDlg = VclPtr<SfxInsertFloatingFrameDialog>::Create( pParent, xStor );

    if ( pDlg )
    {
        pDlg->SetHelpId( OUStringToOString( rCommand, RTL_TEXTENCODING_UTF8 ) );
        return new AbstractInsertObjectDialog_Impl( pDlg );
    }
    return 0;
}

VclAbstractDialog* AbstractDialogFactory_Impl::CreateEditObjectDialog( vcl::Window* pParent,  const OUString& rCommand,
            const Reference < css::embed::XEmbeddedObject >& xObj )
{
    if ( rCommand == ".uno:InsertObjectFloatingFrame" )
    {
        VclPtrInstance<SfxInsertFloatingFrameDialog> pDlg( pParent, xObj );
        pDlg->SetHelpId( OUStringToOString( rCommand, RTL_TEXTENCODING_UTF8 ) );
        return new CuiVclAbstractDialog_Impl( pDlg );
    }
    return 0;
}



SfxAbstractPasteDialog* AbstractDialogFactory_Impl::CreatePasteDialog( vcl::Window* pParent )
{
    return new AbstractPasteDialog_Impl( VclPtr<SvPasteObjectDialog>::Create( pParent ) );
}

SfxAbstractLinksDialog* AbstractDialogFactory_Impl::CreateLinksDialog( vcl::Window* pParent, sfx2::LinkManager* pMgr, bool bHTML, sfx2::SvBaseLink* p)
{
    VclPtrInstance<SvBaseLinksDlg> pLinkDlg( pParent, pMgr, bHTML );
    if ( p )
        pLinkDlg->SetActLink(p);
    return new AbstractLinksDialog_Impl( pLinkDlg );
}

SfxAbstractTabDialog* AbstractDialogFactory_Impl::CreateSvxFormatCellsDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, const SdrObject* /*pObj*/ )
{
    return new CuiAbstractTabDialog_Impl( VclPtr<SvxFormatCellsDialog>::Create( pParent, pAttr, pModel ) );
}

SvxAbstractSplittTableDialog* AbstractDialogFactory_Impl::CreateSvxSplittTableDialog( vcl::Window* pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal )
{
    return new SvxSplitTableDlg( pParent, bIsTableVertical, nMaxVertical, nMaxHorizontal );
}

SvxAbstractNewTableDialog* AbstractDialogFactory_Impl::CreateSvxNewTableDialog( vcl::Window* pParent )
{
    return new SvxNewTableDialog( pParent );
}

VclAbstractDialog* AbstractDialogFactory_Impl::CreateOptionsDialog(
    vcl::Window* pParent, const OUString& rExtensionId, const OUString& /*rApplicationContext*/ )
{
    return new CuiVclAbstractDialog_Impl( VclPtr<OfaTreeOptionsDialog>::Create( pParent, rExtensionId ) );
}

SvxAbstractInsRowColDlg* AbstractDialogFactory_Impl::CreateSvxInsRowColDlg( vcl::Window* pParent, bool bCol, const OString& sHelpId )
{
    return new SvxInsRowColDlg( pParent, bCol, sHelpId );
}

AbstractPasswordToOpenModifyDialog * AbstractDialogFactory_Impl::CreatePasswordToOpenModifyDialog(
    vcl::Window * pParent,
    sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify )
{
    VclPtrInstance<PasswordToOpenModifyDialog> pDlg( pParent, nMinPasswdLen, nMaxPasswdLen, bIsPasswordToModify );
    return new AbstractPasswordToOpenModifyDialog_Impl( pDlg );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
