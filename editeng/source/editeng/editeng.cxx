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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <svtools/ctrltool.hxx>

#include <editeng/svxfont.hxx>
#include <impedit.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editdbg.hxx>
#include <eerdll2.hxx>
#include <editeng/eerdll.hxx>
#include <editeng.hrc>
#include <editeng/acorrcfg.hxx>
#include <editeng/flditem.hxx>
#include <editeng/txtrange.hxx>
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
#include <editeng/charsetcoloritem.hxx>
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
#include <vcl/help.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>

#include <svl/srchdefs.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <editeng/frmdiritem.hxx>
#endif
#include <basegfx/polygon/b2dpolygon.hxx>

// later -> TOOLS\STRING.H (for Grep: WS_TARGET)

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

DBG_NAME( EditEngine )
DBG_NAMEEX( EditView )

#if (OSL_DEBUG_LEVEL > 1) || defined ( DBG_UTIL )
static sal_Bool bDebugPaint = sal_False;
#endif

static SfxItemPool* pGlobalPool=0;

// ----------------------------------------------------------------------
// EditEngine
// ----------------------------------------------------------------------
EditEngine::EditEngine( SfxItemPool* pItemPool )
{
    DBG_CTOR( EditEngine, 0 );
    pImpEditEngine = new ImpEditEngine( this, pItemPool );
}

EditEngine::~EditEngine()
{
    DBG_DTOR( EditEngine, 0 );
    delete pImpEditEngine;
}

void EditEngine::EnableUndo( sal_Bool bEnable )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->EnableUndo( bEnable );
}

sal_Bool EditEngine::IsUndoEnabled()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsUndoEnabled();
}

sal_Bool EditEngine::IsInUndo()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsInUndo();
}

::svl::IUndoManager& EditEngine::GetUndoManager()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetUndoManager();
}

::svl::IUndoManager* EditEngine::SetUndoManager(::svl::IUndoManager* pNew)
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->SetUndoManager(pNew);
}

void EditEngine::UndoActionStart( sal_uInt16 nId )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( !pImpEditEngine->IsInUndo(), "Calling UndoActionStart in Undomode!" );
    if ( !pImpEditEngine->IsInUndo() )
        pImpEditEngine->UndoActionStart( nId );
}

void EditEngine::UndoActionStart(sal_uInt16 nId, const ESelection& rSel)
{
    pImpEditEngine->UndoActionStart(nId, rSel);
}

void EditEngine::UndoActionEnd( sal_uInt16 nId )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( !pImpEditEngine->IsInUndo(), "Calling UndoActionEnd in Undomode!" );
    if ( !pImpEditEngine->IsInUndo() )
        pImpEditEngine->UndoActionEnd( nId );
}

bool EditEngine::HasTriedMergeOnLastAddUndo() const
{
    return pImpEditEngine->mbLastTryMerge;
}

void EditEngine::SetRefDevice( OutputDevice* pRefDev )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetRefDevice( pRefDev );
}

OutputDevice* EditEngine::GetRefDevice() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetRefDevice();
}

void EditEngine::SetRefMapMode( const MapMode& rMapMode )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetRefMapMode( rMapMode );
}

MapMode EditEngine::GetRefMapMode()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetRefMapMode();
}

void EditEngine::SetBackgroundColor( const Color& rColor )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetBackgroundColor( rColor );
}

Color EditEngine::GetBackgroundColor() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetBackgroundColor();
}

Color EditEngine::GetAutoColor() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetAutoColor();
}

void EditEngine::EnableAutoColor( sal_Bool b )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->EnableAutoColor( b );
}

void EditEngine::ForceAutoColor( sal_Bool b )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->ForceAutoColor( b );
}

sal_Bool EditEngine::IsForceAutoColor() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsForceAutoColor();
}

const SfxItemSet& EditEngine::GetEmptyItemSet()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEmptyItemSet();
}

void EditEngine::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect )
{
    DBG_CHKTHIS( EditEngine, 0 );
    Draw( pOutDev, rOutRect, Point( 0, 0 ) );
}

void EditEngine::Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation )
{
    DBG_CHKTHIS( EditEngine, 0 );
    // Create with 2 points, as with positive points it will end up with
    // LONGMAX as Size, Bottom and Right in the range > LONGMAX.
    Rectangle aBigRect( -0x3FFFFFFF, -0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF );
    if( pOutDev->GetConnectMetaFile() )
        pOutDev->Push();
    Point aStartPos( rStartPos );
    if ( IsVertical() )
    {
        aStartPos.X() += GetPaperSize().Width();
        aStartPos = Rotate( aStartPos, nOrientation, rStartPos );
    }
    pImpEditEngine->Paint( pOutDev, aBigRect, aStartPos, sal_False, nOrientation );
    if( pOutDev->GetConnectMetaFile() )
        pOutDev->Pop();
}

void EditEngine::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos )
{
    Draw( pOutDev, rOutRect, rStartDocPos, sal_True );
}

void EditEngine::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos, sal_Bool bClip )
{
    DBG_CHKTHIS( EditEngine, 0 );

#if defined( DBG_UTIL ) || (OSL_DEBUG_LEVEL > 1)
    if ( bDebugPaint )
        EditDbg::ShowEditEngineData( this, sal_False );
#endif

    // Align to the pixel boundary, so that it becomes exactly the same
    // as Paint ().
    Rectangle aOutRect( pOutDev->LogicToPixel( rOutRect ) );
    aOutRect = pOutDev->PixelToLogic( aOutRect );

    Point aStartPos;
    if ( !IsVertical() )
    {
        aStartPos.X() = aOutRect.Left() - rStartDocPos.X();
        aStartPos.Y() = aOutRect.Top() - rStartDocPos.Y();
    }
    else
    {
        aStartPos.X() = aOutRect.Right() + rStartDocPos.Y();
        aStartPos.Y() = aOutRect.Top() - rStartDocPos.X();
    }

    sal_Bool bClipRegion = pOutDev->IsClipRegion();
    sal_Bool bMetafile = pOutDev->GetConnectMetaFile() ? sal_True : sal_False;
    Region aOldRegion = pOutDev->GetClipRegion();

    // If one existed => intersection!
    // Use Push/pop for creating the Meta file
    if ( bMetafile )
        pOutDev->Push();

    // Always use the Intersect method, it is a must for Metafile!
    if ( bClip )
    {
        // Clip only if neccesary...
        if ( !rStartDocPos.X() && !rStartDocPos.Y() &&
             ( rOutRect.GetHeight() >= (long)GetTextHeight() ) &&
             ( rOutRect.GetWidth() >= (long)CalcTextWidth() ) )
        {
            bClip = sal_False;
        }
        else
        {
            // Some printer drivers cause problems if characters graze the
            // ClipRegion, therefore rather add a pixel more ...
            Rectangle aClipRect( aOutRect );
            if ( pOutDev->GetOutDevType() == OUTDEV_PRINTER )
            {
                Size aPixSz( 1, 0 );
                aPixSz = pOutDev->PixelToLogic( aPixSz );
                aClipRect.Right() += aPixSz.Width();
                aClipRect.Bottom() += aPixSz.Width();
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
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_CHKOBJ( pEditView, EditView, 0 );

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
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_CHKOBJ( pView, EditView, 0 );

    pView->HideCursor();
    EditView* pRemoved = NULL;
    ImpEditEngine::ViewsType& rViews = pImpEditEngine->GetEditViews();
    ImpEditEngine::ViewsType::iterator it = std::find(rViews.begin(), rViews.end(), pView);

    DBG_ASSERT( it != rViews.end(), "RemoveView with invalid index" );
    if (it != rViews.end())
    {
        pRemoved = *it;
        rViews.erase(it);
        if ( pImpEditEngine->GetActiveView() == pView )
        {
            pImpEditEngine->SetActiveView( 0 );
            pImpEditEngine->GetSelEngine().SetCurView( 0 );
        }
        pView->pImpEditView->RemoveDragAndDropListeners();

    }
    return pRemoved;
}

EditView* EditEngine::RemoveView(size_t nIndex)
{
    DBG_CHKTHIS( EditEngine, 0 );
    ImpEditEngine::ViewsType& rViews = pImpEditEngine->GetEditViews();
    if (nIndex >= rViews.size())
        return NULL;

    EditView* pView = rViews[nIndex];
    if ( pView )
        return RemoveView( pView );
    return NULL;
}

EditView* EditEngine::GetView(size_t nIndex) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditViews()[nIndex];
}

size_t EditEngine::GetViewCount() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditViews().size();
}

sal_Bool EditEngine::HasView( EditView* pView ) const
{
    ImpEditEngine::ViewsType& rViews = pImpEditEngine->GetEditViews();
    return std::find(rViews.begin(), rViews.end(), pView) != rViews.end();
}

EditView* EditEngine::GetActiveView() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetActiveView();
}

void EditEngine::SetActiveView(EditView* pView)
{
    pImpEditEngine->SetActiveView(pView);
}

void EditEngine::SetDefTab( sal_uInt16 nDefTab )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->GetEditDoc().SetDefTab( nDefTab );
    if ( pImpEditEngine->IsFormatted() )
    {
        pImpEditEngine->FormatFullDoc();
        pImpEditEngine->UpdateViews( (EditView*) 0 );
    }
}

