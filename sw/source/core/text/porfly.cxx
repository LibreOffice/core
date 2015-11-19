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

#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "pam.hxx"
#include <portab.hxx>
#include "flyfrm.hxx"
#include "rootfrm.hxx"
#include "frmfmt.hxx"
#include "viewsh.hxx"
#include "textboxhelper.hxx"

#include <vcl/outdev.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include "flyfrms.hxx"
#include "txatbase.hxx"
#include "porfly.hxx"
#include "porlay.hxx"
#include "inftxt.hxx"

#include <sortedobjs.hxx>

/**
 * class SwFlyPortion => we expect a frame-locale SwRect!
 */

void SwFlyPortion::Paint( const SwTextPaintInfo& ) const
{
}

bool SwFlyPortion::Format( SwTextFormatInfo &rInf )
{
    OSL_ENSURE( Fix() >= rInf.X(), "SwFlyPortion::Format" );

    // tabs must be expanded
    if( rInf.GetLastTab() )
        rInf.GetLastTab()->FormatEOL( rInf );

    rInf.GetLast()->FormatEOL( rInf );
    PrtWidth( static_cast<sal_uInt16>(Fix() - rInf.X() + PrtWidth()) );
    if( !Width() )
    {
        OSL_ENSURE( Width(), "+SwFlyPortion::Format: a fly is a fly is a fly" );
        Width(1);
    }

    // resetting
    rInf.SetFly( nullptr );
    rInf.Width( rInf.RealWidth() );
    rInf.GetParaPortion()->SetFly();

    // trailing blank:
    if( rInf.GetIdx() < rInf.GetText().getLength() &&  1 < rInf.GetIdx()
        && !rInf.GetRest()
        && ' ' == rInf.GetChar( rInf.GetIdx() )
        && ' ' != rInf.GetChar( rInf.GetIdx() - 1 )
        && ( !rInf.GetLast() || !rInf.GetLast()->IsBreakPortion() ) )
    {
        SetBlankWidth( rInf.GetTextSize(OUString(' ')).Width() );
        SetLen( 1 );
    }

    const sal_uInt16 nNewWidth = static_cast<sal_uInt16>(rInf.X() + PrtWidth());
    if( rInf.Width() <= nNewWidth )
    {
        Truncate();
        if( nNewWidth > rInf.Width() )
        {
            PrtWidth( nNewWidth - rInf.Width() );
            SetFixWidth( PrtWidth() );
        }
        return true;
    }
    return false;
}

bool SwFlyCntPortion::Format( SwTextFormatInfo &rInf )
{
    bool bFull = rInf.Width() < rInf.X() + PrtWidth();

    if( bFull )
    {
        // If the line is full, and the character-bound frame is at
        // the beginning of a line
        // If it is not possible to side step into a Fly
        // "Begin of line" criteria ( ! rInf.X() ) has to be extended.
        // KerningPortions at beginning of line, e.g., for grid layout
        // must be considered.
        const SwLinePortion* pLastPor = rInf.GetLast();
        const sal_uInt16 nLeft = ( pLastPor &&
                                    ( pLastPor->IsKernPortion() ||
                                      pLastPor->IsErgoSumPortion() ) ) ?
                               pLastPor->Width() :
                               0;

        if( nLeft == rInf.X() && ! rInf.GetFly() )
        {
            Width( rInf.Width() );
            bFull = false; // so that notes can still be placed in this line
        }
        else
        {
            if( !rInf.GetFly() )
                rInf.SetNewLine( true );
            Width(0);
            SetAscent(0);
            SetLen(0);
            if( rInf.GetLast() )
                rInf.GetLast()->FormatEOL( rInf );

            return bFull;
        }
    }

    rInf.GetParaPortion()->SetFly();
    return bFull;
}

//TODO: improve documentation
/** move character-bound objects inside the given area
 *
 * This allows moving those objects from Master to Follow, or vice versa.
 *
 * @param pNew
 * @param nStart
 * @param nEnd
 */
