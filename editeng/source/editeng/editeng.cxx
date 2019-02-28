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
#include <config_global.h>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>

#include <svtools/ctrltool.hxx>

#include <editeng/svxfont.hxx>
#include "impedit.hxx"
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include "editdbg.hxx"
#include <eerdll2.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>
#include <editeng.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/flditem.hxx>
#include <editeng/txtrange.hxx>
#include <editeng/cmapitem.hxx>
#include <vcl/graph.hxx>

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
#include <editeng/brushitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>

#include <sot/exchange.hxx>
#include <sot/formats.hxx>

#include <editeng/numitem.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/unolingu.hxx>
#include <linguistic/lngprops.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <vcl/help.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>

#include <svl/srchdefs.hxx>

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

static SfxItemPool* pGlobalPool=nullptr;

EditEngine::EditEngine( SfxItemPool* pItemPool )
{
    pImpEditEngine.reset( new ImpEditEngine( this, pItemPool ) );
}

EditEngine::~EditEngine()
{
}

void EditEngine::EnableUndo( bool bEnable )
{
    pImpEditEngine->EnableUndo( bEnable );
}

bool EditEngine::IsUndoEnabled()
{
    return pImpEditEngine->IsUndoEnabled();
}

bool EditEngine::IsInUndo()
{
    return pImpEditEngine->IsInUndo();
}

SfxUndoManager& EditEngine::GetUndoManager()
{
    return pImpEditEngine->GetUndoManager();
}

SfxUndoManager* EditEngine::SetUndoManager(SfxUndoManager* pNew)
{
    return pImpEditEngine->SetUndoManager(pNew);
}

void EditEngine::UndoActionStart( sal_uInt16 nId )
{
    DBG_ASSERT( !pImpEditEngine->IsInUndo(), "Calling UndoActionStart in Undomode!" );
    if ( !pImpEditEngine->IsInUndo() )
        pImpEditEngine->UndoActionStart( nId );
}

void EditEngine::UndoActionStart(sal_uInt16 nId, const ESelection& rSel)
{
    pImpEditEngine->UndoActionStart(nId, rSel);
}

void EditEngine::UndoActionEnd()
{
    DBG_ASSERT( !pImpEditEngine->IsInUndo(), "Calling UndoActionEnd in Undomode!" );
    if ( !pImpEditEngine->IsInUndo() )
        pImpEditEngine->UndoActionEnd();
}

bool EditEngine::HasTriedMergeOnLastAddUndo() const
{
    return pImpEditEngine->mbLastTryMerge;
}

void EditEngine::SetRefDevice( OutputDevice* pRefDev )
{
    pImpEditEngine->SetRefDevice( pRefDev );
}

OutputDevice* EditEngine::GetRefDevice() const
{
    return pImpEditEngine->GetRefDevice();
}

void EditEngine::SetRefMapMode( const MapMode& rMapMode )
{
    pImpEditEngine->SetRefMapMode( rMapMode );
}

MapMode const & EditEngine::GetRefMapMode()
{
    return pImpEditEngine->GetRefMapMode();
}

void EditEngine::SetBackgroundColor( const Color& rColor )
{
    pImpEditEngine->SetBackgroundColor( rColor );
}

Color const & EditEngine::GetBackgroundColor() const
{
    return pImpEditEngine->GetBackgroundColor();
}

Color EditEngine::GetAutoColor() const
{
    return pImpEditEngine->GetAutoColor();
}

void EditEngine::EnableAutoColor( bool b )
{
    pImpEditEngine->EnableAutoColor( b );
}

void EditEngine::ForceAutoColor( bool b )
{
    pImpEditEngine->ForceAutoColor( b );
}

bool EditEngine::IsForceAutoColor() const
{
    return pImpEditEngine->IsForceAutoColor();
}

const SfxItemSet& EditEngine::GetEmptyItemSet()
{
    return pImpEditEngine->GetEmptyItemSet();
}

void EditEngine::Draw( OutputDevice* pOutDev, const tools::Rectangle& rOutRect )
{
    Draw( pOutDev, rOutRect, Point( 0, 0 ) );
}

void EditEngine::Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation )
{
    // Create with 2 points, as with positive points it will end up with
    // LONGMAX as Size, Bottom and Right in the range > LONGMAX.
    tools::Rectangle aBigRect( -0x3FFFFFFF, -0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF );
    if( pOutDev->GetConnectMetaFile() )
        pOutDev->Push();
    Point aStartPos( rStartPos );
    if ( IsVertical() )
    {
        aStartPos.AdjustX(GetPaperSize().Width() );
        aStartPos = Rotate( aStartPos, nOrientation, rStartPos );
    }
    pImpEditEngine->Paint( pOutDev, aBigRect, aStartPos, false, nOrientation );
    if( pOutDev->GetConnectMetaFile() )
        pOutDev->Pop();
}

void EditEngine::Draw( OutputDevice* pOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos )
{
    Draw( pOutDev, rOutRect, rStartDocPos, true );
}

void EditEngine::Draw( OutputDevice* pOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos, bool bClip )
{
#if defined( DBG_UTIL ) || (OSL_DEBUG_LEVEL > 1)
    if ( bDebugPaint )
        EditDbg::ShowEditEngineData( this, false );
#endif

    // Align to the pixel boundary, so that it becomes exactly the same
    // as Paint ()
    tools::Rectangle aOutRect( pOutDev->LogicToPixel( rOutRect ) );
    aOutRect = pOutDev->PixelToLogic( aOutRect );

    Point aStartPos;
    if ( !IsVertical() )
    {
        aStartPos.setX( aOutRect.Left() - rStartDocPos.X() );
        aStartPos.setY( aOutRect.Top() - rStartDocPos.Y() );
    }
    else
    {
        aStartPos.setX( aOutRect.Right() + rStartDocPos.Y() );
        aStartPos.setY( aOutRect.Top() - rStartDocPos.X() );
    }

    bool bClipRegion = pOutDev->IsClipRegion();
    bool bMetafile = pOutDev->GetConnectMetaFile();
    vcl::Region aOldRegion = pOutDev->GetClipRegion();

    // If one existed => intersection!
    // Use Push/pop for creating the Meta file
    if ( bMetafile )
        pOutDev->Push();

    // Always use the Intersect method, it is a must for Metafile!
    if ( bClip )
    {
        // Clip only if necessary...
        if ( rStartDocPos.X() || rStartDocPos.Y() ||
             ( rOutRect.GetHeight() < static_cast<long>(GetTextHeight()) ) ||
             ( rOutRect.GetWidth() < static_cast<long>(CalcTextWidth()) ) )
        {
            // Some printer drivers cause problems if characters graze the
            // ClipRegion, therefore rather add a pixel more ...
            tools::Rectangle aClipRect( aOutRect );
            if ( pOutDev->GetOutDevType() == OUTDEV_PRINTER )
            {
                Size aPixSz( 1, 0 );
                aPixSz = pOutDev->PixelToLogic( aPixSz );
                aClipRect.AdjustRight(aPixSz.Width() );
                aClipRect.AdjustBottom(aPixSz.Width() );
            }
            pOutDev->IntersectClipRegion( aClipRect );
        }
    }

    pImpEditEngine->Paint( pOutDev, aOutRect, aStartPos );

    if ( bMetafile )
        pOutDev->Pop();
    else if ( bClipRegion )
        pOutDev->SetClipRegion( aOldRegion );
    else
        pOutDev->SetClipRegion();
}

void EditEngine::InsertView(EditView* pEditView, size_t nIndex)
{

    if ( nIndex > pImpEditEngine->GetEditViews().size() )
        nIndex = pImpEditEngine->GetEditViews().size();

    ImpEditEngine::ViewsType& rViews = pImpEditEngine->GetEditViews();
    rViews.insert(rViews.begin()+nIndex, pEditView);

    EditSelection aStartSel;
    aStartSel = pImpEditEngine->GetEditDoc().GetStartPaM();
    pEditView->pImpEditView->SetEditSelection( aStartSel );
    if ( !pImpEditEngine->GetActiveView() )
        pImpEditEngine->SetActiveView( pEditView );

    pEditView->pImpEditView->AddDragAndDropListeners();
}

EditView* EditEngine::RemoveView( EditView* pView )
{

    pView->HideCursor();
    EditView* pRemoved = nullptr;
    ImpEditEngine::ViewsType& rViews = pImpEditEngine->GetEditViews();
    ImpEditEngine::ViewsType::iterator it = std::find(rViews.begin(), rViews.end(), pView);

    DBG_ASSERT( it != rViews.end(), "RemoveView with invalid index" );
    if (it != rViews.end())
    {
        pRemoved = *it;
        rViews.erase(it);
        if ( pImpEditEngine->GetActiveView() == pView )
        {
            pImpEditEngine->SetActiveView( nullptr );
            pImpEditEngine->GetSelEngine().SetCurView( nullptr );
        }
        pView->pImpEditView->RemoveDragAndDropListeners();

    }
    return pRemoved;
}

void EditEngine::RemoveView(size_t nIndex)
{
    ImpEditEngine::ViewsType& rViews = pImpEditEngine->GetEditViews();
    if (nIndex >= rViews.size())
        return;

    EditView* pView = rViews[nIndex];
    if ( pView )
        RemoveView( pView );
}

