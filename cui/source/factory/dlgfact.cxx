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

#include <config_extensions.h>

#include <align.hxx>
#include "dlgfact.hxx"

#include <about.hxx>
#include <sfx2/app.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/pageids.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <numfmt.hxx>
#include <splitcelldlg.hxx>
#include <cuiimapwnd.hxx>
#include <cui/cuicharmap.hxx>
#include <srchxtra.hxx>
#include <textanim.hxx>
#include <autocdlg.hxx>
#include <treeopt.hxx>
#include <labdlg.hxx>
#include <hangulhanjadlg.hxx>
#include <showcols.hxx>
#include <zoom.hxx>
#include <cuigaldlg.hxx>
#include <transfrm.hxx>
#include <bbdlg.hxx>
#include <cuisrchdlg.hxx>
#include <cuitbxform.hxx>
#include <optdict.hxx>
#include <dlgname.hxx>
#include <multipat.hxx>
#include <cuihyperdlg.hxx>
#include <cuifmsearch.hxx>
#include <cuigrfflt.hxx>
#include <cuitabarea.hxx>
#include <cuitabline.hxx>
#include <measure.hxx>
#include <connect.hxx>
#include <dbregister.hxx>
#include <cuioptgenrl.hxx>
#include <optasian.hxx>
#include <insdlg.hxx>
#include <pastedlg.hxx>
#include <linkdlg.hxx>
#include <SignatureLineDialog.hxx>
#include <SignSignatureLineDialog.hxx>
#include <QrCodeGenDialog.hxx>
#include <SpellDialog.hxx>
#include <cfg.hxx>
#include <numpages.hxx>
#include <paragrph.hxx>
#include <tabstpge.hxx>
#include <textattr.hxx>
#include <backgrnd.hxx>
#include <border.hxx>
#include <chardlg.hxx>
#include <page.hxx>
#include <postdlg.hxx>
#include <grfpage.hxx>
#include <scriptdlg.hxx>
#include <cfgutil.hxx>
#include <macropg.hxx>
#include <sdrcelldlg.hxx>
#include <newtabledlg.hxx>
#include <macroass.hxx>
#include <insrc.hxx>
#include <passwdomdlg.hxx>
#include <screenshotannotationdlg.hxx>
#include <hyphen.hxx>
#include <thesdlg.hxx>
#include <tipofthedaydlg.hxx>
#include <toolbarmodedlg.hxx>
#include <DiagramDialog.hxx>
#include <fileextcheckdlg.hxx>
#include <TextColumnsPage.hxx>
#include <themepage.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

using ::com::sun::star::uno::Reference;

using namespace svx;

IMPL_ABSTDLG_CLASS(AbstractAdditionsDialog)
IMPL_ABSTDLG_CLASS(AbstractDiagramDialog)
IMPL_ABSTDLG_CLASS(AbstractFmInputRecordNoDialog)
IMPL_ABSTDLG_CLASS(AbstractFmSearchDialog)
IMPL_ABSTDLG_CLASS(AbstractFmShowColsDialog)
IMPL_ABSTDLG_CLASS(AbstractGalleryIdDialog)
IMPL_ABSTDLG_CLASS(AbstractGraphicFilterDialog)
IMPL_ABSTDLG_CLASS(AbstractHangulHanjaConversionDialog)
IMPL_ABSTDLG_CLASS(AbstractHyphenWordDialog)
IMPL_ABSTDLG_CLASS(AbstractInsertObjectDialog)
IMPL_ABSTDLG_CLASS(AbstractLinksDialog)
IMPL_ABSTDLG_CLASS(AbstractPasswordToOpenModifyDialog)
IMPL_ABSTDLG_CLASS(AbstractQrCodeGenDialog)
IMPL_ABSTDLG_CLASS(AbstractScreenshotAnnotationDlg)
IMPL_ABSTDLG_CLASS(AbstractSignatureLineDialog)
IMPL_ABSTDLG_CLASS(AbstractSignSignatureLineDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxCharacterMapDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxHpLinkDlg)
IMPL_ABSTDLG_CLASS(AbstractSvxJSearchOptionsDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxMultiPathDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxNameDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxNewDictionaryDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxObjectNameDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxObjectTitleDescDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxPathSelectDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxPostItDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxSearchSimilarityDialog)
IMPL_ABSTDLG_CLASS(AbstractSvxZoomDialog)
IMPL_ABSTDLG_CLASS(AbstractTitleDialog)
IMPL_ABSTDLG_CLASS(AbstractURLDlg)
IMPL_ABSTDLG_CLASS_ASYNC(AbstractPasteDialog,SfxDialogController)
IMPL_ABSTDLG_CLASS_ASYNC(AbstractScriptSelectorDialog,SfxDialogController)
IMPL_ABSTDLG_CLASS_ASYNC(AbstractSpellDialog,SfxDialogController)
IMPL_ABSTDLG_CLASS_ASYNC(AbstractSvxAreaTabDialog,SfxTabDialogController)
IMPL_ABSTDLG_CLASS_ASYNC(AbstractSvxCaptionDialog,SfxTabDialogController)
IMPL_ABSTDLG_CLASS_ASYNC(AbstractSvxTransformTabDialog,SfxTabDialogController)
IMPL_ABSTDLG_CLASS_ASYNC(AbstractThesaurusDialog,SfxDialogController)
IMPL_ABSTDLG_CLASS_ASYNC(CuiAbstractControllerAsync,weld::DialogController)
IMPL_ABSTDLG_CLASS_ASYNC(CuiAbstractTabController,SfxTabDialogController)
IMPL_ABSTDLG_CLASS(CuiAbstractController)
IMPL_ABSTDLG_CLASS(CuiAbstractSingleTabController)

const SfxItemSet* AbstractSvxCharacterMapDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

void AbstractSvxCharacterMapDialog_Impl::SetText(const OUString& rStr)
{
    m_xDlg->set_title(rStr);
}

