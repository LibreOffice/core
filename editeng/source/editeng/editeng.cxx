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


#if (OSL_DEBUG_LEVEL > 1) || defined ( DBG_UTIL )
static bool bDebugPaint = false;
#endif

static rtl::Reference<SfxItemPool> pGlobalPool;

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
    DBG_ASSERT(!getImpl().IsInUndo(), "Calling UndoActionStart in Undomode!");
    if (!getImpl().IsInUndo())
        getImpl().UndoActionStart(nId);
}

void EditEngine::UndoActionStart(sal_uInt16 nId, const ESelection& rSel)
{
    getImpl().UndoActionStart(nId, rSel);
}

void EditEngine::UndoActionEnd()
{
    DBG_ASSERT(!getImpl().IsInUndo(), "Calling UndoActionEnd in Undomode!");
    if (!getImpl().IsInUndo())
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
    // Create with 2 points, as with positive points it will end up with
    // LONGMAX as Size, Bottom and Right in the range > LONGMAX.
    tools::Rectangle aBigRect( -0x3FFFFFFF, -0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF );
    if( rOutDev.GetConnectMetaFile() )
        rOutDev.Push();
    Point aStartPos( rStartPos );
    if ( IsEffectivelyVertical() )
    {
        aStartPos.AdjustX(GetPaperSize().Width() );
        rStartPos.RotateAround(aStartPos, nOrientation);
    }
    getImpl().Paint(rOutDev, aBigRect, aStartPos, false, nOrientation);
    if (rOutDev.GetConnectMetaFile())
        rOutDev.Pop();
}

void EditEngine::Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos )
{
    Draw( rOutDev, rOutRect, rStartDocPos, true );
}

void EditEngine::Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos, bool bClip )
{
#if defined( DBG_UTIL ) || (OSL_DEBUG_LEVEL > 1)
    if ( bDebugPaint )
        DumpData(this, false);
#endif

    // Align to the pixel boundary, so that it becomes exactly the same
    // as Paint ()
    tools::Rectangle aOutRect( rOutDev.LogicToPixel( rOutRect ) );
    aOutRect = rOutDev.PixelToLogic( aOutRect );

    Point aStartPos;
    if ( !IsEffectivelyVertical() )
    {
        aStartPos.setX( aOutRect.Left() - rStartDocPos.X() );
        aStartPos.setY( aOutRect.Top() - rStartDocPos.Y() );
    }
    else
    {
        aStartPos.setX( aOutRect.Right() + rStartDocPos.Y() );
        aStartPos.setY( aOutRect.Top() - rStartDocPos.X() );
    }

    bool bClipRegion = rOutDev.IsClipRegion();
    bool bMetafile = rOutDev.GetConnectMetaFile();
    vcl::Region aOldRegion = rOutDev.GetClipRegion();

    // If one existed => intersection!
    // Use Push/pop for creating the Meta file
    if ( bMetafile )
        rOutDev.Push();

    // Always use the Intersect method, it is a must for Metafile!
    if ( bClip )
    {
        // Clip only if necessary...
        if ( rStartDocPos.X() || rStartDocPos.Y() ||
             ( rOutRect.GetHeight() < static_cast<tools::Long>(GetTextHeight()) ) ||
             ( rOutRect.GetWidth() < static_cast<tools::Long>(CalcTextWidth()) ) )
        {
            // Some printer drivers cause problems if characters graze the
            // ClipRegion, therefore rather add a pixel more ...
            tools::Rectangle aClipRect( aOutRect );
            if ( rOutDev.GetOutDevType() == OUTDEV_PRINTER )
            {
                Size aPixSz( 1, 0 );
                aPixSz = rOutDev.PixelToLogic( aPixSz );
                aClipRect.AdjustRight(aPixSz.Width() );
                aClipRect.AdjustBottom(aPixSz.Width() );
            }
            rOutDev.IntersectClipRegion( aClipRect );
        }
    }

    getImpl().Paint(rOutDev, aOutRect, aStartPos);

    if ( bMetafile )
        rOutDev.Pop();
    else if ( bClipRegion )
        rOutDev.SetClipRegion( aOldRegion );
    else
        rOutDev.SetClipRegion();
}

void EditEngine::InsertView(EditView* pEditView, size_t nIndex)
{
    if (nIndex > getImpl().GetEditViews().size())
        nIndex = getImpl().GetEditViews().size();

    ImpEditEngine::ViewsType& rViews = getImpl().GetEditViews();
    rViews.insert(rViews.begin()+nIndex, pEditView);

    EditSelection aStartSel = getImpl().GetEditDoc().GetStartPaM();
    pEditView->getImpl().SetEditSelection( aStartSel );
    if (!getImpl().GetActiveView())
        getImpl().SetActiveView(pEditView);

    pEditView->getImpl().AddDragAndDropListeners();
}

EditView* EditEngine::RemoveView( EditView* pView )
{
    pView->HideCursor();
    EditView* pRemoved = nullptr;
    ImpEditEngine::ViewsType& rViews = getImpl().GetEditViews();
    ImpEditEngine::ViewsType::iterator it = std::find(rViews.begin(), rViews.end(), pView);

    DBG_ASSERT( it != rViews.end(), "RemoveView with invalid index" );
    if (it != rViews.end())
    {
        pRemoved = *it;
        rViews.erase(it);
        if (getImpl().GetActiveView() == pView)
        {
            getImpl().SetActiveView(nullptr);
            getImpl().GetSelEngine().SetCurView(nullptr);
        }
        pView->getImpl().RemoveDragAndDropListeners();

    }
    return pRemoved;
}