EditView* EditEngine::GetView(size_t nIndex) const
{
    return pImpEditEngine->GetEditViews()[nIndex];
}

size_t EditEngine::GetViewCount() const
{
    return pImpEditEngine->GetEditViews().size();
}

bool EditEngine::HasView( EditView* pView ) const
{
    ImpEditEngine::ViewsType& rViews = pImpEditEngine->GetEditViews();
    return std::find(rViews.begin(), rViews.end(), pView) != rViews.end();
}

EditView* EditEngine::GetActiveView() const
{
    return pImpEditEngine->GetActiveView();
}

void EditEngine::SetActiveView(EditView* pView)
{
    pImpEditEngine->SetActiveView(pView);
}

void EditEngine::SetDefTab( sal_uInt16 nDefTab )
{
    pImpEditEngine->GetEditDoc().SetDefTab( nDefTab );
    if ( pImpEditEngine->IsFormatted() )
    {
        pImpEditEngine->FormatFullDoc();
        pImpEditEngine->UpdateViews();
    }
}

void EditEngine::SetPaperSize( const Size& rNewSize )
{

    Size aOldSize( pImpEditEngine->GetPaperSize() );
    pImpEditEngine->SetValidPaperSize( rNewSize );
    Size aNewSize( pImpEditEngine->GetPaperSize() );

    bool bAutoPageSize = pImpEditEngine->GetStatus().AutoPageSize();
    if ( bAutoPageSize || ( aNewSize.Width() != aOldSize.Width() ) )
    {
        for (EditView* pView : pImpEditEngine->aEditViews)
        {
            if ( bAutoPageSize )
                pView->pImpEditView->RecalcOutputArea();
            else if ( pView->pImpEditView->DoAutoSize() )
            {
                pView->pImpEditView->ResetOutputArea( tools::Rectangle(
                    pView->pImpEditView->GetOutputArea().TopLeft(), aNewSize ) );
            }
        }

        if ( bAutoPageSize || pImpEditEngine->IsFormatted() )
        {
            // Changing the width has no effect for AutoPageSize, as this is
            // determined by the text width.
            // Optimization first after Vobis delivery was enabled ...
            pImpEditEngine->FormatFullDoc();

            pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );

            if ( pImpEditEngine->GetUpdateMode() && pImpEditEngine->GetActiveView() )
                pImpEditEngine->pActiveView->ShowCursor( false, false );
        }
    }
}

const Size& EditEngine::GetPaperSize() const
{
    return pImpEditEngine->GetPaperSize();
}

void EditEngine::SetVertical( bool bVertical, bool bTopToBottom )
{
    pImpEditEngine->SetVertical( bVertical, bTopToBottom);
}

bool EditEngine::IsVertical() const
{
    return pImpEditEngine->IsVertical();
}

bool EditEngine::IsTopToBottom() const
{
    return pImpEditEngine->IsTopToBottom();
}

void EditEngine::SetFixedCellHeight( bool bUseFixedCellHeight )
{
    pImpEditEngine->SetFixedCellHeight( bUseFixedCellHeight );
}

void EditEngine::SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir )
{
    pImpEditEngine->SetDefaultHorizontalTextDirection( eHTextDir );
}

EEHorizontalTextDirection EditEngine::GetDefaultHorizontalTextDirection() const
{
    return pImpEditEngine->GetDefaultHorizontalTextDirection();
}

SvtScriptType EditEngine::GetScriptType( const ESelection& rSelection ) const
{
    EditSelection aSel( pImpEditEngine->CreateSel( rSelection ) );
    return pImpEditEngine->GetItemScriptType( aSel );
}

LanguageType EditEngine::GetLanguage(const EditPaM& rPaM) const
{
    return pImpEditEngine->GetLanguage(rPaM);
}

LanguageType EditEngine::GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const
{
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    DBG_ASSERT( pNode, "GetLanguage - nPara is invalid!" );
    return pNode ? pImpEditEngine->GetLanguage( EditPaM( pNode, nPos ) ) : LANGUAGE_DONTKNOW;
}


void EditEngine::TransliterateText( const ESelection& rSelection, TransliterationFlags nTransliterationMode )
{
    pImpEditEngine->TransliterateText( pImpEditEngine->CreateSel( rSelection ), nTransliterationMode );
}

EditSelection EditEngine::TransliterateText(const EditSelection& rSelection, TransliterationFlags nTransliterationMode)
{
    return pImpEditEngine->TransliterateText(rSelection, nTransliterationMode);
}

void EditEngine::SetAsianCompressionMode( CharCompressType n )
{
    pImpEditEngine->SetAsianCompressionMode( n );
}

void EditEngine::SetKernAsianPunctuation( bool b )
{
    pImpEditEngine->SetKernAsianPunctuation( b );
}

void EditEngine::SetAddExtLeading( bool b )
{
    pImpEditEngine->SetAddExtLeading( b );
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
    pImpEditEngine->SetTextRanger( std::unique_ptr<TextRanger>(pRanger) );
    pImpEditEngine->SetPaperSize( pRanger->GetBoundRect().GetSize() );
}

void EditEngine::ClearPolygon()
{
    pImpEditEngine->SetTextRanger( nullptr );
}

const Size& EditEngine::GetMinAutoPaperSize() const
{
    return pImpEditEngine->GetMinAutoPaperSize();
}

void EditEngine::SetMinAutoPaperSize( const Size& rSz )
{
    pImpEditEngine->SetMinAutoPaperSize( rSz );
}

const Size& EditEngine::GetMaxAutoPaperSize() const
{
    return pImpEditEngine->GetMaxAutoPaperSize();
}

void EditEngine::SetMaxAutoPaperSize( const Size& rSz )
{
    pImpEditEngine->SetMaxAutoPaperSize( rSz );
}

OUString EditEngine::GetText( LineEnd eEnd ) const
{
    return pImpEditEngine->GetEditDoc().GetText( eEnd );
}

OUString EditEngine::GetText( const ESelection& rESelection ) const
{
    EditSelection aSel( pImpEditEngine->CreateSel( rESelection ) );
    return pImpEditEngine->GetSelected( aSel );
}

sal_uInt32 EditEngine::GetTextLen() const
{
    return pImpEditEngine->GetEditDoc().GetTextLen();
}

sal_Int32 EditEngine::GetParagraphCount() const
{
    return pImpEditEngine->aEditDoc.Count();
}

sal_Int32 EditEngine::GetLineCount( sal_Int32 nParagraph ) const
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineCount( nParagraph );
}

sal_Int32 EditEngine::GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineLen( nParagraph, nLine );
}

void EditEngine::GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineBoundaries( rStart, rEnd, nParagraph, nLine );
}

sal_Int32 EditEngine::GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineNumberAtIndex( nPara, nIndex );
}

sal_uInt32 EditEngine::GetLineHeight( sal_Int32 nParagraph )
{
    // If someone calls GetLineHeight() with an empty Engine.
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineHeight( nParagraph, 0 );
}

tools::Rectangle EditEngine::GetParaBounds( sal_Int32 nPara )
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    Point aPnt = GetDocPosTopLeft( nPara );

    if( IsVertical() )
    {
        sal_Int32 nTextHeight = pImpEditEngine->GetTextHeight();
        sal_Int32 nParaWidth = pImpEditEngine->CalcParaWidth( nPara, true );
        sal_uLong nParaHeight = pImpEditEngine->GetParaHeight( nPara );

        return tools::Rectangle( nTextHeight - aPnt.Y() - nParaHeight, 0, nTextHeight - aPnt.Y(), nParaWidth );
    }
    else
    {
        sal_Int32 nParaWidth = pImpEditEngine->CalcParaWidth( nPara, true );
        sal_uLong nParaHeight = pImpEditEngine->GetParaHeight( nPara );

        return tools::Rectangle( 0, aPnt.Y(), nParaWidth, aPnt.Y() + nParaHeight );
    }
}

sal_uInt32 EditEngine::GetTextHeight( sal_Int32 nParagraph ) const
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nHeight = pImpEditEngine->GetParaHeight( nParagraph );
    return nHeight;
}

OUString EditEngine::GetWord( sal_Int32 nPara, sal_Int32 nIndex )
{
    ESelection aESel( nPara, nIndex, nPara, nIndex );
    EditSelection aSel( pImpEditEngine->CreateSel( aESel ) );
    aSel = pImpEditEngine->SelectWord( aSel );
    return pImpEditEngine->GetSelected( aSel );
}

ESelection EditEngine::GetWord( const ESelection& rSelection, sal_uInt16 nWordType  ) const
{
    // ImpEditEngine-Iteration-Methods should be const!
    EditEngine* pE = const_cast<EditEngine*>(this);

    EditSelection aSel( pE->pImpEditEngine->CreateSel( rSelection ) );
    aSel = pE->pImpEditEngine->SelectWord( aSel, nWordType );
    return pE->pImpEditEngine->CreateESel( aSel );
}

void EditEngine::CursorMoved(const ContentNode* pPrevNode)
{
    pImpEditEngine->CursorMoved(pPrevNode);
}

void EditEngine::CheckIdleFormatter()
{
    pImpEditEngine->CheckIdleFormatter();
}

bool EditEngine::IsIdleFormatterActive() const
{
    return pImpEditEngine->aIdleFormatter.IsActive();
}

ParaPortion* EditEngine::FindParaPortion(ContentNode const * pNode)
{
    return pImpEditEngine->FindParaPortion(pNode);
}