void CuiAbstractTabController_Impl::SetCurPageId( const OString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* CuiAbstractTabController_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer CuiAbstractTabController_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void CuiAbstractTabController_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

//From class Window.
void CuiAbstractTabController_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

std::vector<OString> CuiAbstractTabController_Impl::getAllPageUIXMLDescriptions() const
{
    return m_xDlg->getAllPageUIXMLDescriptions();
}

bool CuiAbstractTabController_Impl::selectPageByUIXMLDescription(const OString& rUIXMLDescription)
{
    return m_xDlg->selectPageByUIXMLDescription(rUIXMLDescription);
}

BitmapEx CuiAbstractTabController_Impl::createScreenshot() const
{
    return m_xDlg->createScreenshot();
}

OString CuiAbstractTabController_Impl::GetScreenshotId() const
{
    return m_xDlg->GetScreenshotId();
}

const SfxItemSet* CuiAbstractSingleTabController_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

void CuiAbstractSingleTabController_Impl::SetText(const OUString& rStr)
{
    m_xDlg->set_title(rStr);
}

void AbstractHangulHanjaConversionDialog_Impl::EndDialog(sal_Int32 nResult)
{
    m_xDlg->response(nResult);
}

void AbstractHangulHanjaConversionDialog_Impl::EnableRubySupport( bool _bVal )
{
    m_xDlg->EnableRubySupport(_bVal);
}

void AbstractHangulHanjaConversionDialog_Impl::SetByCharacter( bool _bByCharacter )
{
    m_xDlg->SetByCharacter(_bByCharacter);
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionDirectionState( bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection )
{
    m_xDlg->SetConversionDirectionState(_bTryBothDirections, _ePrimaryConversionDirection);
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType )
{
    m_xDlg->SetConversionFormat(_eType);
}

void AbstractHangulHanjaConversionDialog_Impl::SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl )
{
    m_xDlg->SetOptionsChangedHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetIgnoreHdl( const Link<weld::Button&,void>& _rHdl )
{
    m_xDlg->SetIgnoreHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetIgnoreAllHdl(const Link<weld::Button&,void>& rHdl)
{
    m_xDlg->SetIgnoreAllHdl(rHdl);
}

void AbstractHangulHanjaConversionDialog_Impl::SetChangeHdl(const Link<weld::Button&,void>& rHdl)
{
    m_xDlg->SetChangeHdl(rHdl);
}

void AbstractHangulHanjaConversionDialog_Impl::SetChangeAllHdl( const Link<weld::Button&,void>& rHdl )
{
    m_xDlg->SetChangeAllHdl(rHdl);
}

void AbstractHangulHanjaConversionDialog_Impl::SetClickByCharacterHdl( const Link<weld::Toggleable&,void>& _rHdl )
{
    m_xDlg->SetClickByCharacterHdl(_rHdl );
}

void AbstractHangulHanjaConversionDialog_Impl::SetConversionFormatChangedHdl(const Link<weld::Toggleable&,void>& rHdl)
{
    m_xDlg->SetConversionFormatChangedHdl(rHdl);
}

void AbstractHangulHanjaConversionDialog_Impl::SetFindHdl( const Link<weld::Button&,void>& rHdl )
{
    m_xDlg->SetFindHdl(rHdl);
}

bool AbstractHangulHanjaConversionDialog_Impl::GetUseBothDirections( ) const
{
    return m_xDlg->GetUseBothDirections();
}

editeng::HangulHanjaConversion::ConversionDirection AbstractHangulHanjaConversionDialog_Impl::GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const
{
    return m_xDlg->GetDirection( _eDefaultDirection );
}

void AbstractHangulHanjaConversionDialog_Impl::SetCurrentString(
                    const OUString& _rNewString,
                    const css::uno::Sequence< OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument
                )
{
     m_xDlg->SetCurrentString(_rNewString,_rSuggestions,_bOriginatesFromDocument);
}

OUString AbstractHangulHanjaConversionDialog_Impl::GetCurrentString( ) const
{
    return m_xDlg->GetCurrentString();
}

editeng::HangulHanjaConversion::ConversionFormat AbstractHangulHanjaConversionDialog_Impl::GetConversionFormat( ) const
{
    return m_xDlg->GetConversionFormat();
}

void AbstractHangulHanjaConversionDialog_Impl::FocusSuggestion( )
{
    m_xDlg->FocusSuggestion();
}

OUString AbstractHangulHanjaConversionDialog_Impl::GetCurrentSuggestion( ) const
{
    return m_xDlg->GetCurrentSuggestion();
}

OUString AbstractThesaurusDialog_Impl::GetWord()
{
    return m_xDlg->GetWord();
};

Reference < css::embed::XEmbeddedObject > AbstractInsertObjectDialog_Impl::GetObject()
{
   return m_xDlg->GetObject();
}

bool AbstractInsertObjectDialog_Impl::IsCreateNew()
{
    return m_xDlg->IsCreateNew();
}

::Reference< css::io::XInputStream > AbstractInsertObjectDialog_Impl::GetIconIfIconified( OUString* pGraphicMediaType )
{
   return m_xDlg->GetIconIfIconified( pGraphicMediaType );
}

void AbstractPasteDialog_Impl::Insert(SotClipboardFormatId nFormat, const OUString& rFormatName)
{
    m_xDlg->Insert(nFormat, rFormatName);
}

void AbstractPasteDialog_Impl::InsertUno(const OUString& sCmd, const OUString& sLabel)
{
    m_xDlg->InsertUno(sCmd, sLabel);
}

void AbstractPasteDialog_Impl::SetObjName(const SvGlobalName & rClass, const OUString& rObjName)
{
    m_xDlg->SetObjName(rClass, rObjName);
}

void AbstractPasteDialog_Impl::PreGetFormat( const TransferableDataHelper& aHelper )
{
    m_xDlg->PreGetFormat(aHelper);
}

SotClipboardFormatId AbstractPasteDialog_Impl::GetFormatOnly()
{
    return m_xDlg->GetFormatOnly();
}

SotClipboardFormatId AbstractPasteDialog_Impl::GetFormat( const TransferableDataHelper& aHelper )
{
    return m_xDlg->GetFormat(aHelper);
}

void AbstractFmShowColsDialog_Impl::SetColumns(const ::Reference< css::container::XIndexContainer>& xCols)
{
     m_xDlg->SetColumns(xCols);
}

void AbstractSvxZoomDialog_Impl::SetLimits( sal_uInt16 nMin, sal_uInt16 nMax )
{
    m_xDlg->SetLimits( nMin, nMax );
}

void AbstractSvxZoomDialog_Impl::HideButton( ZoomButtonId nBtnId )
{
    m_xDlg->HideButton( nBtnId );
}

const SfxItemSet* AbstractSvxZoomDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

void AbstractSpellDialog_Impl::InvalidateDialog()
{
    m_xDlg->InvalidateDialog();
}

std::shared_ptr<SfxDialogController> AbstractSpellDialog_Impl::GetController()
{
    return m_xDlg;
}

SfxBindings& AbstractSpellDialog_Impl::GetBindings()
{
    return m_xDlg->GetBindings();
}

OUString AbstractTitleDialog_Impl::GetTitle() const
{
    return m_xDlg->GetTitle();
}

sal_uInt32 AbstractGalleryIdDialog_Impl::GetId() const
{
    return m_xDlg->GetId();
}

OUString AbstractURLDlg_Impl::GetURL() const
{
    return m_xDlg->GetURL();
}

OUString AbstractURLDlg_Impl::GetAltText() const
{
    return m_xDlg->GetAltText();
}

OUString AbstractURLDlg_Impl::GetDesc() const
{
    return m_xDlg->GetDesc();
}

OUString AbstractURLDlg_Impl::GetTarget() const
{
    return m_xDlg->GetTarget();
}

OUString AbstractURLDlg_Impl::GetName() const
{
    return m_xDlg->GetName();
}

sal_uInt16 AbstractSvxSearchSimilarityDialog_Impl::GetOther()
{
    return m_xDlg->GetOther();
}

sal_uInt16 AbstractSvxSearchSimilarityDialog_Impl::GetShorter()
{
    return m_xDlg->GetShorter();
}

sal_uInt16 AbstractSvxSearchSimilarityDialog_Impl::GetLonger()
{
    return m_xDlg->GetLonger();
}

bool AbstractSvxSearchSimilarityDialog_Impl::IsRelaxed()
{
    return m_xDlg->IsRelaxed();
}

// AbstractSvxTransformTabDialog implementations just forwards everything to the dialog
void AbstractSvxTransformTabDialog_Impl::SetCurPageId( const OString& rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSvxTransformTabDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer AbstractSvxTransformTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void AbstractSvxTransformTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

//From class Window.
void AbstractSvxTransformTabDialog_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

void AbstractSvxTransformTabDialog_Impl::SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink )
{
    m_xDlg->SetValidateFramePosLink( rLink );
}

// AbstractSvxCaptionDialog implementations just forwards everything to the dialog
void AbstractSvxCaptionDialog_Impl::SetCurPageId( const OString& rName )
{
    m_xDlg->SetCurPageId(rName);
}

const SfxItemSet* AbstractSvxCaptionDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer AbstractSvxCaptionDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void AbstractSvxCaptionDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

void AbstractSvxCaptionDialog_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

void AbstractSvxCaptionDialog_Impl::SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink )
{
    m_xDlg->SetValidateFramePosLink( rLink );
}

TransliterationFlags AbstractSvxJSearchOptionsDialog_Impl::GetTransliterationFlags() const
{
    return m_xDlg->GetTransliterationFlags();
}

void AbstractFmInputRecordNoDialog_Impl::SetValue(tools::Long nNew)
{
    m_xDlg->SetValue(nNew);
}

tools::Long AbstractFmInputRecordNoDialog_Impl::GetValue() const
{
    return m_xDlg->GetValue();
}

::Reference< css::linguistic2::XDictionary > AbstractSvxNewDictionaryDialog_Impl::GetNewDictionary()
{
    return m_xDlg->GetNewDictionary();
}

void AbstractSvxNameDialog_Impl::GetName(OUString& rName)
{
    rName = m_xDlg->GetName();
}

void AbstractSvxNameDialog_Impl::SetCheckNameHdl( const Link<AbstractSvxNameDialog&,bool>& rLink, bool bCheckImmediately )
{
    aCheckNameHdl = rLink;
    if( rLink.IsSet() )
        m_xDlg->SetCheckNameHdl( LINK(this, AbstractSvxNameDialog_Impl, CheckNameHdl), bCheckImmediately );
    else
        m_xDlg->SetCheckNameHdl( Link<SvxNameDialog&,bool>(), bCheckImmediately );
}

void AbstractSvxNameDialog_Impl::SetCheckNameTooltipHdl( const Link<AbstractSvxNameDialog&,OUString>& rLink)
{
    aCheckNameTooltipHdl = rLink;
    if( rLink.IsSet() )
        m_xDlg->SetCheckNameTooltipHdl( LINK(this, AbstractSvxNameDialog_Impl, CheckNameTooltipHdl));
    else
        m_xDlg->SetCheckNameTooltipHdl( Link<SvxNameDialog&,OUString>());
}

void AbstractSvxNameDialog_Impl::SetEditHelpId(const OString& rHelpId)
{
    m_xDlg->SetEditHelpId(rHelpId);
}

void AbstractSvxNameDialog_Impl::SetHelpId(const OString& rHelpId)
{
    m_xDlg->set_help_id(rHelpId);
}

void AbstractSvxNameDialog_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

IMPL_LINK_NOARG(AbstractSvxNameDialog_Impl, CheckNameHdl, SvxNameDialog&, bool)
{
    return aCheckNameHdl.Call(*this);
}

IMPL_LINK_NOARG(AbstractSvxNameDialog_Impl, CheckNameTooltipHdl, SvxNameDialog&, OUString)
{
    return aCheckNameTooltipHdl.Call(*this);
}

void AbstractSvxObjectNameDialog_Impl::GetName(OUString& rName)
{
    rName = m_xDlg->GetName();
}

void AbstractSvxObjectNameDialog_Impl::SetCheckNameHdl(const Link<AbstractSvxObjectNameDialog&,bool>& rLink)
{
    aCheckNameHdl = rLink;

    if(rLink.IsSet())
    {
        m_xDlg->SetCheckNameHdl(LINK(this, AbstractSvxObjectNameDialog_Impl, CheckNameHdl));
    }
    else
    {
        m_xDlg->SetCheckNameHdl(Link<SvxObjectNameDialog&,bool>());
    }
}

IMPL_LINK_NOARG(AbstractSvxObjectNameDialog_Impl, CheckNameHdl, SvxObjectNameDialog&, bool)
{
    return aCheckNameHdl.Call(*this);
}

void AbstractSvxObjectTitleDescDialog_Impl::GetTitle(OUString& rTitle)
{
    rTitle = m_xDlg->GetTitle();
}

void AbstractSvxObjectTitleDescDialog_Impl::GetDescription(OUString& rDescription)
{
    rDescription = m_xDlg->GetDescription();
}

OUString AbstractSvxMultiPathDialog_Impl::GetPath() const
{
    return m_xDlg->GetPath();
}

void AbstractSvxMultiPathDialog_Impl::SetPath( const OUString& rPath )
{
    m_xDlg->SetPath( rPath );
}

void AbstractSvxMultiPathDialog_Impl::SetTitle( const OUString& rNewTitle )
{
    m_xDlg->SetTitle(rNewTitle);
}

OUString AbstractSvxPathSelectDialog_Impl::GetPath() const
{
    return m_xDlg->GetPath();
}

void AbstractSvxPathSelectDialog_Impl::SetPath( const OUString& rPath )
{
    m_xDlg->SetPath( rPath );
}

void AbstractSvxPathSelectDialog_Impl::SetTitle( const OUString& rNewTitle )
{
    m_xDlg->SetTitle(rNewTitle);
}

std::shared_ptr<SfxDialogController> AbstractSvxHpLinkDlg_Impl::GetController()
{
    return m_xDlg;
}

bool AbstractSvxHpLinkDlg_Impl::QueryClose()
{
    return m_xDlg->QueryClose();
}

void AbstractFmSearchDialog_Impl::SetFoundHandler(const Link<FmFoundRecordInformation&,void>& lnk)
{
    m_xDlg->SetFoundHandler(lnk);
}

void AbstractFmSearchDialog_Impl::SetCanceledNotFoundHdl(const Link<FmFoundRecordInformation&,void>& lnk)
{
    m_xDlg->SetCanceledNotFoundHdl(lnk);
}

void AbstractFmSearchDialog_Impl::SetActiveField(const OUString& strField)
{
    m_xDlg->SetActiveField(strField);
}

Graphic AbstractGraphicFilterDialog_Impl::GetFilteredGraphic(const Graphic& rGraphic, double fScaleX, double fScaleY)
{
    return m_xDlg->GetFilteredGraphic(rGraphic, fScaleX, fScaleY);
}

// AbstractSvxAreaTabDialog implementations just forwards everything to the dialog
void AbstractSvxAreaTabDialog_Impl::SetCurPageId( const OString& rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSvxAreaTabDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer AbstractSvxAreaTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void AbstractSvxAreaTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

void AbstractSvxAreaTabDialog_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

void AbstractSvxPostItDialog_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

const SfxItemSet* AbstractSvxPostItDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

void AbstractSvxPostItDialog_Impl::EnableTravel(bool bNext, bool bPrev)
{
    m_xDlg->EnableTravel( bNext, bPrev );
}

OUString AbstractSvxPostItDialog_Impl::GetNote()
{
    return m_xDlg->GetNote();
}

void AbstractSvxPostItDialog_Impl::SetNote(const OUString& rTxt)
{
    m_xDlg->SetNote( rTxt );
}

void AbstractSvxPostItDialog_Impl::ShowLastAuthor(const OUString& rAuthor, const OUString& rDate)
{
    m_xDlg->ShowLastAuthor( rAuthor, rDate );
}

void AbstractSvxPostItDialog_Impl::DontChangeAuthor()
{
    m_xDlg->DontChangeAuthor();
}

void AbstractSvxPostItDialog_Impl::HideAuthor()
{
    m_xDlg->HideAuthor();
}

void AbstractSvxPostItDialog_Impl::SetNextHdl( const Link<AbstractSvxPostItDialog&,void>& rLink )
{
    aNextHdl = rLink;
    if( rLink.IsSet() )
        m_xDlg->SetNextHdl( LINK(this, AbstractSvxPostItDialog_Impl, NextHdl ) );
    else
        m_xDlg->SetNextHdl( Link<SvxPostItDialog&,void>() );
}

void AbstractSvxPostItDialog_Impl::SetPrevHdl( const Link<AbstractSvxPostItDialog&,void>& rLink )
{
    aPrevHdl = rLink;
    if( rLink.IsSet() )
        m_xDlg->SetPrevHdl( LINK(this, AbstractSvxPostItDialog_Impl, PrevHdl ) );
    else
        m_xDlg->SetPrevHdl( Link<SvxPostItDialog&,void>() );
}

IMPL_LINK_NOARG(AbstractSvxPostItDialog_Impl, NextHdl, SvxPostItDialog&, void)
{
    aNextHdl.Call(*this);
}

IMPL_LINK_NOARG(AbstractSvxPostItDialog_Impl, PrevHdl, SvxPostItDialog&, void)
{
    aPrevHdl.Call(*this);
}

std::shared_ptr<weld::Dialog> AbstractSvxPostItDialog_Impl::GetDialog()
{
    return m_xDlg->GetDialog();
}

OUString AbstractPasswordToOpenModifyDialog_Impl::GetPasswordToOpen() const
{
    return m_xDlg->GetPasswordToOpen();
}

OUString AbstractPasswordToOpenModifyDialog_Impl::GetPasswordToModify() const
{
    return m_xDlg->GetPasswordToModify();
}

bool AbstractPasswordToOpenModifyDialog_Impl::IsRecommendToOpenReadonly() const
{
    return m_xDlg->IsRecommendToOpenReadonly();
}

// Create dialogs with simplest interface
VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateVclDialog(weld::Window* pParent, sal_uInt32 nResId)
{
    std::unique_ptr<OfaTreeOptionsDialog> xDlg;
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
            xDlg = std::make_unique<OfaTreeOptionsDialog>(pParent, xFrame, bActivateLastSelection);
            if (nResId == SID_OPTIONS_DATABASES)
            {
                xDlg->ActivatePage(SID_SB_DBREGISTEROPTIONS);
            }
            else if (nResId == SID_LANGUAGE_OPTIONS)
            {
                //open the tab page "tools/options/languages"
                xDlg->ActivatePage(OFA_TP_LANGUAGES_FOR_SET_DOCUMENT_LANGUAGE);
            }
        }
        break;
        default:
            break;
    }

    if (xDlg)
        return VclPtr<CuiAbstractController_Impl>::Create(std::move(xDlg));
    return nullptr;
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateFrameDialog(weld::Window* pParent, const Reference< frame::XFrame >& rxFrame,
    sal_uInt32 nResId, const OUString& rParameter )
{
    std::unique_ptr<OfaTreeOptionsDialog> xDlg;
    if (SID_OPTIONS_TREEDIALOG == nResId || SID_OPTIONS_DATABASES == nResId)
    {
        // only activate last page if we don't want to activate a special page
        bool bActivateLastSelection = ( nResId != SID_OPTIONS_DATABASES && rParameter.isEmpty() );
        xDlg = std::make_unique<OfaTreeOptionsDialog>(pParent, rxFrame, bActivateLastSelection);
        if ( nResId == SID_OPTIONS_DATABASES )
            xDlg->ActivatePage(SID_SB_DBREGISTEROPTIONS);
        else if ( !rParameter.isEmpty() )
            xDlg->ActivatePage( rParameter );
    }

    if (xDlg)
        return VclPtr<CuiAbstractController_Impl>::Create(std::move(xDlg));
    return nullptr;
}

// TabDialog outside the drawing layer
VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateAutoCorrTabDialog(weld::Window* pParent, const SfxItemSet* pAttrSet)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<OfaAutoCorrDlg>(pParent, pAttrSet));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateCustomizeTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pAttrSet,
                                                const Reference< frame::XFrame >& xViewFrame )
{
    auto xDlg1 = std::make_shared<SvxConfigDialog>(pParent, pAttrSet);
    xDlg1->SetFrame(xViewFrame);
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::move(xDlg1));
}