void EditEngine::RemoveView(size_t nIndex)
{
    ImpEditEngine::ViewsType& rViews = getImpl().GetEditViews();
    if (nIndex >= rViews.size())
        return;

    EditView* pView = rViews[nIndex];
    if ( pView )
        RemoveView( pView );
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
    ImpEditEngine::ViewsType const& rViews = getImpl().GetEditViews();
    return std::find(rViews.begin(), rViews.end(), pView) != rViews.end();
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
    getImpl().GetEditDoc().SetDefTab(nDefTab);
    if (getImpl().IsFormatted())
    {
        getImpl().FormatFullDoc();
        getImpl().UpdateViews();
    }
}

void EditEngine::SetPaperSize(const Size& rNewSize)
{
    Size aOldSize = getImpl().GetPaperSize();
    getImpl().SetValidPaperSize(rNewSize);
    Size aNewSize = getImpl().GetPaperSize();

    bool bAutoPageSize = getImpl().GetStatus().AutoPageSize();
    if ( !(bAutoPageSize || ( aNewSize.Width() != aOldSize.Width() )) )
        return;

    for (EditView* pView : getImpl().maEditViews)
    {
        if ( bAutoPageSize )
            pView->getImpl().RecalcOutputArea();
        else if (pView->getImpl().DoAutoSize())
        {
            pView->getImpl().ResetOutputArea(tools::Rectangle(pView->getImpl().GetOutputArea().TopLeft(), aNewSize));
        }
    }

    if ( bAutoPageSize || getImpl().IsFormatted() )
    {
        // Changing the width has no effect for AutoPageSize, as this is
        // determined by the text width.
        // Optimization first after Vobis delivery was enabled ...
        getImpl().FormatFullDoc();

        getImpl().UpdateViews(getImpl().GetActiveView());

        if (getImpl().IsUpdateLayout() && getImpl().GetActiveView())
            getImpl().mpActiveView->ShowCursor(false, false);
    }
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
    EditSelection aSel(getImpl().CreateSel(rSelection));
    return getImpl().GetItemScriptType( aSel );
}

editeng::LanguageSpan EditEngine::GetLanguage(const EditPaM& rPaM) const
{
    return getImpl().GetLanguage(rPaM);
}

editeng::LanguageSpan EditEngine::GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const
{
    ContentNode* pNode = getImpl().GetEditDoc().GetObject( nPara );
    DBG_ASSERT( pNode, "GetLanguage - nPara is invalid!" );
    return pNode ? getImpl().GetLanguage( EditPaM( pNode, nPos ) ) : editeng::LanguageSpan{};
}


void EditEngine::TransliterateText( const ESelection& rSelection, TransliterationFlags nTransliterationMode )
{
    getImpl().TransliterateText(getImpl().CreateSel( rSelection ), nTransliterationMode);
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
    bool bSimple(false);

    if(pLinePolyPolygon && 1 == rPolyPolygon.count())
    {
        if(rPolyPolygon.getB2DPolygon(0).isClosed())
        {
            // open polygon
            bSimple = true;
        }
    }

    TextRanger* pRanger = new TextRanger( rPolyPolygon, pLinePolyPolygon, 30, 2, 2, bSimple, true );
    getImpl().SetTextRanger( std::unique_ptr<TextRanger>(pRanger) );
    getImpl().SetPaperSize( pRanger->GetBoundRect().GetSize() );
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
    EditSelection aSel = getImpl().CreateSel(rESelection);
    return getImpl().GetSelected(aSel);
}

sal_Int32 EditEngine::GetTextLen() const
{
    return getImpl().GetEditDoc().GetTextLen();
}

sal_Int32 EditEngine::GetParagraphCount() const
{
    return getImpl().maEditDoc.Count();
}

void EditEngine::ensureDocumentFormatted() const
{
    if (!getImpl().IsFormatted())
        getImpl().FormatDoc();
}

sal_Int32 EditEngine::GetLineCount( sal_Int32 nParagraph ) const
{
    ensureDocumentFormatted();
    return getImpl().GetLineCount(nParagraph);
}

sal_Int32 EditEngine::GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    ensureDocumentFormatted();
    return getImpl().GetLineLen(nParagraph, nLine);
}

void EditEngine::GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    ensureDocumentFormatted();
    return getImpl().GetLineBoundaries(rStart, rEnd, nParagraph, nLine);
}

sal_Int32 EditEngine::GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    ensureDocumentFormatted();
    return getImpl().GetLineNumberAtIndex(nPara, nIndex);
}

sal_uInt32 EditEngine::GetLineHeight( sal_Int32 nParagraph )
{
    // If someone calls GetLineHeight() with an empty Engine.
    ensureDocumentFormatted();
    return getImpl().GetLineHeight( nParagraph, 0 );
}

tools::Rectangle EditEngine::GetParaBounds( sal_Int32 nPara )
{
    ensureDocumentFormatted();
    Point aPnt = GetDocPosTopLeft( nPara );

    if( IsEffectivelyVertical() )
    {
        sal_Int32 nTextHeight = getImpl().GetTextHeight();
        sal_Int32 nParaWidth = getImpl().CalcParaWidth(nPara, true);
        sal_Int32 nParaHeight = getImpl().GetParaHeight(nPara);

        return tools::Rectangle( nTextHeight - aPnt.Y() - nParaHeight, 0, nTextHeight - aPnt.Y(), nParaWidth );
    }
    else
    {
        sal_Int32 nParaWidth = getImpl().CalcParaWidth( nPara, true );
        sal_Int32 nParaHeight = getImpl().GetParaHeight( nPara );

        return tools::Rectangle( 0, aPnt.Y(), nParaWidth, aPnt.Y() + nParaHeight );
    }
}

sal_uInt32 EditEngine::GetTextHeight( sal_Int32 nParagraph ) const
{
    ensureDocumentFormatted();
    sal_uInt32 nHeight = getImpl().GetParaHeight(nParagraph);
    return nHeight;
}

OUString EditEngine::GetWord( sal_Int32 nPara, sal_Int32 nIndex )
{
    ESelection aESel( nPara, nIndex, nPara, nIndex );
    EditSelection aSel(getImpl().CreateSel(aESel));
    aSel = getImpl().SelectWord(aSel);
    return getImpl().GetSelected(aSel);
}