const ParaPortion* EditEngine::FindParaPortion(ContentNode const * pNode) const
{
    return pImpEditEngine->FindParaPortion(pNode);
}

const ParaPortion* EditEngine::GetPrevVisPortion(const ParaPortion* pCurPortion) const
{
    return pImpEditEngine->GetPrevVisPortion(pCurPortion);
}

SvtScriptType EditEngine::GetScriptType(const EditSelection& rSel) const
{
    return pImpEditEngine->GetItemScriptType(rSel);
}

void EditEngine::RemoveParaPortion(sal_Int32 nNode)
{
    pImpEditEngine->GetParaPortions().Remove(nNode);
}

void EditEngine::SetCallParaInsertedOrDeleted(bool b)
{
    pImpEditEngine->SetCallParaInsertedOrDeleted(b);
}

bool EditEngine::IsCallParaInsertedOrDeleted() const
{
    return pImpEditEngine->IsCallParaInsertedOrDeleted();
}

void EditEngine::AppendDeletedNodeInfo(DeletedNodeInfo* pInfo)
{
    pImpEditEngine->aDeletedNodes.push_back(std::unique_ptr<DeletedNodeInfo>(pInfo));
}

void EditEngine::UpdateSelections()
{
    pImpEditEngine->UpdateSelections();
}

void EditEngine::InsertContent(ContentNode* pNode, sal_Int32 nPos)
{
    pImpEditEngine->InsertContent(pNode, nPos);
}

EditPaM EditEngine::SplitContent(sal_Int32 nNode, sal_Int32 nSepPos)
{
    return pImpEditEngine->SplitContent(nNode, nSepPos);
}

EditPaM EditEngine::ConnectContents(sal_Int32 nLeftNode, bool bBackward)
{
    return pImpEditEngine->ConnectContents(nLeftNode, bBackward);
}

void EditEngine::InsertFeature(const EditSelection& rEditSelection, const SfxPoolItem& rItem)
{
    pImpEditEngine->ImpInsertFeature(rEditSelection, rItem);
}

EditSelection EditEngine::MoveParagraphs(const Range& rParagraphs, sal_Int32 nNewPos)
{
    return pImpEditEngine->MoveParagraphs(rParagraphs, nNewPos, nullptr);
}

void EditEngine::RemoveCharAttribs(sal_Int32 nPara, sal_uInt16 nWhich, bool bRemoveFeatures)
{
    pImpEditEngine->RemoveCharAttribs(nPara, nWhich, bRemoveFeatures);
}

void EditEngine::RemoveCharAttribs(const EditSelection& rSel, bool bRemoveParaAttribs, sal_uInt16 nWhich)
{
    pImpEditEngine->RemoveCharAttribs(rSel, bRemoveParaAttribs, nWhich);
}

EditEngine::ViewsType& EditEngine::GetEditViews()
{
    return pImpEditEngine->GetEditViews();
}

const EditEngine::ViewsType& EditEngine::GetEditViews() const
{
    return pImpEditEngine->GetEditViews();
}

void EditEngine::SetUndoMode(bool b)
{
    pImpEditEngine->SetUndoMode(b);
}

void EditEngine::FormatAndUpdate(EditView* pCurView, bool bCalledFromUndo)
{
    pImpEditEngine->FormatAndUpdate(pCurView, bCalledFromUndo);
}

void EditEngine::Undo(EditView* pView)
{
    pImpEditEngine->Undo(pView);
}

void EditEngine::Redo(EditView* pView)
{
    pImpEditEngine->Redo(pView);
}

uno::Reference<datatransfer::XTransferable> EditEngine::CreateTransferable(const EditSelection& rSelection)
{
    return pImpEditEngine->CreateTransferable(rSelection);
}

void EditEngine::ParaAttribsToCharAttribs(ContentNode* pNode)
{
    pImpEditEngine->ParaAttribsToCharAttribs(pNode);
}

EditPaM EditEngine::CreateEditPaM(const EPaM& rEPaM)
{
    return pImpEditEngine->CreateEditPaM(rEPaM);
}

EditPaM EditEngine::ConnectParagraphs(
        ContentNode* pLeft, ContentNode* pRight, bool bBackward)
{
    return pImpEditEngine->ImpConnectParagraphs(pLeft, pRight, bBackward);
}

EditPaM EditEngine::InsertField(const EditSelection& rEditSelection, const SvxFieldItem& rFld)
{
    return pImpEditEngine->InsertField(rEditSelection, rFld);
}

EditPaM EditEngine::InsertText(const EditSelection& aCurEditSelection, const OUString& rStr)
{
    return pImpEditEngine->InsertText(aCurEditSelection, rStr);
}

EditSelection EditEngine::InsertText(const EditTextObject& rTextObject, const EditSelection& rSel)
{
    return pImpEditEngine->InsertText(rTextObject, rSel);
}

EditSelection EditEngine::InsertText(
    uno::Reference<datatransfer::XTransferable > const & rxDataObj,
    const OUString& rBaseURL, const EditPaM& rPaM, bool bUseSpecial)
{
    return pImpEditEngine->PasteText(rxDataObj, rBaseURL, rPaM, bUseSpecial);
}

EditPaM EditEngine::EndOfWord(const EditPaM& rPaM)
{
    return pImpEditEngine->EndOfWord(rPaM);
}

EditPaM EditEngine::GetPaM(const Point& aDocPos, bool bSmart)
{
    return pImpEditEngine->GetPaM(aDocPos, bSmart);
}

EditSelection EditEngine::SelectWord(
        const EditSelection& rCurSelection, sal_Int16 nWordType)
{
    return pImpEditEngine->SelectWord(rCurSelection, nWordType);
}

long EditEngine::GetXPos(
        const ParaPortion* pParaPortion, const EditLine* pLine, sal_Int32 nIndex, bool bPreferPortionStart) const
{
    return pImpEditEngine->GetXPos(pParaPortion, pLine, nIndex, bPreferPortionStart);
}

Range EditEngine::GetLineXPosStartEnd(
        const ParaPortion* pParaPortion, const EditLine* pLine) const
{
    return pImpEditEngine->GetLineXPosStartEnd(pParaPortion, pLine);
}

bool EditEngine::IsFormatted() const
{
    return pImpEditEngine->IsFormatted();
}

EditPaM EditEngine::CursorLeft(const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode)
{
    return pImpEditEngine->CursorLeft(rPaM, nCharacterIteratorMode);
}

EditPaM EditEngine::CursorRight(const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode)
{
    return pImpEditEngine->CursorRight(rPaM, nCharacterIteratorMode);
}

InternalEditStatus& EditEngine::GetInternalEditStatus()
{
    return pImpEditEngine->GetStatus();
}

EditDoc& EditEngine::GetEditDoc()
{
    return pImpEditEngine->GetEditDoc();
}

const EditDoc& EditEngine::GetEditDoc() const
{
    return pImpEditEngine->GetEditDoc();
}

void EditEngine::dumpAsXmlEditDoc(xmlTextWriterPtr pWriter) const
{
    pImpEditEngine->GetEditDoc().dumpAsXml(pWriter);
}

ParaPortionList& EditEngine::GetParaPortions()
{
    return pImpEditEngine->GetParaPortions();
}

const ParaPortionList& EditEngine::GetParaPortions() const
{
    return pImpEditEngine->GetParaPortions();
}

void EditEngine::SeekCursor(ContentNode* pNode, sal_Int32 nPos, SvxFont& rFont)
{
    pImpEditEngine->SeekCursor(pNode, nPos, rFont);
}

EditPaM EditEngine::DeleteSelection(const EditSelection& rSel)
{
    return pImpEditEngine->ImpDeleteSelection(rSel);
}

ESelection EditEngine::CreateESelection(const EditSelection& rSel)
{
    return pImpEditEngine->CreateESel(rSel);
}

EditSelection EditEngine::CreateSelection(const ESelection& rSel)
{
    return pImpEditEngine->CreateSel(rSel);
}

const SfxItemSet& EditEngine::GetBaseParaAttribs(sal_Int32 nPara) const
{
    return pImpEditEngine->GetParaAttribs(nPara);
}

void EditEngine::SetParaAttribsOnly(sal_Int32 nPara, const SfxItemSet& rSet)
{
    pImpEditEngine->SetParaAttribs(nPara, rSet);
}

void EditEngine::SetAttribs(const EditSelection& rSel, const SfxItemSet& rSet, SetAttribsMode nSpecial)
{
    pImpEditEngine->SetAttribs(rSel, rSet, nSpecial);
}

OUString EditEngine::GetSelected(const EditSelection& rSel) const
{
    return pImpEditEngine->GetSelected(rSel);
}

EditPaM EditEngine::DeleteSelected(const EditSelection& rSel)
{
    return pImpEditEngine->DeleteSelected(rSel);
}

void EditEngine::HandleBeginPasteOrDrop(PasteOrDropInfos& rInfos)
{
    pImpEditEngine->aBeginPasteOrDropHdl.Call(rInfos);
}

void EditEngine::HandleEndPasteOrDrop(PasteOrDropInfos& rInfos)
{
    pImpEditEngine->aEndPasteOrDropHdl.Call(rInfos);
}

bool EditEngine::HasText() const
{
    return pImpEditEngine->ImplHasText();
}

