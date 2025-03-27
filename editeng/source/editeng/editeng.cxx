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

#include <memory>
#include <utility>

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <config_global.h>
#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>

#include <tools/stream.hxx>

#include <editeng/svxfont.hxx>
#include "impedit.hxx"
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>
#include <editeng/flditem.hxx>
#include <editeng/txtrange.hxx>
#include <editeng/cmapitem.hxx>

#include <editeng/autokernitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>

#include <sot/exchange.hxx>
#include <sot/formats.hxx>

#include <editeng/numitem.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <vcl/help.hxx>
#include <tools/lazydelete.hxx>
#include <vcl/transfer.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#if OSL_DEBUG_LEVEL > 1
#include <editeng/frmdiritem.hxx>
#endif
#include <basegfx/polygon/b2dpolygon.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;


ImpEditEngine& EditEngine::getImpl() const
{
    return *mpImpEditEngine;
}

EditEngine::EditEngine(SfxItemPool* pItemPool)
    : mpImpEditEngine(new ImpEditEngine(this, pItemPool))
{
}

EditEngine::~EditEngine() = default;

void EditEngine::EnableUndo( bool bEnable )
{
    getImpl().EnableUndo(bEnable);
}

bool EditEngine::IsUndoEnabled() const
{
    return getImpl().IsUndoEnabled();
}

bool EditEngine::IsInUndo() const
{
    return getImpl().IsInUndo();
}

EditUndoManager& EditEngine::GetUndoManager()
{
    return getImpl().GetUndoManager();
}

EditUndoManager* EditEngine::SetUndoManager(EditUndoManager* pNew)
{
    return getImpl().SetUndoManager(pNew);
}

void EditEngine::UndoActionStart( sal_uInt16 nId )
{
    getImpl().UndoActionStart(nId);
}

void EditEngine::UndoActionStart(sal_uInt16 nId, const ESelection& rSel)
{
    getImpl().UndoActionStart(nId, rSel);
}

void EditEngine::UndoActionEnd()
{
    getImpl().UndoActionEnd();
}

bool EditEngine::HasTriedMergeOnLastAddUndo() const
{
    return getImpl().mbLastTryMerge;
}

void EditEngine::SetRefDevice( OutputDevice* pRefDev )
{
    getImpl().SetRefDevice(pRefDev);
}

OutputDevice* EditEngine::GetRefDevice() const
{
    return getImpl().GetRefDevice();
}

void EditEngine::SetRefMapMode( const MapMode& rMapMode )
{
    getImpl().SetRefMapMode(rMapMode);
}

MapMode const & EditEngine::GetRefMapMode() const
{
    return getImpl().GetRefMapMode();
}

void EditEngine::SetBackgroundColor( const Color& rColor )
{
    getImpl().SetBackgroundColor(rColor);
}

Color const & EditEngine::GetBackgroundColor() const
{
    return getImpl().GetBackgroundColor();
}

Color EditEngine::GetAutoColor() const
{
    return getImpl().GetAutoColor();
}

void EditEngine::EnableAutoColor( bool b )
{
    getImpl().EnableAutoColor( b );
}

void EditEngine::ForceAutoColor( bool b )
{
    getImpl().ForceAutoColor( b );
}

bool EditEngine::IsForceAutoColor() const
{
    return getImpl().IsForceAutoColor();
}

const SfxItemSet& EditEngine::GetEmptyItemSet() const
{
    return getImpl().GetEmptyItemSet();
}

void EditEngine::Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect )
{
    Draw( rOutDev, rOutRect, Point( 0, 0 ) );
}

void EditEngine::Draw( OutputDevice& rOutDev, const Point& rStartPos, Degree10 nOrientation )
{
    getImpl().Draw(rOutDev, rStartPos, nOrientation);
}

void EditEngine::Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos )
{
    Draw( rOutDev, rOutRect, rStartDocPos, true );
}

void EditEngine::Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos, bool bClip )
{
    getImpl().Draw(rOutDev, rOutRect, rStartDocPos, bClip);
}

void EditEngine::InsertView(EditView* pEditView, size_t nIndex)
{
    getImpl().InsertView(pEditView, nIndex);
}

EditView* EditEngine::RemoveView( EditView* pView )
{
    return getImpl().RemoveView(pView);
}

void EditEngine::RemoveView(size_t nIndex)
{
    getImpl().RemoveView(nIndex);
}

EditView* EditEngine::GetView(size_t nIndex) const
{
    return getImpl().GetEditViews()[nIndex];
}

size_t EditEngine::GetViewCount() const
{
    return getImpl().GetEditViews().size();
}

bool EditEngine::HasView( EditView* pView ) const
{
    return getImpl().HasView(pView);
}

EditView* EditEngine::GetActiveView() const
{
    return getImpl().GetActiveView();
}

void EditEngine::SetActiveView(EditView* pView)
{
    getImpl().SetActiveView(pView);
}

void EditEngine::SetDefTab( sal_uInt16 nDefTab )
{
    getImpl().SetDefTab(nDefTab);
}

void EditEngine::SetPaperSize(const Size& rNewSize)
{
    getImpl().SetPaperSize(rNewSize);
}

const Size& EditEngine::GetPaperSize() const
{
    return getImpl().GetPaperSize();
}

void EditEngine::SetVertical(bool bVertical)
{
    getImpl().SetVertical(bVertical);
}

void EditEngine::SetRotation(TextRotation nRotation)
{
    getImpl().SetRotation(nRotation);
}

TextRotation EditEngine::GetRotation() const
{
    return getImpl().GetRotation();
}

bool EditEngine::IsEffectivelyVertical() const
{
    return getImpl().IsEffectivelyVertical();
}

bool EditEngine::IsTopToBottom() const
{
    return getImpl().IsTopToBottom();
}

bool EditEngine::GetVertical() const
{
    return getImpl().GetVertical();
}

void EditEngine::SetTextColumns(sal_Int16 nColumns, sal_Int32 nSpacing)
{
    getImpl().SetTextColumns(nColumns, nSpacing);
}

void EditEngine::SetFixedCellHeight( bool bUseFixedCellHeight )
{
    getImpl().SetFixedCellHeight(bUseFixedCellHeight);
}

void EditEngine::SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir )
{
    getImpl().SetDefaultHorizontalTextDirection(eHTextDir);
}

EEHorizontalTextDirection EditEngine::GetDefaultHorizontalTextDirection() const
{
    return getImpl().GetDefaultHorizontalTextDirection();
}

SvtScriptType EditEngine::GetScriptType( const ESelection& rSelection ) const
{
    return getImpl().GetItemScriptType(getImpl().CreateSel(rSelection));
}

editeng::LanguageSpan EditEngine::GetLanguage(const EditPaM& rPaM) const
{
    return getImpl().GetLanguage(rPaM);
}

editeng::LanguageSpan EditEngine::GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const
{
    return getImpl().GetLanguage( nPara, nPos );
}

void EditEngine::TransliterateText( const ESelection& rSelection, TransliterationFlags nTransliterationMode )
{
    TransliterateText(CreateSelection(rSelection), nTransliterationMode);
}

EditSelection EditEngine::TransliterateText(const EditSelection& rSelection, TransliterationFlags nTransliterationMode)
{
    return getImpl().TransliterateText(rSelection, nTransliterationMode);
}

void EditEngine::SetAsianCompressionMode( CharCompressType n )
{
    getImpl().SetAsianCompressionMode( n );
}

void EditEngine::SetKernAsianPunctuation( bool b )
{
    getImpl().SetKernAsianPunctuation( b );
}