void EditEngine::SetPaperSize( const Size& rNewSize )
{
    DBG_CHKTHIS( EditEngine, 0 );

    Size aOldSize( pImpEditEngine->GetPaperSize() );
    pImpEditEngine->SetValidPaperSize( rNewSize );
    Size aNewSize( pImpEditEngine->GetPaperSize() );

    sal_Bool bAutoPageSize = pImpEditEngine->GetStatus().AutoPageSize();
    if ( bAutoPageSize || ( aNewSize.Width() != aOldSize.Width() ) )
    {
        for (size_t nView = 0; nView < pImpEditEngine->aEditViews.size(); ++nView)
        {
            EditView* pView = pImpEditEngine->aEditViews[nView];
            DBG_CHKOBJ( pView, EditView, 0 );
            if ( bAutoPageSize )
                pView->pImpEditView->RecalcOutputArea();
            else if ( pView->pImpEditView->DoAutoSize() )
            {
                pView->pImpEditView->ResetOutputArea( Rectangle(
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
                pImpEditEngine->pActiveView->ShowCursor( sal_False, sal_False );
        }
    }
}

const Size& EditEngine::GetPaperSize() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetPaperSize();
}

void EditEngine::SetVertical( bool bVertical )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetVertical( bVertical );
}

bool EditEngine::IsVertical() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsVertical();
}

void EditEngine::SetFixedCellHeight( sal_Bool bUseFixedCellHeight )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetFixedCellHeight( bUseFixedCellHeight );
}

void EditEngine::SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetDefaultHorizontalTextDirection( eHTextDir );
}

EEHorizontalTextDirection EditEngine::GetDefaultHorizontalTextDirection() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetDefaultHorizontalTextDirection();
}

sal_uInt16 EditEngine::GetScriptType( const ESelection& rSelection ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->CreateSel( rSelection ) );
    return pImpEditEngine->GetScriptType( aSel );
}

LanguageType EditEngine::GetLanguage(const EditPaM& rPaM) const
{
    return pImpEditEngine->GetLanguage(rPaM);
}

LanguageType EditEngine::GetLanguage( sal_Int32 nPara, sal_uInt16 nPos ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    DBG_ASSERT( pNode, "GetLanguage - nPara is invalid!" );
    return pNode ? pImpEditEngine->GetLanguage( EditPaM( pNode, nPos ) ) : LANGUAGE_DONTKNOW;
}


void EditEngine::TransliterateText( const ESelection& rSelection, sal_Int32 nTransliterationMode )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditEngine->TransliterateText( pImpEditEngine->CreateSel( rSelection ), nTransliterationMode );
}

EditSelection EditEngine::TransliterateText(const EditSelection& rSelection, sal_Int32 nTransliterationMode)
{
    return pImpEditEngine->TransliterateText(rSelection, nTransliterationMode);
}

void EditEngine::SetAsianCompressionMode( sal_uInt16 n )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditEngine->SetAsianCompressionMode( n );
}

void EditEngine::SetKernAsianPunctuation( sal_Bool b )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditEngine->SetKernAsianPunctuation( b );
}

void EditEngine::SetAddExtLeading( sal_Bool b )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetAddExtLeading( b );
}

void EditEngine::SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    DBG_CHKTHIS( EditEngine, 0 );
    SetPolygon( rPolyPolygon, 0L );
}

void EditEngine::SetPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon)
{
    DBG_CHKTHIS( EditEngine, 0 );
    sal_Bool bSimple(sal_False);

    if(pLinePolyPolygon && 1L == rPolyPolygon.count())
    {
        if(rPolyPolygon.getB2DPolygon(0L).isClosed())
        {
            // open polygon
            bSimple = sal_True;
        }
    }

    TextRanger* pRanger = new TextRanger( rPolyPolygon, pLinePolyPolygon, 30, 2, 2, bSimple, sal_True );
    pImpEditEngine->SetTextRanger( pRanger );
    pImpEditEngine->SetPaperSize( pRanger->GetBoundRect().GetSize() );
}

void EditEngine::ClearPolygon()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetTextRanger( 0 );
}

const Size& EditEngine::GetMinAutoPaperSize() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetMinAutoPaperSize();
}

void EditEngine::SetMinAutoPaperSize( const Size& rSz )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetMinAutoPaperSize( rSz );
}

const Size& EditEngine::GetMaxAutoPaperSize() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetMaxAutoPaperSize();
}

void EditEngine::SetMaxAutoPaperSize( const Size& rSz )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetMaxAutoPaperSize( rSz );
}

OUString EditEngine::GetText( LineEnd eEnd ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditDoc().GetText( eEnd );
}

XubString EditEngine::GetText( const ESelection& rESelection, const LineEnd eEnd ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->CreateSel( rESelection ) );
    return pImpEditEngine->GetSelected( aSel, eEnd );
}

sal_uInt32 EditEngine::GetTextLen() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditDoc().GetTextLen();
}

sal_Int32 EditEngine::GetParagraphCount() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aEditDoc.Count();
}

sal_uInt16 EditEngine::GetLineCount( sal_Int32 nParagraph ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineCount( nParagraph );
}

sal_uInt16 EditEngine::GetLineLen( sal_Int32 nParagraph, sal_uInt16 nLine ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineLen( nParagraph, nLine );
}

void EditEngine::GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_Int32 nParagraph, sal_uInt16 nLine ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineBoundaries( rStart, rEnd, nParagraph, nLine );
}

sal_uInt16 EditEngine::GetLineNumberAtIndex( sal_Int32 nPara, sal_uInt16 nIndex ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineNumberAtIndex( nPara, nIndex );
}

sal_uInt32 EditEngine::GetLineHeight( sal_Int32 nParagraph, sal_uInt16 nLine )
{
    DBG_CHKTHIS( EditEngine, 0 );
    // If someone calls GetLineHeight() with an empty Engine.
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineHeight( nParagraph, nLine );
}

sal_uInt32 EditEngine::GetTextHeight( sal_Int32 nParagraph ) const
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nHeight = pImpEditEngine->GetParaHeight( nParagraph );
     return nHeight;
}

XubString EditEngine::GetWord( sal_Int32 nPara, sal_uInt16 nIndex )
{
    ESelection aESel( nPara, nIndex, nPara, nIndex );
    EditSelection aSel( pImpEditEngine->CreateSel( aESel ) );
    aSel = pImpEditEngine->SelectWord( aSel );
    return pImpEditEngine->GetSelected( aSel );
}

ESelection EditEngine::GetWord( const ESelection& rSelection, sal_uInt16 nWordType  ) const
{
    // ImpEditEngine-Iteration-Methods should be const!
    EditEngine* pE = (EditEngine*)this;

    EditSelection aSel( pE->pImpEditEngine->CreateSel( rSelection ) );
    aSel = pE->pImpEditEngine->SelectWord( aSel, nWordType );
    return pE->pImpEditEngine->CreateESel( aSel );
}

void EditEngine::CursorMoved(ContentNode* pPrevNode)
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

ParaPortion* EditEngine::FindParaPortion(ContentNode* pNode)
{
    return pImpEditEngine->FindParaPortion(pNode);
}

const ParaPortion* EditEngine::FindParaPortion(ContentNode* pNode) const
{
    return pImpEditEngine->FindParaPortion(pNode);
}

const ParaPortion* EditEngine::GetPrevVisPortion(const ParaPortion* pCurPortion) const
{
    return pImpEditEngine->GetPrevVisPortion(pCurPortion);
}

sal_uInt16 EditEngine::GetScriptType(const EditSelection& rSel) const
{
    return pImpEditEngine->GetScriptType(rSel);
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
    pImpEditEngine->aDeletedNodes.push_back(pInfo);
}

void EditEngine::UpdateSelections()
{
    pImpEditEngine->UpdateSelections();
}

void EditEngine::InsertContent(ContentNode* pNode, sal_Int32 nPos)
{
    pImpEditEngine->InsertContent(pNode, nPos);
}

EditPaM EditEngine::SplitContent(sal_Int32 nNode, sal_uInt16 nSepPos)
{
    return pImpEditEngine->SplitContent(nNode, nSepPos);
}

EditPaM EditEngine::ConnectContents(sal_Int32 nLeftNode, bool bBackward)
{
    return pImpEditEngine->ConnectContents(nLeftNode, bBackward);
}

EditPaM EditEngine::InsertFeature(const EditSelection& rEditSelection, const SfxPoolItem& rItem)
{
    return pImpEditEngine->ImpInsertFeature(rEditSelection, rItem);
}

