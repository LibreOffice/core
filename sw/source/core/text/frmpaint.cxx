/*************************************************************************
 *
 *  $RCSfile: frmpaint.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2001-02-16 09:02:43 $
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

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx> // SW_MOD
#endif

#ifndef _FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>  // SwViewImp
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>  // SwViewOption
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>  // DrawGraphic
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // SwTxtFrm
#endif
#ifndef _ITRPAINT_HXX
#include <itrpaint.hxx>     // SwTxtPainter
#endif
#ifndef _TXTPAINT_HXX
#include <txtpaint.hxx>     // SwSaveClip
#endif
#ifndef _TXTCACHE_HXX
#include <txtcache.hxx> // SwTxtLineAccess
#endif
#ifndef _SWFNTCCH_HXX
#include <swfntcch.hxx> // SwFontAccess
#endif
#ifndef _DRAWFONT_HXX
#include <drawfont.hxx> // SwDrawTextInfo
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>   // SwFlyFrm
#endif
#ifndef _REDLNITR_HXX
#include <redlnitr.hxx> // SwRedlineItr
#endif
#ifndef _DOC_HXX
#include <doc.hxx>      // SwDoc
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx> // SW_MOD
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>   // SwTabFrm (Redlining)
#endif
#include "scrrect.hxx"

// steht im number.cxx
extern const sal_Char __FAR_DATA sBulletFntName[];

extern FASTBOOL bOneBeepOnly;

sal_Bool bInitFont = sal_True;

#define REDLINE_DISTANCE 567/4
#define REDLINE_MINDIST  567/10

class SwExtraPainter
{
    SwSaveClip aClip;
    SwRect aRect;
    ViewShell *pSh;
    SwFont* pFnt;
    const SwLineNumberInfo &rLineInf;
    SwTwips nX;
    SwTwips nRedX;
    ULONG nLineNr;
    MSHORT nDivider;
    sal_Bool bGoLeft;
    sal_Bool bLineNum;
    inline sal_Bool IsClipChg() { return aClip.IsChg(); }
public:
    SwExtraPainter( const SwTxtFrm *pFrm, ViewShell *pVwSh,
        const SwLineNumberInfo &rLnInf, const SwRect &rRct, MSHORT nStart,
        SwHoriOrient eHor, sal_Bool bLnNm );
    ~SwExtraPainter() { delete pFnt; }
    inline SwFont* GetFont() const { return pFnt; }
    inline void IncLineNr() { ++nLineNr; }
    inline sal_Bool HasNumber() { return !( nLineNr % rLineInf.GetCountBy() ); }
    inline HasDivider() { if( !nDivider ) return sal_False;
        return !(nLineNr % rLineInf.GetDividerCountBy()); }

    void PaintExtra( SwTwips nY, long nAsc, long nMax, sal_Bool bRed );
    void PaintRedline( SwTwips nY, long nMax );
};


SwExtraPainter::SwExtraPainter( const SwTxtFrm *pFrm, ViewShell *pVwSh,
    const SwLineNumberInfo &rLnInf, const SwRect &rRct, MSHORT nStart,
    SwHoriOrient eHor, sal_Bool bLnNm )
    : pSh( pVwSh ), pFnt( 0 ), rLineInf( rLnInf ), aRect( rRct ),
      aClip( pVwSh->GetWin() || pFrm->IsUndersized() ? pVwSh->GetOut() : 0 ),
      nLineNr( 1L ), bLineNum( bLnNm )
{
    if( pFrm->IsUndersized() )
    {
        SwTwips nBottom = pFrm->Frm().Bottom();
        if( aRect.Bottom() > nBottom )
            aRect.Bottom( nBottom );
    }
    MSHORT nVirtPageNum = 0;
    if( bLineNum )
    {   /* initialisiert die Member, die bei Zeilennumerierung notwendig sind:

            nDivider,   wie oft ist ein Teilerstring gewuenscht, 0 == nie;
            nX,         X-Position der Zeilennummern;
            pFnt,       der Font der Zeilennummern;
            nLineNr,    die erste Zeilennummer;
        bLineNum wird ggf.wieder auf sal_False gesetzt, wenn die Numerierung sich
        komplett ausserhalb des Paint-Rechtecks aufhaelt. */
        nDivider = rLineInf.GetDivider().Len() ? rLineInf.GetDividerCountBy() : 0;
        nX = pFrm->Frm().Left();
        SwCharFmt* pFmt = rLineInf.GetCharFmt( *((SwDoc*)pFrm->GetNode()->GetDoc()) );
        ASSERT( pFmt, "PaintExtraData without CharFmt" );
        pFnt = new SwFont( &pFmt->GetAttrSet() );
        pFnt->Invalidate();
        pFnt->ChgPhysFnt( pSh, pSh->GetOut() );
        nLineNr += pFrm->GetAllLines() - pFrm->GetThisLines();
        LineNumberPosition ePos = rLineInf.GetPos();
        if( ePos != LINENUMBER_POS_LEFT && ePos != LINENUMBER_POS_RIGHT )
        {
            nVirtPageNum = pFrm->FindPageFrm()->GetVirtPageNum();
            if( nVirtPageNum % 2 )
                ePos = ePos == LINENUMBER_POS_INSIDE ?
                        LINENUMBER_POS_LEFT : LINENUMBER_POS_RIGHT;
            else
                ePos = ePos == LINENUMBER_POS_OUTSIDE ?
                        LINENUMBER_POS_LEFT : LINENUMBER_POS_RIGHT;
        }
        if( LINENUMBER_POS_LEFT == ePos )
        {
            bGoLeft = sal_True;
            nX -= rLineInf.GetPosFromLeft();
            if( nX < aRect.Left() )
                bLineNum = sal_False;
        }
        else
        {
            bGoLeft = sal_False;
            nX += pFrm->Frm().Width() + rLineInf.GetPosFromLeft();
            if( nX > aRect.Right() )
                bLineNum = sal_False;
        }
    }
    if( eHor != HORI_NONE )
    {
        if( HORI_INSIDE == eHor || HORI_OUTSIDE == eHor )
        {
            if( !nVirtPageNum )
                nVirtPageNum = pFrm->FindPageFrm()->GetVirtPageNum();
            if( nVirtPageNum % 2 )
                eHor = eHor == HORI_INSIDE ? HORI_LEFT : HORI_RIGHT;
            else
                eHor = eHor == HORI_OUTSIDE ? HORI_LEFT : HORI_RIGHT;
        }
        const SwFrm* pTmpFrm = pFrm->FindTabFrm();
        if( !pTmpFrm )
            pTmpFrm = pFrm;
        nRedX = HORI_LEFT == eHor ? pTmpFrm->Frm().Left() - REDLINE_DISTANCE :
            pTmpFrm->Frm().Right() + REDLINE_DISTANCE;
    }
}