const EditSelectionEngine& EditEngine::GetSelectionEngine() const
{
    return pImpEditEngine->aSelEngine;
}

void EditEngine::SetInSelectionMode(bool b)
{
    pImpEditEngine->bInSelection = b;
}

bool EditEngine::PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pEditView, vcl::Window const * pFrameWin )
{
    DBG_ASSERT( pEditView, "no View - no cookie !" );

    bool bDone = true;

    bool bModified  = false;
    bool bMoved     = false;
    bool bAllowIdle = true;
    bool bReadOnly  = pEditView->IsReadOnly();

    GetCursorFlags nNewCursorFlags = GetCursorFlags::NONE;
    bool bSetCursorFlags = true;

    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    DBG_ASSERT( !aCurSel.IsInvalid(), "Blinde Selection in EditEngine::PostKeyEvent" );

    OUString aAutoText( pImpEditEngine->GetAutoCompleteText() );
    if (!pImpEditEngine->GetAutoCompleteText().isEmpty())
        pImpEditEngine->SetAutoCompleteText(OUString(), true);

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
                    long n20 = 40 * pImpEditEngine->nOnePixelInRef;
                    for ( sal_Int32 n = 0; n < nParas; n++ )
                    {
                        long nH = GetTextHeight( n );
                        Point P1( aViewStart.X() + n20 + n20*(n%2), aViewStart.Y() + aPos.Y() );
                        Point P2( P1 );
                        P2.AdjustX(n20 );
                        P2.AdjustY(nH );
                        pEditView->GetWindow()->SetLineColor();
                        pEditView->GetWindow()->SetFillColor( (n%2) ? COL_YELLOW : COL_LIGHTGREEN );
                        pEditView->GetWindow()->DrawRect( tools::Rectangle( P1, P2 ) );
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
                                                                  OStringToOUString(aInfo.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US)));
                    xInfoBox->run();

                }
                bDone = false;
            }
            break;
            case KEY_F12:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    EditDbg::ShowEditEngineData( this );
                }
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
                    if ( pImpEditEngine->DoVisualCursorTraveling() && ( ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) /* || ( nCode == KEY_HOME ) || ( nCode == KEY_END ) */ ) )
                        bSetCursorFlags = false;    // Will be manipulated within visual cursor move

                    aCurSel = pImpEditEngine->MoveCursor( rKeyEvent, pEditView );

                    if ( aCurSel.HasRange() ) {
                        Reference<css::datatransfer::clipboard::XClipboard> aSelection(pEditView->GetWindow()->GetPrimarySelection());
                        pEditView->pImpEditView->CutCopy( aSelection, false );
                    }

                    bMoved = true;
                    if ( nCode == KEY_HOME )
                        nNewCursorFlags |= GetCursorFlags::StartOfLine;
                    else if ( nCode == KEY_END )
                        nNewCursorFlags |= GetCursorFlags::EndOfLine;

                }
#if OSL_DEBUG_LEVEL > 1
                GetLanguage( pImpEditEngine->GetEditDoc().GetPos( aCurSel.Max().GetNode() ), aCurSel.Max().GetIndex() );
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
                    const SvxNumberFormat *pFmt = pImpEditEngine->GetNumberFormat( pNode );
                    if (pFmt && nCode == KEY_BACKSPACE &&
                        !aCurSel.HasRange() && aCurSel.Min().GetIndex() == 0)
                    {
                        // if the bullet is still visible, just make it invisible.
                        // Otherwise continue as usual.


                        sal_Int32 nPara = pImpEditEngine->GetEditDoc().GetPos( pNode );
                        SfxBoolItem aBulletState( pImpEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE ) );

                        if ( aBulletState.GetValue() )
                        {

                            aBulletState.SetValue( false );
                            SfxItemSet aSet( pImpEditEngine->GetParaAttribs( nPara ) );
                            aSet.Put( aBulletState );
                            pImpEditEngine->SetParaAttribs( nPara, aSet );

                            // have this and the following paragraphs formatted and repainted.
                            // (not painting a numbering in the list may cause the following
                            // numberings to have different numbers than before and thus the
                            // length may have changed as well )
                            pImpEditEngine->FormatAndUpdate( pImpEditEngine->GetActiveView() );

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

                    pEditView->pImpEditView->DrawSelectionXOR();
                    pImpEditEngine->UndoActionStart( EDITUNDO_DELETE );
                    aCurSel = pImpEditEngine->DeleteLeftOrRight( aCurSel, nDel, nMode );
                    pImpEditEngine->UndoActionEnd();
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
                            pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
                        if ( pImpEditEngine->GetStatus().DoAutoCorrect() )
                            aCurSel = pImpEditEngine->AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                        aCurSel = pImpEditEngine->InsertTab( aCurSel );
                        if ( bSel )
                            pImpEditEngine->UndoActionEnd();
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
                    pEditView->pImpEditView->DrawSelectionXOR();
                    if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    {
                        pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
                        if ( rKeyEvent.GetKeyCode().IsShift() )
                        {
                            aCurSel = pImpEditEngine->AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                            aCurSel = pImpEditEngine->InsertLineBreak( aCurSel );
                        }
                        else
                        {
                            if (aAutoText.isEmpty())
                            {
                                if ( pImpEditEngine->GetStatus().DoAutoCorrect() )
                                    aCurSel = pImpEditEngine->AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                                aCurSel = pImpEditEngine->InsertParaBreak( aCurSel );
                            }
                            else
                            {
                                DBG_ASSERT( !aCurSel.HasRange(), "Selection on complete?!" );
                                EditPaM aStart( pImpEditEngine->WordLeft( aCurSel.Max() ) );
                                aCurSel = pImpEditEngine->InsertText(
                                                EditSelection( aStart, aCurSel.Max() ), aAutoText );
                                pImpEditEngine->SetAutoCompleteText( OUString(), true );
                            }
                        }
                        pImpEditEngine->UndoActionEnd();
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
                    pEditView->pImpEditView->DrawSelectionXOR();
                    // Autocorrection?
                    if ( ( pImpEditEngine->GetStatus().DoAutoCorrect() ) &&
                        ( SvxAutoCorrect::IsAutoCorrectChar( nCharCode ) ||
                          pImpEditEngine->IsNbspRunNext() ) )
                    {
                        aCurSel = pImpEditEngine->AutoCorrect(
                            aCurSel, nCharCode, !pEditView->IsInsertMode(), pFrameWin );
                    }
                    else
                    {
                        aCurSel = pImpEditEngine->InsertTextUserInput( aCurSel, nCharCode, !pEditView->IsInsertMode() );
                    }
                    // AutoComplete ???
                    if ( pImpEditEngine->GetStatus().DoAutoComplete() && ( nCharCode != ' ' ) )
                    {
                        // Only at end of word...
                        sal_Int32 nIndex = aCurSel.Max().GetIndex();
                        if ( ( nIndex >= aCurSel.Max().GetNode()->Len() ) ||
                             ( pImpEditEngine->aWordDelimiters.indexOf( aCurSel.Max().GetNode()->GetChar( nIndex ) ) != -1 ) )
                        {
                            EditPaM aStart( pImpEditEngine->WordLeft( aCurSel.Max() ) );
                            OUString aWord = pImpEditEngine->GetSelected( EditSelection( aStart, aCurSel.Max() ) );
                            if ( aWord.getLength() >= 3 )
                            {
                                OUString aComplete;

                                LanguageType eLang = pImpEditEngine->GetLanguage( EditPaM( aStart.GetNode(), aStart.GetIndex()+1));
                                LanguageTag aLanguageTag( eLang);

                                if (!pImpEditEngine->xLocaleDataWrapper.isInitialized())
                                    pImpEditEngine->xLocaleDataWrapper.init( SvtSysLocale().GetLocaleData().getComponentContext(), aLanguageTag);
                                else
                                    pImpEditEngine->xLocaleDataWrapper.changeLocale( aLanguageTag);

                                if (!pImpEditEngine->xTransliterationWrapper.isInitialized())
                                    pImpEditEngine->xTransliterationWrapper.init( SvtSysLocale().GetLocaleData().getComponentContext(), eLang);
                                else
                                    pImpEditEngine->xTransliterationWrapper.changeLocale( eLang);

                                const ::utl::TransliterationWrapper* pTransliteration = pImpEditEngine->xTransliterationWrapper.get();
                                Sequence< i18n::CalendarItem2 > xItem = pImpEditEngine->xLocaleDataWrapper->getDefaultCalendarDays();
                                sal_Int32 nCount = xItem.getLength();
                                const i18n::CalendarItem2* pArr = xItem.getArray();
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
                                    xItem = pImpEditEngine->xLocaleDataWrapper->getDefaultCalendarMonths();
                                    sal_Int32 nMonthCount = xItem.getLength();
                                    const i18n::CalendarItem2* pMonthArr = xItem.getArray();
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
                                    pImpEditEngine->SetAutoCompleteText( aComplete, false );
                                    Point aPos = pImpEditEngine->PaMtoEditCursor( aCurSel.Max() ).TopLeft();
                                    aPos = pEditView->pImpEditView->GetWindowPos( aPos );
                                    aPos = pEditView->pImpEditView->GetWindow()->LogicToPixel( aPos );
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

    pEditView->pImpEditView->SetEditSelection( aCurSel );
    if (comphelper::LibreOfficeKit::isActive())
    {
        pEditView->pImpEditView->DrawSelectionXOR();
    }
    pImpEditEngine->UpdateSelections();

    if ( ( !IsVertical() && ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) ) ||
         ( IsVertical() && ( nCode != KEY_LEFT ) && ( nCode != KEY_RIGHT ) ))
    {
        pEditView->pImpEditView->nTravelXPos = TRAVEL_X_DONTKNOW;
    }

    if ( /* ( nCode != KEY_HOME ) && ( nCode != KEY_END ) && */
        ( !IsVertical() && ( nCode != KEY_LEFT ) && ( nCode != KEY_RIGHT ) ) ||
         ( IsVertical() && ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) ))
    {
        pEditView->pImpEditView->SetCursorBidiLevel( CURSOR_BIDILEVEL_DONTKNOW );
    }

    if ( bSetCursorFlags )
        pEditView->pImpEditView->nExtraCursorFlags = nNewCursorFlags;

    if ( bModified )
    {
        DBG_ASSERT( !bReadOnly, "ReadOnly but modified???" );
        // Idle-Formatter only when AnyInput.
        if ( bAllowIdle && pImpEditEngine->GetStatus().UseIdleFormatter()
                && Application::AnyInput( VclInputFlags::KEYBOARD) )
            pImpEditEngine->IdleFormatAndUpdate( pEditView );
        else
            pImpEditEngine->FormatAndUpdate( pEditView );
    }
    else if ( bMoved )
    {
        bool bGotoCursor = pEditView->pImpEditView->DoAutoScroll();
        pEditView->pImpEditView->ShowCursor( bGotoCursor, true );
        pImpEditEngine->CallStatusHdl();
    }

    return bDone;
}

sal_uInt32 EditEngine::GetTextHeight() const
{

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nHeight = !IsVertical() ? pImpEditEngine->GetTextHeight() : pImpEditEngine->CalcTextWidth( true );
    return nHeight;
}

sal_uInt32 EditEngine::GetTextHeightNTP() const
{

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    if ( IsVertical() )
        return pImpEditEngine->CalcTextWidth( true );

    return pImpEditEngine->GetTextHeightNTP();
}

sal_uInt32 EditEngine::CalcTextWidth()
{

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nWidth = !IsVertical() ? pImpEditEngine->CalcTextWidth( true ) : pImpEditEngine->GetTextHeight();
    return nWidth;
}

void EditEngine::SetUpdateMode( bool bUpdate )
{
    pImpEditEngine->SetUpdateMode( bUpdate );
    if ( pImpEditEngine->pActiveView )
        pImpEditEngine->pActiveView->ShowCursor( false, false, /*bActivate=*/true );
}

bool EditEngine::GetUpdateMode() const
{
    return pImpEditEngine->GetUpdateMode();
}

void EditEngine::Clear()
{
    pImpEditEngine->Clear();
}

void EditEngine::SetText( const OUString& rText )
{
    pImpEditEngine->SetText( rText );
    if ( !rText.isEmpty() )
        pImpEditEngine->FormatAndUpdate();
}

ErrCode EditEngine::Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs /* = NULL */ )
{
    bool bUndoEnabled = pImpEditEngine->IsUndoEnabled();
    pImpEditEngine->EnableUndo( false );
    pImpEditEngine->SetText( OUString() );
    EditPaM aPaM( pImpEditEngine->GetEditDoc().GetStartPaM() );
    pImpEditEngine->Read( rInput, rBaseURL, eFormat, EditSelection( aPaM, aPaM ), pHTTPHeaderAttrs );
    pImpEditEngine->EnableUndo( bUndoEnabled );
    return rInput.GetError();
}

void EditEngine::Write( SvStream& rOutput, EETextFormat eFormat )
{
    EditPaM aStartPaM( pImpEditEngine->GetEditDoc().GetStartPaM() );
    EditPaM aEndPaM( pImpEditEngine->GetEditDoc().GetEndPaM() );
    pImpEditEngine->Write( rOutput, eFormat, EditSelection( aStartPaM, aEndPaM ) );
}

std::unique_ptr<EditTextObject> EditEngine::CreateTextObject()
{
    return pImpEditEngine->CreateTextObject();
}

std::unique_ptr<EditTextObject> EditEngine::CreateTextObject( const ESelection& rESelection )
{
    EditSelection aSel( pImpEditEngine->CreateSel( rESelection ) );
    return pImpEditEngine->CreateTextObject( aSel );
}

std::unique_ptr<EditTextObject> EditEngine::GetEmptyTextObject() const
{
    return pImpEditEngine->GetEmptyTextObject();
}


void EditEngine::SetText( const EditTextObject& rTextObject )
{
    pImpEditEngine->SetText( rTextObject );
    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::ShowParagraph( sal_Int32 nParagraph, bool bShow )
{
    pImpEditEngine->ShowParagraph( nParagraph, bShow );
}

void EditEngine::SetNotifyHdl( const Link<EENotify&,void>& rLink )
{
    pImpEditEngine->SetNotifyHdl( rLink );
}

Link<EENotify&,void> const & EditEngine::GetNotifyHdl() const
{
    return pImpEditEngine->GetNotifyHdl();
}

void EditEngine::SetStatusEventHdl( const Link<EditStatus&, void>& rLink )
{
    pImpEditEngine->SetStatusEventHdl( rLink );
}

Link<EditStatus&, void> const & EditEngine::GetStatusEventHdl() const
{
    return pImpEditEngine->GetStatusEventHdl();
}

void EditEngine::SetHtmlImportHdl( const Link<HtmlImportInfo&,void>& rLink )
{
    pImpEditEngine->aHtmlImportHdl = rLink;
}

const Link<HtmlImportInfo&,void>& EditEngine::GetHtmlImportHdl() const
{
    return pImpEditEngine->aHtmlImportHdl;
}

void EditEngine::SetRtfImportHdl( const Link<RtfImportInfo&,void>& rLink )
{
    pImpEditEngine->aRtfImportHdl = rLink;
}

const Link<RtfImportInfo&,void>& EditEngine::GetRtfImportHdl() const
{
    return pImpEditEngine->aRtfImportHdl;
}

void EditEngine::SetBeginMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink )
{
    pImpEditEngine->aBeginMovingParagraphsHdl = rLink;
}

void EditEngine::SetEndMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink )
{
    pImpEditEngine->aEndMovingParagraphsHdl = rLink;
}