EditSelection EditEngine::MoveParagraphs(const Range& rParagraphs, sal_Int32 nNewPos, EditView* pCurView)
{
    return pImpEditEngine->MoveParagraphs(rParagraphs, nNewPos, pCurView);
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

void EditEngine::FormatAndUpdate(EditView* pCurView)
{
    pImpEditEngine->FormatAndUpdate(pCurView);
}

bool EditEngine::Undo(EditView* pView)
{
    return pImpEditEngine->Undo(pView);
}

bool EditEngine::Redo(EditView* pView)
{
    return pImpEditEngine->Redo(pView);
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

EditPaM EditEngine::InsertText(const EditSelection& aCurEditSelection, const String& rStr)
{
    return pImpEditEngine->InsertText(aCurEditSelection, rStr);
}

EditSelection EditEngine::InsertText(const EditTextObject& rTextObject, const EditSelection& rSel)
{
    return pImpEditEngine->InsertText(rTextObject, rSel);
}

EditSelection EditEngine::InsertText(
    uno::Reference<datatransfer::XTransferable >& rxDataObj,
    const String& rBaseURL, const EditPaM& rPaM, bool bUseSpecial)
{
    return pImpEditEngine->InsertText(rxDataObj, rBaseURL, rPaM, bUseSpecial);
}

EditPaM EditEngine::EndOfWord(const EditPaM& rPaM, sal_Int16 nWordType)
{
    return pImpEditEngine->EndOfWord(rPaM, nWordType);
}

EditPaM EditEngine::GetPaM(const Point& aDocPos, bool bSmart)
{
    return pImpEditEngine->GetPaM(aDocPos, bSmart);
}

EditSelection EditEngine::SelectWord(
        const EditSelection& rCurSelection, sal_Int16 nWordType, bool bAcceptStartOfWord)
{
    return pImpEditEngine->SelectWord(rCurSelection, nWordType, bAcceptStartOfWord);
}

long EditEngine::GetXPos(
        const ParaPortion* pParaPortion, const EditLine* pLine, sal_uInt16 nIndex, bool bPreferPortionStart) const
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

sal_uInt16 EditEngine::GetOnePixelInRef() const
{
    return pImpEditEngine->nOnePixelInRef;
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

ParaPortionList& EditEngine::GetParaPortions()
{
    return pImpEditEngine->GetParaPortions();
}

const ParaPortionList& EditEngine::GetParaPortions() const
{
    return pImpEditEngine->GetParaPortions();
}

void EditEngine::SeekCursor(
        ContentNode* pNode, sal_uInt16 nPos, SvxFont& rFont, OutputDevice* pOut, sal_uInt16 nIgnoreWhich)
{
    pImpEditEngine->SeekCursor(pNode, nPos, rFont, pOut, nIgnoreWhich);
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

void EditEngine::SetAttribs(const EditSelection& rSel, const SfxItemSet& rSet, sal_uInt8 nSpecial)
{
    pImpEditEngine->SetAttribs(rSel, rSet, nSpecial);
}

String EditEngine::GetSelected(const EditSelection& rSel, const LineEnd eParaSep) const
{
    return pImpEditEngine->GetSelected(rSel, eParaSep);
}

EditPaM EditEngine::DeleteSelected(const EditSelection& rSel)
{
    return pImpEditEngine->DeleteSelected(rSel);
}

void EditEngine::HandleBeginPasteOrDrop(PasteOrDropInfos& rInfos)
{
    pImpEditEngine->aBeginPasteOrDropHdl.Call(&rInfos);
}

void EditEngine::HandleEndPasteOrDrop(PasteOrDropInfos& rInfos)
{
    pImpEditEngine->aEndPasteOrDropHdl.Call(&rInfos);
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

sal_Bool EditEngine::PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pEditView, Window* pFrameWin )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_CHKOBJ( pEditView, EditView, 0 );
    DBG_ASSERT( pEditView, "no View - no cookie !" );

    sal_Bool bDone = sal_True;

    sal_Bool bModified  = sal_False;
    sal_Bool bMoved     = sal_False;
    sal_Bool bAllowIdle = sal_True;
    sal_Bool bReadOnly  = pEditView->IsReadOnly();

    sal_uInt16 nNewCursorFlags = 0;
    sal_Bool bSetCursorFlags = sal_True;

    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    DBG_ASSERT( !aCurSel.IsInvalid(), "Blinde Selection in EditEngine::PostKeyEvent" );

    String aAutoText( pImpEditEngine->GetAutoCompleteText() );
    if ( pImpEditEngine->GetAutoCompleteText().Len() )
        pImpEditEngine->SetAutoCompleteText( String(), sal_True );

    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_UNDO:
            {
                if ( !bReadOnly )
                    pEditView->Undo();
                return sal_True;
            }
            // break;
            case KEYFUNC_REDO:
            {
                if ( !bReadOnly )
                    pEditView->Redo();
                return sal_True;
            }
            // break;

            default:    // is then possible edited below.
                        eFunc = KEYFUNC_DONTKNOW;
        }
    }

    pImpEditEngine->EnterBlockNotifications();

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_INPUT_START );
        aNotify.pEditEngine = this;
        pImpEditEngine->CallNotify( aNotify );
    }

    if ( eFunc == KEYFUNC_DONTKNOW )
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
                        P2.X() += n20;
                        P2.Y() += nH;
                        pEditView->GetWindow()->SetLineColor();
                        pEditView->GetWindow()->SetFillColor( Color( (n%2) ? COL_YELLOW : COL_LIGHTGREEN ) );
                        pEditView->GetWindow()->DrawRect( Rectangle( P1, P2 ) );
                        aPos.Y() += nH;
                    }
                }
                bDone = sal_False;
            }
            break;
            case KEY_F11:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    bDebugPaint = !bDebugPaint;
                    OStringBuffer aInfo(RTL_CONSTASCII_STRINGPARAM(
                        "DebugPaint: "));
                    aInfo.append(bDebugPaint ? "On" : "Off");
                    InfoBox(NULL, OStringToOUString(aInfo.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US)).Execute();
                }
                bDone = sal_False;
            }
            break;
            case KEY_F12:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    EditDbg::ShowEditEngineData( this );
                }
                bDone = sal_False;
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
            case com::sun::star::awt::Key::MOVE_WORD_FORWARD:
            case com::sun::star::awt::Key::SELECT_WORD_FORWARD:
            case com::sun::star::awt::Key::MOVE_WORD_BACKWARD:
            case com::sun::star::awt::Key::SELECT_WORD_BACKWARD:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_LINE:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_LINE:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_DOCUMENT:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod2() || ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) )
                {
                    if ( pImpEditEngine->DoVisualCursorTraveling( aCurSel.Max().GetNode() ) && ( ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) /* || ( nCode == KEY_HOME ) || ( nCode == KEY_END ) */ ) )
                        bSetCursorFlags = sal_False;    // Will be manipulated within visual cursor move

                    aCurSel = pImpEditEngine->MoveCursor( rKeyEvent, pEditView );

                    if ( aCurSel.HasRange() ) {
                        Reference<com::sun::star::datatransfer::clipboard::XClipboard> aSelection(pEditView->GetWindow()->GetPrimarySelection());
                        pEditView->pImpEditView->CutCopy( aSelection, sal_False );
                    }

                    bMoved = sal_True;
                    if ( nCode == KEY_HOME )
                        nNewCursorFlags |= GETCRSR_STARTOFLINE;
                    else if ( nCode == KEY_END )
                        nNewCursorFlags |= GETCRSR_ENDOFLINE;

                }
#if OSL_DEBUG_LEVEL > 1
                GetLanguage( pImpEditEngine->GetEditDoc().GetPos( aCurSel.Max().GetNode() ), aCurSel.Max().GetIndex() );