/*************************************************************************
 * SwExtraPainter::PaintExtra()
 **************************************************************************/

void SwExtraPainter::PaintExtra( SwTwips nY, long nAsc, long nMax, sal_Bool bRed )
{
    //Zeilennummer ist staerker als der Teiler
    XubString aTmp( HasNumber() ? rLineInf.GetNumType().GetNumStr( nLineNr )
                                : rLineInf.GetDivider() );

    SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), aTmp, 0, aTmp.Len(), 0, sal_False );
    aDrawInf.SetSpace( 0 );
    aDrawInf.SetWrong( NULL );
    aDrawInf.SetLeft( 0 );
    aDrawInf.SetRight( LONG_MAX );
    sal_Bool bTooBig = pFnt->GetSize( pFnt->GetActual() ).Height() > nMax &&
                pFnt->GetHeight( pSh, pSh->GetOut() ) > nMax;
    SwFont* pTmpFnt;
    if( bTooBig )
    {
        pTmpFnt = new SwFont( *GetFont() );
        if( nMax >= 20 )
        {
            nMax *= 17;
            nMax /= 20;
        }
        pTmpFnt->SetSize( Size( 0, nMax ), pTmpFnt->GetActual() );
    }
    else
        pTmpFnt = GetFont();
    Point aTmpPos( nX, nY + nAsc );
    sal_Bool bPaint = sal_True;
    if( !IsClipChg() )
    {
        Size aSize = pTmpFnt->_GetTxtSize( pSh, pSh->GetOut(), aTmp );
        if( bGoLeft )
            aTmpPos.X() -= aSize.Width();
        SwRect aRct( aTmpPos, aSize );
        if( !aRect.IsInside( aRct ) )
        {
            if( aRct.Intersection( aRect ).IsEmpty() )
                bPaint = sal_False;
            else
                aClip.ChgClip( aRect );
        }
    }
    else if( bGoLeft )
        aTmpPos.X() -= pTmpFnt->_GetTxtSize( pSh, pSh->GetOut(), aTmp ).Width();
    aDrawInf.SetPos( aTmpPos );
    if( bPaint )
        pTmpFnt->_DrawText( aDrawInf );
    if( bTooBig )
        delete pTmpFnt;
    if( bRed )
    {
        long nDiff = bGoLeft ? nRedX - nX : nX - nRedX;
        if( nDiff > REDLINE_MINDIST )
            PaintRedline( nY, nMax );
    }
}