void EditEngine::SetAddExtLeading( bool b )
{
    getImpl().SetAddExtLeading( b );
}

void EditEngine::SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    SetPolygon( rPolyPolygon, nullptr );
}

void EditEngine::SetPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon)
{
    getImpl().SetPolygon(rPolyPolygon, pLinePolyPolygon);
}

void EditEngine::ClearPolygon()
{
    getImpl().SetTextRanger( nullptr );
}

const Size& EditEngine::GetMinAutoPaperSize() const
{
    return getImpl().GetMinAutoPaperSize();
}

void EditEngine::SetMinAutoPaperSize( const Size& rSz )
{
    getImpl().SetMinAutoPaperSize( rSz );
}

const Size& EditEngine::GetMaxAutoPaperSize() const
{
    return getImpl().GetMaxAutoPaperSize();
}

void EditEngine::SetMaxAutoPaperSize(const Size& rSize)
{
    getImpl().SetMaxAutoPaperSize(rSize);
}

void EditEngine::SetMinColumnWrapHeight(tools::Long nVal)
{
    getImpl().SetMinColumnWrapHeight(nVal);
}

OUString EditEngine::GetText( LineEnd eEnd ) const
{
    return getImpl().GetEditDoc().GetText(eEnd);
}

OUString EditEngine::GetText( const ESelection& rESelection ) const
{
    return getImpl().GetText(rESelection);
}

sal_Int32 EditEngine::GetTextLen() const
{
    return getImpl().GetEditDoc().GetTextLen();
}

sal_Int32 EditEngine::GetParagraphCount() const
{
    return getImpl().maEditDoc.Count();
}

sal_Int32 EditEngine::GetLineCount( sal_Int32 nParagraph ) const
{
    return getImpl().GetLineCount(nParagraph);
}

sal_Int32 EditEngine::GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    return getImpl().GetLineLen(nParagraph, nLine);
}

void EditEngine::GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    return getImpl().GetLineBoundaries(rStart, rEnd, nParagraph, nLine);
}

sal_Int32 EditEngine::GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    return getImpl().GetLineNumberAtIndex(nPara, nIndex);
}

sal_uInt32 EditEngine::GetLineHeight( sal_Int32 nParagraph, sal_Int32 nLine )
{
    // If someone calls GetLineHeight() with an empty Engine.
    return getImpl().GetLineHeight( nParagraph, nLine );
}

tools::Rectangle EditEngine::GetParaBounds( sal_Int32 nPara )
{
    return getImpl().GetParaBounds(nPara);
}

sal_uInt32 EditEngine::GetTextHeight( sal_Int32 nParagraph ) const
{
    getImpl().EnsureDocumentFormatted();
    sal_uInt32 nHeight = getImpl().GetParaHeight(nParagraph);
    return nHeight;
}

OUString EditEngine::GetWord(const EPaM& rPos)
{
    EditSelection aSel(CreateSelection(ESelection(rPos)));
    aSel = SelectWord(aSel);
    return GetSelected(aSel);
}

ESelection EditEngine::GetWord( const ESelection& rSelection, sal_uInt16 nWordType  ) const
{
    // ImpEditEngine-Iteration-Methods should be const!
    EditSelection aSel(getImpl().CreateSel(rSelection));
    aSel = getImpl().SelectWord(aSel, nWordType);
    return CreateESelection(aSel);
}

void EditEngine::CheckIdleFormatter()
{
    getImpl().CheckIdleFormatter();
}

bool EditEngine::IsIdleFormatterActive() const
{
    return getImpl().maIdleFormatter.IsActive();
}

ParaPortion* EditEngine::FindParaPortion(ContentNode const * pNode)
{
    return getImpl().FindParaPortion(pNode);
}

const ParaPortion* EditEngine::FindParaPortion(ContentNode const * pNode) const
{
    return getImpl().FindParaPortion(pNode);
}

const ParaPortion* EditEngine::GetPrevVisPortion(const ParaPortion* pCurPortion) const
{
    return getImpl().GetPrevVisPortion(pCurPortion);
}

SvtScriptType EditEngine::GetScriptType(const EditSelection& rSel) const
{
    return getImpl().GetItemScriptType(rSel);
}

void EditEngine::RemoveParaPortion(sal_Int32 nNode)
{
    getImpl().GetParaPortions().Remove(nNode);
}

void EditEngine::SetCallParaInsertedOrDeleted(bool b)
{
    getImpl().SetCallParaInsertedOrDeleted(b);
}

bool EditEngine::IsCallParaInsertedOrDeleted() const
{
    return getImpl().IsCallParaInsertedOrDeleted();
}

void EditEngine::AppendDeletedNodeInfo(DeletedNodeInfo* pInfo)
{
    getImpl().maDeletedNodes.push_back(std::unique_ptr<DeletedNodeInfo>(pInfo));
}

void EditEngine::UpdateSelections()
{
    getImpl().UpdateSelections();
}

void EditEngine::InsertContent(std::unique_ptr<ContentNode> pNode, sal_Int32 nPos)
{
    getImpl().InsertContent(std::move(pNode), nPos);
}

EditPaM EditEngine::SplitContent(sal_Int32 nNode, sal_Int32 nSepPos)
{
    return getImpl().SplitContent(nNode, nSepPos);
}

EditPaM EditEngine::ConnectContents(sal_Int32 nLeftNode, bool bBackward)
{
    return getImpl().ConnectContents(nLeftNode, bBackward);
}

void EditEngine::InsertFeature(const EditSelection& rEditSelection, const SfxPoolItem& rItem)
{
    getImpl().ImpInsertFeature(rEditSelection, rItem);
}

EditSelection EditEngine::MoveParagraphs(const Range& rParagraphs, sal_Int32 nNewPos)
{
    return getImpl().MoveParagraphs(rParagraphs, nNewPos, nullptr);
}

void EditEngine::RemoveCharAttribs(sal_Int32 nPara, sal_uInt16 nWhich, bool bRemoveFeatures)
{
    getImpl().RemoveCharAttribs(nPara, nWhich, bRemoveFeatures);
}

void EditEngine::RemoveCharAttribs(const EditSelection& rSel, bool bRemoveParaAttribs, sal_uInt16 nWhich)
{
    const EERemoveParaAttribsMode eMode = bRemoveParaAttribs?
        EERemoveParaAttribsMode::RemoveAll :
        EERemoveParaAttribsMode::RemoveCharItems;
    getImpl().RemoveCharAttribs(rSel, eMode, nWhich);
}

void EditEngine::RemoveCharAttribs(const EditSelection& rSel, EERemoveParaAttribsMode eMode, sal_uInt16 nWhich)
{
    getImpl().RemoveCharAttribs(rSel, eMode, nWhich);
}

EditEngine::ViewsType& EditEngine::GetEditViews()
{
    return getImpl().GetEditViews();
}

const EditEngine::ViewsType& EditEngine::GetEditViews() const
{
    return getImpl().GetEditViews();
}

void EditEngine::SetUndoMode(bool b)
{
    getImpl().SetUndoMode(b);
}

void EditEngine::FormatAndLayout(EditView* pCurView, bool bCalledFromUndo)
{
    getImpl().FormatAndLayout(pCurView, bCalledFromUndo);
}

void EditEngine::Undo(EditView* pView)
{
    getImpl().Undo(pView);
}

void EditEngine::Redo(EditView* pView)
{
    getImpl().Redo(pView);
}

uno::Reference<datatransfer::XTransferable> EditEngine::CreateTransferable(const EditSelection& rSelection)
{
    return getImpl().CreateTransferable(rSelection);
}