#endif
            }
            break;
            case KEY_BACKSPACE:
            case KEY_DELETE:
            case com::sun::star::awt::Key::DELETE_WORD_BACKWARD:
            case com::sun::star::awt::Key::DELETE_WORD_FORWARD:
            case com::sun::star::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH:
            case com::sun::star::awt::Key::DELETE_TO_END_OF_PARAGRAPH:
            {
                if ( !bReadOnly && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    // check if we are behind a bullet and using the backspace key
                    ContentNode *pNode = aCurSel.Min().GetNode();
                    const SvxNumberFormat *pFmt = pImpEditEngine->GetNumberFormat( pNode );
                    if (pFmt && nCode == KEY_BACKSPACE &&
                        !aCurSel.HasRange() && aCurSel.Min().GetIndex() == 0)
                    {
                        // if the bullet is still visible just do not paint it from
                        // now on and that will be all. Otherwise continue as usual.
                        // ...

                        sal_Int32 nPara = pImpEditEngine->GetEditDoc().GetPos( pNode );
                        SfxBoolItem aBulletState( (const SfxBoolItem&) pImpEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE ) );
                        bool bBulletIsVisible = aBulletState.GetValue() ? true : false;

                        // just toggling EE_PARA_BULLETSTATE should be fine for both cases...
                        aBulletState.SetValue( !bBulletIsVisible );
                        SfxItemSet aSet( pImpEditEngine->GetParaAttribs( nPara ) );
                        aSet.Put( aBulletState );
                        pImpEditEngine->SetParaAttribs( nPara, aSet );

                        // have this and the following paragraphs formatted and repainted.
                        // (not painting a numbering in the list may cause the following
                        // numberings to have different numbers than before and thus the
                        // length may have changed as well )
                        pImpEditEngine->FormatAndUpdate( pImpEditEngine->GetActiveView() );

                        if (bBulletIsVisible)   // bullet just turned invisible...
                            break;
                    }

                    sal_uInt8 nDel = 0, nMode = 0;
                    switch( nCode )
                    {
                    case com::sun::star::awt::Key::DELETE_WORD_BACKWARD:
                        nMode = DELMODE_RESTOFWORD;
                        nDel = DEL_LEFT;
                        break;
                    case com::sun::star::awt::Key::DELETE_WORD_FORWARD:
                        nMode = DELMODE_RESTOFWORD;
                        nDel = DEL_RIGHT;
                        break;
                    case com::sun::star::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH:
                        nMode = DELMODE_RESTOFCONTENT;
                        nDel = DEL_LEFT;
                        break;
                    case com::sun::star::awt::Key::DELETE_TO_END_OF_PARAGRAPH:
                        nMode = DELMODE_RESTOFCONTENT;
                        nDel = DEL_RIGHT;
                        break;
                    default:
                        nDel = ( nCode == KEY_DELETE ) ? DEL_RIGHT : DEL_LEFT;
                        nMode = rKeyEvent.GetKeyCode().IsMod1() ? DELMODE_RESTOFWORD : DELMODE_SIMPLE;
                        if ( ( nMode == DELMODE_RESTOFWORD ) && rKeyEvent.GetKeyCode().IsShift() )
                            nMode = DELMODE_RESTOFCONTENT;
                        break;
                    }

                    pEditView->pImpEditView->DrawSelection();
                    pImpEditEngine->UndoActionStart( EDITUNDO_DELETE );
                    aCurSel = pImpEditEngine->DeleteLeftOrRight( aCurSel, nDel, nMode );
                    pImpEditEngine->UndoActionEnd( EDITUNDO_DELETE );
                    bModified = sal_True;
                    bAllowIdle = sal_False;
                }
            }
            break;
            case KEY_TAB:
            {
                if ( !bReadOnly && !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    sal_Bool bShift = rKeyEvent.GetKeyCode().IsShift();
                    if ( pImpEditEngine->GetStatus().DoTabIndenting() &&
                        ( aCurSel.Min().GetNode() != aCurSel.Max().GetNode() ) )
                    {
                        pImpEditEngine->IndentBlock( pEditView, !bShift );
                    }
                    else if ( !bShift )
                    {
                        sal_Bool bSel = pEditView->HasSelection();
                        if ( bSel )
                            pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
                        if ( pImpEditEngine->GetStatus().DoAutoCorrect() )
                            aCurSel = pImpEditEngine->AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                        aCurSel = pImpEditEngine->InsertTab( aCurSel );
                        if ( bSel )
                            pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
                        bModified = sal_True;
                    }
                }
                else
                    bDone = sal_False;
            }
            break;
            case KEY_RETURN:
            {
                if ( !bReadOnly )
                {
                    pEditView->pImpEditView->DrawSelection();
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
                            if ( !aAutoText.Len() )
                            {
                                if ( pImpEditEngine->GetStatus().DoAutoCorrect() )
                                    aCurSel = pImpEditEngine->AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                                aCurSel = pImpEditEngine->InsertParaBreak( aCurSel );
                            }
                            else
                            {
                                DBG_ASSERT( !aCurSel.HasRange(), "Selektion bei Complete?!" );
                                EditPaM aStart( pImpEditEngine->WordLeft( aCurSel.Max() ) );
                                aCurSel = pImpEditEngine->InsertText(
                                                EditSelection( aStart, aCurSel.Max() ), aAutoText );
                                pImpEditEngine->SetAutoCompleteText( String(), sal_True );
                            }
                        }
                        pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
                        bModified = sal_True;
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
                        SvxFrameDirectionItem aNewItem( FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR );
                        if ( rCurrentWritingMode.GetValue() != FRMDIR_HORI_RIGHT_TOP )
                            aNewItem.SetValue( FRMDIR_HORI_RIGHT_TOP );
                        aAttribs.Put( aNewItem );
                        pEditView->SetAttribs( aAttribs );
                    }
                #endif
                if ( !bReadOnly && IsSimpleCharInput( rKeyEvent ) )
                {
                    sal_Unicode nCharCode = rKeyEvent.GetCharCode();
                    pEditView->pImpEditView->DrawSelection();
                    // Autocorrection?
                    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
                    if ( ( pImpEditEngine->GetStatus().DoAutoCorrect() ) &&
                        ( SvxAutoCorrect::IsAutoCorrectChar( nCharCode ) ||
                          pAutoCorrect->HasRunNext() ) )
                    {
                        aCurSel = pImpEditEngine->AutoCorrect(
                            aCurSel, nCharCode, !pEditView->IsInsertMode(), pFrameWin );
                    }
                    else
                    {
                        aCurSel = pImpEditEngine->InsertText( (const EditSelection&)aCurSel, nCharCode, !pEditView->IsInsertMode(), sal_True );
                    }
                    // AutoComplete ???
                    if ( pImpEditEngine->GetStatus().DoAutoComplete() && ( nCharCode != ' ' ) )
                    {
                        // Only at end of word...
                        sal_uInt16 nIndex = aCurSel.Max().GetIndex();
                        if ( ( nIndex >= aCurSel.Max().GetNode()->Len() ) ||
                             ( pImpEditEngine->aWordDelimiters.indexOf( aCurSel.Max().GetNode()->GetChar( nIndex ) ) != -1 ) )
                        {
                            EditPaM aStart( pImpEditEngine->WordLeft( aCurSel.Max() ) );
                            String aWord = pImpEditEngine->GetSelected( EditSelection( aStart, aCurSel.Max() ) );
                            if ( aWord.Len() >= 3 )
                            {
                                String aComplete;

                                LanguageType eLang = pImpEditEngine->GetLanguage( EditPaM( aStart.GetNode(), aStart.GetIndex()+1));
                                LanguageTag aLanguageTag( eLang);

                                if (!pImpEditEngine->xLocaleDataWrapper.isInitialized())
                                    pImpEditEngine->xLocaleDataWrapper.init( SvtSysLocale().GetLocaleData().getComponentContext(), aLanguageTag);
                                else
                                    pImpEditEngine->xLocaleDataWrapper.changeLocale( aLanguageTag);

                                if (!pImpEditEngine->xTransliterationWrapper.isInitialized())
                                    pImpEditEngine->xTransliterationWrapper.init( SvtSysLocale().GetLocaleData().getComponentContext(), eLang, i18n::TransliterationModules_IGNORE_CASE);
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

                                if ( !aComplete.Len() )
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

                                if( aComplete.Len() && ( ( aWord.Len() + 1 ) < aComplete.Len() ) )
                                {
                                    pImpEditEngine->SetAutoCompleteText( aComplete, sal_False );
                                    Point aPos = pImpEditEngine->PaMtoEditCursor( aCurSel.Max() ).TopLeft();
                                    aPos = pEditView->pImpEditView->GetWindowPos( aPos );
                                    aPos = pEditView->pImpEditView->GetWindow()->LogicToPixel( aPos );
                                    aPos = pEditView->GetWindow()->OutputToScreenPixel( aPos );
                                    aPos.Y() -= 3;
                                    Help::ShowQuickHelp( pEditView->GetWindow(), Rectangle( aPos, Size( 1, 1 ) ), aComplete, QUICKHELP_BOTTOM|QUICKHELP_LEFT );
                                }
                            }
                        }
                    }
                    bModified = sal_True;
                }
                else
                    bDone = sal_False;
            }
        }
    }

    pEditView->pImpEditView->SetEditSelection( aCurSel );
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
        pEditView->pImpEditView->SetCursorBidiLevel( 0xFFFF );
    }

    if ( bSetCursorFlags )
        pEditView->pImpEditView->nExtraCursorFlags = nNewCursorFlags;

    if ( bModified )
    {
        DBG_ASSERT( !bReadOnly, "ReadOnly but modified???" );
        // Idle-Formatter only when AnyInput.
        if ( bAllowIdle && pImpEditEngine->GetStatus().UseIdleFormatter()
                && Application::AnyInput( VCL_INPUT_KEYBOARD) )
            pImpEditEngine->IdleFormatAndUpdate( pEditView );
        else
            pImpEditEngine->FormatAndUpdate( pEditView );
    }
    else if ( bMoved )
    {
        sal_Bool bGotoCursor = pEditView->pImpEditView->DoAutoScroll();
        pEditView->pImpEditView->ShowCursor( bGotoCursor, sal_True );
        pImpEditEngine->CallStatusHdl();
    }

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_INPUT_END );
        aNotify.pEditEngine = this;
        pImpEditEngine->CallNotify( aNotify );
    }

    pImpEditEngine->LeaveBlockNotifications();

    return bDone;
}

sal_uInt32 EditEngine::GetTextHeight() const
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nHeight = !IsVertical() ? pImpEditEngine->GetTextHeight() : pImpEditEngine->CalcTextWidth( sal_True );
    return nHeight;
}

sal_uInt32 EditEngine::GetTextHeightNTP() const
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    if ( IsVertical() )
        return pImpEditEngine->CalcTextWidth( sal_True );

    return pImpEditEngine->GetTextHeightNTP();
}

sal_uInt32 EditEngine::CalcTextWidth()
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nWidth = !IsVertical() ? pImpEditEngine->CalcTextWidth( sal_True ) : pImpEditEngine->GetTextHeight();
     return nWidth;
}

void EditEngine::SetUpdateMode( sal_Bool bUpdate )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetUpdateMode( bUpdate );
    if ( pImpEditEngine->pActiveView )
        pImpEditEngine->pActiveView->ShowCursor( sal_False, sal_False );
}

sal_Bool EditEngine::GetUpdateMode() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetUpdateMode();
}

void EditEngine::Clear()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->Clear();
}

void EditEngine::SetText( const OUString& rText )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetText( rText );
    if ( !rText.isEmpty() )
        pImpEditEngine->FormatAndUpdate();
}

sal_uLong EditEngine::Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs /* = NULL */ )
{
    DBG_CHKTHIS( EditEngine, 0 );
    sal_Bool bUndoEnabled = pImpEditEngine->IsUndoEnabled();
    pImpEditEngine->EnableUndo( sal_False );
    pImpEditEngine->SetText( XubString() );
    EditPaM aPaM( pImpEditEngine->GetEditDoc().GetStartPaM() );
    pImpEditEngine->Read( rInput, rBaseURL, eFormat, EditSelection( aPaM, aPaM ), pHTTPHeaderAttrs );
    pImpEditEngine->EnableUndo( bUndoEnabled );
    return rInput.GetError();
}