ESelection EditEngine::GetWord( const ESelection& rSelection, sal_uInt16 nWordType  ) const
{
    // ImpEditEngine-Iteration-Methods should be const!
    EditEngine* pNonConstEditEngine = const_cast<EditEngine*>(this);

    EditSelection aSel(pNonConstEditEngine->getImpl().CreateSel( rSelection ) );
    aSel = pNonConstEditEngine->getImpl().SelectWord( aSel, nWordType );
    return pNonConstEditEngine->getImpl().CreateESel( aSel );
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
    DBG_ASSERT( pEditView, "no View - no cookie !" );

    bool bDone = true;

    bool bModified  = false;
    bool bMoved     = false;
    bool bAllowIdle = true;
    bool bReadOnly  = pEditView->IsReadOnly();

    CursorFlags aNewCursorFlags;
    bool bSetCursorFlags = true;

    EditSelection aCurSel( pEditView->getImpl().GetEditSelection() );
    DBG_ASSERT( !aCurSel.IsInvalid(), "Blinde Selection in EditEngine::PostKeyEvent" );

    OUString aAutoText(getImpl().GetAutoCompleteText());
    if (!getImpl().GetAutoCompleteText().isEmpty())
        getImpl().SetAutoCompleteText(OUString(), true);

    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KeyFuncType::DONTKNOW )
    {
        switch ( eFunc )
        {
            case KeyFuncType::UNDO:
            {
                if ( !bReadOnly )
                    pEditView->Undo();
                return true;
            }
            case KeyFuncType::REDO:
            {
                if ( !bReadOnly )
                    pEditView->Redo();
                return true;
            }

            default:    // is then possible edited below.
                        eFunc = KeyFuncType::DONTKNOW;
        }
    }

    if ( eFunc == KeyFuncType::DONTKNOW )
    {
        switch ( nCode )
        {
#if defined( DBG_UTIL ) || (OSL_DEBUG_LEVEL > 1)
            case KEY_F1:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    sal_Int32 nParas = GetParagraphCount();
                    Point aPos;
                    Point aViewStart( pEditView->GetOutputArea().TopLeft() );
                    tools::Long n20 = 40 * getImpl().mnOnePixelInRef;
                    for ( sal_Int32 n = 0; n < nParas; n++ )
                    {
                        tools::Long nH = GetTextHeight( n );
                        Point P1( aViewStart.X() + n20 + n20*(n%2), aViewStart.Y() + aPos.Y() );
                        Point P2( P1 );
                        P2.AdjustX(n20 );
                        P2.AdjustY(nH );
                        pEditView->GetWindow()->GetOutDev()->SetLineColor();
                        pEditView->GetWindow()->GetOutDev()->SetFillColor( (n%2) ? COL_YELLOW : COL_LIGHTGREEN );
                        pEditView->GetWindow()->GetOutDev()->DrawRect( tools::Rectangle( P1, P2 ) );
                        aPos.AdjustY(nH );
                    }
                }
                bDone = false;
            }
            break;
            case KEY_F11:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    bDebugPaint = !bDebugPaint;
                    OStringBuffer aInfo("DebugPaint: ");
                    aInfo.append(bDebugPaint ? "On" : "Off");
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pEditView->GetWindow()->GetFrameWeld(),
                                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                                  OStringToOUString(aInfo, RTL_TEXTENCODING_ASCII_US)));
                    xInfoBox->run();

                }
                bDone = false;
            }
            break;
            case KEY_F12:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                    DumpData(this, true);
                bDone = false;
            }
            break;
#endif
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case css::awt::Key::MOVE_WORD_FORWARD:
            case css::awt::Key::SELECT_WORD_FORWARD:
            case css::awt::Key::MOVE_WORD_BACKWARD:
            case css::awt::Key::SELECT_WORD_BACKWARD:
            case css::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            case css::awt::Key::MOVE_TO_END_OF_LINE:
            case css::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            case css::awt::Key::SELECT_TO_END_OF_LINE:
            case css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            case css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            case css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            case css::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            case css::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            case css::awt::Key::SELECT_TO_END_OF_DOCUMENT:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod2() || ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) )
                {
                    if ( ImpEditEngine::DoVisualCursorTraveling() && ( ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) /* || ( nCode == KEY_HOME ) || ( nCode == KEY_END ) */ ) )
                        bSetCursorFlags = false;    // Will be manipulated within visual cursor move

                    aCurSel = getImpl().MoveCursor( rKeyEvent, pEditView );

                    if ( aCurSel.HasRange() ) {
                        Reference<css::datatransfer::clipboard::XClipboard> aSelection(GetSystemPrimarySelection());
                        pEditView->getImpl().CutCopy( aSelection, false );
                    }

                    bMoved = true;
                    if ( nCode == KEY_HOME )
                        aNewCursorFlags.bStartOfLine = true;
                    else if ( nCode == KEY_END )
                        aNewCursorFlags.bEndOfLine = true;

                }
#if OSL_DEBUG_LEVEL > 1
                GetLanguage(getImpl().GetEditDoc().GetPos( aCurSel.Max().GetNode() ), aCurSel.Max().GetIndex());