void SwTextFrame::MoveFlyInCnt( SwTextFrame *pNew, sal_Int32 nStart, sal_Int32 nEnd )
{
    SwSortedObjs *pObjs = nullptr;
    if ( nullptr != (pObjs = GetDrawObjs()) )
    {
        for ( size_t i = 0; GetDrawObjs() && i < pObjs->size(); ++i )
        {
            // Consider changed type of <SwSortedList> entries
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            const SwFormatAnchor& rAnch = pAnchoredObj->GetFrameFormat().GetAnchor();
            if (rAnch.GetAnchorId() == FLY_AS_CHAR)
            {
                const SwPosition* pPos = rAnch.GetContentAnchor();
                const sal_Int32 nIdx = pPos->nContent.GetIndex();
                if ( nIdx >= nStart && nEnd > nIdx )
                {
                    if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                    {
                        RemoveFly( static_cast<SwFlyFrame*>(pAnchoredObj) );
                        pNew->AppendFly( static_cast<SwFlyFrame*>(pAnchoredObj) );
                    }
                    else if ( dynamic_cast< const SwAnchoredDrawObject *>( pAnchoredObj ) !=  nullptr )
                    {
                        RemoveDrawObj( *pAnchoredObj );
                        pNew->AppendDrawObj( *pAnchoredObj );
                    }
                    --i;
                }
            }
        }
    }
}

sal_Int32 SwTextFrame::CalcFlyPos( SwFrameFormat* pSearch )
{
    SwpHints* pHints = GetTextNode()->GetpSwpHints();
    OSL_ENSURE( pHints, "CalcFlyPos: Why me?" );
    if( !pHints )
        return COMPLETE_STRING;
    SwTextAttr* pFound = nullptr;
    for ( size_t i = 0; i < pHints->Count(); ++i )
    {
        SwTextAttr *pHt = pHints->Get( i );
        if( RES_TXTATR_FLYCNT == pHt->Which() )
        {
            SwFrameFormat* pFrameFormat = pHt->GetFlyCnt().GetFrameFormat();
            if( pFrameFormat == pSearch )
                pFound = pHt;
        }
    }
    OSL_ENSURE( pHints, "CalcFlyPos: Not Found!" );
    if( !pFound )
        return COMPLETE_STRING;
    return pFound->GetStart();
}

void SwFlyCntPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( bDraw )
    {
        if( !static_cast<SwDrawContact*>(pContact)->GetAnchorFrame() )
        {
            // No direct positioning of the drawing object is needed
            SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(pContact);
            pDrawContact->ConnectToLayout();
        }
    }
    else
    {
        // Baseline output
        // Re-paint everything at a CompletePaint call
        SwRect aRepaintRect( rInf.GetPaintRect() );

        if ( rInf.GetTextFrame()->IsRightToLeft() )
            rInf.GetTextFrame()->SwitchLTRtoRTL( aRepaintRect );

        if ( rInf.GetTextFrame()->IsVertical() )
            rInf.GetTextFrame()->SwitchHorizontalToVertical( aRepaintRect );

        if( (GetFlyFrame()->IsCompletePaint() ||
             GetFlyFrame()->Frame().IsOver( aRepaintRect )) &&
             SwFlyFrame::IsPaint( const_cast<SwVirtFlyDrawObj*>(GetFlyFrame()->GetVirtDrawObj()),
                                GetFlyFrame()->getRootFrame()->GetCurrShell() ))
        {
            SwRect aRect( GetFlyFrame()->Frame() );
            if( !GetFlyFrame()->IsCompletePaint() )
                aRect._Intersection( aRepaintRect );

            // GetFlyFrame() may change the layout mode at the output device.
            {
                SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
                GetFlyFrame()->Paint( const_cast<vcl::RenderContext&>(*rInf.GetOut()), aRect );
            }
            ((SwTextPaintInfo&)rInf).GetRefDev()->SetLayoutMode(
                    rInf.GetOut()->GetLayoutMode() );

            // As the OutputDevice might be anything, the font must be re-selected.
            // Being in const method should not be a problem.
            ((SwTextPaintInfo&)rInf).SelectFont();

            OSL_ENSURE( ! rInf.GetVsh() || rInf.GetVsh()->GetOut() == rInf.GetOut(),
                    "SwFlyCntPortion::Paint: Outdev has changed" );
            if( rInf.GetVsh() )
                ((SwTextPaintInfo&)rInf).SetOut( rInf.GetVsh()->GetOut() );
        }
    }
}

