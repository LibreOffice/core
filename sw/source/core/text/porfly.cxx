/*************************************************************************
 *
 *  $RCSfile: porfly.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-19 12:38:41 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "dcontact.hxx" // SwDrawContact
#include "dflyobj.hxx"  // SwVirtFlyDrawObj
#include "pam.hxx"      // SwPosition
#include "flyfrm.hxx"   // SwFlyInCntFrm
#include "frmfmt.hxx"   // SwFrmFmt
#include "viewsh.hxx"

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

/*************************************************************************
 *                class SwFlyPortion
 *
 * Wir erwarten ein framelokales SwRect !
 *************************************************************************/

#ifdef OLDRECYCLE

sal_Bool SwFlyPortion::MayRecycle() const { return sal_False; }

#endif

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
#ifdef USED
    long nFirstDiff;

    if( !Fix() )
    {
        nFirstDiff = rInf.Left() - long( rInf.First() );
        if( rInf.GetLineStart() )
        {
            if( nFirstDiff < 0 )
                nFirstDiff = 0;
        }
        else
        {
            if( nFirstDiff > 0 )
                nFirstDiff = 0;
            else
                nFirstDiff = -nFirstDiff;
        }
        nFirstDiff += rInf.GetTxtFrm()->Prt().Left();
    }
    else
        nFirstDiff = 0;
    PrtWidth( (Fix() - rInf.X()) + PrtWidth() + nFirstDiff );
#else
    PrtWidth( (Fix() - rInf.X()) + PrtWidth() );