void EditEngine::ParaAttribsToCharAttribs(ContentNode* pNode)
{
    getImpl().ParaAttribsToCharAttribs(pNode);
}

EditPaM EditEngine::CreateEditPaM(const EPaM& rEPaM)
{
    return getImpl().CreateEditPaM(rEPaM);
}

EditPaM EditEngine::ConnectParagraphs(
        ContentNode* pLeft, ContentNode* pRight, bool bBackward)
{
    return getImpl().ImpConnectParagraphs(pLeft, pRight, bBackward);
}

EditPaM EditEngine::InsertField(const EditSelection& rEditSelection, const SvxFieldItem& rFld)
{
    return getImpl().InsertField(rEditSelection, rFld);
}

EditPaM EditEngine::InsertText(const EditSelection& aCurEditSelection, const OUString& rStr)
{
    return getImpl().InsertText(aCurEditSelection, rStr);
}

EditSelection EditEngine::InsertText(const EditTextObject& rTextObject, const EditSelection& rSel)
{
    return getImpl().InsertText(rTextObject, rSel);
}

EditSelection EditEngine::InsertText(
    uno::Reference<datatransfer::XTransferable > const & rxDataObj,
    const OUString& rBaseURL, const EditPaM& rPaM, bool bUseSpecial, SotClipboardFormatId format)
{
    return getImpl().PasteText(rxDataObj, rBaseURL, rPaM, bUseSpecial, format);
}

EditPaM EditEngine::EndOfWord(const EditPaM& rPaM)
{
    return getImpl().EndOfWord(rPaM);
}

EditPaM EditEngine::GetPaM(const Point& aDocPos, bool bSmart)
{
    return getImpl().GetPaM(aDocPos, bSmart);
}

EditSelection EditEngine::SelectWord(
        const EditSelection& rCurSelection, sal_Int16 nWordType)
{
    return getImpl().SelectWord(rCurSelection, nWordType);
}

tools::Long EditEngine::GetXPos(ParaPortion const& rParaPortion, EditLine const& rLine, sal_Int32 nIndex, bool bPreferPortionStart) const
{
    return getImpl().GetXPos(rParaPortion, rLine, nIndex, bPreferPortionStart);
}

Range EditEngine::GetLineXPosStartEnd(ParaPortion const& rParaPortion, EditLine const& rLine) const
{
    return getImpl().GetLineXPosStartEnd(rParaPortion, rLine);
}

bool EditEngine::IsFormatted() const
{
    return getImpl().IsFormatted();
}

EditPaM EditEngine::CursorLeft(const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode)
{
    return getImpl().CursorLeft(rPaM, nCharacterIteratorMode);
}

EditPaM EditEngine::CursorRight(const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode)
{
    return getImpl().CursorRight(rPaM, nCharacterIteratorMode);
}

InternalEditStatus& EditEngine::GetInternalEditStatus()
{
    return getImpl().GetStatus();
}

EditDoc& EditEngine::GetEditDoc()
{
    return getImpl().GetEditDoc();
}

const EditDoc& EditEngine::GetEditDoc() const
{
    return getImpl().GetEditDoc();
}

void EditEngine::dumpAsXmlEditDoc(xmlTextWriterPtr pWriter) const
{
    getImpl().GetEditDoc().dumpAsXml(pWriter);
}

ParaPortionList& EditEngine::GetParaPortions()
{
    return getImpl().GetParaPortions();
}

const ParaPortionList& EditEngine::GetParaPortions() const
{
    return getImpl().GetParaPortions();
}

void EditEngine::SeekCursor(ContentNode* pNode, sal_Int32 nPos, SvxFont& rFont)
{
    getImpl().SeekCursor(pNode, nPos, rFont);
}

EditPaM EditEngine::DeleteSelection(const EditSelection& rSel)
{
    return getImpl().ImpDeleteSelection(rSel);
}

ESelection EditEngine::CreateESelection(const EditSelection& rSel) const
{
    return getImpl().CreateESel(rSel);
}

EditSelection EditEngine::CreateSelection(const ESelection& rSel)
{
    return getImpl().CreateSel(rSel);
}

ESelection EditEngine::NormalizeESelection(const ESelection& rSel) const
{
    return CreateESelection(getImpl().CreateNormalizedSel(rSel));
}

EPaM EditEngine::GetEnd() const
{
    auto para = GetEditDoc().Count();
    if (para)
        --para;
    return { para, GetTextLen(para) };
}

const SfxItemSet& EditEngine::GetBaseParaAttribs(sal_Int32 nPara) const
{
    return getImpl().GetParaAttribs(nPara);
}

void EditEngine::SetParaAttribsOnly(sal_Int32 nPara, const SfxItemSet& rSet)
{
    getImpl().SetParaAttribs(nPara, rSet);
}

void EditEngine::SetAttribs(const EditSelection& rSel, const SfxItemSet& rSet, SetAttribsMode nSpecial)
{
    getImpl().SetAttribs(rSel, rSet, nSpecial);
}

OUString EditEngine::GetSelected(const EditSelection& rSel) const
{
    return getImpl().GetSelected(rSel);
}

EditPaM EditEngine::DeleteSelected(const EditSelection& rSel)
{
    return getImpl().DeleteSelected(rSel);
}

void EditEngine::HandleBeginPasteOrDrop(PasteOrDropInfos& rInfos)
{
    getImpl().maBeginPasteOrDropHdl.Call(rInfos);
}

void EditEngine::HandleEndPasteOrDrop(PasteOrDropInfos& rInfos)
{
    getImpl().maEndPasteOrDropHdl.Call(rInfos);
}

bool EditEngine::HasText() const
{
    return getImpl().ImplHasText();
}

const EditSelectionEngine& EditEngine::GetSelectionEngine() const
{
    return getImpl().maSelEngine;
}

void EditEngine::SetInSelectionMode(bool b)
{
    getImpl().mbInSelection = b;
}

bool EditEngine::PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pEditView, vcl::Window const * pFrameWin )
{
    return getImpl().PostKeyEvent(rKeyEvent, pEditView, pFrameWin);
}

sal_uInt32 EditEngine::GetTextHeight() const
{
    getImpl().EnsureDocumentFormatted();
    sal_uInt32 nHeight = !IsEffectivelyVertical() ? getImpl().GetTextHeight() : getImpl().CalcTextWidth( true );
    return nHeight;
}

sal_uInt32 EditEngine::CalcTextWidth()
{
    getImpl().EnsureDocumentFormatted();
    sal_uInt32 nWidth = !IsEffectivelyVertical() ? getImpl().CalcTextWidth(true) : getImpl().GetTextHeight();
    return nWidth;
}

bool EditEngine::SetUpdateLayout(bool bUpdate, bool bRestoring)
{
    bool bPrevUpdateLayout = getImpl().SetUpdateLayout(bUpdate);
    if (getImpl().mpActiveView)
    {
        // Not an activation if we are restoring the previous update mode.
        getImpl().mpActiveView->ShowCursor(false, false, /*bActivate=*/!bRestoring);
    }
    return bPrevUpdateLayout;
}

bool EditEngine::IsUpdateLayout() const
{
    return getImpl().IsUpdateLayout();
}

void EditEngine::Clear()
{
    getImpl().Clear();
}

void EditEngine::SetText( const OUString& rText )
{
    getImpl().SetText(rText);
    if (!rText.isEmpty() && getImpl().IsUpdateLayout())
        getImpl().FormatAndLayout();
}