#endif
            }
            break;
            case KEY_BACKSPACE:
            case KEY_DELETE:
            case css::awt::Key::DELETE_WORD_BACKWARD:
            case css::awt::Key::DELETE_WORD_FORWARD:
            case css::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::DELETE_TO_END_OF_PARAGRAPH:
            {
                if ( !bReadOnly && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    // check if we are behind a bullet and using the backspace key
                    ContentNode *pNode = aCurSel.Min().GetNode();
                    const SvxNumberFormat *pFmt = getImpl().GetNumberFormat( pNode );
                    if (pFmt && nCode == KEY_BACKSPACE &&
                        !aCurSel.HasRange() && aCurSel.Min().GetIndex() == 0)
                    {
                        // if the bullet is still visible, just make it invisible.
                        // Otherwise continue as usual.


                        sal_Int32 nPara = getImpl().GetEditDoc().GetPos( pNode );
                        SfxBoolItem aBulletState(getImpl().GetParaAttrib(nPara, EE_PARA_BULLETSTATE));

                        if ( aBulletState.GetValue() )
                        {

                            aBulletState.SetValue( false );
                            SfxItemSet aSet( getImpl().GetParaAttribs( nPara ) );
                            aSet.Put( aBulletState );
                            getImpl().SetParaAttribs( nPara, aSet );

                            // have this and the following paragraphs formatted and repainted.
                            // (not painting a numbering in the list may cause the following
                            // numberings to have different numbers than before and thus the
                            // length may have changed as well )
                            getImpl().FormatAndLayout(getImpl().GetActiveView());

                            break;
                        }
                    }

                    sal_uInt8 nDel = 0;
                    DeleteMode nMode = DeleteMode::Simple;
                    switch( nCode )
                    {
                    case css::awt::Key::DELETE_WORD_BACKWARD:
                        nMode = DeleteMode::RestOfWord;
                        nDel = DEL_LEFT;
                        break;
                    case css::awt::Key::DELETE_WORD_FORWARD:
                        nMode = DeleteMode::RestOfWord;
                        nDel = DEL_RIGHT;
                        break;
                    case css::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH:
                        nMode = DeleteMode::RestOfContent;
                        nDel = DEL_LEFT;
                        break;
                    case css::awt::Key::DELETE_TO_END_OF_PARAGRAPH:
                        nMode = DeleteMode::RestOfContent;
                        nDel = DEL_RIGHT;
                        break;
                    default:
                        nDel = ( nCode == KEY_DELETE ) ? DEL_RIGHT : DEL_LEFT;
                        nMode = rKeyEvent.GetKeyCode().IsMod1() ? DeleteMode::RestOfWord : DeleteMode::Simple;
                        if ( ( nMode == DeleteMode::RestOfWord ) && rKeyEvent.GetKeyCode().IsShift() )
                            nMode = DeleteMode::RestOfContent;
                        break;
                    }

                    pEditView->getImpl().DrawSelectionXOR();
                    getImpl().UndoActionStart( EDITUNDO_DELETE );
                    aCurSel = getImpl().DeleteLeftOrRight( aCurSel, nDel, nMode );
                    getImpl().UndoActionEnd();
                    bModified = true;
                    bAllowIdle = false;
                }
            }
            break;
            case KEY_TAB:
            {
                if ( !bReadOnly && !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    bool bShift = rKeyEvent.GetKeyCode().IsShift();
                    if ( !bShift )
                    {
                        bool bSel = pEditView->HasSelection();
                        if ( bSel )
                            getImpl().UndoActionStart( EDITUNDO_INSERT );
                        if ( getImpl().GetStatus().DoAutoCorrect() )
                            aCurSel = getImpl().AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                        aCurSel = getImpl().InsertTab( aCurSel );
                        if ( bSel )
                            getImpl().UndoActionEnd();
                        bModified = true;
                    }
                }
                else
                    bDone = false;
            }
            break;
            case KEY_RETURN:
            {
                if ( !bReadOnly )
                {
                    pEditView->getImpl().DrawSelectionXOR();
                    if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    {
                        getImpl().UndoActionStart( EDITUNDO_INSERT );
                        if ( rKeyEvent.GetKeyCode().IsShift() )
                        {
                            aCurSel = getImpl().AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                            aCurSel = getImpl().InsertLineBreak( aCurSel );
                        }
                        else
                        {
                            if (aAutoText.isEmpty())
                            {
                                if (getImpl().GetStatus().DoAutoCorrect())
                                    aCurSel = getImpl().AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                                aCurSel = getImpl().InsertParaBreak( aCurSel );
                            }
                            else
                            {
                                DBG_ASSERT( !aCurSel.HasRange(), "Selection on complete?!" );
                                EditPaM aStart = getImpl().WordLeft(aCurSel.Max());
                                EditSelection aSelection(aStart, aCurSel.Max());
                                aCurSel = getImpl().InsertText(aSelection, aAutoText);
                                getImpl().SetAutoCompleteText( OUString(), true );
                            }
                        }
                        getImpl().UndoActionEnd();
                        bModified = true;
                    }
                }
            }
            break;
            case KEY_INSERT:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    pEditView->SetInsertMode( !pEditView->IsInsertMode() );
            }
            break;
            default:
            {
                #if (OSL_DEBUG_LEVEL > 1) && defined(DBG_UTIL)
                    if ( ( nCode == KEY_W ) && rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                    {
                        SfxItemSet aAttribs = pEditView->GetAttribs();
                        const SvxFrameDirectionItem& rCurrentWritingMode = (const SvxFrameDirectionItem&)aAttribs.Get( EE_PARA_WRITINGDIR );
                        SvxFrameDirectionItem aNewItem( SvxFrameDirection::Horizontal_LR_TB, EE_PARA_WRITINGDIR );
                        if ( rCurrentWritingMode.GetValue() != SvxFrameDirection::Horizontal_RL_TB )
                            aNewItem.SetValue( SvxFrameDirection::Horizontal_RL_TB );
                        aAttribs.Put( aNewItem );
                        pEditView->SetAttribs( aAttribs );
                    }
                #endif
                if ( !bReadOnly && IsSimpleCharInput( rKeyEvent ) )
                {
                    sal_Unicode nCharCode = rKeyEvent.GetCharCode();
                    pEditView->getImpl().DrawSelectionXOR();
                    // Autocorrection?
                    if ((getImpl().GetStatus().DoAutoCorrect()) &&
                        (SvxAutoCorrect::IsAutoCorrectChar(nCharCode) ||
                            getImpl().IsNbspRunNext()))
                    {
                        aCurSel = getImpl().AutoCorrect(
                            aCurSel, nCharCode, !pEditView->IsInsertMode(), pFrameWin );
                    }
                    else
                    {
                        aCurSel = getImpl().InsertTextUserInput( aCurSel, nCharCode, !pEditView->IsInsertMode() );
                    }
                    // AutoComplete ???
                    if ( getImpl().GetStatus().DoAutoComplete() && ( nCharCode != ' ' ) )
                    {
                        // Only at end of word...
                        sal_Int32 nIndex = aCurSel.Max().GetIndex();
                        if ((nIndex >= aCurSel.Max().GetNode()->Len()) ||
                             (getImpl().maWordDelimiters.indexOf(aCurSel.Max().GetNode()->GetChar(nIndex)) != -1))
                        {
                            EditPaM aStart(getImpl().WordLeft(aCurSel.Max()));
                            OUString aWord = getImpl().GetSelected(EditSelection(aStart, aCurSel.Max()));
                            if ( aWord.getLength() >= 3 )
                            {
                                OUString aComplete;

                                LanguageType eLang = getImpl().GetLanguage(EditPaM( aStart.GetNode(), aStart.GetIndex()+1)).nLang;
                                LanguageTag aLanguageTag( eLang);

                                if (!getImpl().mxLocaleDataWrapper.isInitialized())
                                    getImpl().mxLocaleDataWrapper.init( SvtSysLocale().GetLocaleData().getComponentContext(), aLanguageTag);
                                else
                                    getImpl().mxLocaleDataWrapper.changeLocale( aLanguageTag);

                                if (!getImpl().mxTransliterationWrapper.isInitialized())
                                    getImpl().mxTransliterationWrapper.init( SvtSysLocale().GetLocaleData().getComponentContext(), eLang);
                                else
                                    getImpl().mxTransliterationWrapper.changeLocale( eLang);

                                const ::utl::TransliterationWrapper* pTransliteration = getImpl().mxTransliterationWrapper.get();
                                Sequence< i18n::CalendarItem2 > xItem = getImpl().mxLocaleDataWrapper->getDefaultCalendarDays();
                                sal_Int32 nCount = xItem.getLength();
                                const i18n::CalendarItem2* pArr = xItem.getConstArray();
                                for( sal_Int32 n = 0; n <= nCount; ++n )
                                {
                                    const OUString& rDay = pArr[n].FullName;
                                    if( pTransliteration->isMatch( aWord, rDay) )
                                    {
                                        aComplete = rDay;
                                        break;
                                    }
                                }

                                if ( aComplete.isEmpty() )
                                {
                                    xItem = getImpl().mxLocaleDataWrapper->getDefaultCalendarMonths();
                                    sal_Int32 nMonthCount = xItem.getLength();
                                    const i18n::CalendarItem2* pMonthArr = xItem.getConstArray();
                                    for( sal_Int32 n = 0; n <= nMonthCount; ++n )
                                    {
                                        const OUString& rMon = pMonthArr[n].FullName;
                                        if( pTransliteration->isMatch( aWord, rMon) )
                                        {
                                            aComplete = rMon;
                                            break;
                                        }
                                    }
                                }

                                if( !aComplete.isEmpty() && ( ( aWord.getLength() + 1 ) < aComplete.getLength() ) )
                                {
                                    getImpl().SetAutoCompleteText( aComplete, false );
                                    Point aPos = getImpl().PaMtoEditCursor( aCurSel.Max() ).TopLeft();
                                    aPos = pEditView->getImpl().GetWindowPos( aPos );
                                    aPos = pEditView->getImpl().GetWindow()->LogicToPixel( aPos );
                                    aPos = pEditView->GetWindow()->OutputToScreenPixel( aPos );
                                    aPos.AdjustY( -3 );
                                    Help::ShowQuickHelp( pEditView->GetWindow(), tools::Rectangle( aPos, Size( 1, 1 ) ), aComplete, QuickHelpFlags::Bottom|QuickHelpFlags::Left );
                                }
                            }
                        }
                    }
                    bModified = true;
                }
                else
                    bDone = false;
            }
        }
    }

    pEditView->getImpl().SetEditSelection( aCurSel );
    if (comphelper::LibreOfficeKit::isActive())
    {
        pEditView->getImpl().DrawSelectionXOR();
    }
    getImpl().UpdateSelections();

    if ( ( !IsEffectivelyVertical() && ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) ) ||
         ( IsEffectivelyVertical() && ( nCode != KEY_LEFT ) && ( nCode != KEY_RIGHT ) ))
    {
        pEditView->getImpl().mnTravelXPos = TRAVEL_X_DONTKNOW;
    }

    if ( /* ( nCode != KEY_HOME ) && ( nCode != KEY_END ) && */
        ( !IsEffectivelyVertical() && ( nCode != KEY_LEFT ) && ( nCode != KEY_RIGHT ) ) ||
         ( IsEffectivelyVertical() && ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) ))
    {
        pEditView->getImpl().SetCursorBidiLevel( CURSOR_BIDILEVEL_DONTKNOW );
    }

    if (bSetCursorFlags)
        pEditView->getImpl().maExtraCursorFlags = aNewCursorFlags;

    if ( bModified )
    {
        DBG_ASSERT( !bReadOnly, "ReadOnly but modified???" );
        // Idle-Formatter only when AnyInput.
        if ( bAllowIdle && getImpl().GetStatus().UseIdleFormatter()
                && Application::AnyInput( VclInputFlags::KEYBOARD) )
            getImpl().IdleFormatAndLayout( pEditView );
        else
            getImpl().FormatAndLayout( pEditView );
    }
    else if ( bMoved )
    {
        bool bGotoCursor = pEditView->getImpl().DoAutoScroll();
        pEditView->getImpl().ShowCursor( bGotoCursor, true );
        getImpl().CallStatusHdl();
    }

    return bDone;
}