void SwExtraPainter::PaintRedline( SwTwips nY, long nMax )
{
    Point aStart( nRedX, nY );
    Point aEnd( nRedX, nY + nMax );
    if( !IsClipChg() )
    {
        SwRect aRct( aStart, aEnd );
        if( !aRect.IsInside( aRct ) )
        {
            if( aRct.Intersection( aRect ).IsEmpty() )
                return;
            aClip.ChgClip( aRect );
        }
    }
    const Color aOldCol( pSh->GetOut()->GetLineColor() );
    pSh->GetOut()->SetLineColor( SW_MOD()->GetRedlineMarkColor() );
    pSh->GetOut()->DrawLine( aStart, aEnd );
    pSh->GetOut()->SetLineColor( aOldCol );
}

void SwTxtFrm::PaintExtraData( const SwRect &rRect ) const
{
    if( Frm().Top() > rRect.Bottom() || Frm().Bottom() < rRect.Top() )
        return;
    const SwTxtNode& rTxtNode = *GetTxtNode();
    const SwLineNumberInfo &rLineInf = rTxtNode.GetDoc()->GetLineNumberInfo();
    const SwFmtLineNumber &rLineNum = GetAttrSet()->GetLineNumber();
    sal_Bool bLineNum = !IsInTab() && rLineInf.IsPaintLineNumbers() &&
               ( !IsInFly() || rLineInf.IsCountInFlys() ) && rLineNum.IsCount();
    SwHoriOrient eHor = (SwHoriOrient)SW_MOD()->GetRedlineMarkPos();
    sal_Bool bRedLine = eHor != HORI_NONE;
    if ( bLineNum || bRedLine )
    {
        if( IsLocked() || IsHiddenNow() || !Prt().Height() )
            return;
        ViewShell *pSh = GetShell();
        SwExtraPainter aExtra( this, pSh, rLineInf, rRect,
            rLineNum.GetStartValue(), eHor, bLineNum );

        if( HasPara() )
        {
            SwTxtFrmLocker aLock((SwTxtFrm*)this);

            SwTxtLineAccess aAccess( (SwTxtFrm*)this );
            SwParaPortion *pPara = aAccess.GetPara();

            OutputDevice *pOldRef = pSh->GetReferenzDevice();
            pSh->SetReferenzDevice( NULL );
            SwTxtPaintInfo aInf( (SwTxtFrm*)this, rRect );
            pSh->SetReferenzDevice( pOldRef );

            SwTxtPainter  aLine( (SwTxtFrm*)this, &aInf );
            sal_Bool bNoDummy = !aLine.GetNext(); // Nur eine Leerzeile!

            while( aLine.Y() + aLine.GetLineHeight() <= rRect.Top() )
            {
                if( !aLine.GetCurr()->IsDummy() &&
                    ( rLineInf.IsCountBlankLines() ||
                      aLine.GetCurr()->HasCntnt() ) )
                    aExtra.IncLineNr();
                if( !aLine.Next() )
                    return;
            }

            long nBottom = rRect.Bottom();

            sal_Bool bNoPrtLine;
            if( !( bNoPrtLine = 0 == GetMinPrtLine() ) )
            {
                while ( aLine.Y() < GetMinPrtLine() )
                {
                    if( ( rLineInf.IsCountBlankLines() || aLine.GetCurr()->HasCntnt() )
                        && !aLine.GetCurr()->IsDummy() )
                        aExtra.IncLineNr();
                    if( !aLine.Next() )
                        break;
                }
                bNoPrtLine = aLine.Y() >= GetMinPrtLine();
            }
            if( bNoPrtLine )
            {
                do
                {
                    if( bNoDummy || !aLine.GetCurr()->IsDummy() )
                    {
                        sal_Bool bRed = bRedLine && aLine.GetCurr()->HasRedline();
                        if( rLineInf.IsCountBlankLines() || aLine.GetCurr()->HasCntnt() )
                        {
                            if( bLineNum &&
                                ( aExtra.HasNumber() || aExtra.HasDivider() ) )
                            {
                                KSHORT nTmpHeight, nTmpAscent;
                                aLine.CalcAscentAndHeight( nTmpAscent, nTmpHeight );
                                aExtra.PaintExtra( aLine.Y(), nTmpAscent,
                                    nTmpHeight, bRed );
                                bRed = sal_False;
                            }
                            aExtra.IncLineNr();
                        }
                        if( bRed )
                            aExtra.PaintRedline( aLine.Y(), aLine.GetLineHeight() );
                    }
                } while( aLine.Next() && aLine.Y() <= nBottom );
            }
        }
        else
        {
            bRedLine &= (MSHRT_MAX!=rTxtNode.GetDoc()->GetRedlinePos(rTxtNode));

            if( bLineNum && rLineInf.IsCountBlankLines() &&
                ( aExtra.HasNumber() || aExtra.HasDivider() ) )
            {
                aExtra.PaintExtra( Frm().Top()+Prt().Top(), aExtra.GetFont()
                    ->GetAscent( pSh, pSh->GetOut() ), Prt().Height(), bRedLine );
            }
            else if( bRedLine )
                aExtra.PaintRedline( Frm().Top()+Prt().Top(), Prt().Height() );
        }
    }
}