// TabDialog that use functionality of the drawing layer
VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateTextTabDialog(weld::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxTextTabDialog>(pParent, pAttrSet, pView));
}

// TabDialog that use functionality of the drawing layer and add AnchorTypes -- for SvxCaptionTabDialog
VclPtr<AbstractSvxCaptionDialog> AbstractDialogFactory_Impl::CreateCaptionDialog(weld::Window* pParent,
                                                                                 const SdrView* pView,
                                                                                 SvxAnchorIds nAnchorTypes)
{
    return VclPtr<AbstractSvxCaptionDialog_Impl>::Create(std::make_shared<SvxCaptionTabDialog>(pParent, pView, nAnchorTypes));
}

VclPtr<AbstractHangulHanjaConversionDialog> AbstractDialogFactory_Impl::CreateHangulHanjaConversionDialog(weld::Widget* pParent)
{
    return VclPtr<AbstractHangulHanjaConversionDialog_Impl>::Create(std::make_unique<HangulHanjaConversionDialog>(pParent));
}

VclPtr<AbstractThesaurusDialog> AbstractDialogFactory_Impl::CreateThesaurusDialog(weld::Widget* pParent,
                                css::uno::Reference<css::linguistic2::XThesaurus> xThesaurus,
                                const OUString &rWord, LanguageType nLanguage)
{
    return VclPtr<AbstractThesaurusDialog_Impl>::Create(std::make_shared<SvxThesaurusDialog>(pParent, xThesaurus, rWord, nLanguage));
}