/**
 * Use the dimensions of pFly->OutRect()
 */
SwFlyCntPortion::SwFlyCntPortion( const SwTextFrame& rFrame,
                                  SwFlyInContentFrame *pFly, const Point &rBase,
                                  long nLnAscent, long nLnDescent,
                                  long nFlyAsc, long nFlyDesc,
                                  objectpositioning::AsCharFlags nFlags ) :
    pContact( pFly ),
    bDraw( false ),
    bMax( false ),
    nAlign( 0 )
{
    OSL_ENSURE( pFly, "SwFlyCntPortion::SwFlyCntPortion: no SwFlyInContentFrame!" );
    nLineLength = 1;
    nFlags |= AS_CHAR_ULSPACE | AS_CHAR_INIT;
    SetBase( rFrame, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );
    SetWhichPor( POR_FLYCNT );
}

SwFlyCntPortion::SwFlyCntPortion( const SwTextFrame& rFrame,
                                  SwDrawContact *pDrawContact, const Point &rBase,
                                  long nLnAscent, long nLnDescent,
                                  long nFlyAsc, long nFlyDesc,
                                  objectpositioning::AsCharFlags nFlags ) :
    pContact( pDrawContact ),
    bDraw( true ),
    bMax( false ),
    nAlign( 0 )
{
    OSL_ENSURE( pDrawContact, "SwFlyCntPortion::SwFlyCntPortion: no SwDrawContact!" );
    if( !pDrawContact->GetAnchorFrame() )
    {
        // No direct positioning needed any more
        pDrawContact->ConnectToLayout();

        // Move object to visible layer
        pDrawContact->MoveObjToVisibleLayer( pDrawContact->GetMaster() );
    }
    nLineLength = 1;
    nFlags |= AS_CHAR_ULSPACE | AS_CHAR_INIT;

    SetBase( rFrame, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );

    SetWhichPor( POR_FLYCNT );
}

/**
 * After setting the RefPoints, the ascent needs to be recalculated
 * because it is dependent on RelPos
 *
 * @param rBase CAUTION: needs to be an absolute value!
 */