/*************************************************************************
 *                      SwTxtFrm::Paint()
 *************************************************************************/

SwRect SwTxtFrm::Paint()
{
#ifdef DEBUG
    const SwTwips nDbgY = Frm().Top();
#endif

    // finger layout
    ASSERT( GetValidPosFlag(), "+SwTxtFrm::Paint: no Calc()" );

    SwRect aRet( Prt() );
    if ( IsEmpty() || !HasPara() )
        aRet += Frm().Pos();
    else
    {
        // AMA: Wir liefern jetzt mal das richtige Repaintrechteck zurueck,
        //      d.h. als linken Rand den berechneten PaintOfst!
        SwRepaint *pRepaint = GetPara()->GetRepaint();
        long l;
        if( pRepaint->GetOfst() )
            pRepaint->Left( pRepaint->GetOfst() );

        l = pRepaint->GetRightOfst();
        if( l && ( pRepaint->GetOfst() || l > pRepaint->Right() ) )
             pRepaint->Right( l );
        l = Frm().Bottom();
        pRepaint->SetOfst( 0 );
        if ( pRepaint->Bottom() > l )
            pRepaint->Bottom( l );
        aRet = *pRepaint;
    }
    ResetRepaint();
    return aRet;
}

/*************************************************************************
 *                      SwTxtFrm::Paint()
 *************************************************************************/