VclPtr<AbstractHyphenWordDialog> AbstractDialogFactory_Impl::CreateHyphenWordDialog(weld::Widget* pParent,
                                                const OUString &rWord, LanguageType nLang,
                                                css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper)
{
    return VclPtr<AbstractHyphenWordDialog_Impl>::Create(std::make_unique<SvxHyphenWordDialog>(rWord, nLang, pParent, xHyphen, pWrapper));
}

VclPtr<AbstractFmShowColsDialog> AbstractDialogFactory_Impl::CreateFmShowColsDialog(weld::Window* pParent)
{
    return VclPtr<AbstractFmShowColsDialog_Impl>::Create(std::make_unique<FmShowColsDialog>(pParent));
}

VclPtr<AbstractSvxZoomDialog> AbstractDialogFactory_Impl::CreateSvxZoomDialog(weld::Window* pParent, const SfxItemSet& rCoreSet)
{
    return VclPtr<AbstractSvxZoomDialog_Impl>::Create(std::make_unique<SvxZoomDialog>(pParent, rCoreSet));
}

VclPtr<AbstractSpellDialog> AbstractDialogFactory_Impl::CreateSvxSpellDialog(
                        weld::Window* pParent,
                        SfxBindings* pBindings,
                        svx::SpellDialogChildWindow* pSpellChildWindow)
{
    return VclPtr<AbstractSpellDialog_Impl>::Create(std::make_shared<svx::SpellDialog>(pSpellChildWindow, pParent, pBindings));
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateActualizeProgressDialog(weld::Widget* pParent,
                                                                                               GalleryTheme* pThm)
{
   return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<ActualizeProgress>(pParent, pThm));
}