sal_uLong EditEngine::Write( SvStream& rOutput, EETextFormat eFormat )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditPaM aStartPaM( pImpEditEngine->GetEditDoc().GetStartPaM() );
    EditPaM aEndPaM( pImpEditEngine->GetEditDoc().GetEndPaM() );
    pImpEditEngine->Write( rOutput, eFormat, EditSelection( aStartPaM, aEndPaM ) );
    return rOutput.GetError();
}

EditTextObject* EditEngine::CreateTextObject()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->CreateTextObject();
}

EditTextObject* EditEngine::CreateTextObject( const ESelection& rESelection )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->CreateSel( rESelection ) );
    return pImpEditEngine->CreateTextObject( aSel );
}

void EditEngine::SetText( const EditTextObject& rTextObject )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->EnterBlockNotifications();
    pImpEditEngine->SetText( rTextObject );
    pImpEditEngine->FormatAndUpdate();
    pImpEditEngine->LeaveBlockNotifications();
}

void EditEngine::ShowParagraph( sal_Int32 nParagraph, sal_Bool bShow )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->ShowParagraph( nParagraph, bShow );
}

void EditEngine::SetNotifyHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetNotifyHdl( rLink );
}

Link EditEngine::GetNotifyHdl() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetNotifyHdl();
}

void EditEngine::SetStatusEventHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetStatusEventHdl( rLink );
}

Link EditEngine::GetStatusEventHdl() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStatusEventHdl();
}

void EditEngine::SetImportHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->aImportHdl = rLink;
}

Link EditEngine::GetImportHdl() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aImportHdl;
}

void EditEngine::SetBeginMovingParagraphsHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->aBeginMovingParagraphsHdl = rLink;
}

void EditEngine::SetEndMovingParagraphsHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->aEndMovingParagraphsHdl = rLink;
}

void EditEngine::SetBeginPasteOrDropHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );

    pImpEditEngine->aBeginPasteOrDropHdl = rLink;
}

void EditEngine::SetEndPasteOrDropHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->aEndPasteOrDropHdl = rLink;
}

EditTextObject* EditEngine::CreateTextObject( sal_Int32 nPara, sal_Int32 nParas )
{
    DBG_CHKTHIS( EditEngine, 0 );
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
    return 0;
}

void EditEngine::RemoveParagraph( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
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

sal_uInt16 EditEngine::GetTextLen( sal_Int32 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    DBG_ASSERT( pNode, "Paragraph not found: GetTextLen" );
    if ( pNode )
        return pNode->Len();
    return 0;
}

XubString EditEngine::GetText( sal_Int32 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    XubString aStr;
    if ( 0 <= nPara && nPara < pImpEditEngine->GetEditDoc().Count() )
        aStr = pImpEditEngine->GetEditDoc().GetParaAsString( nPara );
    return aStr;
}

void EditEngine::SetModifyHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetModifyHdl( rLink );
}

Link EditEngine::GetModifyHdl() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetModifyHdl();
}


void EditEngine::ClearModifyFlag()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetModifyFlag( sal_False );
}

void EditEngine::SetModified()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetModifyFlag( sal_True );
}

sal_Bool EditEngine::IsModified() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsModified();
}

sal_Bool EditEngine::IsInSelectionMode() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return ( pImpEditEngine->IsInSelectionMode() ||
                pImpEditEngine->GetSelEngine().IsInSelection() );
}

void EditEngine::InsertParagraph( sal_Int32 nPara, const EditTextObject& rTxtObj )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( nPara > GetParagraphCount() )
    {
        DBG_ASSERTWARNING( nPara == EE_PARA_APPEND, "Paragraph number too large, but not EE_PARA_APPEND!" );
        nPara = GetParagraphCount();
    }

    pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );

    // No Undo componding needed.
    EditPaM aPaM( pImpEditEngine->InsertParagraph( nPara ) );
    // When InsertParagraph from the outside, no hard attributes
    // should be taken over!
    pImpEditEngine->RemoveCharAttribs( nPara );
    pImpEditEngine->InsertText( rTxtObj, EditSelection( aPaM, aPaM ) );

    pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );

    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::InsertParagraph( sal_Int32 nPara, const XubString& rTxt )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( nPara > GetParagraphCount() )
    {
        DBG_ASSERTWARNING( nPara == EE_PARA_APPEND, "Paragraph number too large, but not EE_PARA_APPEND!" );
        nPara = GetParagraphCount();
    }

    pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM( pImpEditEngine->InsertParagraph( nPara ) );
    // When InsertParagraph from the outside, no hard attributes
    // should be taken over!
    pImpEditEngine->RemoveCharAttribs( nPara );
    pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
    pImpEditEngine->ImpInsertText( EditSelection( aPaM, aPaM ), rTxt );
    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::SetText( sal_Int32 nPara, const XubString& rTxt )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection* pSel = pImpEditEngine->SelectParagraph( nPara );
    if ( pSel )
    {
        pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
        pImpEditEngine->ImpInsertText( *pSel, rTxt );
        pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
        pImpEditEngine->FormatAndUpdate();
        delete pSel;
    }
}

void EditEngine::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetParaAttribs( nPara, rSet );
    pImpEditEngine->FormatAndUpdate();
}

const SfxItemSet& EditEngine::GetParaAttribs( sal_Int32 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetParaAttribs( nPara );
}

sal_Bool EditEngine::HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->HasParaAttrib( nPara, nWhich );
}

const SfxPoolItem& EditEngine::GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetParaAttrib( nPara, nWhich );
}

void EditEngine::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->GetCharAttribs( nPara, rLst );
}

SfxItemSet EditEngine::GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
    return pImpEditEngine->GetAttribs( aSel, bOnlyHardAttrib );
}

SfxItemSet EditEngine::GetAttribs( sal_Int32 nPara, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt8 nFlags ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetAttribs( nPara, nStart, nEnd, nFlags );
}

void EditEngine::RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditEngine, 0 );

    pImpEditEngine->UndoActionStart( EDITUNDO_RESETATTRIBS );
    EditSelection aSel( pImpEditEngine->ConvertSelection( rSelection.nStartPara, rSelection.nStartPos, rSelection.nEndPara, rSelection.nEndPos ) );
    pImpEditEngine->RemoveCharAttribs( aSel, bRemoveParaAttribs, nWhich  );
    pImpEditEngine->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    pImpEditEngine->FormatAndUpdate();
}

Font EditEngine::GetStandardFont( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return GetStandardSvxFont( nPara );
}

SvxFont EditEngine::GetStandardSvxFont( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    return pNode->GetCharAttribs().GetDefFont();
}

void EditEngine::StripPortions()
{
    DBG_CHKTHIS( EditEngine, 0 );
    VirtualDevice aTmpDev;
    Rectangle aBigRect( Point( 0, 0 ), Size( 0x7FFFFFFF, 0x7FFFFFFF ) );
    if ( IsVertical() )
    {
        aBigRect.Right() = 0;
        aBigRect.Left() = -0x7FFFFFFF;
    }
    pImpEditEngine->Paint( &aTmpDev, aBigRect, Point(), sal_True );
}

void EditEngine::GetPortions( sal_Int32 nPara, std::vector<sal_uInt16>& rList )
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatFullDoc();

    const ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nPara );
    if ( pParaPortion )
    {
        sal_uInt16 nEnd = 0;
        sal_uInt16 nTextPortions = pParaPortion->GetTextPortions().Count();
        for ( sal_uInt16 n = 0; n < nTextPortions; n++ )
        {
            nEnd = nEnd + pParaPortion->GetTextPortions()[n]->GetLen();
            rList.push_back( nEnd );
        }
    }
}

void EditEngine::SetFlatMode( sal_Bool bFlat)
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetFlatMode( bFlat );
}

sal_Bool EditEngine::IsFlatMode() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return !( pImpEditEngine->aStatus.UseCharAttribs() );
}

void EditEngine::SetControlWord( sal_uInt32 nWord )
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( nWord != pImpEditEngine->aStatus.GetControlWord() )
    {
        sal_uInt32 nPrev = pImpEditEngine->aStatus.GetControlWord();
        pImpEditEngine->aStatus.GetControlWord() = nWord;

        sal_uInt32 nChanges = nPrev ^ nWord;
        if ( pImpEditEngine->IsFormatted() )
        {
            // possibly reformat:
            if ( ( nChanges & EE_CNTRL_USECHARATTRIBS ) ||
                 ( nChanges & EE_CNTRL_USEPARAATTRIBS ) ||
                 ( nChanges & EE_CNTRL_ONECHARPERLINE ) ||
                 ( nChanges & EE_CNTRL_STRETCHING ) ||
                 ( nChanges & EE_CNTRL_OUTLINER ) ||
                 ( nChanges & EE_CNTRL_NOCOLORS ) ||
                 ( nChanges & EE_CNTRL_OUTLINER2 ) )
            {
                if ( ( nChanges & EE_CNTRL_USECHARATTRIBS ) ||
                     ( nChanges & EE_CNTRL_USEPARAATTRIBS ) )
                {
                    sal_Bool bUseCharAttribs = ( nWord & EE_CNTRL_USECHARATTRIBS ) ? sal_True : sal_False;
                    pImpEditEngine->GetEditDoc().CreateDefFont( bUseCharAttribs );
                }

                pImpEditEngine->FormatFullDoc();
                pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );
            }
        }

        sal_Bool bSpellingChanged = nChanges & EE_CNTRL_ONLINESPELLING ? sal_True : sal_False;

        if ( bSpellingChanged )
        {
            pImpEditEngine->StopOnlineSpellTimer();
            if ( bSpellingChanged && ( nWord & EE_CNTRL_ONLINESPELLING ) )
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
                    bool bWrongs = ( bSpellingChanged || ( nWord & EE_CNTRL_ONLINESPELLING ) ) ? !pNode->GetWrongList()->empty() : false;
                    if ( bSpellingChanged )
                        pNode->DestroyWrongList();
                    if ( bWrongs )
                    {
                        pImpEditEngine->aInvalidRect.Left() = 0;
                        pImpEditEngine->aInvalidRect.Right() = pImpEditEngine->GetPaperSize().Width();
                        pImpEditEngine->aInvalidRect.Top() = nY+1;
                        pImpEditEngine->aInvalidRect.Bottom() = nY+pPortion->GetHeight()-1;
                        pImpEditEngine->UpdateViews( pImpEditEngine->pActiveView );
                    }
                    nY += pPortion->GetHeight();
                }
            }
        }
    }
}

