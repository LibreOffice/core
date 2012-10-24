/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "dcontact.hxx" // SwDrawContact
#include "dflyobj.hxx"  // SwVirtFlyDrawObj
#include "pam.hxx"      // SwPosition
#include "flyfrm.hxx"   // SwFlyInCntFrm
#include "rootfrm.hxx"
#include "frmfmt.hxx"   // SwFrmFmt
#include "viewsh.hxx"

#include <vcl/outdev.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include "flyfrms.hxx"
#include "txatbase.hxx" // SwTxtAttr
#include "porfly.hxx"
#include "porlay.hxx"   // SetFly
#include "inftxt.hxx"   // SwTxtPaintInfo

// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>


/*************************************************************************
 *                class SwFlyPortion
 *
 * Wir erwarten ein framelokales SwRect !
 *************************************************************************/

void SwFlyPortion::Paint( const SwTxtPaintInfo& ) const
{
}

/*************************************************************************
 *                 virtual SwFlyPortion::Format()
 *************************************************************************/
sal_Bool SwFlyPortion::Format( SwTxtFormatInfo &rInf )
{
    OSL_ENSURE( Fix() >= rInf.X(), "SwFlyPortion::Format: rush hour" );
    // 8537: Tabs muessen expandiert werden.
    if( rInf.GetLastTab() )
        ((SwLinePortion*)rInf.GetLastTab())->FormatEOL( rInf );

    // Der Glue wird aufgespannt.
    rInf.GetLast()->FormatEOL( rInf );
    PrtWidth( static_cast<sal_uInt16>(Fix() - rInf.X() + PrtWidth()) );
    if( !Width() )
    {
        OSL_ENSURE( Width(), "+SwFlyPortion::Format: a fly is a fly is a fly" );
        Width(1);
    }

    // Restaurierung
    rInf.SetFly( 0 );
    rInf.Width( rInf.RealWidth() );
    rInf.GetParaPortion()->SetFly( sal_True );

    // trailing blank:
    if( rInf.GetIdx() < rInf.GetTxt().Len() &&  1 < rInf.GetIdx()
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
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 *                 virtual SwFlyCntPortion::Format()
 *************************************************************************/
sal_Bool SwFlyCntPortion::Format( SwTxtFormatInfo &rInf )
{
    sal_Bool bFull = rInf.Width() < rInf.X() + PrtWidth();

    if( bFull )
    {
        // 3924: wenn die Zeile voll ist und der zeichengebundene Frame am
        // Anfang der Zeile steht.
        // 5157: nicht wenn einem Fly ausgewichen werden kann!
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
            bFull = sal_False; // Damit Notizen noch in dieser Zeile landen
        }
        else
        {
            if( !rInf.GetFly() )
                rInf.SetNewLine( sal_True );
            Width(0);
            SetAscent(0);
            SetLen(0);
            if( rInf.GetLast() )
                rInf.GetLast()->FormatEOL( rInf );

            return bFull;
        }
    }

    rInf.GetParaPortion()->SetFly( sal_True );
    return bFull;
}

/*************************************************************************
 *  SwTxtFrm::MoveFlyInCnt() haengt jetzt die zeichengebundenen Objekte
 *  innerhalb des angegebenen Bereichs um, damit koennen diese vom Master
 *  zum Follow oder umgekehrt wandern.
 *************************************************************************/
void SwTxtFrm::MoveFlyInCnt( SwTxtFrm *pNew, xub_StrLen nStart, xub_StrLen nEnd )
{
    SwSortedObjs *pObjs = 0L;
    if ( 0 != (pObjs = GetDrawObjs()) )
    {
        for ( sal_uInt32 i = 0; GetDrawObjs() && i < pObjs->Count(); ++i )
        {
            // OD 2004-03-29 #i26791#
            // #i28701# - consider changed type of
            // <SwSortedList> entries
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            const SwFmtAnchor& rAnch = pAnchoredObj->GetFrmFmt().GetAnchor();
            if (rAnch.GetAnchorId() == FLY_AS_CHAR)
            {
                const SwPosition* pPos = rAnch.GetCntntAnchor();
                xub_StrLen nIdx = pPos->nContent.GetIndex();
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

/*************************************************************************
 *                SwTxtFrm::CalcFlyPos()
 *************************************************************************/
xub_StrLen SwTxtFrm::CalcFlyPos( SwFrmFmt* pSearch )
{
    SwpHints* pHints = GetTxtNode()->GetpSwpHints();
    OSL_ENSURE( pHints, "CalcFlyPos: Why me?" );
    if( !pHints )
        return STRING_LEN;
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
        return STRING_LEN;
    return *pFound->GetStart();
}

/*************************************************************************
 *                 virtual SwFlyCntPortion::Paint()
 *************************************************************************/
void SwFlyCntPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( bDraw )
    {
        if( !((SwDrawContact*)pContact)->GetAnchorFrm() )
        {
            // OD 2004-04-01 #i26791# - no direct positioning of the drawing
            // object is needed.
            SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(pContact);
            pDrawContact->ConnectToLayout();
        }
    }
    else
    {
        // Baseline-Ausgabe !
        // 7922: Bei CompletePaint alles painten
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


            // GetFlyFrm() may change the layout mode at the output device.
            {
                SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
                GetFlyFrm()->Paint( aRect );
            }
            ((SwTxtPaintInfo&)rInf).GetRefDev()->SetLayoutMode(
                    rInf.GetOut()->GetLayoutMode() );

            // Es hilft alles nichts, im zeichengebundenen Frame kann wer weiss
            // was am OutputDevice eingestellt sein, wir muessen unseren Font
            // wieder hineinselektieren. Dass wir im const stehen, soll uns
            // daran nicht hindern:
            ((SwTxtPaintInfo&)rInf).SelectFont();

            // I want to know if this can really happen. So here comes a new
            OSL_ENSURE( ! rInf.GetVsh() || rInf.GetVsh()->GetOut() == rInf.GetOut(),
                    "SwFlyCntPortion::Paint: Outdev has changed" );
            if( rInf.GetVsh() )
                ((SwTxtPaintInfo&)rInf).SetOut( rInf.GetVsh()->GetOut() );
        }
    }
}

/*************************************************************************
 *                  SwFlyCntPortion::SwFlyCntPortion()
 *
 * Es werden die Masze vom pFly->OutRect() eingestellt.
 * Es erfolgt ein SetBase() !
 *************************************************************************/
// OD 29.07.2003 #110978# - use new datatype for parameter <nFlags>
SwFlyCntPortion::SwFlyCntPortion( const SwTxtFrm& rFrm,
                                  SwFlyInCntFrm *pFly, const Point &rBase,
                                  long nLnAscent, long nLnDescent,
                                  long nFlyAsc, long nFlyDesc,
                                  objectpositioning::AsCharFlags nFlags ) :
    pContact( pFly ),
    bDraw( sal_False ),
    bMax( sal_False ),
    nAlign( 0 )
{
    OSL_ENSURE( pFly, "SwFlyCntPortion::SwFlyCntPortion: no SwFlyInCntFrm!" );
    nLineLength = 1;
    nFlags |= AS_CHAR_ULSPACE | AS_CHAR_INIT;
    SetBase( rFrm, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );
    SetWhichPor( POR_FLYCNT );
}

// OD 29.07.2003 #110978# - use new datatype for parameter <nFlags>
SwFlyCntPortion::SwFlyCntPortion( const SwTxtFrm& rFrm,
                                  SwDrawContact *pDrawContact, const Point &rBase,
                                  long nLnAscent, long nLnDescent,
                                  long nFlyAsc, long nFlyDesc,
                                  objectpositioning::AsCharFlags nFlags ) :
    pContact( pDrawContact ),
    bDraw( sal_True ),
    bMax( sal_False ),
    nAlign( 0 )
{
    OSL_ENSURE( pDrawContact, "SwFlyCntPortion::SwFlyCntPortion: no SwDrawContact!" );
    if( !pDrawContact->GetAnchorFrm() )
    {
        // OD 2004-04-01 #i26791# - no direct positioning needed any more
        pDrawContact->ConnectToLayout();
        // #i40333# - follow-up of #i35635#
        // move object to visible layer
        pDrawContact->MoveObjToVisibleLayer( pDrawContact->GetMaster() );
    }
    nLineLength = 1;
    nFlags |= AS_CHAR_ULSPACE | AS_CHAR_INIT;

    SetBase( rFrm, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );

    SetWhichPor( POR_FLYCNT );
}


/*************************************************************************
 *                  SwFlyCntPortion::SetBase()
 *
 * Nach dem Setzen des RefPoints muss der Ascent neu berechnet werden,
 * da er von der RelPos abhaengt.
 * pFly->GetRelPos().Y() bezeichnet die relative Position zur Baseline.
 * Bei 0 liegt der obere Rand des FlyCnt auf der Baseline der Zeile.
 *************************************************************************/
// OD 29.07.2003 #110978# - use new datatype for parameter <nFlags>
void SwFlyCntPortion::SetBase( const SwTxtFrm& rFrm, const Point &rBase,
                               long nLnAscent, long nLnDescent,
                               long nFlyAsc, long nFlyDesc,
                               objectpositioning::AsCharFlags nFlags )
{
    // Note: rBase have to be an absolute value

    // OD 28.10.2003 #113049# - use new class to position object
    // determine drawing object
    SdrObject* pSdrObj = 0L;
    if( bDraw )
    {
        // OD 20.06.2003 #108784# - determine drawing object ('master' or 'virtual')
        // by frame.
        pSdrObj = GetDrawContact()->GetDrawObjectByAnchorFrm( rFrm );
        if ( !pSdrObj )
        {
            OSL_FAIL( "SwFlyCntPortion::SetBase(..) - No drawing object found by <GetDrawContact()->GetDrawObjectByAnchorFrm( rFrm )>" );
            pSdrObj = GetDrawContact()->GetMaster();
        }
        // #i65798#
        // call <SwAnchoredDrawObject::MakeObjPos()> to assure that flag at
        // the <DrawFrmFmt> and at the <SwAnchoredDrawObject> instance are
        // correctly set.
        if ( pSdrObj )
        {
            GetDrawContact()->GetAnchoredObj( pSdrObj )->MakeObjPos();
        }
    }
    else
    {
        pSdrObj = GetFlyFrm()->GetVirtDrawObj();
    }

    // position object
    objectpositioning::SwAsCharAnchoredObjectPosition aObjPositioning(
                                    *pSdrObj,
                                    rBase, nFlags,
                                    nLnAscent, nLnDescent, nFlyAsc, nFlyDesc );

    // OD 2004-04-13 #i26791# - scope of local variable <aObjPosInProgress>
    {
        // OD 2004-04-13 #i26791#
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

/*************************************************************************
 *              virtual SwFlyCntPortion::GetFlyCrsrOfst()
 *************************************************************************/

xub_StrLen SwFlyCntPortion::GetFlyCrsrOfst( const KSHORT nOfst,
    const Point &rPoint, SwPosition *pPos, SwCrsrMoveState* pCMS ) const
{
    // Da die FlyCnt nicht an der Seite haengen, wird ihr
    // GetCrsrOfst() nicht gerufen. Um die Layoutseite
    // von unnoetiger Verwaltung zu entlasten, ruft der Absatz
    // das GetCrsrOfst des FlyFrm, wenn es erforderlich ist.
    Point aPoint( rPoint );
    if( !pPos || bDraw || !( GetFlyFrm()->GetCrsrOfst( pPos, aPoint, pCMS ) ) )
        return SwLinePortion::GetCrsrOfst( nOfst );
    else
        return 0;
}

/*************************************************************************
 *              virtual SwFlyCntPortion::GetCrsrOfst()
 *************************************************************************/

xub_StrLen SwFlyCntPortion::GetCrsrOfst( const KSHORT nOfst ) const
{
    // OSL_ENSURE( !this, "SwFlyCntPortion::GetCrsrOfst: use GetFlyCrsrOfst()" );
    return SwLinePortion::GetCrsrOfst( nOfst );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
