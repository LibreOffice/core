/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "pam.hxx"
#include "flyfrm.hxx"
#include "rootfrm.hxx"
#include "frmfmt.hxx"
#include "viewsh.hxx"

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



void SwFlyPortion::Paint( const SwTxtPaintInfo& ) const
{
}

bool SwFlyPortion::Format( SwTxtFormatInfo &rInf )
{
    OSL_ENSURE( Fix() >= rInf.X(), "SwFlyPortion::Format" );
    
    if( rInf.GetLastTab() )
        ((SwLinePortion*)rInf.GetLastTab())->FormatEOL( rInf );

    rInf.GetLast()->FormatEOL( rInf );
    PrtWidth( static_cast<sal_uInt16>(Fix() - rInf.X() + PrtWidth()) );
    if( !Width() )
    {
        OSL_ENSURE( Width(), "+SwFlyPortion::Format: a fly is a fly is a fly" );
        Width(1);
    }

    
    rInf.SetFly( 0 );
    rInf.Width( rInf.RealWidth() );
    rInf.GetParaPortion()->SetFly( true );

    
    if( rInf.GetIdx() < rInf.GetTxt().getLength() &&  1 < rInf.GetIdx()
        && !rInf.GetRest()
        && ' ' == rInf.GetChar( rInf.GetIdx() )
        && ' ' != rInf.GetChar( rInf.GetIdx() - 1 )
        && ( !rInf.GetLast() || !rInf.GetLast()->IsBreakPortion() ) )
    {
        SetBlankWidth( rInf.GetTxtSize(OUString(' ')).Width() );
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

bool SwFlyCntPortion::Format( SwTxtFormatInfo &rInf )
{
    bool bFull = rInf.Width() < rInf.X() + PrtWidth();

    if( bFull )
    {
        
        
        
        
        
        
        const SwLinePortion* pLastPor = rInf.GetLast();
        const sal_uInt16 nLeft = ( pLastPor &&
                                    ( pLastPor->IsKernPortion() ||
                                      pLastPor->IsErgoSumPortion() ) ) ?
                               pLastPor->Width() :
                               0;

        if( nLeft == rInf.X() && ! rInf.GetFly() )
        {
            Width( rInf.Width() );
            bFull = false; 
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

    rInf.GetParaPortion()->SetFly( true );
    return bFull;
}


/** move character-bound objects inside the given area
 *
 * This allows moving those objects from Master to Follow, or vice versa.
 *
 * @param pNew
 * @param nStart
 * @param nEnd
 */
void SwTxtFrm::MoveFlyInCnt( SwTxtFrm *pNew, sal_Int32 nStart, sal_Int32 nEnd )
{
    SwSortedObjs *pObjs = 0L;
    if ( 0 != (pObjs = GetDrawObjs()) )
    {
        for ( sal_uInt32 i = 0; GetDrawObjs() && i < pObjs->Count(); ++i )
        {
            
            
            
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            const SwFmtAnchor& rAnch = pAnchoredObj->GetFrmFmt().GetAnchor();
            if (rAnch.GetAnchorId() == FLY_AS_CHAR)
            {
                const SwPosition* pPos = rAnch.GetCntntAnchor();
                const sal_Int32 nIdx = pPos->nContent.GetIndex();
                if ( nIdx >= nStart && nEnd > nIdx )
                {
                    if ( pAnchoredObj->ISA(SwFlyFrm) )
                    {
                        RemoveFly( static_cast<SwFlyFrm*>(pAnchoredObj) );
                        pNew->AppendFly( static_cast<SwFlyFrm*>(pAnchoredObj) );
                    }
                    else if ( pAnchoredObj->ISA(SwAnchoredDrawObject) )
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

sal_Int32 SwTxtFrm::CalcFlyPos( SwFrmFmt* pSearch )
{
    SwpHints* pHints = GetTxtNode()->GetpSwpHints();
    OSL_ENSURE( pHints, "CalcFlyPos: Why me?" );
    if( !pHints )
        return COMPLETE_STRING;
    SwTxtAttr* pFound = NULL;
    for ( sal_uInt16 i = 0; i < pHints->Count(); i++)
    {
        SwTxtAttr *pHt = pHints->GetTextHint( i );
        if( RES_TXTATR_FLYCNT == pHt->Which() )
        {
            SwFrmFmt* pFrmFmt = pHt->GetFlyCnt().GetFrmFmt();
            if( pFrmFmt == pSearch )
                pFound = pHt;
        }
    }
    OSL_ENSURE( pHints, "CalcFlyPos: Not Found!" );
    if( !pFound )
        return COMPLETE_STRING;
    return *pFound->GetStart();
}

void SwFlyCntPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( bDraw )
    {
        if( !((SwDrawContact*)pContact)->GetAnchorFrm() )
        {
            
            
            SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(pContact);
            pDrawContact->ConnectToLayout();
        }
    }
    else
    {
        
        
        SwRect aRepaintRect( rInf.GetPaintRect() );

        if ( rInf.GetTxtFrm()->IsRightToLeft() )
            rInf.GetTxtFrm()->SwitchLTRtoRTL( aRepaintRect );

        if ( rInf.GetTxtFrm()->IsVertical() )
            rInf.GetTxtFrm()->SwitchHorizontalToVertical( aRepaintRect );

        if( (GetFlyFrm()->IsCompletePaint() ||
             GetFlyFrm()->Frm().IsOver( aRepaintRect )) &&
             SwFlyFrm::IsPaint( (SdrObject*)GetFlyFrm()->GetVirtDrawObj(),
                                GetFlyFrm()->getRootFrm()->GetCurrShell() ))
        {
            SwRect aRect( GetFlyFrm()->Frm() );
            if( !GetFlyFrm()->IsCompletePaint() )
                aRect._Intersection( aRepaintRect );

            
            {
                SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
                GetFlyFrm()->Paint( aRect );
            }
            ((SwTxtPaintInfo&)rInf).GetRefDev()->SetLayoutMode(
                    rInf.GetOut()->GetLayoutMode() );

            
            
            ((SwTxtPaintInfo&)rInf).SelectFont();

            OSL_ENSURE( ! rInf.GetVsh() || rInf.GetVsh()->GetOut() == rInf.GetOut(),
                    "SwFlyCntPortion::Paint: Outdev has changed" );
            if( rInf.GetVsh() )
                ((SwTxtPaintInfo&)rInf).SetOut( rInf.GetVsh()->GetOut() );
        }
    }
}



SwFlyCntPortion::SwFlyCntPortion( const SwTxtFrm& rFrm,
                                  SwFlyInCntFrm *pFly, const Point &rBase,
                                  long nLnAscent, long nLnDescent,
                                  long nFlyAsc, long nFlyDesc,
                                  objectpositioning::AsCharFlags nFlags ) :
    pContact( pFly ),
    bDraw( false ),
    bMax( false ),
    nAlign( 0 )
{
    OSL_ENSURE( pFly, "SwFlyCntPortion::SwFlyCntPortion: no SwFlyInCntFrm!" );
    nLineLength = 1;
    nFlags |= AS_CHAR_ULSPACE | AS_CHAR_INIT;
    SetBase( rFrm, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );
    SetWhichPor( POR_FLYCNT );
}


SwFlyCntPortion::SwFlyCntPortion( const SwTxtFrm& rFrm,
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
    if( !pDrawContact->GetAnchorFrm() )
    {
        
        pDrawContact->ConnectToLayout();
        
        
        pDrawContact->MoveObjToVisibleLayer( pDrawContact->GetMaster() );
    }
    nLineLength = 1;
    nFlags |= AS_CHAR_ULSPACE | AS_CHAR_INIT;

    SetBase( rFrm, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );

    SetWhichPor( POR_FLYCNT );
}




void SwFlyCntPortion::SetBase( const SwTxtFrm& rFrm, const Point &rBase,
                               long nLnAscent, long nLnDescent,
                               long nFlyAsc, long nFlyDesc,
                               objectpositioning::AsCharFlags nFlags )
{
    
    
    SdrObject* pSdrObj = 0L;
    if( bDraw )
    {
        
        
        pSdrObj = GetDrawContact()->GetDrawObjectByAnchorFrm( rFrm );
        if ( !pSdrObj )
        {
            OSL_FAIL( "SwFlyCntPortion::SetBase(..) - No drawing object found by <GetDrawContact()->GetDrawObjectByAnchorFrm( rFrm )>" );
            pSdrObj = GetDrawContact()->GetMaster();
        }
        
        
        
        
        if ( pSdrObj )
        {
            GetDrawContact()->GetAnchoredObj( pSdrObj )->MakeObjPos();
        }
    }
    else
    {
        pSdrObj = GetFlyFrm()->GetVirtDrawObj();
    }

    if (!pSdrObj)
        return;

    
    objectpositioning::SwAsCharAnchoredObjectPosition aObjPositioning(
                                    *pSdrObj,
                                    rBase, nFlags,
                                    nLnAscent, nLnDescent, nFlyAsc, nFlyDesc );

    
    {
        
        SwObjPositioningInProgress aObjPosInProgress( *pSdrObj );
        aObjPositioning.CalcPosition();
    }

    SetAlign( aObjPositioning.GetLineAlignment() );

    aRef = aObjPositioning.GetAnchorPos();
    if( nFlags & AS_CHAR_ROTATE )
        SvXSize( aObjPositioning.GetObjBoundRectInclSpacing().SSize() );
    else
        SvLSize( aObjPositioning.GetObjBoundRectInclSpacing().SSize() );
    if( Height() )
    {
        
        
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

sal_Int32 SwFlyCntPortion::GetFlyCrsrOfst( const KSHORT nOfst,
    const Point &rPoint, SwPosition *pPos, SwCrsrMoveState* pCMS ) const
{
    
    
    
    
    Point aPoint( rPoint );
    if( !pPos || bDraw || !( GetFlyFrm()->GetCrsrOfst( pPos, aPoint, pCMS ) ) )
        return SwLinePortion::GetCrsrOfst( nOfst );
    else
        return 0;
}

sal_Int32 SwFlyCntPortion::GetCrsrOfst( const KSHORT nOfst ) const
{
    
    return SwLinePortion::GetCrsrOfst( nOfst );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