void EditEngine::SetBeginPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink )
{

    pImpEditEngine->aBeginPasteOrDropHdl = rLink;
}

void EditEngine::SetEndPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink )
{
    pImpEditEngine->aEndPasteOrDropHdl = rLink;
}

std::unique_ptr<EditTextObject> EditEngine::CreateTextObject( sal_Int32 nPara, sal_Int32 nParas )
{
    DBG_ASSERT( 0 <= nPara && nPara < pImpEditEngine->GetEditDoc().Count(), "CreateTextObject: Startpara out of Range" );
    DBG_ASSERT( nParas <= pImpEditEngine->GetEditDoc().Count() - nPara, "CreateTextObject: Endpara out of Range" );

    ContentNode* pStartNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    ContentNode* pEndNode = pImpEditEngine->GetEditDoc().GetObject( nPara+nParas-1 );
    DBG_ASSERT( pStartNode, "Start-Paragraph does not exist: CreateTextObject" );
    DBG_ASSERT( pEndNode, "End-Paragraph does not exist: CreateTextObject" );

    if ( pStartNode && pEndNode )
    {
        EditSelection aTmpSel;
        aTmpSel.Min() = EditPaM( pStartNode, 0 );
        aTmpSel.Max() = EditPaM( pEndNode, pEndNode->Len() );
        return pImpEditEngine->CreateTextObject( aTmpSel );
    }
    return nullptr;
}

void EditEngine::RemoveParagraph( sal_Int32 nPara )
{
    DBG_ASSERT( pImpEditEngine->GetEditDoc().Count() > 1, "The first paragraph should not be deleted!" );
    if( pImpEditEngine->GetEditDoc().Count() <= 1 )
        return;

    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    const ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nPara );
    DBG_ASSERT( pPortion && pNode, "Paragraph not found: RemoveParagraph" );
    if ( pNode && pPortion )
    {
        // No Undo encapsulation needed.
        pImpEditEngine->ImpRemoveParagraph( nPara );
        pImpEditEngine->InvalidateFromParagraph( nPara );
        pImpEditEngine->UpdateSelections();
        pImpEditEngine->FormatAndUpdate();
    }
}

sal_Int32 EditEngine::GetTextLen( sal_Int32 nPara ) const
{
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    DBG_ASSERT( pNode, "Paragraph not found: GetTextLen" );
    if ( pNode )
        return pNode->Len();
    return 0;
}

OUString EditEngine::GetText( sal_Int32 nPara ) const
{
    OUString aStr;
    if ( 0 <= nPara && nPara < pImpEditEngine->GetEditDoc().Count() )
        aStr = pImpEditEngine->GetEditDoc().GetParaAsString( nPara );
    return aStr;
}

void EditEngine::SetModifyHdl( const Link<LinkParamNone*,void>& rLink )
{
    pImpEditEngine->SetModifyHdl( rLink );
}

Link<LinkParamNone*,void> const & EditEngine::GetModifyHdl() const
{
    return pImpEditEngine->GetModifyHdl();
}