ErrCode EditEngine::Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs /* = NULL */ )
{
    return getImpl().Read(rInput, rBaseURL, eFormat, pHTTPHeaderAttrs);
}

void EditEngine::Write( SvStream& rOutput, EETextFormat eFormat )
{
    getImpl().Write(rOutput, eFormat);
}

std::unique_ptr<EditTextObject> EditEngine::CreateTextObject()
{
    return getImpl().CreateTextObject();
}

std::unique_ptr<EditTextObject> EditEngine::CreateTextObject( const ESelection& rESelection )
{
    return getImpl().CreateTextObject(CreateSelection(rESelection));
}

std::unique_ptr<EditTextObject> EditEngine::GetEmptyTextObject()
{
    return getImpl().GetEmptyTextObject();
}

void EditEngine::SetText( const EditTextObject& rTextObject )
{
    getImpl().SetText(rTextObject);
    getImpl().FormatAndLayout();
}

void EditEngine::ShowParagraph( sal_Int32 nParagraph, bool bShow )
{
    getImpl().ShowParagraph(nParagraph, bShow);
}

void EditEngine::SetNotifyHdl( const Link<EENotify&,void>& rLink )
{
    getImpl().SetNotifyHdl(rLink);
}

Link<EENotify&,void> const & EditEngine::GetNotifyHdl() const
{
    return getImpl().GetNotifyHdl();
}

void EditEngine::SetStatusEventHdl( const Link<EditStatus&, void>& rLink )
{
    getImpl().SetStatusEventHdl(rLink);
}

Link<EditStatus&, void> const & EditEngine::GetStatusEventHdl() const
{
    return getImpl().GetStatusEventHdl();
}

void EditEngine::SetHtmlImportHdl( const Link<HtmlImportInfo&,void>& rLink )
{
    getImpl().maHtmlImportHdl = rLink;
}

const Link<HtmlImportInfo&,void>& EditEngine::GetHtmlImportHdl() const
{
    return getImpl().maHtmlImportHdl;
}

void EditEngine::SetRtfImportHdl( const Link<RtfImportInfo&,void>& rLink )
{
    getImpl().maRtfImportHdl = rLink;
}

const Link<RtfImportInfo&,void>& EditEngine::GetRtfImportHdl() const
{
    return getImpl().maRtfImportHdl;
}

void EditEngine::SetBeginMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink )
{
    getImpl().maBeginMovingParagraphsHdl = rLink;
}

void EditEngine::SetEndMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink )
{
    getImpl().maEndMovingParagraphsHdl = rLink;
}

void EditEngine::SetBeginPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink )
{

    getImpl().maBeginPasteOrDropHdl = rLink;
}

void EditEngine::SetEndPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink )
{
    getImpl().maEndPasteOrDropHdl = rLink;
}

std::unique_ptr<EditTextObject> EditEngine::CreateTextObject( sal_Int32 nPara, sal_Int32 nParas )
{
    return getImpl().CreateTextObject(nPara, nParas);
}

void EditEngine::RemoveParagraph( sal_Int32 nPara )
{
    getImpl().RemoveParagraph(nPara);
}

sal_Int32 EditEngine::GetTextLen( sal_Int32 nPara ) const
{
    ContentNode* pNode = getImpl().GetEditDoc().GetObject(nPara);
    DBG_ASSERT( pNode, "Paragraph not found: GetTextLen" );
    if ( pNode )
        return pNode->Len();
    return 0;
}

OUString EditEngine::GetText( sal_Int32 nPara ) const
{
    OUString aStr;
    if (0 <= nPara && nPara < getImpl().GetEditDoc().Count())
        aStr = getImpl().GetEditDoc().GetParaAsString(nPara);
    return aStr;
}

void EditEngine::SetModifyHdl( const Link<LinkParamNone*,void>& rLink )
{
    getImpl().SetModifyHdl(rLink);
}

void EditEngine::ClearModifyFlag()
{
    getImpl().SetModifyFlag(false);
}

void EditEngine::SetModified()
{
    getImpl().SetModifyFlag(true);
}

bool EditEngine::IsModified() const
{
    return getImpl().IsModified();
}

bool EditEngine::IsInSelectionMode() const
{
    return getImpl().IsInSelectionMode() || getImpl().GetSelEngine().IsInSelection();
}

void EditEngine::InsertParagraph( sal_Int32 nPara, const EditTextObject& rTxtObj, bool bAppend )
{
    getImpl().InsertParagraph(nPara, rTxtObj, bAppend);
}

void EditEngine::InsertParagraph(sal_Int32 nPara, const OUString& rTxt)
{
    getImpl().InsertParagraph(nPara, rTxt);
}

void EditEngine::SetText(sal_Int32 nPara, const OUString& rTxt)
{
    getImpl().SetParagraphText(nPara, rTxt);
}

void EditEngine::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    getImpl().SetParaAttribs(nPara, rSet);
    if (getImpl().IsUpdateLayout())
        getImpl().FormatAndLayout();
}

const SfxItemSet& EditEngine::GetParaAttribs( sal_Int32 nPara ) const
{
    return getImpl().GetParaAttribs(nPara);
}

bool EditEngine::HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    return getImpl().HasParaAttrib(nPara, nWhich);
}

const SfxPoolItem& EditEngine::GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    return getImpl().GetParaAttrib(nPara, nWhich);
}

void EditEngine::SetCharAttribs(sal_Int32 nPara, const SfxItemSet& rSet)
{
    EditSelection aSel(getImpl().ConvertSelection(nPara, 0, nPara, GetTextLen(nPara)));
    // This is called by sd::View::OnBeginPasteOrDrop(), updating the cursor position on undo is not
    // wanted.
    getImpl().SetAttribs(aSel, rSet, /*nSpecial=*/SetAttribsMode::NONE, /*bSetSelection=*/false);
    if (getImpl().IsUpdateLayout())
        getImpl().FormatAndLayout();
}

void EditEngine::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rList ) const
{
    getImpl().GetCharAttribs(nPara, rList);
}

SfxItemSet EditEngine::GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib )
{
    return getImpl().GetAttribs(getImpl().CreateNormalizedSel(rSel), nOnlyHardAttrib);
}

SfxItemSet EditEngine::GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags ) const
{
    return getImpl().GetAttribs(nPara, nStart, nEnd, nFlags);
}

void EditEngine::RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    getImpl().RemoveAttribs(rSelection, bRemoveParaAttribs, nWhich);
}

vcl::Font EditEngine::GetStandardFont( sal_Int32 nPara )
{
    return GetStandardSvxFont( nPara );
}

SvxFont EditEngine::GetStandardSvxFont( sal_Int32 nPara )
{
    ContentNode* pNode = getImpl().GetEditDoc().GetObject( nPara );
    return pNode->GetCharAttribs().GetDefFont();
}

void EditEngine::StripPortions(
    const std::function<void(const DrawPortionInfo&)>& rDrawPortion,
    const std::function<void(const DrawBulletInfo&)>& rDrawBullet)
{
    if (!rDrawPortion && !rDrawBullet)
        return;

    ScopedVclPtrInstance< VirtualDevice > aTmpDev;
    tools::Rectangle aBigRect( Point( 0, 0 ), Size( 0x7FFFFFFF, 0x7FFFFFFF ) );
    if ( IsEffectivelyVertical() )
    {
        if( IsTopToBottom() )
        {
            aBigRect.SetRight( 0 );
            aBigRect.SetLeft( -0x7FFFFFFF );
        }
        else
        {
            aBigRect.SetTop( -0x7FFFFFFF );
            aBigRect.SetBottom( 0 );
        }
    }

    getImpl().Paint(*aTmpDev, aBigRect, Point(), 0_deg10, rDrawPortion, rDrawBullet);
}