sal_uInt32 EditEngine::GetTextHeight() const
{
    ensureDocumentFormatted();
    sal_uInt32 nHeight = !IsEffectivelyVertical() ? getImpl().GetTextHeight() : getImpl().CalcTextWidth( true );
    return nHeight;
}

sal_uInt32 EditEngine::GetTextHeightNTP() const
{
    ensureDocumentFormatted();

    if (IsEffectivelyVertical())
        return getImpl().CalcTextWidth(true);

    return getImpl().GetTextHeightNTP();
}

sal_uInt32 EditEngine::CalcTextWidth()
{
    ensureDocumentFormatted();
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
    bool bUndoEnabled = getImpl().IsUndoEnabled();
    getImpl().EnableUndo(false);
    getImpl().SetText(OUString());
    EditPaM aPaM(getImpl().GetEditDoc().GetStartPaM());
    getImpl().Read(rInput, rBaseURL, eFormat, EditSelection(aPaM, aPaM), pHTTPHeaderAttrs);
    getImpl().EnableUndo(bUndoEnabled);
    return rInput.GetError();
}

void EditEngine::Write( SvStream& rOutput, EETextFormat eFormat )
{
    EditPaM aStartPaM(getImpl().GetEditDoc().GetStartPaM());
    EditPaM aEndPaM(getImpl().GetEditDoc().GetEndPaM());
    getImpl().Write(rOutput, eFormat, EditSelection(aStartPaM, aEndPaM));
}