sal_uInt32 EditEngine::GetControlWord() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aStatus.GetControlWord();
}

long EditEngine::GetFirstLineStartX( sal_Int32 nParagraph )
{
    DBG_CHKTHIS( EditEngine, 0 );

    long nX = 0;
    const ParaPortion* pPPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nParagraph );
    if ( pPPortion )
    {
        DBG_ASSERT( pImpEditEngine->IsFormatted() || !pImpEditEngine->IsFormatting(), "GetFirstLineStartX: Doc not formatted - unable to format!" );
        if ( !pImpEditEngine->IsFormatted() )
            pImpEditEngine->FormatDoc();
        const EditLine* pFirstLine = pPPortion->GetLines()[0];
        nX = pFirstLine->GetStartPosX();
    }
    return nX;
}

Point EditEngine::GetDocPos( const Point& rPaperPos ) const
{
    Point aDocPos( rPaperPos );
    if ( IsVertical() )
    {
        aDocPos.X() = rPaperPos.Y();
        aDocPos.Y() = GetPaperSize().Width() - rPaperPos.X();
    }
    return aDocPos;
}

Point EditEngine::GetDocPosTopLeft( sal_Int32 nParagraph )
{
    DBG_CHKTHIS( EditEngine, 0 );
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
            const EditLine* pFirstLine = pPPortion->GetLines()[0];
            aPoint.X() = pFirstLine->GetStartPosX();
        }
        else
        {
            const SvxLRSpaceItem& rLRItem = pImpEditEngine->GetLRSpaceItem( pPPortion->GetNode() );
// TL_NF_LR         aPoint.X() = pImpEditEngine->GetXValue( (short)(rLRItem.GetTxtLeft() + rLRItem.GetTxtFirstLineOfst()) );
            sal_Int32 nSpaceBefore = 0;
            pImpEditEngine->GetSpaceBeforeAndMinLabelWidth( pPPortion->GetNode(), &nSpaceBefore );
            short nX = (short)(rLRItem.GetTxtLeft()
                            + rLRItem.GetTxtFirstLineOfst()
                            + nSpaceBefore);
            aPoint.X() = pImpEditEngine->GetXValue( nX
                             );
        }
        aPoint.Y() = pImpEditEngine->GetParaPortions().GetYOffset( pPPortion );
    }
    return aPoint;
}

const SvxNumberFormat* EditEngine::GetNumberFormat( sal_Int32 nPara ) const
{
    // derived objects may overload this function to give access to
    // bullet information (see Outliner)
    (void) nPara;
    return 0;
}

sal_Bool EditEngine::IsRightToLeft( sal_Int32 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsRightToLeft( nPara );
}

sal_Bool EditEngine::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder )
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_Bool bTextPos = sal_False;
    // take unrotated positions for calculation here
    Point aDocPos = GetDocPos( rPaperPos );

    if ( ( aDocPos.Y() > 0  ) && ( aDocPos.Y() < (long)pImpEditEngine->GetTextHeight() ) )
    {
        EditPaM aPaM = pImpEditEngine->GetPaM( aDocPos, sal_False );
        if ( aPaM.GetNode() )
        {
            const ParaPortion* pParaPortion = pImpEditEngine->FindParaPortion( aPaM.GetNode() );
            DBG_ASSERT( pParaPortion, "ParaPortion?" );

            sal_uInt16 nLine = pParaPortion->GetLineNumber( aPaM.GetIndex() );
            const EditLine* pLine = pParaPortion->GetLines()[nLine];
            Range aLineXPosStartEnd = pImpEditEngine->GetLineXPosStartEnd( pParaPortion, pLine );
            if ( ( aDocPos.X() >= aLineXPosStartEnd.Min() - nBorder ) &&
                 ( aDocPos.X() <= aLineXPosStartEnd.Max() + nBorder ) )
            {
                 bTextPos = sal_True;
            }
        }
    }
    return bTextPos;
}

void EditEngine::SetEditTextObjectPool( SfxItemPool* pPool )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetEditTextObjectPool( pPool );
}

SfxItemPool* EditEngine::GetEditTextObjectPool() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditTextObjectPool();
}

void EditEngine::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->SetAttribs( aSel, rSet );
}

void EditEngine::QuickMarkInvalid( const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( rSel.nStartPara < pImpEditEngine->GetEditDoc().Count(), "MarkInvalid: Start out of Range!" );
    DBG_ASSERT( rSel.nEndPara < pImpEditEngine->GetEditDoc().Count(), "MarkInvalid: End out of Range!" );
    for ( sal_Int32 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nPara );
        if ( pPortion )
            pPortion->MarkSelectionInvalid( 0, pPortion->GetNode()->Len() );
    }
}

void EditEngine::QuickInsertText( const XubString& rText, const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpInsertText( aSel, rText );
}

void EditEngine::QuickDelete( const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpDeleteSelection( aSel );
}

void EditEngine::QuickMarkToBeRepainted( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
    ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SafeGetObject( nPara );
    if ( pPortion )
        pPortion->SetMustRepaint( sal_True );
}

void EditEngine::QuickInsertLineBreak( const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->InsertLineBreak( aSel );
}

void EditEngine::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpInsertFeature( aSel, rFld );
}

void EditEngine::QuickFormatDoc( sal_Bool bFull )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( bFull )
        pImpEditEngine->FormatFullDoc();
    else
        pImpEditEngine->FormatDoc();

    // Don't pass active view, maybe selection is not updated yet...
    pImpEditEngine->UpdateViews( NULL );
}

void EditEngine::QuickRemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->RemoveCharAttribs( nPara, nWhich );
}

void EditEngine::SetStyleSheet(const EditSelection& aSel, SfxStyleSheet* pStyle)
{
    pImpEditEngine->SetStyleSheet(aSel, pStyle);
}

void EditEngine::SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetStyleSheet( nPara, pStyle );
}

const SfxStyleSheet* EditEngine::GetStyleSheet( sal_Int32 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStyleSheet( nPara );
}

SfxStyleSheet* EditEngine::GetStyleSheet( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStyleSheet( nPara );
}

void EditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetStyleSheetPool( pSPool );
}

SfxStyleSheetPool* EditEngine::GetStyleSheetPool()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStyleSheetPool();
}

void EditEngine::SetWordDelimiters( const OUString& rDelimiters )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->aWordDelimiters = rDelimiters;
    if (pImpEditEngine->aWordDelimiters.indexOf(CH_FEATURE) == -1)
        pImpEditEngine->aWordDelimiters += OUString(CH_FEATURE);
}

OUString EditEngine::GetWordDelimiters() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aWordDelimiters;
}

void EditEngine::EraseVirtualDevice()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->EraseVirtualDevice();
}

void EditEngine::SetSpeller( Reference< XSpellChecker1 >  &xSpeller )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetSpeller( xSpeller );
}

Reference< XSpellChecker1 >  EditEngine::GetSpeller()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetSpeller();
}

void EditEngine::SetHyphenator( Reference< XHyphenator > & xHyph )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetHyphenator( xHyph );
}

void EditEngine::SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetForbiddenCharsTable( xForbiddenChars );
}

void EditEngine::SetDefaultLanguage( LanguageType eLang )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetDefaultLanguage( eLang );
}

LanguageType EditEngine::GetDefaultLanguage() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetDefaultLanguage();
}

sal_Bool EditEngine::SpellNextDocument()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return sal_False;
}

EESpellState EditEngine::HasSpellErrors()
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->GetSpeller().is()  )
        return EE_SPELL_NOSPELLER;

    return pImpEditEngine->HasSpellErrors();
}

void EditEngine::StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc)
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->StartSpelling(rEditView, bMultipleDoc);
}

bool EditEngine::SpellSentence(EditView& rView, ::svx::SpellPortions& rToFill, bool bIsGrammarChecking )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->SpellSentence( rView, rToFill, bIsGrammarChecking );
}

void EditEngine::PutSpellingToSentenceStart( EditView& rEditView )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->PutSpellingToSentenceStart( rEditView );
}

void EditEngine::ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions, bool bRecheck )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->ApplyChangedSentence( rEditView, rNewPortions, bRecheck  );
}

sal_Bool EditEngine::HasConvertibleTextPortion( LanguageType nLang )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->HasConvertibleTextPortion( nLang );
}

sal_Bool EditEngine::ConvertNextDocument()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return sal_False;
}

sal_Bool EditEngine::HasText( const SvxSearchItem& rSearchItem )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->HasText( rSearchItem );
}