void EditEngine::GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList )
{
    if (!getImpl().IsFormatted())
        getImpl().FormatFullDoc();

    const ParaPortion* pParaPortion = getImpl().GetParaPortions().SafeGetObject(nPara);
    if ( pParaPortion )
    {
        sal_Int32 nEnd = 0;
        sal_Int32 nTextPortions = pParaPortion->GetTextPortions().Count();
        for ( sal_Int32 n = 0; n < nTextPortions; n++ )
        {
            nEnd = nEnd + pParaPortion->GetTextPortions()[n].GetLen();
            rList.push_back( nEnd );
        }
    }
}

void EditEngine::SetFlatMode( bool bFlat)
{
    getImpl().SetFlatMode(bFlat);
}

bool EditEngine::IsFlatMode() const
{
    return !getImpl().GetStatus().UseCharAttribs();
}

void EditEngine::SetSingleLine(bool bValue)
{
    if (bValue == getImpl().GetStatus().IsSingleLine())
        return;

    if (bValue)
        getImpl().GetStatus().TurnOnFlags(EEControlBits::SINGLELINE);
    else
        getImpl().GetStatus().TurnOffFlags(EEControlBits::SINGLELINE);
}

void EditEngine::SetControlWord( EEControlBits nWord )
{
    getImpl().SetControlWord(nWord);
}

EEControlBits EditEngine::GetControlWord() const
{
    return getImpl().GetStatus().GetControlWord();
}

tools::Long EditEngine::GetFirstLineStartX( sal_Int32 nParagraph )
{

    tools::Long nX = 0;
    const ParaPortion* pPPortion = getImpl().GetParaPortions().SafeGetObject(nParagraph);
    if ( pPPortion )
    {
        DBG_ASSERT(getImpl().IsFormatted() || !getImpl().IsFormatting(), "GetFirstLineStartX: Doc not formatted - unable to format!");
        getImpl().EnsureDocumentFormatted();
        const EditLine& rFirstLine = pPPortion->GetLines()[0];
        nX = rFirstLine.GetStartPosX();
    }
    return nX;
}

Point EditEngine::GetDocPos( const Point& rPaperPos ) const
{
    Point aDocPos( rPaperPos );
    if ( IsEffectivelyVertical() )
    {
        if ( IsTopToBottom() )
        {
            aDocPos.setX( rPaperPos.Y() );
            aDocPos.setY( GetPaperSize().Width() - rPaperPos.X() );
        }
        else
        {
            aDocPos.setX( rPaperPos.Y() );
            aDocPos.setY( rPaperPos.X() );
        }
    }
    return aDocPos;
}

Point EditEngine::GetDocPosTopLeft( sal_Int32 nParagraph )
{
    return getImpl().GetDocPosTopLeft(nParagraph);
}

const SvxNumberFormat* EditEngine::GetNumberFormat( sal_Int32 ) const
{
    // derived objects may override this function to give access to
    // bullet information (see Outliner)
    return nullptr;
}

bool EditEngine::IsRightToLeft( sal_Int32 nPara ) const
{
    return getImpl().IsRightToLeft(nPara);
}

bool EditEngine::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder )
{
    getImpl().EnsureDocumentFormatted();

    // take unrotated positions for calculation here
    Point aDocPos = GetDocPos( rPaperPos );

    if ((aDocPos.Y() > 0) && (o3tl::make_unsigned(aDocPos.Y()) < getImpl().GetTextHeight()))
        return getImpl().IsTextPos(aDocPos, nBorder);
    return false;
}

void EditEngine::SetEditTextObjectPool( SfxItemPool* pPool )
{
    getImpl().SetEditTextObjectPool(pPool);
}

SfxItemPool* EditEngine::GetEditTextObjectPool() const
{
    return getImpl().GetEditTextObjectPool();
}

void EditEngine::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    getImpl().SetAttribs(getImpl().CreateNormalizedSel(rSel), rSet);
}

void EditEngine::QuickMarkInvalid( const ESelection& rSel )
{
    DBG_ASSERT(rSel.start.nPara < getImpl().GetEditDoc().Count(), "MarkInvalid: Start out of Range!");
    DBG_ASSERT(rSel.end.nPara < getImpl().GetEditDoc().Count(), "MarkInvalid: End out of Range!");
    for (sal_Int32 nPara = rSel.start.nPara; nPara <= rSel.end.nPara; nPara++)
    {
        ParaPortion* pPortion = getImpl().GetParaPortions().SafeGetObject(nPara);
        if ( pPortion )
            pPortion->MarkSelectionInvalid( 0 );
    }
}

void EditEngine::QuickInsertText(const OUString& rText, const ESelection& rSel)
{
    getImpl().ImpInsertText(getImpl().CreateNormalizedSel(rSel), rText);
}

void EditEngine::QuickDelete( const ESelection& rSel )
{
    getImpl().ImpDeleteSelection(getImpl().CreateNormalizedSel(rSel));
}

void EditEngine::QuickMarkToBeRepainted( sal_Int32 nPara )
{
    ParaPortion* pPortion = getImpl().GetParaPortions().SafeGetObject(nPara);
    if ( pPortion )
        pPortion->SetMustRepaint( true );
}

void EditEngine::QuickInsertLineBreak( const ESelection& rSel )
{
    getImpl().InsertLineBreak(getImpl().CreateNormalizedSel(rSel));
}

void EditEngine::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    getImpl().ImpInsertFeature(getImpl().CreateNormalizedSel(rSel), rFld);
}

void EditEngine::QuickFormatDoc( bool bFull )
{
    if (bFull)
        getImpl().FormatFullDoc();
    else
        getImpl().FormatDoc();

    // Don't pass active view, maybe selection is not updated yet...
    getImpl().UpdateViews();
}

void EditEngine::SetStyleSheet(const EditSelection& aSel, SfxStyleSheet* pStyle)
{
    getImpl().SetStyleSheet(aSel, pStyle);
}

void EditEngine::SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle )
{
    getImpl().SetStyleSheet(nPara, pStyle);
}

const SfxStyleSheet* EditEngine::GetStyleSheet( sal_Int32 nPara ) const
{
    return getImpl().GetStyleSheet(nPara);
}

SfxStyleSheet* EditEngine::GetStyleSheet( sal_Int32 nPara )
{
    return getImpl().GetStyleSheet(nPara);
}

void EditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    getImpl().SetStyleSheetPool(pSPool);
}

SfxStyleSheetPool* EditEngine::GetStyleSheetPool()
{
    return getImpl().GetStyleSheetPool();
}

void EditEngine::SetWordDelimiters( const OUString& rDelimiters )
{
    getImpl().maWordDelimiters = rDelimiters;
    if (getImpl().maWordDelimiters.indexOf(CH_FEATURE) == -1)
        getImpl().maWordDelimiters += OUStringChar(CH_FEATURE);
}

const OUString& EditEngine::GetWordDelimiters() const
{
    return getImpl().maWordDelimiters;
}

void EditEngine::EraseVirtualDevice()
{
    getImpl().EraseVirtualDevice();
}

void EditEngine::SetSpeller( Reference< XSpellChecker1 > const &xSpeller )
{
    getImpl().SetSpeller(xSpeller);
}

Reference< XSpellChecker1 > const & EditEngine::GetSpeller()
{
    return getImpl().GetSpeller();
}