std::unique_ptr<EditTextObject> EditEngine::CreateTextObject()
{
    return getImpl().CreateTextObject();
}

std::unique_ptr<EditTextObject> EditEngine::CreateTextObject( const ESelection& rESelection )
{
    EditSelection aSel(getImpl().CreateSel(rESelection));
    return getImpl().CreateTextObject(aSel);
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
    DBG_ASSERT(0 <= nPara && nPara < getImpl().GetEditDoc().Count(), "CreateTextObject: Startpara out of Range");
    DBG_ASSERT(nParas <= getImpl().GetEditDoc().Count() - nPara, "CreateTextObject: Endpara out of Range");

    ContentNode* pStartNode = getImpl().GetEditDoc().GetObject(nPara);
    ContentNode* pEndNode = getImpl().GetEditDoc().GetObject(nPara + nParas - 1);
    DBG_ASSERT( pStartNode, "Start-Paragraph does not exist: CreateTextObject" );
    DBG_ASSERT( pEndNode, "End-Paragraph does not exist: CreateTextObject" );

    if ( pStartNode && pEndNode )
    {
        EditSelection aTmpSel;
        aTmpSel.Min() = EditPaM( pStartNode, 0 );
        aTmpSel.Max() = EditPaM( pEndNode, pEndNode->Len() );
        return getImpl().CreateTextObject(aTmpSel);
    }
    return nullptr;
}

void EditEngine::RemoveParagraph( sal_Int32 nPara )
{
    DBG_ASSERT(getImpl().GetEditDoc().Count() > 1, "The first paragraph should not be deleted!");
    if (getImpl().GetEditDoc().Count() <= 1)
        return;

    ContentNode* pNode = getImpl().GetEditDoc().GetObject(nPara);
    const ParaPortion* pPortion = getImpl().GetParaPortions().SafeGetObject(nPara);
    DBG_ASSERT( pPortion && pNode, "Paragraph not found: RemoveParagraph" );
    if ( pNode && pPortion )
    {
        // No Undo encapsulation needed.
        getImpl().ImpRemoveParagraph(nPara);
        getImpl().InvalidateFromParagraph(nPara);
        getImpl().UpdateSelections();
        if (getImpl().IsUpdateLayout())
            getImpl().FormatAndLayout();
    }
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
    if ( nPara > GetParagraphCount() )
    {
        SAL_WARN_IF( nPara != EE_PARA_APPEND, "editeng", "Paragraph number too large, but not EE_PARA_APPEND!" );
        nPara = GetParagraphCount();
    }

    getImpl().UndoActionStart(EDITUNDO_INSERT);

    // No Undo compounding needed.
    EditPaM aPaM(getImpl().InsertParagraph(nPara));
    // When InsertParagraph from the outside, no hard attributes
    // should be taken over!
    getImpl().RemoveCharAttribs(nPara);
    getImpl().InsertText(rTxtObj, EditSelection(aPaM, aPaM));

    if ( bAppend && nPara )
        getImpl().ConnectContents(nPara - 1, /*bBackwards=*/false);

    getImpl().UndoActionEnd();

    if (getImpl().IsUpdateLayout())
        getImpl().FormatAndLayout();
}

void EditEngine::InsertParagraph(sal_Int32 nPara, const OUString& rTxt)
{
    if ( nPara > GetParagraphCount() )
    {
        SAL_WARN_IF( nPara != EE_PARA_APPEND, "editeng", "Paragraph number too large, but not EE_PARA_APPEND!" );
        nPara = GetParagraphCount();
    }

    getImpl().UndoActionStart(EDITUNDO_INSERT);
    EditPaM aPaM(getImpl().InsertParagraph(nPara));
    // When InsertParagraph from the outside, no hard attributes
    // should be taken over!
    getImpl().RemoveCharAttribs(nPara);
    getImpl().UndoActionEnd();
    getImpl().ImpInsertText(EditSelection(aPaM, aPaM), rTxt);
    if (getImpl().IsUpdateLayout())
        getImpl().FormatAndLayout();
}

void EditEngine::SetText(sal_Int32 nPara, const OUString& rTxt)
{
    std::optional<EditSelection> pSel = getImpl().SelectParagraph(nPara);
    if ( pSel )
    {
        getImpl().UndoActionStart(EDITUNDO_INSERT);
        getImpl().ImpInsertText(*pSel, rTxt);
        getImpl().UndoActionEnd();
        if (getImpl().IsUpdateLayout())
            getImpl().FormatAndLayout();
    }
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
    EditSelection aSel(getImpl().ConvertSelection(rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos));
    return getImpl().GetAttribs(aSel, nOnlyHardAttrib);
}