void AbstractDialogFactory_Impl::ShowAsyncScriptErrorDialog(weld::Window* pParent, const css::uno::Any& rException)
{
    return SvxScriptErrorDialog::ShowAsyncErrorDialog(pParent, rException);
}

VclPtr<AbstractScriptSelectorDialog> AbstractDialogFactory_Impl::CreateScriptSelectorDialog(weld::Window* pParent,
        const Reference<frame::XFrame>& rxFrame)
{
    return VclPtr<AbstractScriptSelectorDialog_Impl>::Create(std::make_shared<SvxScriptSelectorDialog>(pParent, rxFrame));
}

OUString AbstractScriptSelectorDialog_Impl::GetScriptURL() const
{
    return m_xDlg->GetScriptURL();
}

void AbstractScriptSelectorDialog_Impl::SetRunLabel()
{
    m_xDlg->SetRunLabel();
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateSvxScriptOrgDialog(weld::Window* pParent,
                                            const OUString& rLanguage)
{
    return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<SvxScriptOrgDialog>(pParent, rLanguage));
}

VclPtr<AbstractTitleDialog> AbstractDialogFactory_Impl::CreateTitleDialog(weld::Widget* pParent,
                                            const OUString& rOldText)
{
   return VclPtr<AbstractTitleDialog_Impl>::Create(std::make_unique<TitleDialog>(pParent, rOldText));
}

VclPtr<AbstractGalleryIdDialog> AbstractDialogFactory_Impl::CreateGalleryIdDialog(weld::Widget* pParent,
                                            GalleryTheme* pThm)
{
   return VclPtr<AbstractGalleryIdDialog_Impl>::Create(std::make_unique<GalleryIdDialog>(pParent, pThm));
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateGalleryThemePropertiesDialog(weld::Widget* pParent,
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<GalleryThemeProperties>(
                                                         pParent, pData, pItemSet));
}