void EditEngine::SetHyphenator( Reference< XHyphenator > const & xHyph )
{
    getImpl().SetHyphenator(xHyph);
}

void EditEngine::GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const
{
    getImpl().GetAllMisspellRanges(rRanges);
}

void EditEngine::SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges )
{
    getImpl().SetAllMisspellRanges(rRanges);
}

void EditEngine::SetForbiddenCharsTable(const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars)
{
    ImpEditEngine::SetForbiddenCharsTable( xForbiddenChars );
}

void EditEngine::SetDefaultLanguage( LanguageType eLang )
{
    getImpl().SetDefaultLanguage(eLang);
}

LanguageType EditEngine::GetDefaultLanguage() const
{
    return getImpl().GetDefaultLanguage();
}

bool EditEngine::SpellNextDocument()
{
    return false;
}

EESpellState EditEngine::HasSpellErrors()
{
    if (!getImpl().GetSpeller().is())
        return EESpellState::NoSpeller;

    return getImpl().HasSpellErrors();
}

void EditEngine::ClearSpellErrors()
{
    getImpl().ClearSpellErrors();
}

bool EditEngine::SpellSentence(EditView const & rView, svx::SpellPortions& rToFill )
{
    return getImpl().SpellSentence(rView, rToFill);
}

void EditEngine::PutSpellingToSentenceStart( EditView const & rEditView )
{
    getImpl().PutSpellingToSentenceStart(rEditView);
}

void EditEngine::ApplyChangedSentence(EditView const & rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck )
{
    getImpl().ApplyChangedSentence(rEditView, rNewPortions, bRecheck);
}

bool EditEngine::HasConvertibleTextPortion( LanguageType nLang )
{
    return getImpl().HasConvertibleTextPortion(nLang);
}

bool EditEngine::ConvertNextDocument()
{
    return false;
}

bool EditEngine::HasText( const SvxSearchItem& rSearchItem )
{
    return getImpl().HasText(rSearchItem);
}

const ScalingParameters & EditEngine::getScalingParameters() const
{
    return getImpl().getScalingParameters();
}

void EditEngine::setScalingParameters(ScalingParameters const& rScalingParameters)
{
    getImpl().setScalingParameters(rScalingParameters);
}

void EditEngine::setRoundFontSizeToPt(bool bRound)
{
    getImpl().setRoundToNearestPt(bRound);
}

bool EditEngine::ShouldCreateBigTextObject() const
{
    sal_Int32 nTextPortions = 0;
    sal_Int32 nParas = getImpl().GetEditDoc().Count();
    for (sal_Int32 nPara = 0; nPara < nParas; nPara++)
    {
        ParaPortion& rParaPortion = getImpl().GetParaPortions().getRef(nPara);
        nTextPortions = nTextPortions + rParaPortion.GetTextPortions().Count();
    }
    return nTextPortions >= getImpl().GetBigTextObjectStart();
}

std::vector<EFieldInfo> EditEngine::GetFieldInfo( sal_Int32 nPara ) const
{
    std::vector<EFieldInfo> aFieldInfos;
    ContentNode* pNode = getImpl().GetEditDoc().GetObject(nPara);
    if ( pNode )
    {
        for (auto const& attrib : pNode->GetCharAttribs().GetAttribs())
        {
            const EditCharAttrib& rAttr = *attrib;
            if (rAttr.Which() == EE_FEATURE_FIELD)
            {
                const SvxFieldItem* p = static_cast<const SvxFieldItem*>(rAttr.GetItem());
                EFieldInfo aInfo(*p, nPara, rAttr.GetStart());
                aInfo.aCurrentText = static_cast<const EditCharAttribField&>(rAttr).GetFieldValue();
                aFieldInfos.push_back(aInfo);
            }
        }
    }
    return aFieldInfos;
}


bool EditEngine::UpdateFields()
{
    bool bChanges = getImpl().UpdateFields();
    if (bChanges && getImpl().IsUpdateLayout())
        getImpl().FormatAndLayout();
    return bChanges;
}

bool EditEngine::UpdateFieldsOnly()
{
    return getImpl().UpdateFields();
}

void EditEngine::RemoveFields( const std::function<bool ( const SvxFieldData* )>& isFieldData )
{
    getImpl().UpdateFields();

    sal_Int32 nParas = getImpl().GetEditDoc().Count();
    for ( sal_Int32 nPara = 0; nPara < nParas; nPara++  )
    {
        ContentNode* pNode = getImpl().GetEditDoc().GetObject(nPara);
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for (size_t nAttr = rAttrs.size(); nAttr; )
        {
            const EditCharAttrib& rAttr = *rAttrs[--nAttr];
            if (rAttr.Which() == EE_FEATURE_FIELD)
            {
                const SvxFieldData* pFldData = static_cast<const SvxFieldItem*>(rAttr.GetItem())->GetField();
                if ( pFldData && ( isFieldData( pFldData )  ) )
                {
                    DBG_ASSERT( dynamic_cast<const SvxFieldItem*>(rAttr.GetItem()), "no field item..." );
                    EditSelection aSel( EditPaM(pNode, rAttr.GetStart()), EditPaM(pNode, rAttr.GetEnd()) );
                    OUString aFieldText = static_cast<const EditCharAttribField&>(rAttr).GetFieldValue();
                    getImpl().ImpInsertText(aSel, aFieldText);
                }
            }
        }
    }
}

bool EditEngine::HasOnlineSpellErrors() const
{
    sal_Int32 nNodes = getImpl().GetEditDoc().Count();
    for ( sal_Int32 n = 0; n < nNodes; n++ )
    {
        ContentNode* pNode = getImpl().GetEditDoc().GetObject(n);
        if ( pNode->GetWrongList() && !pNode->GetWrongList()->empty() )
            return true;
    }
    return false;
}

void EditEngine::CompleteOnlineSpelling()
{
    if (getImpl().GetStatus().DoOnlineSpelling())
    {
        if (!getImpl().IsFormatted())
            getImpl().FormatAndLayout();

        getImpl().StopOnlineSpellTimer();
        getImpl().DoOnlineSpelling(nullptr, true, false);
    }
}

sal_Int32 EditEngine::FindParagraph( tools::Long nDocPosY )
{
    return getImpl().GetParaPortions().FindParagraph(nDocPosY);
}

EPaM EditEngine::FindDocPosition(const Point& rDocPos) const
{
    // From the point of the API, this is const...
    EditPaM aPaM = getImpl().GetPaM(rDocPos, false);
    if ( aPaM.GetNode() )
        return getImpl().CreateEPaM(aPaM);
    return EPaM::NotFound();
}

tools::Rectangle EditEngine::GetCharacterBounds(const EPaM& rPos) const
{
    tools::Rectangle aBounds;
    ContentNode* pNode = getImpl().GetEditDoc().GetObject(rPos.nPara);

    // Check against index, not paragraph
    if ( pNode && ( rPos.nIndex < pNode->Len() ) )
    {
        aBounds = getImpl().PaMtoEditCursor(EditPaM(pNode, rPos.nIndex), CursorFlags{.bTextOnly = true});
        CursorFlags aFlags { .bTextOnly = true, .bEndOfLine = true};
        tools::Rectangle aR2 = getImpl().PaMtoEditCursor(EditPaM(pNode, rPos.nIndex + 1), aFlags);
        if ( aR2.Right() > aBounds.Right() )
            aBounds.SetRight( aR2.Right() );
    }
    return aBounds;
}