SfxItemSet EditEngine::GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags ) const
{
    return getImpl().GetAttribs(nPara, nStart, nEnd, nFlags);
}

void EditEngine::RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    const EERemoveParaAttribsMode eMode = bRemoveParaAttribs?
        EERemoveParaAttribsMode::RemoveAll :
        EERemoveParaAttribsMode::RemoveCharItems;

    getImpl().UndoActionStart(EDITUNDO_RESETATTRIBS);
    EditSelection aSel(getImpl().ConvertSelection(rSelection.nStartPara, rSelection.nStartPos, rSelection.nEndPara, rSelection.nEndPos));
    getImpl().RemoveCharAttribs(aSel, eMode, nWhich);
    getImpl().UndoActionEnd();
    if (getImpl().IsUpdateLayout())
        getImpl().FormatAndLayout();
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

void EditEngine::StripPortions()
{
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
    getImpl().Paint(*aTmpDev, aBigRect, Point(), true);
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

    if (nWord == getImpl().GetStatus().GetControlWord())
        return;

    EEControlBits nPrev = getImpl().GetStatus().GetControlWord();
    getImpl().GetStatus().GetControlWord() = nWord;

    EEControlBits nChanges = nPrev ^ nWord;
    if (getImpl().IsFormatted())
    {
        // possibly reformat:
        if ( ( nChanges & EEControlBits::USECHARATTRIBS ) ||
             ( nChanges & EEControlBits::ONECHARPERLINE ) ||
             ( nChanges & EEControlBits::STRETCHING ) ||
             ( nChanges & EEControlBits::OUTLINER ) ||
             ( nChanges & EEControlBits::NOCOLORS ) ||
             ( nChanges & EEControlBits::OUTLINER2 ) )
        {
            if ( nChanges & EEControlBits::USECHARATTRIBS )
            {
                getImpl().GetEditDoc().CreateDefFont(true);
            }

            getImpl().FormatFullDoc();
            getImpl().UpdateViews(getImpl().GetActiveView());
        }
    }

    bool bSpellingChanged = bool(nChanges & EEControlBits::ONLINESPELLING);

    if ( !bSpellingChanged )
        return;

    getImpl().StopOnlineSpellTimer();
    if (nWord & EEControlBits::ONLINESPELLING)
    {
        // Create WrongList, start timer...
        sal_Int32 nNodes = getImpl().GetEditDoc().Count();
        for (sal_Int32 nNode = 0; nNode < nNodes; nNode++)
        {
            ContentNode* pNode = getImpl().GetEditDoc().GetObject(nNode);
            pNode->CreateWrongList();
        }
        if (getImpl().IsFormatted())
            getImpl().StartOnlineSpellTimer();
    }
    else
    {
        tools::Long nY = 0;
        sal_Int32 nNodes = getImpl().GetEditDoc().Count();
        for ( sal_Int32 nNode = 0; nNode < nNodes; nNode++)
        {
            ContentNode* pNode = getImpl().GetEditDoc().GetObject(nNode);
            ParaPortion const& rPortion = getImpl().GetParaPortions().getRef(nNode);
            bool bWrongs = false;
            if (pNode->GetWrongList() != nullptr)
                bWrongs = !pNode->GetWrongList()->empty();
            pNode->DestroyWrongList();
            if ( bWrongs )
            {
                getImpl().maInvalidRect.SetLeft(0);
                getImpl().maInvalidRect.SetRight(getImpl().GetPaperSize().Width());
                getImpl().maInvalidRect.SetTop(nY + 1);
                getImpl().maInvalidRect.SetBottom(nY + rPortion.GetHeight() - 1);
                getImpl().UpdateViews(getImpl().mpActiveView);
            }
            nY += rPortion.GetHeight();
        }
    }
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
        ensureDocumentFormatted();
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
    const ParaPortion* pPPortion = getImpl().GetParaPortions().SafeGetObject(nParagraph);
    DBG_ASSERT( pPPortion, "Paragraph not found: GetWindowPosTopLeft" );
    Point aPoint;
    if ( pPPortion )
    {
        // If someone calls GetLineHeight() with an empty Engine.
        DBG_ASSERT(getImpl().IsFormatted() || !getImpl().IsFormatting(), "GetDocPosTopLeft: Doc not formatted - unable to format!");
        if (!getImpl().IsFormatted())
            getImpl().FormatAndLayout();
        if (pPPortion->GetLines().Count())
        {
            // Correct it if large Bullet.
            const EditLine& rFirstLine = pPPortion->GetLines()[0];
            aPoint.setX( rFirstLine.GetStartPosX() );
        }
        else
        {
            const SvxLRSpaceItem& rLRItem = getImpl().GetLRSpaceItem(pPPortion->GetNode());
            sal_Int32 nSpaceBefore = 0;
            getImpl().GetSpaceBeforeAndMinLabelWidth(pPPortion->GetNode(), &nSpaceBefore);
            short nX = static_cast<short>(rLRItem.GetTextLeft()
                            + rLRItem.GetTextFirstLineOffset()
                            + nSpaceBefore);

            aPoint.setX(getImpl().scaleXSpacingValue(nX));
        }
        aPoint.setY(getImpl().GetParaPortions().GetYOffset(pPPortion));
    }
    return aPoint;
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
    ensureDocumentFormatted();

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
    EditSelection aSel(getImpl().ConvertSelection(rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos));
    getImpl().SetAttribs(aSel, rSet);
}