VclPtr<AbstractURLDlg> AbstractDialogFactory_Impl::CreateURLDialog(weld::Widget* pParent,
                                            const OUString& rURL, const OUString& rAltText, const OUString& rDescription,
                                            const OUString& rTarget, const OUString& rName,
                                            TargetList& rTargetList )
{
    return VclPtr<AbstractURLDlg_Impl>::Create(std::make_unique<URLDlg>(pParent, rURL, rAltText, rDescription,
                                                                        rTarget, rName, rTargetList));

}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateTabItemDialog(weld::Window* pParent,
    const SfxItemSet& rSet)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxSearchFormatDialog>(
                                                         pParent, rSet));
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateSvxSearchAttributeDialog(weld::Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const WhichRangesContainer& pWhRanges )
{
    return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<SvxSearchAttributeDialog>(pParent, rLst, pWhRanges));
}

VclPtr<AbstractSvxSearchSimilarityDialog> AbstractDialogFactory_Impl::CreateSvxSearchSimilarityDialog(weld::Window* pParent,
                                                            bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger)
{
    return VclPtr<AbstractSvxSearchSimilarityDialog_Impl>::Create(std::make_unique<SvxSearchSimilarityDialog>(pParent, bRelax, nOther, nShorter, nLonger));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateSvxBorderBackgroundDlg(
    weld::Window* pParent,
    const SfxItemSet& rCoreSet,
    bool bEnableDrawingLayerFillStyles)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxBorderBackgroundDlg>(
        pParent,
        rCoreSet,
        /*bEnableSelector*/true,
        bEnableDrawingLayerFillStyles));
}

VclPtr<AbstractSvxTransformTabDialog> AbstractDialogFactory_Impl::CreateSvxTransformTabDialog(weld::Window* pParent,
                                                                                              const SfxItemSet* pAttr,
                                                                                              const SdrView* pView,
                                                                                              SvxAnchorIds nAnchorTypes)
{
    return VclPtr<AbstractSvxTransformTabDialog_Impl>::Create(std::make_shared<SvxTransformTabDialog>(pParent, pAttr,pView, nAnchorTypes));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateSchTransformTabDialog(weld::Window* pParent,
                                                                                     const SfxItemSet* pAttr,
                                                                                     const SdrView* pSdrView,
                                                                                     bool bSizeTabPage)
{
    auto pDlg = std::make_shared<SvxTransformTabDialog>(pParent, pAttr, pSdrView,
            bSizeTabPage ? SvxAnchorIds::NoProtect :  SvxAnchorIds::NoProtect|SvxAnchorIds::NoResize);
    pDlg->RemoveTabPage( "RID_SVXPAGE_ANGLE" );
    pDlg->RemoveTabPage( "RID_SVXPAGE_SLANT" );
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::move(pDlg));
}

VclPtr<AbstractSvxJSearchOptionsDialog> AbstractDialogFactory_Impl::CreateSvxJSearchOptionsDialog(weld::Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            TransliterationFlags nInitialFlags)
{
    return VclPtr<AbstractSvxJSearchOptionsDialog_Impl>::Create(std::make_unique<SvxJSearchOptionsDialog>(pParent, rOptionsSet, nInitialFlags));
}

VclPtr<AbstractFmInputRecordNoDialog> AbstractDialogFactory_Impl::CreateFmInputRecordNoDialog(weld::Window* pParent)
{
    return VclPtr<AbstractFmInputRecordNoDialog_Impl>::Create(std::make_unique<FmInputRecordNoDialog>(pParent));
}

VclPtr<AbstractSvxNewDictionaryDialog> AbstractDialogFactory_Impl::CreateSvxNewDictionaryDialog(weld::Window* pParent)
{
    return VclPtr<AbstractSvxNewDictionaryDialog_Impl>::Create(std::make_unique<SvxNewDictionaryDialog>(pParent));
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateSvxEditDictionaryDialog(weld::Window* pParent, const OUString& rName)
{
    return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<SvxEditDictionaryDialog>(pParent, rName));
}

VclPtr<AbstractSvxNameDialog> AbstractDialogFactory_Impl::CreateSvxNameDialog(weld::Window* pParent,
                                    const OUString& rName, const OUString& rDesc)
{
    return VclPtr<AbstractSvxNameDialog_Impl>::Create(std::make_unique<SvxNameDialog>(pParent, rName, rDesc));
}

VclPtr<AbstractSvxObjectNameDialog> AbstractDialogFactory_Impl::CreateSvxObjectNameDialog(weld::Window* pParent, const OUString& rName)
{
    return VclPtr<AbstractSvxObjectNameDialog_Impl>::Create(std::make_unique<SvxObjectNameDialog>(pParent, rName));
}

VclPtr<AbstractSvxObjectTitleDescDialog> AbstractDialogFactory_Impl::CreateSvxObjectTitleDescDialog(weld::Window* pParent, const OUString& rTitle, const OUString& rDescription)
{
    return VclPtr<AbstractSvxObjectTitleDescDialog_Impl>::Create(std::make_unique<SvxObjectTitleDescDialog>(pParent, rTitle, rDescription));
}

VclPtr<AbstractSvxMultiPathDialog> AbstractDialogFactory_Impl::CreateSvxMultiPathDialog(weld::Window* pParent)
{
    return VclPtr<AbstractSvxMultiPathDialog_Impl>::Create(std::make_unique<SvxMultiPathDialog>(pParent));
}

VclPtr<AbstractSvxMultiPathDialog> AbstractDialogFactory_Impl::CreateSvxPathSelectDialog(weld::Window* pParent)
{
    return VclPtr<AbstractSvxPathSelectDialog_Impl>::Create(std::make_unique<SvxPathSelectDialog>(pParent));
}

VclPtr<AbstractSvxHpLinkDlg> AbstractDialogFactory_Impl::CreateSvxHpLinkDlg(SfxChildWindow* pChild, SfxBindings* pBindings, weld::Window* pParent)
{
    return VclPtr<AbstractSvxHpLinkDlg_Impl>::Create(std::make_shared<SvxHpLinkDlg>(pBindings, pChild, pParent));
}