ParagraphInfos EditEngine::GetParagraphInfos( sal_Int32 nPara )
{
    // This only works if not already in the format ...
    getImpl().EnsureDocumentFormatted();

    ParagraphInfos aInfos;
    aInfos.bValid = getImpl().IsFormatted();
    if (getImpl().IsFormatted() && getImpl().GetParaPortions().exists(nPara))
    {
        ParaPortion const& rParaPortion = getImpl().GetParaPortions().getRef(nPara);
        const EditLine* pLine = (rParaPortion.GetLines().Count()) ? &rParaPortion.GetLines()[0] : nullptr;
        DBG_ASSERT(pLine, "GetParagraphInfos - Paragraph out of range");
        if (pLine)
        {
            aInfos.nFirstLineHeight = pLine->GetHeight();
            aInfos.nFirstLineTextHeight = pLine->GetTxtHeight();
            aInfos.nFirstLineMaxAscent = pLine->GetMaxAscent();
        }
    }
    return aInfos;
}

uno::Reference<datatransfer::XTransferable>
EditEngine::CreateTransferable(const ESelection& rSelection)
{
    return getImpl().CreateTransferable(CreateSelection(rSelection));
}


// ======================    Virtual Methods    ========================

void EditEngine::PaintingFirstLine(sal_Int32, const Point&, const Point&, Degree10, OutputDevice&,
    const std::function<void(const DrawPortionInfo&)>&,
    const std::function<void(const DrawBulletInfo&)>&)
{
}

void EditEngine::ParagraphInserted( sal_Int32 nPara )
{
    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_PARAGRAPHINSERTED );
        aNotify.nParagraph = nPara;
        getImpl().GetNotifyHdl().Call(aNotify);
    }
}

void EditEngine::ParagraphDeleted( sal_Int32 nPara )
{
    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_PARAGRAPHREMOVED );
        aNotify.nParagraph = nPara;
        getImpl().GetNotifyHdl().Call(aNotify);
    }
}

void EditEngine::ParagraphConnected( sal_Int32 /*nLeftParagraph*/, sal_Int32 /*nRightParagraph*/ )
{
}

void EditEngine::ParaAttribsChanged( sal_Int32 /* nParagraph */ )
{
}

void EditEngine::StyleSheetChanged( SfxStyleSheet* /* pStyle */ )
{
}

void EditEngine::ParagraphHeightChanged( sal_Int32 nPara )
{
    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_TextHeightChanged );
        aNotify.nParagraph = nPara;
        getImpl().GetNotifyHdl().Call(aNotify);
    }

    for (EditView* pView : getImpl().maEditViews)
        pView->getImpl().ScrollStateChange();
}

OUString EditEngine::GetUndoComment( sal_uInt16 nId ) const
{
    OUString aComment;
    switch ( nId )
    {
        case EDITUNDO_REMOVECHARS:
        case EDITUNDO_CONNECTPARAS:
        case EDITUNDO_DELCONTENT:
        case EDITUNDO_DELETE:
        case EDITUNDO_CUT:
            aComment = EditResId(RID_EDITUNDO_DEL);
        break;
        case EDITUNDO_MOVEPARAGRAPHS:
        case EDITUNDO_MOVEPARAS:
        case EDITUNDO_DRAGANDDROP:
            aComment = EditResId(RID_EDITUNDO_MOVE);
        break;
        case EDITUNDO_INSERTFEATURE:
        case EDITUNDO_SPLITPARA:
        case EDITUNDO_INSERTCHARS:
        case EDITUNDO_PASTE:
        case EDITUNDO_INSERT:
        case EDITUNDO_READ:
            aComment = EditResId(RID_EDITUNDO_INSERT);
        break;
        case EDITUNDO_REPLACEALL:
            aComment = EditResId(RID_EDITUNDO_REPLACE);
        break;
        case EDITUNDO_ATTRIBS:
        case EDITUNDO_PARAATTRIBS:
            aComment = EditResId(RID_EDITUNDO_SETATTRIBS);
        break;
        case EDITUNDO_RESETATTRIBS:
            aComment = EditResId(RID_EDITUNDO_RESETATTRIBS);
        break;
        case EDITUNDO_STYLESHEET:
            aComment = EditResId(RID_EDITUNDO_SETSTYLE);
        break;
        case EDITUNDO_TRANSLITERATE:
            aComment = EditResId(RID_EDITUNDO_TRANSLITERATE);
        break;
        case EDITUNDO_INDENTBLOCK:
        case EDITUNDO_UNINDENTBLOCK:
            aComment = EditResId(RID_EDITUNDO_INDENT);
        break;
    }
    return aComment;
}

tools::Rectangle EditEngine::GetBulletArea( sal_Int32 )
{
    return tools::Rectangle( Point(), Point() );
}

OUString EditEngine::CalcFieldValue( const SvxFieldItem&, sal_Int32, sal_Int32, std::optional<Color>&, std::optional<Color>&, std::optional<FontLineStyle>& )
{
    return OUString(' ');
}

bool EditEngine::FieldClicked( const SvxFieldItem& )
{
    return false;
}


// ======================     Static Methods     =======================

rtl::Reference<SfxItemPool> EditEngine::CreatePool()
{
    return new EditEngineItemPool();
}


SfxItemPool& EditEngine::GetGlobalItemPool()
{
    static tools::DeleteOnDeinit<rtl::Reference<SfxItemPool>> pGlobalPool(CreatePool());
    return **pGlobalPool.get();
}

void EditEngine::SetFontInfoInItemSet( SfxItemSet& rSet, const vcl::Font& rFont )
{
    SvxFont aSvxFont( rFont );
    SetFontInfoInItemSet( rSet, aSvxFont );

}

void EditEngine::SetFontInfoInItemSet( SfxItemSet& rSet, const SvxFont& rFont )
{
    rSet.Put( SvxLanguageItem( rFont.GetLanguage(), EE_CHAR_LANGUAGE ) );
    rSet.Put( SvxFontItem( rFont.GetFamilyType(), rFont.GetFamilyName(), OUString(), rFont.GetPitch(), rFont.GetCharSet(), EE_CHAR_FONTINFO ) );
    rSet.Put( SvxFontHeightItem( rFont.GetFontSize().Height(), 100, EE_CHAR_FONTHEIGHT )  );
    rSet.Put( SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH ) );
    rSet.Put( SvxShadowedItem( rFont.IsShadow(), EE_CHAR_SHADOW )  );
    rSet.Put( SvxEscapementItem( rFont.GetEscapement(), rFont.GetPropr(), EE_CHAR_ESCAPEMENT )  );
    rSet.Put( SvxWeightItem( rFont.GetWeight(), EE_CHAR_WEIGHT )  );
    rSet.Put( SvxColorItem( rFont.GetColor(), EE_CHAR_COLOR )  );
    rSet.Put( SvxColorItem( rFont.GetFillColor(), EE_CHAR_BKGCOLOR )  );
    rSet.Put( SvxUnderlineItem( rFont.GetUnderline(), EE_CHAR_UNDERLINE )  );
    rSet.Put( SvxOverlineItem( rFont.GetOverline(), EE_CHAR_OVERLINE )  );
    rSet.Put( SvxCrossedOutItem( rFont.GetStrikeout(), EE_CHAR_STRIKEOUT )  );
    rSet.Put( SvxCaseMapItem( rFont.GetCaseMap(), EE_CHAR_CASEMAP )  );
    rSet.Put( SvxPostureItem( rFont.GetItalic(), EE_CHAR_ITALIC )  );
    rSet.Put( SvxContourItem( rFont.IsOutline(), EE_CHAR_OUTLINE )  );
    rSet.Put( SvxAutoKernItem( rFont.IsKerning(), EE_CHAR_PAIRKERNING ) );
    rSet.Put( SvxKerningItem( rFont.GetFixKerning(), EE_CHAR_KERNING ) );
    rSet.Put( SvxWordLineModeItem( rFont.IsWordLineMode(), EE_CHAR_WLM ) );
    rSet.Put( SvxEmphasisMarkItem( rFont.GetEmphasisMark(), EE_CHAR_EMPHASISMARK ) );
    rSet.Put( SvxCharReliefItem( rFont.GetRelief(), EE_CHAR_RELIEF ) );
}