sal_Bool SwTxtFrm::PaintEmpty( const SwRect &rRect, sal_Bool bCheck ) const
{
    ViewShell *pSh = GetShell();
    if( pSh && ( pSh->GetViewOptions()->IsParagraph() || bInitFont ) )
    {
        bInitFont = sal_False;
        SwTxtFly aTxtFly( this );
        aTxtFly.SetTopRule();
        SwRect aRect;
        if( bCheck && aTxtFly.IsOn() && aTxtFly.IsAnyObj( aRect ) )
            return sal_False;
        else if( OUTDEV_PRINTER != pSh->GetOut()->GetOutDevType() )
        {
            SwFont *pFnt;
            const SwTxtNode& rTxtNode = *GetTxtNode();
            if ( rTxtNode.HasSwAttrSet() )
            {
                const SwAttrSet *pAttrSet = &( rTxtNode.GetSwAttrSet() );
                pFnt = new SwFont( pAttrSet );
            }
            else
            {
//FEATURE::CONDCOLL
//                  SwFontAccess aFontAccess( GetTxtNode()->GetFmtColl() );
                SwFontAccess aFontAccess( &rTxtNode.GetAnyFmtColl(), pSh );
//FEATURE::CONDCOLL
                pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
            }

            const SwDoc* pDoc = rTxtNode.GetDoc();
            if( ::IsShowChanges( pDoc->GetRedlineMode() ) )
            {
                MSHORT nRedlPos = pDoc->GetRedlinePos( rTxtNode );
                if( MSHRT_MAX != nRedlPos )
                    SwRedlineItr aRedln( rTxtNode, *pFnt, nRedlPos, sal_True );
            }

            if( pSh->GetViewOptions()->IsParagraph() && Prt().Height() )
            {
                const sal_Bool bAlter =
                    ( RTL_TEXTENCODING_SYMBOL == pFnt->GetCharSet( SW_LATIN ) )
#if defined( PM2 )
                            || ( System::GetCharSet() != CHARSET_IBMPC_850 )
#endif
                    ;

                if( bAlter && COMPARE_EQUAL !=
                    pFnt->GetName( SW_LATIN ).CompareToAscii( sBulletFntName ) )
                {
                    pFnt->SetFamily( FAMILY_DONTKNOW, SW_LATIN );
                    pFnt->SetName( XubString( sBulletFntName,
                        RTL_TEXTENCODING_MS_1252 ), SW_LATIN );
                    pFnt->SetStyleName( aEmptyStr, SW_LATIN );
                    pFnt->SetCharSet( RTL_TEXTENCODING_SYMBOL, SW_LATIN );
                }
                pFnt->Invalidate();
                pFnt->ChgPhysFnt( pSh, pSh->GetOut() );
                Point aPos = Frm().Pos() + Prt().Pos();
                SwSaveClip *pClip;
                if( IsUndersized() )
                {
                    pClip = new SwSaveClip( pSh->GetOut() );
                    pClip->ChgClip( rRect );
                }
                else
                    pClip = NULL;
                aPos.Y() += pFnt->GetAscent( pSh, pSh->GetOut() );
                const XubString aTmp( sal_Char( bAlter ? CH_PAR_ALTER : CH_PAR ),
                    RTL_TEXTENCODING_MS_1252 );
                SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), aTmp, 0, 1, 0,
                                            sal_False );
                aDrawInf.SetLeft( rRect.Left() );
                aDrawInf.SetRight( rRect.Right() );
                aDrawInf.SetPos( aPos );
                aDrawInf.SetSpace( 0 );
                aDrawInf.SetWrong( NULL );
                pFnt->_DrawText( aDrawInf );
                delete pClip;
            }
            delete pFnt;
            return sal_True;
        }
    }
    else
        return sal_True;
    return sal_False;
}

/*************************************************************************
 *                      SwTxtFrm::Paint()
 *************************************************************************/