VclPtr<AbstractFmSearchDialog> AbstractDialogFactory_Impl::CreateFmSearchDialog(weld::Window* pParent,
                                                        const OUString& strInitialText,
                                                        const std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier)
{
    return VclPtr<AbstractFmSearchDialog_Impl>::Create(std::make_unique<FmSearchDialog>(pParent,
                                                         strInitialText, _rContexts, nInitialContext, lnkContextSupplier));

}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterEmboss(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterEmboss>(pParent, rGraphic, RectPoint::MM));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterPoster(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterPoster>(pParent, rGraphic, 16));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterSepia(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterSepia>(pParent, rGraphic, 10));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterSmooth(weld::Window* pParent,
                                            const Graphic& rGraphic, double nRadius)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterSmooth>(pParent, rGraphic, nRadius));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterSolarize(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterSolarize>(pParent, rGraphic, 128, false /*bInvert*/));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterMosaic(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterMosaic>(pParent, rGraphic, 4, 4, false /*bEnhanceEdges*/));
}

VclPtr<AbstractSvxAreaTabDialog> AbstractDialogFactory_Impl::CreateSvxAreaTabDialog(weld::Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            bool bShadow)
{
    return VclPtr<AbstractSvxAreaTabDialog_Impl>::Create(std::make_shared<SvxAreaTabDialog>(pParent, pAttr, pModel, bShadow));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateSvxLineTabDialog(weld::Window* pParent, const SfxItemSet* pAttr, //add forSvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj ,
                                                                 bool bHasObj)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxLineTabDialog>(pParent, pAttr, pModel, pObj,bHasObj));
}

VclPtr<SfxAbstractDialog> AbstractDialogFactory_Impl::CreateCharMapDialog(weld::Window* pParent, const SfxItemSet& rAttr,
                                                                          const Reference< XFrame >& rDocumentFrame)
{
    return VclPtr<AbstractSvxCharacterMapDialog_Impl>::Create(std::make_unique<SvxCharacterMap>(pParent, &rAttr, rDocumentFrame));
}

VclPtr<SfxAbstractDialog> AbstractDialogFactory_Impl::CreateEventConfigDialog(weld::Widget* pParent,
                                                                              const SfxItemSet& rAttr,
                                                                              const Reference< XFrame >& rDocumentFrame)
{
    return VclPtr<CuiAbstractSingleTabController_Impl>::Create(std::make_unique<SfxMacroAssignDlg>(pParent, rDocumentFrame, rAttr));
}

VclPtr<SfxAbstractDialog> AbstractDialogFactory_Impl::CreateSfxDialog(weld::Window* pParent,
                                                                      const SfxItemSet& rAttr,
                                                                      const SdrView* pView,
                                                                      sal_uInt32 nResId)
{
    switch ( nResId )
    {
        case RID_SVXPAGE_MEASURE:
            return VclPtr<CuiAbstractSingleTabController_Impl>::Create(std::make_unique<SvxMeasureDialog>(pParent, rAttr, pView));
        case RID_SVXPAGE_CONNECTION:
            return VclPtr<CuiAbstractSingleTabController_Impl>::Create(std::make_unique<SvxConnectionDialog>(pParent, rAttr, pView));
        case RID_SFXPAGE_DBREGISTER:
            return VclPtr<CuiAbstractSingleTabController_Impl>::Create(std::make_unique<DatabaseRegistrationDialog>(pParent, rAttr));
    }

    return nullptr;
}

VclPtr<AbstractSvxPostItDialog> AbstractDialogFactory_Impl::CreateSvxPostItDialog(weld::Widget* pParent,
                                                                                  const SfxItemSet& rCoreSet,
                                                                                  bool bPrevNext)
{
    return VclPtr<AbstractSvxPostItDialog_Impl>::Create(std::make_unique<SvxPostItDialog>(pParent, rCoreSet, bPrevNext));
}