#endif
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

    const KSHORT nNewWidth = rInf.X() + PrtWidth();
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
        if( !rInf.X() && !rInf.GetFly() )
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
    SwDrawObjs *pObjs;
    if ( 0 != (pObjs = GetDrawObjs()) )
    {
        for ( int i = 0; GetDrawObjs() && i < int(pObjs->Count()); ++i )
        {
            SdrObject *pO = (*pObjs)[MSHORT(i)];
            if ( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if( pFly->IsFlyInCntFrm() )
                {
                    const SwFmtAnchor &rAnch = pFly->GetFmt()->GetAnchor();
                    const SwPosition *pPos = rAnch.GetCntntAnchor();
                    xub_StrLen nIdx = pPos->nContent.GetIndex();
                    if ( nIdx >= nStart && nEnd > nIdx )
                    {
                        RemoveFly( pFly );
                        pNew->AppendFly( pFly );
                        --i;
                    }
                }
            }
            else
            {
                SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pO);
                const SwFmtAnchor &rAnch = pContact->GetFmt()->GetAnchor();
                if ( FLY_IN_CNTNT == rAnch.GetAnchorId() )
                {
                    const SwPosition *pPos = rAnch.GetCntntAnchor();
                    xub_StrLen nIdx = pPos->nContent.GetIndex();
                    if ( nIdx >= nStart && nEnd > nIdx )
                    {
                        RemoveDrawObj( pContact );
                        pNew->AppendDrawObj( pContact );
                        --i;
                    }
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
        if( !((SwDrawContact*)pContact)->GetAnchor() )
        {
            Point aAnchorPos =
                ((SwDrawContact*)pContact)->GetMaster()->GetAnchorPos();
            ((SwDrawContact*)pContact)->ConnectToLayout();
            ((SwDrawContact*)pContact)->GetMaster()->SetAnchorPos( aAnchorPos );
        }
    }
    else
    {
        // Baseline-Ausgabe !
        // 7922: Bei CompletePaint alles painten
        if( (GetFlyFrm()->IsCompletePaint() ||
             GetFlyFrm()->Frm().IsOver( rInf.GetPaintRect() )) &&
             SwFlyFrm::IsPaint( (SdrObject*)GetFlyFrm()->GetVirtDrawObj(),
                                GetFlyFrm()->GetShell() ))
        {
            SwRect aRect( GetFlyFrm()->Frm() );
            if( !GetFlyFrm()->IsCompletePaint() )
                aRect._Intersection( rInf.GetPaintRect() );

            GetFlyFrm()->Paint( aRect );
            // Es hilft alles nichts, im zeichengebundenen Frame kann wer weiss
            // was am OutputDevice eingestellt sein, wir muessen unseren Font
            // wieder hineinselektieren. Dass wir im const stehen, soll uns
            // daran nicht hindern:
            ((SwTxtPaintInfo&)rInf).SelectFont();
            rInf.SelectOut();
        }
    }
}

/*************************************************************************
 *                  SwFlyCntPortion::SwFlyCntPortion()
 *
 * Es werden die Masze vom pFly->OutRect() eingestellt.
 * Es erfolgt ein SetBase() !
 *************************************************************************/
SwFlyCntPortion::SwFlyCntPortion( SwFlyInCntFrm *pFly, const Point &rBase,
    long nLnAscent, long nLnDescent, long nFlyAsc, long nFlyDesc, sal_Bool bQuick ) :
    pContact( pFly ),
    bDraw( sal_False ),
    bMax( sal_False ),
    nAlign( 0 )
{
    ASSERT( pFly, "SwFlyCntPortion::SwFlyCntPortion: no SwFlyInCntFrm!" );
    nLineLength = 1;
    sal_uInt8 nFlags = SETBASE_ULSPACE | SETBASE_INIT;
    if( bQuick )
        nFlags |= SETBASE_QUICK;
    SetBase( rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );
    SetWhichPor( POR_FLYCNT );
}
SwFlyCntPortion::SwFlyCntPortion(  SwDrawContact *pDrawContact,
        const Point &rBase, long nLnAscent, long nLnDescent,
        long nFlyAsc, long nFlyDesc, sal_Bool bQuick ) :
    pContact( pDrawContact ),
    bDraw( sal_True ),
    bMax( sal_False ),
    nAlign( 0 )
{
    ASSERT( pDrawContact, "SwFlyCntPortion::SwFlyCntPortion: no SwDrawContact!" );
    if( !pDrawContact->GetAnchor() )
    {
        if( bQuick )
        {
            Point aAnchorPos = pDrawContact->GetMaster()->GetAnchorPos();
            pDrawContact->ConnectToLayout();
            pDrawContact->GetMaster()->SetAnchorPos( aAnchorPos );
        }
        else
            pDrawContact->ConnectToLayout();
    }
    nLineLength = 1;
    sal_uInt8 nFlags = SETBASE_ULSPACE | SETBASE_INIT;
    if( bQuick )
        nFlags |= SETBASE_QUICK;
    SetBase( rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );
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

void SwFlyCntPortion::SetBase( const Point &rBase, long nLnAscent,
    long nLnDescent, long nFlyAsc, long nFlyDesc, sal_uInt8 nFlags )
{
    Point aBase( rBase );
    const SwFrmFmt* pFmt = GetFrmFmt();
    const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
    const SwVertOrient eOri = rVert.GetVertOrient();
    const SvxLRSpaceItem &rLRSpace = pFmt->GetLRSpace();
    const SvxULSpaceItem &rULSpace = pFmt->GetULSpace();

    //Die vertikale Position wird berechnet, die relative horizontale
    //Position ist stets 0.

    Point aRelPos;
    SdrObject *pSdrObj;
    SwRect aBoundRect;
    long nOldWidth;
    if( bDraw )
    {
        pSdrObj = GetDrawContact()->GetMaster();
        aBoundRect = pSdrObj->GetBoundRect();
    }
    else
    {
        aBoundRect = GetFlyFrm()->Frm();
        nOldWidth = aBoundRect.Width();
    }

    if( nFlags & SETBASE_ULSPACE )
        aBase.X() += rLRSpace.GetLeft();
    aBase.Y() += rULSpace.GetUpper();
    if( bDraw )
    {
        if( nFlags & SETBASE_ULSPACE )
            aBase.X() += pSdrObj->GetSnapRect().Left() - aBoundRect.Left();
        aBase.Y() += pSdrObj->GetSnapRect().Top() - aBoundRect.Top();
    }
    aBoundRect.Left( aBoundRect.Left() - rLRSpace.GetLeft() );
    aBoundRect.Width( aBoundRect.Width() + rLRSpace.GetRight() );
    aBoundRect.Top( aBoundRect.Top() - rULSpace.GetUpper() );
    aBoundRect.Height( aBoundRect.Height() + rULSpace.GetLower() );

    if ( eOri == VERT_NONE )
        aRelPos.Y() = rVert.GetPos();
    else
    {
        aRelPos.Y() = 0;
        if ( eOri == VERT_CENTER )
            aRelPos.Y() -= aBoundRect.Height() /  2;
        else if ( eOri == VERT_TOP )
            aRelPos.Y() -= aBoundRect.Height();
        else if ( eOri == VERT_BOTTOM )
            ;
        else if ( eOri == VERT_CHAR_CENTER )
            aRelPos.Y() -= ( aBoundRect.Height() + nLnAscent - nLnDescent ) / 2;
        else if ( eOri == VERT_CHAR_TOP )
            aRelPos.Y() -= nLnAscent;
        else if ( eOri == VERT_CHAR_BOTTOM )
            aRelPos.Y() += nLnDescent - aBoundRect.Height();
        else
        {
            if( aBoundRect.Height() >= nFlyAsc + nFlyDesc )
            {
                // wenn ich genauso gross bin wie die Zeile, brauche ich mich
                // nicht an der Zeile nicht weiter ausrichten, ich lasse
                // dann auch den max. Ascent der Zeile zunaechst unveraendert
                aRelPos.Y() -= nFlyAsc;
                if ( eOri == VERT_LINE_CENTER )
                    SetAlign( 2 );
                else if ( eOri == VERT_LINE_TOP )
                    SetAlign( 1 );
                else if ( eOri == VERT_LINE_BOTTOM )
                    SetAlign( 3 );
            }
            else if ( eOri == VERT_LINE_CENTER )
            {
                aRelPos.Y() -= ( aBoundRect.Height() +nFlyAsc -nFlyDesc ) / 2;
                SetAlign( 2 );
            }
            else if ( eOri == VERT_LINE_TOP )
            {
                aRelPos.Y() -= nFlyAsc;
                SetAlign( 1 );
            }
            else if ( eOri == VERT_LINE_BOTTOM )
            {
                aRelPos.Y() += nFlyDesc - aBoundRect.Height();
                SetAlign( 3 );
            }
        }
    }

    if( nFlags & SETBASE_INIT && aRelPos.Y() < 0 && nFlyAsc < -aRelPos.Y() )
        aBase.Y() -= nFlyAsc + aRelPos.Y();

    if( bDraw )
    {
        if( !( nFlags & SETBASE_QUICK ) )
        {
            if( rVert.GetPos() != aRelPos.Y() && eOri != VERT_NONE )
            {
                // Das aRelPos wird gepflegt, weil sonst SwDrawContact::_Changed
                // auf die Idee kommen koennte, auf VERT_NONE umzuschalten.
                SwFmtVertOrient aVert( rVert );
                aVert.SetPos( aRelPos.Y() );
                ((SwFrmFmt*)pFmt)->LockModify();
                ((SwFrmFmt*)pFmt)->SetAttr( aVert );
                ((SwFrmFmt*)pFmt)->UnlockModify();
            }
            Point aDiff = aRelPos + aBase - pSdrObj->GetSnapRect().TopLeft();
            pSdrObj->ImpSetAnchorPos( aBase );
            pSdrObj->NbcMove( Size( aDiff.X(), aDiff.Y() ) );
        }
    }
    else
    {
        if ( !(nFlags & SETBASE_QUICK) && (aBase != GetFlyFrm()->GetRefPoint() ||
                         aRelPos != GetFlyFrm()->GetCurRelPos()) )
        {
            GetFlyFrm()->SetRefPoint( aBase, aRelPos );
            if( nOldWidth != GetFlyFrm()->Frm().Width() )
            {
                aBoundRect = GetFlyFrm()->Frm();
                aBoundRect.Left( aBoundRect.Left() - rLRSpace.GetLeft() );
                aBoundRect.Width( aBoundRect.Width() + rLRSpace.GetRight() );
                aBoundRect.Top( aBoundRect.Top() - rULSpace.GetUpper() );
                aBoundRect.Height( aBoundRect.Height() + rULSpace.GetLower() );
            }
        }
        ASSERT( GetFlyFrm()->Frm().Height(),
            "SwFlyCntPortion::SetBase: flyfrm has an invalid height" );
    }
    aRef = aBase;
    SvLSize( aBoundRect.SSize() );
    if( Height() )
    {
        if ( aRelPos.Y() < 0 )
        {
            nAscent = Abs( int( aRelPos.Y() ) );
            if( nAscent > Height() )
                Height( nAscent );
        }
        else
        {
            nAscent = 0;
            Height( Height() + int( aRelPos.Y() ) );
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
    const Point &rPoint, SwPosition *pPos, const SwCrsrMoveState* pCMS ) const
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