void EditEngine::ClearModifyFlag()
{
    pImpEditEngine->SetModifyFlag( false );
}

void EditEngine::SetModified()
{
    pImpEditEngine->SetModifyFlag( true );
}

bool EditEngine::IsModified() const
{
    return pImpEditEngine->IsModified();
}

bool EditEngine::IsInSelectionMode() const
{
    return ( pImpEditEngine->IsInSelectionMode() ||
                pImpEditEngine->GetSelEngine().IsInSelection() );
}

void EditEngine::InsertParagraph( sal_Int32 nPara, const EditTextObject& rTxtObj )
{
    if ( nPara > GetParagraphCount() )
    {
        SAL_WARN_IF( nPara != EE_PARA_APPEND, "editeng", "Paragraph number too large, but not EE_PARA_APPEND!" );
        nPara = GetParagraphCount();
    }

    pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );

    // No Undo compounding needed.
    EditPaM aPaM( pImpEditEngine->InsertParagraph( nPara ) );
    // When InsertParagraph from the outside, no hard attributes
    // should be taken over!
    pImpEditEngine->RemoveCharAttribs( nPara );
    pImpEditEngine->InsertText( rTxtObj, EditSelection( aPaM, aPaM ) );

    pImpEditEngine->UndoActionEnd();

    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::InsertParagraph(sal_Int32 nPara, const OUString& rTxt)
{
    if ( nPara > GetParagraphCount() )
    {
        SAL_WARN_IF( nPara != EE_PARA_APPEND, "editeng", "Paragraph number too large, but not EE_PARA_APPEND!" );
        nPara = GetParagraphCount();
    }

    pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM( pImpEditEngine->InsertParagraph( nPara ) );
    // When InsertParagraph from the outside, no hard attributes
    // should be taken over!
    pImpEditEngine->RemoveCharAttribs( nPara );
    pImpEditEngine->UndoActionEnd();
    pImpEditEngine->ImpInsertText( EditSelection( aPaM, aPaM ), rTxt );
    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::SetText(sal_Int32 nPara, const OUString& rTxt)
{
    std::unique_ptr<EditSelection> pSel = pImpEditEngine->SelectParagraph( nPara );
    if ( pSel )
    {
        pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
        pImpEditEngine->ImpInsertText( *pSel, rTxt );
        pImpEditEngine->UndoActionEnd();
        pImpEditEngine->FormatAndUpdate();
    }
}

void EditEngine::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    pImpEditEngine->SetParaAttribs( nPara, rSet );
    pImpEditEngine->FormatAndUpdate();
}

const SfxItemSet& EditEngine::GetParaAttribs( sal_Int32 nPara ) const
{
    return pImpEditEngine->GetParaAttribs( nPara );
}

bool EditEngine::HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    return pImpEditEngine->HasParaAttrib( nPara, nWhich );
}

const SfxPoolItem& EditEngine::GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich )
{
    return pImpEditEngine->GetParaAttrib( nPara, nWhich );
}

void EditEngine::SetCharAttribs(sal_Int32 nPara, const SfxItemSet& rSet)
{
    EditSelection aSel(pImpEditEngine->ConvertSelection(nPara, 0, nPara, GetTextLen(nPara)));
    pImpEditEngine->SetAttribs(aSel, rSet);
    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    pImpEditEngine->GetCharAttribs( nPara, rLst );
}

SfxItemSet EditEngine::GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib )
{
    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
    return pImpEditEngine->GetAttribs( aSel, nOnlyHardAttrib );
}

SfxItemSet EditEngine::GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags ) const
{
    return pImpEditEngine->GetAttribs( nPara, nStart, nEnd, nFlags );
}

void EditEngine::RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich )
{

    pImpEditEngine->UndoActionStart( EDITUNDO_RESETATTRIBS );
    EditSelection aSel( pImpEditEngine->ConvertSelection( rSelection.nStartPara, rSelection.nStartPos, rSelection.nEndPara, rSelection.nEndPos ) );
    pImpEditEngine->RemoveCharAttribs( aSel, bRemoveParaAttribs, nWhich  );
    pImpEditEngine->UndoActionEnd();
    pImpEditEngine->FormatAndUpdate();
}

vcl::Font EditEngine::GetStandardFont( sal_Int32 nPara )
{
    return GetStandardSvxFont( nPara );
}

SvxFont EditEngine::GetStandardSvxFont( sal_Int32 nPara )
{
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    return pNode->GetCharAttribs().GetDefFont();
}

void EditEngine::StripPortions()
{
    ScopedVclPtrInstance< VirtualDevice > aTmpDev;
    tools::Rectangle aBigRect( Point( 0, 0 ), Size( 0x7FFFFFFF, 0x7FFFFFFF ) );
    if ( IsVertical() )
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
    pImpEditEngine->Paint( aTmpDev.get(), aBigRect, Point(), true );
}

void EditEngine::GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList )
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatFullDoc();

    const ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nPara );
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
    pImpEditEngine->SetFlatMode( bFlat );
}

bool EditEngine::IsFlatMode() const
{
    return !( pImpEditEngine->aStatus.UseCharAttribs() );
}

void EditEngine::SetControlWord( EEControlBits nWord )
{

    if ( nWord != pImpEditEngine->aStatus.GetControlWord() )
    {
        EEControlBits nPrev = pImpEditEngine->aStatus.GetControlWord();
        pImpEditEngine->aStatus.GetControlWord() = nWord;

        EEControlBits nChanges = nPrev ^ nWord;
        if ( pImpEditEngine->IsFormatted() )
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
                    pImpEditEngine->GetEditDoc().CreateDefFont( true );
                }

                pImpEditEngine->FormatFullDoc();
                pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );
            }
        }

        bool bSpellingChanged = bool(nChanges & EEControlBits::ONLINESPELLING);

        if ( bSpellingChanged )
        {
            pImpEditEngine->StopOnlineSpellTimer();
            if (nWord & EEControlBits::ONLINESPELLING)
            {
                // Create WrongList, start timer...
                sal_Int32 nNodes = pImpEditEngine->GetEditDoc().Count();
                for ( sal_Int32 n = 0; n < nNodes; n++ )
                {
                    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( n );
                    pNode->CreateWrongList();
                }
                if (pImpEditEngine->IsFormatted())
                    pImpEditEngine->StartOnlineSpellTimer();
            }
            else
            {
                long nY = 0;
                sal_Int32 nNodes = pImpEditEngine->GetEditDoc().Count();
                for ( sal_Int32 n = 0; n < nNodes; n++ )
                {
                    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( n );
                    const ParaPortion* pPortion = pImpEditEngine->GetParaPortions()[n];
                    bool bWrongs = false;
                    if (pNode->GetWrongList() != nullptr)
                        bWrongs = !pNode->GetWrongList()->empty();
                    pNode->DestroyWrongList();
                    if ( bWrongs )
                    {
                        pImpEditEngine->aInvalidRect.SetLeft( 0 );
                        pImpEditEngine->aInvalidRect.SetRight( pImpEditEngine->GetPaperSize().Width() );
                        pImpEditEngine->aInvalidRect.SetTop( nY+1 );
                        pImpEditEngine->aInvalidRect.SetBottom( nY+pPortion->GetHeight()-1 );
                        pImpEditEngine->UpdateViews( pImpEditEngine->pActiveView );
                    }
                    nY += pPortion->GetHeight();
                }
            }
        }
    }
}

EEControlBits EditEngine::GetControlWord() const
{
    return pImpEditEngine->aStatus.GetControlWord();
}

long EditEngine::GetFirstLineStartX( sal_Int32 nParagraph )
{

    long nX = 0;
    const ParaPortion* pPPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nParagraph );
    if ( pPPortion )
    {
        DBG_ASSERT( pImpEditEngine->IsFormatted() || !pImpEditEngine->IsFormatting(), "GetFirstLineStartX: Doc not formatted - unable to format!" );
        if ( !pImpEditEngine->IsFormatted() )
            pImpEditEngine->FormatDoc();
        const EditLine& rFirstLine = pPPortion->GetLines()[0];
        nX = rFirstLine.GetStartPosX();
    }
    return nX;
}