vcl::Font EditEngine::CreateFontFromItemSet( const SfxItemSet& rItemSet, SvtScriptType nScriptType )
{
    SvxFont aFont;
    CreateFont( aFont, rItemSet, true, nScriptType );
#if HAVE_P1155R3
    return aFont;
#else
    return std::move(aFont);
#endif
}

SvxFont EditEngine::CreateSvxFontFromItemSet( const SfxItemSet& rItemSet )
{
    SvxFont aFont;
    CreateFont( aFont, rItemSet );
    return aFont;
}

bool EditEngine::DoesKeyMoveCursor( const KeyEvent& rKeyEvent )
{
    bool bDoesMove = false;

    switch ( rKeyEvent.GetKeyCode().GetCode() )
    {
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        case KEY_HOME:
        case KEY_END:
        case KEY_PAGEUP:
        case KEY_PAGEDOWN:
        {
            if ( !rKeyEvent.GetKeyCode().IsMod2() )
                bDoesMove = true;
        }
        break;
    }
    return bDoesMove;
}

bool EditEngine::DoesKeyChangeText( const KeyEvent& rKeyEvent )
{
    bool bDoesChange = false;

    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KeyFuncType::DONTKNOW )
    {
        switch ( eFunc )
        {
            case KeyFuncType::UNDO:
            case KeyFuncType::REDO:
            case KeyFuncType::CUT:
            case KeyFuncType::PASTE: bDoesChange = true;
            break;
            default:    // is then possibly edited below.
                        eFunc = KeyFuncType::DONTKNOW;
        }
    }
    if ( eFunc == KeyFuncType::DONTKNOW )
    {
        switch ( rKeyEvent.GetKeyCode().GetCode() )
        {
            case KEY_DELETE:
            case KEY_BACKSPACE: bDoesChange = true;
            break;
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    bDoesChange = true;
            }
            break;
            default:
            {
                bDoesChange = IsSimpleCharInput( rKeyEvent );
            }
        }
    }
    return bDoesChange;
}

bool EditEngine::IsSimpleCharInput( const KeyEvent& rKeyEvent )
{
    return ImpEditEngine::IsSimpleCharInput(rKeyEvent);
}

bool EditEngine::HasValidData( const css::uno::Reference< css::datatransfer::XTransferable >& rTransferable )
{
    bool bValidData = false;

    if ( comphelper::LibreOfficeKit::isActive())
        return true;

    if ( rTransferable.is() )
    {
        // Every application that copies rtf or any other text format also copies plain text into the clipboard...
        datatransfer::DataFlavor aFlavor;
        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
        bValidData = rTransferable->isDataFlavorSupported( aFlavor );

        if (!bValidData)
        {
            // Allow HTML-only clipboard, i.e. without plain text.
            SotExchange::GetFormatDataFlavor(SotClipboardFormatId::HTML, aFlavor);
            bValidData = rTransferable->isDataFlavorSupported(aFlavor);
        }
    }

    return bValidData;
}

/** sets a link that is called at the beginning of a drag operation at an edit view */
void EditEngine::SetBeginDropHdl( const Link<EditView*,void>& rLink )
{
    getImpl().SetBeginDropHdl(rLink);
}

Link<EditView*,void> const & EditEngine::GetBeginDropHdl() const
{
    return getImpl().GetBeginDropHdl();
}

/** sets a link that is called at the end of a drag operation at an edit view */
void EditEngine::SetEndDropHdl( const Link<EditView*,void>& rLink )
{
    getImpl().SetEndDropHdl(rLink);
}

Link<EditView*,void> const & EditEngine::GetEndDropHdl() const
{
    return getImpl().GetEndDropHdl();
}

void EditEngine::SetFirstWordCapitalization( bool bCapitalize )
{
    getImpl().SetFirstWordCapitalization(bCapitalize);
}

void EditEngine::SetReplaceLeadingSingleQuotationMark( bool bReplace )
{
    getImpl().SetReplaceLeadingSingleQuotationMark(bReplace);
}

bool EditEngine::IsHtmlImportHandlerSet() const
{
    return getImpl().maHtmlImportHdl.IsSet();
}

bool EditEngine::IsRtfImportHandlerSet() const
{
    return getImpl().maRtfImportHdl.IsSet();
}

bool EditEngine::IsImportRTFStyleSheetsSet() const
{
    return getImpl().GetStatus().DoImportRTFStyleSheets();
}

void EditEngine::CallHtmlImportHandler(HtmlImportInfo& rInfo)
{
    getImpl().maHtmlImportHdl.Call(rInfo);
}

void EditEngine::CallRtfImportHandler(RtfImportInfo& rInfo)
{
    getImpl().maRtfImportHdl.Call(rInfo);
}

EditPaM EditEngine::InsertParaBreak(const EditSelection& rEditSelection)
{
    return getImpl().ImpInsertParaBreak(rEditSelection);
}

EditPaM EditEngine::InsertLineBreak(const EditSelection& rEditSelection)
{
    return getImpl().InsertLineBreak(rEditSelection);
}

sal_Int32 EditEngine::GetOverflowingParaNum() const {
    return getImpl().GetOverflowingParaNum();
}

sal_Int32 EditEngine::GetOverflowingLineNum() const {
    return getImpl().GetOverflowingLineNum();
}

void EditEngine::ClearOverflowingParaNum() {
    getImpl().ClearOverflowingParaNum();
}

bool EditEngine::IsPageOverflow() {
    getImpl().CheckPageOverflow();
    return getImpl().IsPageOverflow();
}

void EditEngine::DisableAttributeExpanding() {
    getImpl().GetEditDoc().DisableAttributeExpanding();
}

void EditEngine::EnableSkipOutsideFormat(bool bValue)
{
    getImpl().EnableSkipOutsideFormat(bValue);
}

void EditEngine::SetLOKSpecialPaperSize(const Size& rSize)
{
    getImpl().SetLOKSpecialPaperSize(rSize);
}

const Size& EditEngine::GetLOKSpecialPaperSize() const
{
    return getImpl().GetLOKSpecialPaperSize();
}

EFieldInfo::EFieldInfo()
{
}


EFieldInfo::EFieldInfo( const SvxFieldItem& rFieldItem, sal_Int32 nPara, sal_Int32 nPos ) :
    pFieldItem( new SvxFieldItem( rFieldItem ) ),
    aPosition( nPara, nPos )
{
}

EFieldInfo::~EFieldInfo()
{
}

EFieldInfo::EFieldInfo( const EFieldInfo& rFldInfo )
{
    *this = rFldInfo;
}

EFieldInfo& EFieldInfo::operator= ( const EFieldInfo& rFldInfo )
{
    if( this == &rFldInfo )
        return *this;

    pFieldItem.reset( rFldInfo.pFieldItem ? new SvxFieldItem( *rFldInfo.pFieldItem ) : nullptr );
    aCurrentText = rFldInfo.aCurrentText;
    aPosition = rFldInfo.aPosition;

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