namespace {

class SvxMacroAssignDialog_Impl : public VclAbstractDialog
{
public:
    SvxMacroAssignDialog_Impl( weld::Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
            const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
        :m_aItems( SfxGetpApp()->GetPool(), svl::Items<SID_ATTR_MACROITEM, SID_ATTR_MACROITEM> )
    {
        m_aItems.Put( SfxBoolItem( SID_ATTR_MACROITEM, _bUnoDialogMode ) );
        m_xDlg.reset(new SvxMacroAssignDlg(_pParent, _rxDocumentFrame, m_aItems, _rxEvents, _nInitiallySelectedEvent));
    }

    virtual short Execute() override;

private:
    SfxItemSet                              m_aItems;
    std::unique_ptr<SvxMacroAssignDlg> m_xDlg;
};

IMPL_ABSTDLG_CLASS(SvxMacroAssignDialog)
}


VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateSvxMacroAssignDlg(
    weld::Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
    const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
{
    return VclPtr<SvxMacroAssignDialog_Impl>::Create(_pParent, _rxDocumentFrame, _bUnoDialogMode, _rxEvents, _nInitiallySelectedEvent);
}

// Factories for TabPages
CreateTabPage AbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
{
    switch ( nId )
    {
        case RID_SW_TP_BACKGROUND :
        case RID_SVXPAGE_BKG :
            return SvxBkgTabPage::Create;
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
        case RID_SVXPAGE_TEXTCOLUMNS:
            return SvxTextColumnsPage::Create;
        case RID_SVXPAGE_THEME:
            return SvxThemePage::Create;
        default:
            break;
    }

    return nullptr;
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
        case RID_SVXPAGE_BKG:
            return SvxBkgTabPage::GetRanges;
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
        case RID_SVXPAGE_TEXTCOLUMNS:
            return SvxTextColumnsPage::GetRanges;
        case RID_SVXPAGE_THEME:
            return SvxThemePage::GetRanges;
        default:
            break;
    }

    return nullptr;
}

VclPtr<SfxAbstractInsertObjectDialog> AbstractDialogFactory_Impl::CreateInsertObjectDialog(weld::Window* pParent, const OUString& rCommand,
            const Reference <css::embed::XStorage>& xStor, const SvObjectServerList* pList)
{
    std::unique_ptr<InsertObjectDialog_Impl> pDlg;
    if ( rCommand == ".uno:InsertObject" )
        pDlg.reset(new SvInsertOleDlg(pParent, xStor, pList));
    else if ( rCommand == ".uno:InsertObjectFloatingFrame" )
        pDlg.reset(new SfxInsertFloatingFrameDialog(pParent, xStor));

    if ( pDlg )
    {
        pDlg->SetHelpId( OUStringToOString( rCommand, RTL_TEXTENCODING_UTF8 ) );
        return VclPtr<AbstractInsertObjectDialog_Impl>::Create( std::move(pDlg) );
    }
    return nullptr;
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateEditObjectDialog(weld::Window* pParent, const OUString& rCommand,
            const Reference<css::embed::XEmbeddedObject>& xObj)
{
    if ( rCommand == ".uno:InsertObjectFloatingFrame" )
    {
        auto pDlg = std::make_unique<SfxInsertFloatingFrameDialog>(pParent, xObj);
        pDlg->SetHelpId( OUStringToOString( rCommand, RTL_TEXTENCODING_UTF8 ) );
        return VclPtr<AbstractInsertObjectDialog_Impl>::Create( std::move(pDlg) );
    }
    return nullptr;
}

VclPtr<SfxAbstractPasteDialog> AbstractDialogFactory_Impl::CreatePasteDialog(weld::Window* pParent)
{
    return VclPtr<AbstractPasteDialog_Impl>::Create(std::make_shared<SvPasteObjectDialog>(pParent));
}

VclPtr<SfxAbstractLinksDialog> AbstractDialogFactory_Impl::CreateLinksDialog(weld::Window* pParent, sfx2::LinkManager* pMgr, bool bHTML, sfx2::SvBaseLink* p)
{
    auto xLinkDlg(std::make_unique<SvBaseLinksDlg>(pParent, pMgr, bHTML));
    if (p)
        xLinkDlg->SetActLink(p);
    return VclPtr<AbstractLinksDialog_Impl>::Create(std::move(xLinkDlg));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateSvxFormatCellsDialog(weld::Window* pParent, const SfxItemSet* pAttr, const SdrModel& rModel)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxFormatCellsDialog>(pParent, pAttr, rModel));
}

VclPtr<SvxAbstractSplitTableDialog> AbstractDialogFactory_Impl::CreateSvxSplitTableDialog(weld::Window* pParent, bool bIsTableVertical, tools::Long nMaxVertical)
{
    return VclPtr<SvxAbstractSplitTableDialog_Impl>::Create( std::make_shared<SvxSplitTableDlg>(pParent, bIsTableVertical, nMaxVertical, 99 ));
}

std::shared_ptr<SvxAbstractNewTableDialog> AbstractDialogFactory_Impl::CreateSvxNewTableDialog(weld::Window* pParent)
{
    return std::make_shared<SvxNewTableDialogWrapper>(pParent);
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateOptionsDialog(weld::Window* pParent, const OUString& rExtensionId)
{
    return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<OfaTreeOptionsDialog>(pParent, rExtensionId));
}

VclPtr<SvxAbstractInsRowColDlg> AbstractDialogFactory_Impl::CreateSvxInsRowColDlg(weld::Window* pParent, bool bCol, const OString& rHelpId)
{
    return VclPtr<SvxInsRowColDlg>::Create(pParent, bCol, rHelpId);
}

VclPtr<AbstractPasswordToOpenModifyDialog> AbstractDialogFactory_Impl::CreatePasswordToOpenModifyDialog(
    weld::Window * pParent, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify)
{
    return VclPtr<AbstractPasswordToOpenModifyDialog_Impl>::Create(std::make_unique<PasswordToOpenModifyDialog>(pParent, nMaxPasswdLen, bIsPasswordToModify));
}

VclPtr<AbstractScreenshotAnnotationDlg> AbstractDialogFactory_Impl::CreateScreenshotAnnotationDlg(weld::Dialog& rParentDialog)
{
    return VclPtr<AbstractScreenshotAnnotationDlg_Impl>::Create(std::make_unique<ScreenshotAnnotationDlg>(rParentDialog));
}

VclPtr<AbstractSignatureLineDialog> AbstractDialogFactory_Impl::CreateSignatureLineDialog(
    weld::Window* pParent, const Reference<XModel> xModel, bool bEditExisting)
{
    return VclPtr<AbstractSignatureLineDialog_Impl>::Create(
        std::make_unique<SignatureLineDialog>(pParent, xModel, bEditExisting));
}

VclPtr<AbstractSignSignatureLineDialog>
AbstractDialogFactory_Impl::CreateSignSignatureLineDialog(weld::Window* pParent,
                                                          const Reference<XModel> xModel)
{
    return VclPtr<AbstractSignSignatureLineDialog_Impl>::Create(
        std::make_unique<SignSignatureLineDialog>(pParent, xModel));
}

VclPtr<AbstractQrCodeGenDialog> AbstractDialogFactory_Impl::CreateQrCodeGenDialog(
    weld::Window* pParent, const Reference<XModel> xModel, bool bEditExisting)
{
    return VclPtr<AbstractQrCodeGenDialog_Impl>::Create(
        std::make_unique<QrCodeGenDialog>(pParent, xModel, bEditExisting));
}

VclPtr<AbstractAdditionsDialog> AbstractDialogFactory_Impl::CreateAdditionsDialog(
    weld::Window* pParent, const OUString& sAdditionsTag)
{
#if HAVE_FEATURE_EXTENSIONS
    return VclPtr<AbstractAdditionsDialog_Impl>::Create(
        std::make_unique<AdditionsDialog>(pParent, sAdditionsTag));
#else
    (void) pParent;
    (void) sAdditionsTag;
    return VclPtr<AbstractAdditionsDialog>(nullptr);
#endif
}

VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateAboutDialog(weld::Window* pParent)
{
    return VclPtr<CuiAbstractControllerAsync_Impl>::Create(
        std::make_shared<AboutDialog>(pParent));
}

VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateTipOfTheDayDialog(weld::Window* pParent)
{
    return VclPtr<CuiAbstractControllerAsync_Impl>::Create(
        std::make_shared<TipOfTheDayDialog>(pParent));
}

VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateToolbarmodeDialog(weld::Window* pParent)
{
    return VclPtr<CuiAbstractController_Impl>::Create(
        std::make_unique<ToolbarmodeDialog>(pParent));
}

VclPtr<AbstractDiagramDialog>
AbstractDialogFactory_Impl::CreateDiagramDialog(weld::Window* pParent, std::shared_ptr<DiagramDataInterface> pDiagramData)
{
    return VclPtr<AbstractDiagramDialog_Impl>::Create(
        std::make_unique<DiagramDialog>(pParent, pDiagramData));
}

#ifdef _WIN32
VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateFileExtCheckDialog(weld::Window* pParent, const OUString& sTitle,
                                                     const OUString& sMsg)
{
    return VclPtr<CuiAbstractController_Impl>::Create(
        std::make_unique<FileExtCheckDialog>(pParent, sTitle, sMsg));
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