void EditEngine::SetGlobalCharStretching( sal_uInt16 nX, sal_uInt16 nY )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetCharStretching( nX, nY );
}

void EditEngine::GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->GetCharStretching( rX, rY );
}

sal_Bool EditEngine::ShouldCreateBigTextObject() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    sal_uInt32 nTextPortions = 0;
    sal_Int32 nParas = pImpEditEngine->GetEditDoc().Count();
    for ( sal_Int32 nPara = 0; nPara < nParas; nPara++  )
    {
        ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions()[nPara];
        nTextPortions = nTextPortions + pParaPortion->GetTextPortions().Count();
    }
    return ( nTextPortions >= pImpEditEngine->GetBigTextObjectStart() ) ? sal_True : sal_False;
}

sal_uInt16 EditEngine::GetFieldCount( sal_Int32 nPara ) const
{
    sal_uInt16 nFields = 0;
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
    if ( pNode )
    {
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        CharAttribList::AttribsType::const_iterator it = rAttrs.begin(), itEnd = rAttrs.end();
        for (; it != itEnd; ++it)
        {
            if (it->Which() == EE_FEATURE_FIELD)
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
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        CharAttribList::AttribsType::const_iterator it = rAttrs.begin(), itEnd = rAttrs.end();
        for (; it != itEnd; ++it)
        {
            const EditCharAttrib& rAttr = *it;
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
    DBG_CHKTHIS( EditEngine, 0 );
    sal_Bool bChanges = pImpEditEngine->UpdateFields();
    if ( bChanges )
        pImpEditEngine->FormatAndUpdate();
    return bChanges;
}

bool EditEngine::UpdateFieldsOnly()
{
    return pImpEditEngine->UpdateFields();
}

void EditEngine::RemoveFields( sal_Bool bKeepFieldText, TypeId aType )
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( bKeepFieldText )
        pImpEditEngine->UpdateFields();

    sal_Int32 nParas = pImpEditEngine->GetEditDoc().Count();
    for ( sal_Int32 nPara = 0; nPara < nParas; nPara++  )
    {
        ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for (size_t nAttr = rAttrs.size(); nAttr; )
        {
            const EditCharAttrib& rAttr = rAttrs[--nAttr];
            if (rAttr.Which() == EE_FEATURE_FIELD)
            {
                const SvxFieldData* pFldData = static_cast<const SvxFieldItem*>(rAttr.GetItem())->GetField();
                if ( pFldData && ( !aType || ( pFldData->IsA( aType ) ) ) )
                {
                    DBG_ASSERT( dynamic_cast<const SvxFieldItem*>(rAttr.GetItem()), "no field item..." );
                    EditSelection aSel( EditPaM(pNode, rAttr.GetStart()), EditPaM(pNode, rAttr.GetEnd()) );
                    String aFieldText = static_cast<const EditCharAttribField&>(rAttr).GetFieldValue();
                    pImpEditEngine->ImpInsertText( aSel, aFieldText );
                }
            }
        }
    }
}

sal_Bool EditEngine::HasOnlineSpellErrors() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    sal_Int32 nNodes = pImpEditEngine->GetEditDoc().Count();
    for ( sal_Int32 n = 0; n < nNodes; n++ )
    {
        ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( n );
        if ( pNode->GetWrongList() && !pNode->GetWrongList()->empty() )
            return sal_True;
    }
    return sal_False;
}

void EditEngine::CompleteOnlineSpelling()
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( pImpEditEngine->GetStatus().DoOnlineSpelling() )
    {
        if( !pImpEditEngine->IsFormatted() )
            pImpEditEngine->FormatAndUpdate();

        pImpEditEngine->StopOnlineSpellTimer();
        pImpEditEngine->DoOnlineSpelling( 0, sal_True, sal_False );
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
    EditPaM aPaM = ((EditEngine*)this)->pImpEditEngine->GetPaM( rDocPos, sal_False );
    if ( aPaM.GetNode() )
    {
        aPos.nPara = pImpEditEngine->aEditDoc.GetPos( aPaM.GetNode() );
        aPos.nIndex = aPaM.GetIndex();
    }
    return aPos;
}

Rectangle EditEngine::GetCharacterBounds( const EPosition& rPos ) const
{
    Rectangle aBounds;
    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( rPos.nPara );

    // Check against index, not paragraph
    if ( pNode && ( rPos.nIndex < pNode->Len() ) )
    {
        aBounds = pImpEditEngine->PaMtoEditCursor( EditPaM( pNode, rPos.nIndex ), GETCRSR_TXTONLY );
        Rectangle aR2 = pImpEditEngine->PaMtoEditCursor( EditPaM( pNode, rPos.nIndex+1 ), GETCRSR_TXTONLY|GETCRSR_ENDOFLINE );
        if ( aR2.Right() > aBounds.Right() )
            aBounds.Right() = aR2.Right();
    }
    return aBounds;
}

ParagraphInfos EditEngine::GetParagraphInfos( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );

    // This only works if not already in the format ...
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    ParagraphInfos aInfos;
    aInfos.bValid = pImpEditEngine->IsFormatted();
    if ( pImpEditEngine->IsFormatted() )
    {
        const ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions()[nPara];
        const EditLine* pLine = (pParaPortion && pParaPortion->GetLines().Count()) ?
                pParaPortion->GetLines()[0] : NULL;
        DBG_ASSERT( pParaPortion && pLine, "GetParagraphInfos - Paragraph out of range" );
        if ( pParaPortion && pLine )
        {
            aInfos.nParaHeight = (sal_uInt16)pParaPortion->GetHeight();
            aInfos.nLines = pParaPortion->GetLines().Count();
            aInfos.nFirstLineStartX = pLine->GetStartPosX();
            aInfos.nFirstLineOffset = pParaPortion->GetFirstLineOffset();
            aInfos.nFirstLineHeight = pLine->GetHeight();
            aInfos.nFirstLineTextHeight = pLine->GetTxtHeight();
            aInfos.nFirstLineMaxAscent = pLine->GetMaxAscent();
        }
    }
    return aInfos;
}

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >
                    EditEngine::CreateTransferable( const ESelection& rSelection ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->CreateSel( rSelection ) );
    return pImpEditEngine->CreateTransferable( aSel );
}

// =====================================================================
// ======================    Virtual Methods    ========================
// =====================================================================
void EditEngine::DrawingText( const Point&, const OUString&, sal_uInt16, sal_uInt16,
    const sal_Int32*, const SvxFont&, sal_Int32, sal_uInt16, sal_uInt8,
    const EEngineData::WrongSpellVector*, const SvxFieldData*, bool, bool, bool,
    const ::com::sun::star::lang::Locale*, const Color&, const Color&)

{
    DBG_CHKTHIS( EditEngine, 0 );
}

void EditEngine::DrawingTab( const Point& /*rStartPos*/, long /*nWidth*/,
                                      const String& /*rChar*/, const SvxFont& /*rFont*/,
                                      sal_Int32 /*nPara*/, xub_StrLen /*nIndex*/,
                                      sal_uInt8 /*nRightToLeft*/, bool /*bEndOfLine*/,
                                      bool /*bEndOfParagraph*/, const Color& /*rOverlineColor*/,
                                      const Color& /*rTextLineColor*/)
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void EditEngine::PaintingFirstLine( sal_Int32, const Point&, long, const Point&, short, OutputDevice* )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void EditEngine::ParagraphInserted( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_PARAGRAPHINSERTED );
        aNotify.pEditEngine = this;
        aNotify.nParagraph = nPara;
        pImpEditEngine->CallNotify( aNotify );
    }
}

void EditEngine::ParagraphDeleted( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_PARAGRAPHREMOVED );
        aNotify.pEditEngine = this;
        aNotify.nParagraph = nPara;
        pImpEditEngine->CallNotify( aNotify );
    }
}
void EditEngine::ParagraphConnected( sal_Int32 /*nLeftParagraph*/, sal_Int32 /*nRightParagraph*/ )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

sal_Bool EditEngine::FormattingParagraph( sal_Int32 )
{
    // return sal_True, if the Attribute was changed ...
    DBG_CHKTHIS( EditEngine, 0 );
    return sal_False;
}

void EditEngine::ParaAttribsChanged( sal_Int32 /* nParagraph */ )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void EditEngine::StyleSheetChanged( SfxStyleSheet* /* pStyle */ )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void EditEngine::ParagraphHeightChanged( sal_Int32 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_TEXTHEIGHTCHANGED );
        aNotify.pEditEngine = this;
        aNotify.nParagraph = nPara;
        pImpEditEngine->CallNotify( aNotify );
    }
}