void EditEngine::QuickMarkInvalid( const ESelection& rSel )
{
    DBG_ASSERT(rSel.nStartPara < getImpl().GetEditDoc().Count(), "MarkInvalid: Start out of Range!");
    DBG_ASSERT(rSel.nEndPara < getImpl().GetEditDoc().Count(), "MarkInvalid: End out of Range!");
    for (sal_Int32 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++)
    {
        ParaPortion* pPortion = getImpl().GetParaPortions().SafeGetObject(nPara);
        if ( pPortion )
            pPortion->MarkSelectionInvalid( 0 );
    }
}

void EditEngine::QuickInsertText(const OUString& rText, const ESelection& rSel)
{
    EditSelection aSel(getImpl().ConvertSelection(rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos));
    getImpl().ImpInsertText(aSel, rText);
}

void EditEngine::QuickDelete( const ESelection& rSel )
{
    EditSelection aSel(getImpl().ConvertSelection(rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos));
    getImpl().ImpDeleteSelection( aSel );
}

void EditEngine::QuickMarkToBeRepainted( sal_Int32 nPara )
{
    ParaPortion* pPortion = getImpl().GetParaPortions().SafeGetObject(nPara);
    if ( pPortion )
        pPortion->SetMustRepaint( true );
}

void EditEngine::QuickInsertLineBreak( const ESelection& rSel )
{
    EditSelection aSel(getImpl().ConvertSelection(rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos));
    getImpl().InsertLineBreak( aSel );
}

void EditEngine::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{

    EditSelection aSel(getImpl().ConvertSelection(rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos));
    getImpl().ImpInsertFeature(aSel, rFld);
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

ScalingParameters EditEngine::getScalingParameters() const
{
    return getImpl().getScalingParameters();
}

void EditEngine::resetScalingParameters()
{
    getImpl().resetScalingParameters();
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

sal_uInt16 EditEngine::GetFieldCount( sal_Int32 nPara ) const
{
    sal_uInt16 nFields = 0;
    ContentNode* pNode = getImpl().GetEditDoc().GetObject(nPara);
    if ( pNode )
    {
        for (auto const& attrib : pNode->GetCharAttribs().GetAttribs())
        {
            if (attrib->Which() == EE_FEATURE_FIELD)
                ++nFields;
        }
    }

    return nFields;
}

EFieldInfo EditEngine::GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const
{
    ContentNode* pNode = getImpl().GetEditDoc().GetObject(nPara);
    if ( pNode )
    {
        sal_uInt16 nCurrentField = 0;
        for (auto const& attrib : pNode->GetCharAttribs().GetAttribs())
        {
            const EditCharAttrib& rAttr = *attrib;
            if (rAttr.Which() == EE_FEATURE_FIELD)
            {
                if ( nCurrentField == nField )
                {
                    const SvxFieldItem* p = static_cast<const SvxFieldItem*>(rAttr.GetItem());
                    EFieldInfo aInfo(*p, nPara, rAttr.GetStart());
                    aInfo.aCurrentText = static_cast<const EditCharAttribField&>(rAttr).GetFieldValue();
                    return aInfo;
                }

                ++nCurrentField;
            }
        }
    }
    return EFieldInfo();
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

EPosition EditEngine::FindDocPosition( const Point& rDocPos ) const
{
    EPosition aPos;
    // From the point of the API, this is const...
    EditPaM aPaM = getImpl().GetPaM(rDocPos, false);
    if ( aPaM.GetNode() )
    {
        aPos.nPara = getImpl().maEditDoc.GetPos(aPaM.GetNode());
        aPos.nIndex = aPaM.GetIndex();
    }
    return aPos;
}

tools::Rectangle EditEngine::GetCharacterBounds( const EPosition& rPos ) const
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
    ensureDocumentFormatted();

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
    EditSelection aSel(getImpl().CreateSel(rSelection));
    return getImpl().CreateTransferable(aSel);
}


// ======================    Virtual Methods    ========================

void EditEngine::DrawingText( const Point&, const OUString&, sal_Int32, sal_Int32,
                              std::span<const sal_Int32>, std::span<const sal_Bool>,
                              const SvxFont&, sal_Int32 /*nPara*/, sal_uInt8 /*nRightToLeft*/,
                              const EEngineData::WrongSpellVector*, const SvxFieldData*, bool, bool,
                              const css::lang::Locale*, const Color&, const Color&)

{
}

void EditEngine::DrawingTab( const Point& /*rStartPos*/, tools::Long /*nWidth*/,
                             const OUString& /*rChar*/, const SvxFont& /*rFont*/,
                             sal_Int32 /*nPara*/, sal_uInt8 /*nRightToLeft*/, bool /*bEndOfLine*/,
                             bool /*bEndOfParagraph*/, const Color& /*rOverlineColor*/,
                             const Color& /*rTextLineColor*/)
{
}

void EditEngine::PaintingFirstLine(sal_Int32, const Point&, const Point&, Degree10, OutputDevice&)
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


/** If we let the libc runtime clean us up, we trigger a crash */
namespace
{
class TerminateListener : public ::cppu::WeakImplHelper< css::frame::XTerminateListener >
{
    void SAL_CALL queryTermination( const lang::EventObject& ) override
    {}
    void SAL_CALL notifyTermination( const lang::EventObject& ) override
    {
        pGlobalPool.clear();
    }
    virtual void SAL_CALL disposing( const ::css::lang::EventObject& ) override
    {}
};
};

SfxItemPool& EditEngine::GetGlobalItemPool()
{
    if ( !pGlobalPool )
    {
        pGlobalPool = CreatePool();
#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
        // TerminateListener option not available, force it to leak
        pGlobalPool->acquire();
#else
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(comphelper::getProcessComponentContext());
        uno::Reference< frame::XTerminateListener > xListener( new TerminateListener );
        xDesktop->addTerminateListener( xListener );
#endif
    }
    return *pGlobalPool;
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
    return EditEngine::IsPrintable( rKeyEvent.GetCharCode() ) &&
        ( KEY_MOD2 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT ) ) &&
        ( KEY_MOD1 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT ) );
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
