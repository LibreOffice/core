/*************************************************************************
 *
 *  $RCSfile: porfly.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:15:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#include "dcontact.hxx" // SwDrawContact
#include "dflyobj.hxx"  // SwVirtFlyDrawObj
#include "pam.hxx"      // SwPosition
#include "flyfrm.hxx"   // SwFlyInCntFrm
#include "frmfmt.hxx"   // SwFrmFmt
#include "viewsh.hxx"

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#include "flyfrms.hxx"
#include "txatbase.hxx" // SwTxtAttr
#include "porfly.hxx"
#include "porlay.hxx"   // SetFly
#include "inftxt.hxx"   // SwTxtPaintInfo
#include "frmsh.hxx"
// OD 28.10.2003 #113049#
#ifndef _ASCHARANCHOREDOBJECTPOSITION_HXX
#include <ascharanchoredobjectposition.hxx>
#endif
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif
// OD 2004-04-13 #i26791#
#ifndef _ANCHOREDDRAWOBJECT_HXX
#include <anchoreddrawobject.hxx>
#endif

/*************************************************************************
 *                class SwFlyPortion
 *
 * Wir erwarten ein framelokales SwRect !
 *************************************************************************/

void SwFlyPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
}

/*************************************************************************
 *                 virtual SwFlyPortion::Format()
 *************************************************************************/
sal_Bool SwFlyPortion::Format( SwTxtFormatInfo &rInf )
{
    ASSERT( Fix() >= rInf.X(), "SwFlyPortion::Format: rush hour" );
    // 8537: Tabs muessen expandiert werden.
    if( rInf.GetLastTab() )
        ((SwLinePortion*)rInf.GetLastTab())->FormatEOL( rInf );

    // Der Glue wird aufgespannt.
    rInf.GetLast()->FormatEOL( rInf );
    PrtWidth( (Fix() - rInf.X()) + PrtWidth() );
    if( !Width() )
    {
        ASSERT( Width(), "+SwFlyPortion::Format: a fly is a fly is a fly" );
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
        SetBlankWidth( rInf.GetTxtSize( ' ' ).Width() );
        SetLen( 1 );
    }

    const USHORT nNewWidth = rInf.X() + PrtWidth();
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
        const USHORT nLeft = ( pLastPor &&
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
        for ( sal_uInt32 i = 0; GetDrawObjs() && i < int(pObjs->Count()); ++i )
        {
            // OD 2004-03-29 #i26791#
            // --> OD 2004-07-06 #i28701# - consider changed type of
            // <SwSortedList> entries
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            const SwFmtAnchor& rAnch = pAnchoredObj->GetFrmFmt().GetAnchor();
            if ( rAnch.GetAnchorId() == FLY_IN_CNTNT )
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
            // <--
        }
    }
}

/*************************************************************************
 *                SwTxtFrm::CalcFlyPos()
 *************************************************************************/
xub_StrLen SwTxtFrm::CalcFlyPos( SwFrmFmt* pSearch )
{
    SwpHints* pHints = GetTxtNode()->GetpSwpHints();
    ASSERT( pHints, "CalcFlyPos: Why me?" );
    if( !pHints )
        return STRING_LEN;
    SwTxtAttr* pFound = NULL;
    for( MSHORT i = 0; i < pHints->Count(); i++)
    {
        SwTxtAttr *pHt = pHints->GetHt( i );
        if( RES_TXTATR_FLYCNT == pHt->Which() )
        {
            SwFrmFmt* pFrmFmt = pHt->GetFlyCnt().GetFrmFmt();
            if( pFrmFmt == pSearch )
                pFound = pHt;
        }
    }
    ASSERT( pHints, "CalcFlyPos: Not Found!" );
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
                                GetFlyFrm()->GetShell() ))
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
            ASSERT( ! rInf.GetVsh() || rInf.GetVsh()->GetOut() == rInf.GetOut(),
                    "SwFlyCntPortion::Paint: Outdev has changed" )
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
    ASSERT( pFly, "SwFlyCntPortion::SwFlyCntPortion: no SwFlyInCntFrm!" );
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
    ASSERT( pDrawContact, "SwFlyCntPortion::SwFlyCntPortion: no SwDrawContact!" );
    if( !pDrawContact->GetAnchorFrm() )
    {
        // OD 2004-04-01 #i26791# - no direct positioning needed any more
        pDrawContact->ConnectToLayout();
    }
    nLineLength = 1;
    nFlags |= AS_CHAR_ULSPACE | AS_CHAR_INIT;

    SetBase( rFrm, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );

    SetWhichPor( POR_FLYCNT );
}

const SwFrmFmt *SwFlyCntPortion::GetFrmFmt() const
{
    if( bDraw )
        return GetDrawContact()->GetFmt();
    else
        return GetFlyFrm()->GetFmt();
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
            ASSERT( false, "SwFlyCntPortion::SetBase(..) - No drawing object found by <GetDrawContact()->GetDrawObjectByAnchorFrm( rFrm )>" );
            pSdrObj = GetDrawContact()->GetMaster();
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
            nAscent = Abs( int( nRelPos ) );
            if( nAscent > Height() )
                Height( nAscent );
        }
        else
        {
            nAscent = 0;
            Height( Height() + int( nRelPos ) );
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
    // ASSERT( !this, "SwFlyCntPortion::GetCrsrOfst: use GetFlyCrsrOfst()" );
    return SwLinePortion::GetCrsrOfst( nOfst );
}