OUString EditEngine::GetUndoComment( sal_uInt16 nId ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    XubString aComment;
    switch ( nId )
    {
        case EDITUNDO_REMOVECHARS:
        case EDITUNDO_CONNECTPARAS:
        case EDITUNDO_REMOVEFEATURE:
        case EDITUNDO_DELCONTENT:
        case EDITUNDO_DELETE:
        case EDITUNDO_CUT:
            aComment = EE_RESSTR(RID_EDITUNDO_DEL);
        break;
        case EDITUNDO_MOVEPARAGRAPHS:
        case EDITUNDO_MOVEPARAS:
        case EDITUNDO_DRAGANDDROP:
            aComment = EE_RESSTR(RID_EDITUNDO_MOVE);
        break;
        case EDITUNDO_INSERTFEATURE:
        case EDITUNDO_SPLITPARA:
        case EDITUNDO_INSERTCHARS:
        case EDITUNDO_PASTE:
        case EDITUNDO_INSERT:
        case EDITUNDO_READ:
            aComment = EE_RESSTR(RID_EDITUNDO_INSERT);
        break;
        case EDITUNDO_SRCHANDREPL:
        case EDITUNDO_REPLACEALL:
            aComment = EE_RESSTR(RID_EDITUNDO_REPLACE);
        break;
        case EDITUNDO_ATTRIBS:
        case EDITUNDO_PARAATTRIBS:
        case EDITUNDO_STRETCH:
            aComment = EE_RESSTR(RID_EDITUNDO_SETATTRIBS);
        break;
        case EDITUNDO_RESETATTRIBS:
            aComment = EE_RESSTR(RID_EDITUNDO_RESETATTRIBS);
        break;
        case EDITUNDO_STYLESHEET:
            aComment = EE_RESSTR(RID_EDITUNDO_SETSTYLE);
        break;
        case EDITUNDO_TRANSLITERATE:
            aComment = EE_RESSTR(RID_EDITUNDO_TRANSLITERATE);
        break;
        case EDITUNDO_INDENTBLOCK:
        case EDITUNDO_UNINDENTBLOCK:
            aComment = EE_RESSTR(RID_EDITUNDO_INDENT);
        break;
    }
    return aComment;
}

Rectangle EditEngine::GetBulletArea( sal_Int32 )
{
    return Rectangle( Point(), Point() );
}

OUString EditEngine::CalcFieldValue( const SvxFieldItem&, sal_Int32, sal_uInt16, Color*&, Color*& )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return OUString(' ');
}

void EditEngine::FieldClicked( const SvxFieldItem&, sal_Int32, sal_uInt16 )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void EditEngine::FieldSelected( const SvxFieldItem&, sal_Int32, sal_uInt16 )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

// =====================================================================
// ======================     Static Methods     =======================
// =====================================================================
SfxItemPool* EditEngine::CreatePool( sal_Bool bPersistentRefCounts )
{
    SfxItemPool* pPool = new EditEngineItemPool( bPersistentRefCounts );
    return pPool;
}

SfxItemPool& EditEngine::GetGlobalItemPool()
{
    if ( !pGlobalPool )
        pGlobalPool = CreatePool();
    return *pGlobalPool;
}

void EditEngine::SetFontInfoInItemSet( SfxItemSet& rSet, const Font& rFont )
{
    SvxFont aSvxFont( rFont );
    SetFontInfoInItemSet( rSet, aSvxFont );

}

void EditEngine::SetFontInfoInItemSet( SfxItemSet& rSet, const SvxFont& rFont )
{
    rSet.Put( SvxLanguageItem( rFont.GetLanguage(), EE_CHAR_LANGUAGE ) );
    rSet.Put( SvxFontItem( rFont.GetFamily(), rFont.GetName(), XubString(), rFont.GetPitch(), rFont.GetCharSet(), EE_CHAR_FONTINFO ) );
    rSet.Put( SvxFontHeightItem( rFont.GetSize().Height(), 100, EE_CHAR_FONTHEIGHT )  );
    rSet.Put( SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH ) );
    rSet.Put( SvxShadowedItem( rFont.IsShadow(), EE_CHAR_SHADOW )  );
    rSet.Put( SvxEscapementItem( rFont.GetEscapement(), rFont.GetPropr(), EE_CHAR_ESCAPEMENT )  );
    rSet.Put( SvxWeightItem( rFont.GetWeight(), EE_CHAR_WEIGHT )  );
    rSet.Put( SvxColorItem( rFont.GetColor(), EE_CHAR_COLOR )  );
    rSet.Put( SvxUnderlineItem( rFont.GetUnderline(), EE_CHAR_UNDERLINE )  );
    rSet.Put( SvxOverlineItem( rFont.GetOverline(), EE_CHAR_OVERLINE )  );
    rSet.Put( SvxCrossedOutItem( rFont.GetStrikeout(), EE_CHAR_STRIKEOUT )  );
    rSet.Put( SvxPostureItem( rFont.GetItalic(), EE_CHAR_ITALIC )  );
    rSet.Put( SvxContourItem( rFont.IsOutline(), EE_CHAR_OUTLINE )  );
    rSet.Put( SvxAutoKernItem( rFont.IsKerning(), EE_CHAR_PAIRKERNING ) );
    rSet.Put( SvxKerningItem( rFont.GetFixKerning(), EE_CHAR_KERNING ) );
    rSet.Put( SvxWordLineModeItem( rFont.IsWordLineMode(), EE_CHAR_WLM ) );
    rSet.Put( SvxEmphasisMarkItem( rFont.GetEmphasisMark(), EE_CHAR_EMPHASISMARK ) );
    rSet.Put( SvxCharReliefItem( rFont.GetRelief(), EE_CHAR_RELIEF ) );
}

Font EditEngine::CreateFontFromItemSet( const SfxItemSet& rItemSet, sal_uInt16 nScriptType )
{
    SvxFont aFont;
    CreateFont( aFont, rItemSet, true, nScriptType );
    return aFont;
}

SvxFont EditEngine::CreateSvxFontFromItemSet( const SfxItemSet& rItemSet )
{
    SvxFont aFont;
    CreateFont( aFont, rItemSet );
    return aFont;
}

sal_Bool EditEngine::DoesKeyMoveCursor( const KeyEvent& rKeyEvent )
{
    sal_Bool bDoesMove = sal_False;

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
                bDoesMove = sal_True;
        }
        break;
    }
    return bDoesMove;
}

sal_Bool EditEngine::DoesKeyChangeText( const KeyEvent& rKeyEvent )
{
    sal_Bool bDoesChange = sal_False;

    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_UNDO:
            case KEYFUNC_REDO:
            case KEYFUNC_CUT:
            case KEYFUNC_PASTE: bDoesChange = sal_True;
            break;
            default:    // is then possibly edited below.
                        eFunc = KEYFUNC_DONTKNOW;
        }
    }
    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( rKeyEvent.GetKeyCode().GetCode() )
        {
            case KEY_DELETE:
            case KEY_BACKSPACE: bDoesChange = sal_True;
            break;
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    bDoesChange = sal_True;
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

sal_Bool EditEngine::IsSimpleCharInput( const KeyEvent& rKeyEvent )
{
    if( EditEngine::IsPrintable( rKeyEvent.GetCharCode() ) &&
        ( KEY_MOD2 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT ) ) &&
        ( KEY_MOD1 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT ) ) )
    {
        return sal_True;
    }
    return sal_False;
}

sal_Bool EditEngine::HasValidData( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rTransferable )
{
    sal_Bool bValidData = sal_False;

    if ( rTransferable.is() )
    {
        // Every application that copies rtf or any other text format also copies plain text into the clipboard....
        datatransfer::DataFlavor aFlavor;
        SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
        bValidData = rTransferable->isDataFlavorSupported( aFlavor );
    }

    return bValidData;
}

/** sets a link that is called at the beginning of a drag operation at an edit view */
void EditEngine::SetBeginDropHdl( const Link& rLink )
{
    pImpEditEngine->SetBeginDropHdl( rLink );
}

Link EditEngine::GetBeginDropHdl() const
{
    return pImpEditEngine->GetBeginDropHdl();
}

/** sets a link that is called at the end of a drag operation at an edit view */
void EditEngine::SetEndDropHdl( const Link& rLink )
{
    pImpEditEngine->SetEndDropHdl( rLink );
}

Link EditEngine::GetEndDropHdl() const
{
    return pImpEditEngine->GetEndDropHdl();
}

void EditEngine::SetFirstWordCapitalization( sal_Bool bCapitalize )
{
    pImpEditEngine->SetFirstWordCapitalization( bCapitalize );
}

bool EditEngine::IsImportHandlerSet() const
{
    return pImpEditEngine->aImportHdl.IsSet();
}

bool EditEngine::IsImportRTFStyleSheetsSet() const
{
    return pImpEditEngine->GetStatus().DoImportRTFStyleSheets();
}

void EditEngine::CallImportHandler(ImportInfo& rInfo)
{
    pImpEditEngine->aImportHdl.Call(&rInfo);
}

EditPaM EditEngine::InsertParaBreak(
        const EditSelection& rEditSelection, bool bKeepEndingAttribs)
{
    return pImpEditEngine->ImpInsertParaBreak(rEditSelection, bKeepEndingAttribs);
}

EditPaM EditEngine::InsertLineBreak(const EditSelection& rEditSelection)
{
    return pImpEditEngine->InsertLineBreak(rEditSelection);
}

EFieldInfo::EFieldInfo()
{
    pFieldItem = NULL;
}


EFieldInfo::EFieldInfo( const SvxFieldItem& rFieldItem, sal_Int32 nPara, sal_uInt16 nPos ) : aPosition( nPara, nPos )
{
    pFieldItem = new SvxFieldItem( rFieldItem );
}

EFieldInfo::~EFieldInfo()
{
    delete pFieldItem;
}

EFieldInfo::EFieldInfo( const EFieldInfo& rFldInfo )
{
    *this = rFldInfo;
}

EFieldInfo& EFieldInfo::operator= ( const EFieldInfo& rFldInfo )
{
    if( this == &rFldInfo )
        return *this;

    pFieldItem = rFldInfo.pFieldItem ? new SvxFieldItem( *rFldInfo.pFieldItem ) : 0;
    aCurrentText = rFldInfo.aCurrentText;
    aPosition = rFldInfo.aPosition;

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