Point EditEngine::GetDocPos( const Point& rPaperPos ) const
{
    Point aDocPos( rPaperPos );
    if ( IsVertical() )
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
    const ParaPortion* pPPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Paragraph not found: GetWindowPosTopLeft" );
    Point aPoint;
    if ( pPPortion )
    {

        // If someone calls GetLineHeight() with an empty Engine.
        DBG_ASSERT( pImpEditEngine->IsFormatted() || !pImpEditEngine->IsFormatting(), "GetDocPosTopLeft: Doc not formatted - unable to format!" );
        if ( !pImpEditEngine->IsFormatted() )
            pImpEditEngine->FormatAndUpdate();
        if ( pPPortion->GetLines().Count() )
        {
            // Correct it if large Bullet.
            const EditLine& rFirstLine = pPPortion->GetLines()[0];
            aPoint.setX( rFirstLine.GetStartPosX() );
        }
        else
        {
            const SvxLRSpaceItem& rLRItem = pImpEditEngine->GetLRSpaceItem( pPPortion->GetNode() );
// TL_NF_LR         aPoint.X() = pImpEditEngine->GetXValue( (short)(rLRItem.GetTextLeft() + rLRItem.GetTextFirstLineOfst()) );
            sal_Int32 nSpaceBefore = 0;
            pImpEditEngine->GetSpaceBeforeAndMinLabelWidth( pPPortion->GetNode(), &nSpaceBefore );
            short nX = static_cast<short>(rLRItem.GetTextLeft()
                            + rLRItem.GetTextFirstLineOfst()
                            + nSpaceBefore);
            aPoint.setX( pImpEditEngine->GetXValue( nX
                             ) );
        }
        aPoint.setY( pImpEditEngine->GetParaPortions().GetYOffset( pPPortion ) );
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
    return pImpEditEngine->IsRightToLeft( nPara );
}

bool EditEngine::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder )
{

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    bool bTextPos = false;
    // take unrotated positions for calculation here
    Point aDocPos = GetDocPos( rPaperPos );

    if ( ( aDocPos.Y() > 0  ) && ( aDocPos.Y() < static_cast<long>(pImpEditEngine->GetTextHeight()) ) )
    {
        EditPaM aPaM = pImpEditEngine->GetPaM( aDocPos, false );
        if ( aPaM.GetNode() )
        {
            const ParaPortion* pParaPortion = pImpEditEngine->FindParaPortion( aPaM.GetNode() );
            DBG_ASSERT( pParaPortion, "ParaPortion?" );

            sal_Int32 nLine = pParaPortion->GetLineNumber( aPaM.GetIndex() );
            const EditLine& rLine = pParaPortion->GetLines()[nLine];
            Range aLineXPosStartEnd = pImpEditEngine->GetLineXPosStartEnd( pParaPortion, &rLine );
            if ( ( aDocPos.X() >= aLineXPosStartEnd.Min() - nBorder ) &&
                 ( aDocPos.X() <= aLineXPosStartEnd.Max() + nBorder ) )
            {
                 bTextPos = true;
            }
        }
    }
    return bTextPos;
}

void EditEngine::SetEditTextObjectPool( SfxItemPool* pPool )
{
    pImpEditEngine->SetEditTextObjectPool( pPool );
}

SfxItemPool* EditEngine::GetEditTextObjectPool() const
{
    return pImpEditEngine->GetEditTextObjectPool();
}

void EditEngine::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->SetAttribs( aSel, rSet );
}

void EditEngine::QuickMarkInvalid( const ESelection& rSel )
{
    DBG_ASSERT( rSel.nStartPara < pImpEditEngine->GetEditDoc().Count(), "MarkInvalid: Start out of Range!" );
    DBG_ASSERT( rSel.nEndPara < pImpEditEngine->GetEditDoc().Count(), "MarkInvalid: End out of Range!" );
    for ( sal_Int32 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nPara );
        if ( pPortion )
            pPortion->MarkSelectionInvalid( 0 );
    }
}

void EditEngine::QuickInsertText(const OUString& rText, const ESelection& rSel)
{

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpInsertText( aSel, rText );
}

void EditEngine::QuickDelete( const ESelection& rSel )
{

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpDeleteSelection( aSel );
}

void EditEngine::QuickMarkToBeRepainted( sal_Int32 nPara )
{
    ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nPara );
    if ( pPortion )
        pPortion->SetMustRepaint( true );
}

void EditEngine::QuickInsertLineBreak( const ESelection& rSel )
{

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->InsertLineBreak( aSel );
}

void EditEngine::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpInsertFeature( aSel, rFld );
}

void EditEngine::QuickFormatDoc( bool bFull )
{
    if ( bFull )
        pImpEditEngine->FormatFullDoc();
    else
        pImpEditEngine->FormatDoc();

    // Don't pass active view, maybe selection is not updated yet...
    pImpEditEngine->UpdateViews();
}

void EditEngine::SetStyleSheet(const EditSelection& aSel, SfxStyleSheet* pStyle)
{
    pImpEditEngine->SetStyleSheet(aSel, pStyle);
}

void EditEngine::SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle )
{
    pImpEditEngine->SetStyleSheet( nPara, pStyle );
}

const SfxStyleSheet* EditEngine::GetStyleSheet( sal_Int32 nPara ) const
{
    return pImpEditEngine->GetStyleSheet( nPara );
}

SfxStyleSheet* EditEngine::GetStyleSheet( sal_Int32 nPara )
{
    return pImpEditEngine->GetStyleSheet( nPara );
}

void EditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    pImpEditEngine->SetStyleSheetPool( pSPool );
}

SfxStyleSheetPool* EditEngine::GetStyleSheetPool()
{
    return pImpEditEngine->GetStyleSheetPool();
}

void EditEngine::SetWordDelimiters( const OUString& rDelimiters )
{
    pImpEditEngine->aWordDelimiters = rDelimiters;
    if (pImpEditEngine->aWordDelimiters.indexOf(CH_FEATURE) == -1)
        pImpEditEngine->aWordDelimiters += OUStringLiteral1(CH_FEATURE);
}

const OUString& EditEngine::GetWordDelimiters() const
{
    return pImpEditEngine->aWordDelimiters;
}

void EditEngine::EraseVirtualDevice()
{
    pImpEditEngine->EraseVirtualDevice();
}

void EditEngine::SetSpeller( Reference< XSpellChecker1 > const &xSpeller )
{
    pImpEditEngine->SetSpeller( xSpeller );
}

Reference< XSpellChecker1 > const & EditEngine::GetSpeller()
{
    return pImpEditEngine->GetSpeller();
}

void EditEngine::SetHyphenator( Reference< XHyphenator > const & xHyph )
{
    pImpEditEngine->SetHyphenator( xHyph );
}

void EditEngine::GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const
{
    pImpEditEngine->GetAllMisspellRanges(rRanges);
}

void EditEngine::SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges )
{
    pImpEditEngine->SetAllMisspellRanges(rRanges);
}

void EditEngine::SetForbiddenCharsTable(const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars)
{
    ImpEditEngine::SetForbiddenCharsTable( xForbiddenChars );
}

void EditEngine::SetDefaultLanguage( LanguageType eLang )
{
    pImpEditEngine->SetDefaultLanguage( eLang );
}

LanguageType EditEngine::GetDefaultLanguage() const
{
    return pImpEditEngine->GetDefaultLanguage();
}

bool EditEngine::SpellNextDocument()
{
    return false;
}

EESpellState EditEngine::HasSpellErrors()
{
    if ( !pImpEditEngine->GetSpeller().is()  )
        return EESpellState::NoSpeller;

    return pImpEditEngine->HasSpellErrors();
}

void EditEngine::ClearSpellErrors()
{
    pImpEditEngine->ClearSpellErrors();
}

bool EditEngine::SpellSentence(EditView const & rView, svx::SpellPortions& rToFill )
{
    return pImpEditEngine->SpellSentence( rView, rToFill );
}

void EditEngine::PutSpellingToSentenceStart( EditView const & rEditView )
{
    pImpEditEngine->PutSpellingToSentenceStart( rEditView );
}

void EditEngine::ApplyChangedSentence(EditView const & rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck )
{
    pImpEditEngine->ApplyChangedSentence( rEditView, rNewPortions, bRecheck  );
}

bool EditEngine::HasConvertibleTextPortion( LanguageType nLang )
{
    return pImpEditEngine->HasConvertibleTextPortion( nLang );
}

bool EditEngine::ConvertNextDocument()
{
    return false;
}

bool EditEngine::HasText( const SvxSearchItem& rSearchItem )
{
    return pImpEditEngine->HasText( rSearchItem );
}

void EditEngine::SetGlobalCharStretching( sal_uInt16 nX, sal_uInt16 nY )
{
    pImpEditEngine->SetCharStretching( nX, nY );
}

void EditEngine::GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const
{
    pImpEditEngine->GetCharStretching( rX, rY );
}

bool EditEngine::ShouldCreateBigTextObject() const
{
    sal_Int32 nTextPortions = 0;
    sal_Int32 nParas = pImpEditEngine->GetEditDoc().Count();
    for ( sal_Int32 nPara = 0; nPara < nParas; nPara++  )
    {
        ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions()[nPara];
        nTextPortions = nTextPortions + pParaPortion->GetTextPortions().Count();
    }
    return nTextPortions >= pImpEditEngine->GetBigTextObjectStart();
}

sal_uInt16 EditEngine::GetFieldCount( sal_Int32 nPara ) const
{
    sal_uInt16 nFields = 0;
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
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
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
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
    bool bChanges = pImpEditEngine->UpdateFields();
    if ( bChanges )
        pImpEditEngine->FormatAndUpdate();
    return bChanges;
}

bool EditEngine::UpdateFieldsOnly()
{
    return pImpEditEngine->UpdateFields();
}

void EditEngine::RemoveFields( const std::function<bool ( const SvxFieldData* )>& isFieldData )
{
    pImpEditEngine->UpdateFields();

    sal_Int32 nParas = pImpEditEngine->GetEditDoc().Count();
    for ( sal_Int32 nPara = 0; nPara < nParas; nPara++  )
    {
        ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for (size_t nAttr = rAttrs.size(); nAttr; )
        {
            const EditCharAttrib& rAttr = *rAttrs[--nAttr].get();
            if (rAttr.Which() == EE_FEATURE_FIELD)
            {
                const SvxFieldData* pFldData = static_cast<const SvxFieldItem*>(rAttr.GetItem())->GetField();
                if ( pFldData && ( isFieldData( pFldData )  ) )
                {
                    DBG_ASSERT( dynamic_cast<const SvxFieldItem*>(rAttr.GetItem()), "no field item..." );
                    EditSelection aSel( EditPaM(pNode, rAttr.GetStart()), EditPaM(pNode, rAttr.GetEnd()) );
                    OUString aFieldText = static_cast<const EditCharAttribField&>(rAttr).GetFieldValue();
                    pImpEditEngine->ImpInsertText( aSel, aFieldText );
                }
            }
        }
    }
}