void SwTxtFrm::Paint(const SwRect &rRect ) const
{
    ResetRepaint();
    DBG_LOOP_RESET;
    if( !IsEmpty() || !PaintEmpty( rRect, sal_True ) )
    {
#ifdef DEBUG
        const SwTwips nDbgY = Frm().Top();
#endif

#ifdef DBGTXT
        if( IsDbg( this ) )
            DBTXTFRM << "Paint()" << endl;
#endif
        if( IsLocked() || IsHiddenNow() || !Prt().Height() )
            return;

        //Kann gut sein, dass mir der IdleCollector mir die gecachten
        //Informationen entzogen hat.
        if( !HasPara() )
        {
            ASSERT( GetValidPosFlag(), "+SwTxtFrm::Paint: no Calc()" );
            ((SwTxtFrm*)this)->GetFormatted();
            if( IsEmpty() )
            {
                PaintEmpty( rRect, sal_False );
                return;
            }
            if( !HasPara() )
            {
                ASSERT( !this, "+SwTxtFrm::Paint: missing format information" );
                return;
            }
        }

        // Waehrend wir painten, wollen wir nicht gestoert werden.
        // Aber erst hinter dem Format() !
        SwTxtFrmLocker aLock((SwTxtFrm*)this);

        //Hier wird ggf. nur der Teil des TxtFrm ausgegeben, der sich veraendert
        //hat und der in dem Bereich liegt, dessen Ausgabe angefordert wurde.
        //Man kann jetzt auf die Idee kommen, dass der Bereich rRect ausgegeben
        //werden _muss_ obwohl rRepaint gesetzt ist; in der Tat kann dieses
        //Problem nicht formal vermieden werden. Gluecklicherweise koennen
        //wir davon ausgehen, dass rRepaint immer dann leer ist, wenn der Frm
        //komplett gepainted werden muss.
        SwTxtLineAccess aAccess( (SwTxtFrm*)this );
        SwParaPortion *pPara = aAccess.GetPara();

        SwRepaint &rRepaint = *(pPara->GetRepaint());

        // Das Recycling muss abgeschaltet werden, wenn wir uns im
        // FlyCntFrm befinden, weil ein DrawRect fuer die Retusche der
        // Zeile aufgerufen wird.
        if( rRepaint.GetOfst() )
        {
            const SwFlyFrm *pFly = FindFlyFrm();
            if( pFly && pFly->IsFlyInCntFrm() )
                rRepaint.SetOfst( 0 );
        }

        // Hier holen wir uns den String fuer die Ausgabe, besonders
        // die Laenge ist immer wieder interessant.

        ViewShell *pSh = GetShell();
        OutputDevice *pOldRef = pSh->GetReferenzDevice();
        pSh->SetReferenzDevice( NULL );
        SwTxtPaintInfo aInf( (SwTxtFrm*)this, rRect );
        pSh->SetReferenzDevice( pOldRef );
        aInf.SetWrongList( ( (SwTxtNode*)GetTxtNode() )->GetWrong() );
        aInf.GetTxtFly()->SetTopRule();

        SwTxtPainter  aLine( (SwTxtFrm*)this, &aInf );
        // Eine Optimierung, die sich lohnt: wenn kein freifliegender Frame
        // in unsere Zeile ragt, schaltet sich der SwTxtFly einfach ab:
        aInf.GetTxtFly()->Relax();

        OutputDevice *pOut = aInf.GetOut();
        const sal_Bool bOnWin = pSh->GetWin() != 0;

        SwSaveClip aClip( bOnWin || IsUndersized() ? pOut : 0 );

        // Ausgabeschleife: Fuer jede Zeile ... (die noch zu sehen ist) ...
        // rRect muss angepasst werden (Top+1, Bottom-1), weil der Iterator
        // die Zeilen nahtlos aneinanderfuegt.
        aLine.TwipsToLine( rRect.Top() + 1 );
        long nBottom = rRect.Bottom();

        sal_Bool bNoPrtLine;
        if( !( bNoPrtLine = 0 == GetMinPrtLine() ) )
        {
            while ( aLine.Y() < GetMinPrtLine() && aLine.Next() )
                ;
            bNoPrtLine = aLine.Y() >= GetMinPrtLine();
        }
        if( bNoPrtLine )
        {
            do
            {   DBG_LOOP;
                aLine.DrawTextLine( rRect, aClip, IsUndersized() );

            } while( aLine.Next() && aLine.Y() <= nBottom );
        }

        // Einmal reicht:
        if( aLine.IsPaintDrop() )
        {
#if NIE
            if( !bRetouche )
            {
                const SvxBrushItem *pItem; SwRect aOrigRect;
                GetBackgroundBrush( pItem, aOrigRect, sal_False, sal_True );
                aInf.SetBack( pItem, aOrigRect );
            }
#endif
            aLine.PaintDropPortion();
        }

#ifdef USED
        if( pSh && pSh->GetViewOptions()->IsTest2() )
            aInf.GetTxtFly()->ShowContour( pOut );
#endif

        // "Beep"
        if( bOneBeepOnly && bOnWin &&
            Frm().Top() + Prt().Top() + Prt().Height() < aLine.Y() &&
            ( GetFollow() || IsFollow() || GetNext() ) )
        {
            bOneBeepOnly = sal_False;
            Sound::Beep();
        }
        if( rRepaint.HasArea() )
            rRepaint.Clear();
    }
}

void SwTxtFrm::CriticalLines( const OutputDevice& rOut, SwStripes &rStripes,
    long nOffs)
{
    GetFormatted();
    if( HasPara() )
    {
        SwStripe aStripe( Frm().Top(), Prt().Top() );
        if( Prt().Top() )
        {
            rStripes.Insert( aStripe, rStripes.Count() );
            aStripe.Y() += Prt().Top();
        }
        SwLineLayout* pLay = GetPara();
        do
        {
            SwTwips nBase = aStripe.GetY() + pLay->GetAscent();
            if( rOut.LogicToPixel( Point( 0, nBase ) ).Y() !=
                rOut.LogicToPixel( Point( 0, nBase - nOffs ) ).Y() +
                rOut.LogicToPixel( Size( 0, nOffs ) ).Height() )
            {
                aStripe.Height() = pLay->GetRealHeight();
                rStripes.Insert( aStripe, rStripes.Count() );
            }
            aStripe.Y() += pLay->GetRealHeight();
            pLay = pLay->GetNext();
        } while( pLay );
        if( Prt().Top() + Prt().Height() < Frm().Height() )
        {
            aStripe.Height() = Frm().Height() - Prt().Top() - Prt().Height();
            rStripes.Insert( aStripe, rStripes.Count() );
        }
    }
    else if( Frm().Height() )
        rStripes.Insert(SwStripe(Frm().Top(),Frm().Height()), rStripes.Count());
}