void SwFlyCntPortion::SetBase( const SwTextFrame& rFrame, const Point &rBase,
                               long nLnAscent, long nLnDescent,
                               long nFlyAsc, long nFlyDesc,
                               objectpositioning::AsCharFlags nFlags )
{
    // Use new class to position object
    // Determine drawing object
    SdrObject* pSdrObj = nullptr;
    if( bDraw )
    {
        // Determine drawing object ('master' or 'virtual') by frame
        pSdrObj = GetDrawContact()->GetDrawObjectByAnchorFrame( rFrame );
        if ( !pSdrObj )
        {
            OSL_FAIL( "SwFlyCntPortion::SetBase(..) - No drawing object found by <GetDrawContact()->GetDrawObjectByAnchorFrame( rFrame )>" );
            pSdrObj = GetDrawContact()->GetMaster();
        }

        // Call <SwAnchoredDrawObject::MakeObjPos()> to assure that flag at
        // the <DrawFrameFormat> and at the <SwAnchoredDrawObject> instance are
        // correctly set
        if ( pSdrObj )
        {
            GetDrawContact()->GetAnchoredObj( pSdrObj )->MakeObjPos();
        }
    }
    else
    {
        pSdrObj = GetFlyFrame()->GetVirtDrawObj();
    }

    if (!pSdrObj)
        return;

    // position object
    objectpositioning::SwAsCharAnchoredObjectPosition aObjPositioning(
                                    *pSdrObj,
                                    rBase, nFlags,
                                    nLnAscent, nLnDescent, nFlyAsc, nFlyDesc );

    // Scope of local variable <aObjPosInProgress>
    {
        SwObjPositioningInProgress aObjPosInProgress( *pSdrObj );
        aObjPositioning.CalcPosition();
    }

    SwFrameFormat* pShape = FindFrameFormat(pSdrObj);
    const SwFormatAnchor& rAnchor(pShape->GetAnchor());
    if (rAnchor.GetAnchorId() == FLY_AS_CHAR)
    {
        // This is an inline draw shape, see if it has a textbox.
        SwFrameFormat* pTextBox = SwTextBoxHelper::findTextBox(pShape);
        if (pTextBox)
        {
            // It has, so look up its text rectangle, and adjust the position
            // of the textbox accordingly.
            Rectangle aTextRectangle = SwTextBoxHelper::getTextRectangle(pShape);

            SwFormatHoriOrient aHori(pTextBox->GetHoriOrient());
            aHori.SetHoriOrient(css::text::HoriOrientation::NONE);
            sal_Int32 nLeft = aTextRectangle.getX() - rFrame.Frame().Left();
            aHori.SetPos(nLeft);

            SwFormatVertOrient aVert(pTextBox->GetVertOrient());
            aVert.SetVertOrient(css::text::VertOrientation::NONE);
            sal_Int32 nTop = aTextRectangle.getY() - rFrame.Frame().Top() - nFlyAsc;
            aVert.SetPos(nTop);

            pTextBox->LockModify();
            pTextBox->SetFormatAttr(aHori);
            pTextBox->SetFormatAttr(aVert);
            pTextBox->UnlockModify();
        }
    }

    SetAlign( aObjPositioning.GetLineAlignment() );

    aRef = aObjPositioning.GetAnchorPos();
    if( nFlags & AS_CHAR_ROTATE )
        SvXSize( aObjPositioning.GetObjBoundRectInclSpacing().SSize() );
    else
        SvLSize( aObjPositioning.GetObjBoundRectInclSpacing().SSize() );
    if( Height() )
    {
        // GetRelPosY returns the relative position to baseline (if 0, the
        // upper border of the FlyCnt if on the baseline of a line)
        SwTwips nRelPos = aObjPositioning.GetRelPosY();
        if ( nRelPos < 0 )
        {
            nAscent = static_cast<sal_uInt16>(-nRelPos);
            if( nAscent > Height() )
                Height( nAscent );
        }
        else
        {
            nAscent = 0;
            Height( Height() + static_cast<sal_uInt16>(nRelPos) );
        }
    }
    else
    {
        Height( 1 );
        nAscent = 0;
    }
}

sal_Int32 SwFlyCntPortion::GetFlyCursorOfst( const sal_uInt16 nOfst,
    const Point &rPoint, SwPosition *pPos, SwCursorMoveState* pCMS ) const
{
    // As the FlyCnt are not attached to the side, their GetCursorOfst() will
    // not be called.
    // In order to reduce management overhead for the layout page, the paragraph
    // calls the FlyFrame's GetCursorOfst() only when needed
    Point aPoint( rPoint );
    if( !pPos || bDraw || !( GetFlyFrame()->GetCursorOfst( pPos, aPoint, pCMS ) ) )
        return SwLinePortion::GetCursorOfst( nOfst );
    else
        return 0;
}

sal_Int32 SwFlyCntPortion::GetCursorOfst( const sal_uInt16 nOfst ) const
{
    // OSL_FAIL("SwFlyCntPortion::GetCursorOfst: use GetFlyCursorOfst()");
    return SwLinePortion::GetCursorOfst( nOfst );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