bool EditEngine::HasOnlineSpellErrors() const
{
    sal_Int32 nNodes = pImpEditEngine->GetEditDoc().Count();
    for ( sal_Int32 n = 0; n < nNodes; n++ )
    {
        ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( n );
        if ( pNode->GetWrongList() && !pNode->GetWrongList()->empty() )
            return true;
    }
    return false;
}

void EditEngine::CompleteOnlineSpelling()
{
    if ( pImpEditEngine->GetStatus().DoOnlineSpelling() )
    {
        if( !pImpEditEngine->IsFormatted() )
            pImpEditEngine->FormatAndUpdate();

        pImpEditEngine->StopOnlineSpellTimer();
        pImpEditEngine->DoOnlineSpelling( nullptr, true, false );
    }
}

sal_Int32 EditEngine::FindParagraph( long nDocPosY )
{
    return pImpEditEngine->GetParaPortions().FindParagraph( nDocPosY );
}

EPosition EditEngine::FindDocPosition( const Point& rDocPos ) const
{
    EPosition aPos;
    // From the point of the API, this is const....
    EditPaM aPaM = const_cast<EditEngine*>(this)->pImpEditEngine->GetPaM( rDocPos, false );
    if ( aPaM.GetNode() )
    {
        aPos.nPara = pImpEditEngine->aEditDoc.GetPos( aPaM.GetNode() );
        aPos.nIndex = aPaM.GetIndex();
    }
    return aPos;
}

tools::Rectangle EditEngine::GetCharacterBounds( const EPosition& rPos ) const
{
    tools::Rectangle aBounds;
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( rPos.nPara );

    // Check against index, not paragraph
    if ( pNode && ( rPos.nIndex < pNode->Len() ) )
    {
        aBounds = pImpEditEngine->PaMtoEditCursor( EditPaM( pNode, rPos.nIndex ), GetCursorFlags::TextOnly );
        tools::Rectangle aR2 = pImpEditEngine->PaMtoEditCursor( EditPaM( pNode, rPos.nIndex+1 ), GetCursorFlags::TextOnly|GetCursorFlags::EndOfLine );
        if ( aR2.Right() > aBounds.Right() )
            aBounds.SetRight( aR2.Right() );
    }
    return aBounds;
}

ParagraphInfos EditEngine::GetParagraphInfos( sal_Int32 nPara )
{

    // This only works if not already in the format ...
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    ParagraphInfos aInfos;
    aInfos.bValid = pImpEditEngine->IsFormatted();
    if ( pImpEditEngine->IsFormatted() )
    {
        const ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions()[nPara];
        const EditLine* pLine = (pParaPortion && pParaPortion->GetLines().Count()) ?
                &pParaPortion->GetLines()[0] : nullptr;
        DBG_ASSERT( pParaPortion && pLine, "GetParagraphInfos - Paragraph out of range" );
        if ( pParaPortion && pLine )
        {
            aInfos.nFirstLineHeight = pLine->GetHeight();
            aInfos.nFirstLineTextHeight = pLine->GetTxtHeight();
            aInfos.nFirstLineMaxAscent = pLine->GetMaxAscent();
        }
    }
    return aInfos;
}

css::uno::Reference< css::datatransfer::XTransferable >
                    EditEngine::CreateTransferable( const ESelection& rSelection ) const
{
    EditSelection aSel( pImpEditEngine->CreateSel( rSelection ) );
    return pImpEditEngine->CreateTransferable( aSel );
}


// ======================    Virtual Methods    ========================

void EditEngine::DrawingText( const Point&, const OUString&, sal_Int32, sal_Int32,
                              const long*, const SvxFont&, sal_Int32 /*nPara*/, sal_uInt8 /*nRightToLeft*/,
                              const EEngineData::WrongSpellVector*, const SvxFieldData*, bool, bool,
                              const css::lang::Locale*, const Color&, const Color&)

{
}

void EditEngine::DrawingTab( const Point& /*rStartPos*/, long /*nWidth*/,
                             const OUString& /*rChar*/, const SvxFont& /*rFont*/,
                             sal_Int32 /*nPara*/, sal_uInt8 /*nRightToLeft*/, bool /*bEndOfLine*/,
                             bool /*bEndOfParagraph*/, const Color& /*rOverlineColor*/,
                             const Color& /*rTextLineColor*/)
{
}

void EditEngine::PaintingFirstLine( sal_Int32, const Point&, long, const Point&, short, OutputDevice* )
{
}

void EditEngine::ParagraphInserted( sal_Int32 nPara )
{

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_PARAGRAPHINSERTED );
        aNotify.nParagraph = nPara;
        pImpEditEngine->GetNotifyHdl().Call( aNotify );
    }
}

void EditEngine::ParagraphDeleted( sal_Int32 nPara )
{

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_PARAGRAPHREMOVED );
        aNotify.nParagraph = nPara;
        pImpEditEngine->GetNotifyHdl().Call( aNotify );
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
        pImpEditEngine->GetNotifyHdl().Call( aNotify );
    }
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

OUString EditEngine::CalcFieldValue( const SvxFieldItem&, sal_Int32, sal_Int32, boost::optional<Color>&, boost::optional<Color>& )
{
    return OUString(' ');
}

void EditEngine::FieldClicked( const SvxFieldItem&, sal_Int32, sal_Int32 )
{
}


// ======================     Static Methods     =======================

SfxItemPool* EditEngine::CreatePool()
{
    SfxItemPool* pPool = new EditEngineItemPool();
    return pPool;
}

SfxItemPool& EditEngine::GetGlobalItemPool()
{
    if ( !pGlobalPool )
        pGlobalPool = CreatePool();
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
    rSet.Put( SvxBackgroundColorItem( rFont.GetFillColor(), EE_CHAR_BKGCOLOR )  );
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
#if HAVE_GCC_BUG_87150
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

    if ( rTransferable.is() )
    {
        // Every application that copies rtf or any other text format also copies plain text into the clipboard....
        datatransfer::DataFlavor aFlavor;
        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
        bValidData = rTransferable->isDataFlavorSupported( aFlavor );
    }

    return bValidData;
}

/** sets a link that is called at the beginning of a drag operation at an edit view */
void EditEngine::SetBeginDropHdl( const Link<EditView*,void>& rLink )
{
    pImpEditEngine->SetBeginDropHdl( rLink );
}

Link<EditView*,void> const & EditEngine::GetBeginDropHdl() const
{
    return pImpEditEngine->GetBeginDropHdl();
}

/** sets a link that is called at the end of a drag operation at an edit view */
void EditEngine::SetEndDropHdl( const Link<EditView*,void>& rLink )
{
    pImpEditEngine->SetEndDropHdl( rLink );
}

Link<EditView*,void> const & EditEngine::GetEndDropHdl() const
{
    return pImpEditEngine->GetEndDropHdl();
}

void EditEngine::SetFirstWordCapitalization( bool bCapitalize )
{
    pImpEditEngine->SetFirstWordCapitalization( bCapitalize );
}

void EditEngine::SetReplaceLeadingSingleQuotationMark( bool bReplace )
{
    pImpEditEngine->SetReplaceLeadingSingleQuotationMark( bReplace );
}

bool EditEngine::IsHtmlImportHandlerSet() const
{
    return pImpEditEngine->aHtmlImportHdl.IsSet();
}

bool EditEngine::IsRtfImportHandlerSet() const
{
    return pImpEditEngine->aRtfImportHdl.IsSet();
}

bool EditEngine::IsImportRTFStyleSheetsSet() const
{
    return pImpEditEngine->GetStatus().DoImportRTFStyleSheets();
}

void EditEngine::CallHtmlImportHandler(HtmlImportInfo& rInfo)
{
    pImpEditEngine->aHtmlImportHdl.Call(rInfo);
}

void EditEngine::CallRtfImportHandler(RtfImportInfo& rInfo)
{
    pImpEditEngine->aRtfImportHdl.Call(rInfo);
}

EditPaM EditEngine::InsertParaBreak(const EditSelection& rEditSelection)
{
    return pImpEditEngine->ImpInsertParaBreak(rEditSelection);
}

EditPaM EditEngine::InsertLineBreak(const EditSelection& rEditSelection)
{
    return pImpEditEngine->InsertLineBreak(rEditSelection);
}

sal_Int32 EditEngine::GetOverflowingParaNum() const {
    return pImpEditEngine->GetOverflowingParaNum();
}

sal_Int32 EditEngine::GetOverflowingLineNum() const {
    return pImpEditEngine->GetOverflowingLineNum();
}

void EditEngine::ClearOverflowingParaNum() {
    pImpEditEngine->ClearOverflowingParaNum();
}

bool EditEngine::IsPageOverflow() {
    pImpEditEngine->CheckPageOverflow();
    return pImpEditEngine->IsPageOverflow();
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
