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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/text/HoriOrientation.hpp>
#include <hintids.hxx>
#include <vcl/sound.hxx>
#include <tools/shl.hxx> // SW_MOD
#include <editeng/pgrditem.hxx>
#include <editeng/lrspitem.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <paratr.hxx>

#include <fmtline.hxx>
#include <lineinfo.hxx>
#include <charfmt.hxx>
#include <pagefrm.hxx>
#include <viewsh.hxx>   // ViewShell
#include <viewimp.hxx>  // SwViewImp
#include <viewopt.hxx>  // SwViewOption
#include <frmtool.hxx>  // DrawGraphic
#include <txtcfg.hxx>
#include <txtfrm.hxx>       // SwTxtFrm
#include <itrpaint.hxx>     // SwTxtPainter
#include <txtpaint.hxx>     // SwSaveClip
#include <txtcache.hxx> // SwTxtLineAccess
#include <flyfrm.hxx>   // SwFlyFrm
#include <redlnitr.hxx> // SwRedlineItr
#include <swmodule.hxx> // SW_MOD
#include <tabfrm.hxx>   // SwTabFrm (Redlining)
#include <SwGrammarMarkUp.hxx>

// --> FME 2004-06-08 #i12836# enhanced pdf export
#include <EnhancedPDFExportHelper.hxx>
// <--

#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentLineNumberAccess.hxx>

// --> OD 2006-06-27 #b6440955#
// variable moved to class <numfunc:GetDefBulletConfig>
//extern const sal_Char __FAR_DATA sBulletFntName[];
namespace numfunc
{
    extern const String& GetDefBulletFontname();
    extern bool IsDefBulletFontUserDefined();
}
// <--


#define REDLINE_DISTANCE 567/4
#define REDLINE_MINDIST  567/10

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////

sal_Bool bInitFont = sal_True;

class SwExtraPainter
{
    SwSaveClip aClip;
    SwRect aRect;
    const SwTxtFrm* pTxtFrm;
    ViewShell *pSh;
    SwFont* pFnt;
    const SwLineNumberInfo &rLineInf;
    SwTwips nX;
    SwTwips nRedX;
    sal_uLong nLineNr;
    MSHORT nDivider;
    sal_Bool bGoLeft;
    sal_Bool bLineNum;
    inline sal_Bool IsClipChg() { return aClip.IsChg(); }
public:
    SwExtraPainter( const SwTxtFrm *pFrm, ViewShell *pVwSh,
        const SwLineNumberInfo &rLnInf, const SwRect &rRct,
        sal_Int16 eHor, sal_Bool bLnNm );
    ~SwExtraPainter() { delete pFnt; }
    inline SwFont* GetFont() const { return pFnt; }
    inline void IncLineNr() { ++nLineNr; }
    inline sal_Bool HasNumber() { return !( nLineNr % rLineInf.GetCountBy() ); }
    inline sal_Bool HasDivider() { if( !nDivider ) return sal_False;
        return !(nLineNr % rLineInf.GetDividerCountBy()); }

    void PaintExtra( SwTwips nY, long nAsc, long nMax, sal_Bool bRed );
    void PaintRedline( SwTwips nY, long nMax );
};


SwExtraPainter::SwExtraPainter( const SwTxtFrm *pFrm, ViewShell *pVwSh,
    const SwLineNumberInfo &rLnInf, const SwRect &rRct,
    sal_Int16 eHor, sal_Bool bLnNm )
    : aClip( pVwSh->GetWin() || pFrm->IsUndersized() ? pVwSh->GetOut() : 0 ),
      aRect( rRct ), pTxtFrm( pFrm ), pSh( pVwSh ), pFnt( 0 ), rLineInf( rLnInf ),
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
        SwCharFmt* pFmt = rLineInf.GetCharFmt( const_cast<IDocumentStylePoolAccess&>(*pFrm->GetNode()->getIDocumentStylePoolAccess()) );
        ASSERT( pFmt, "PaintExtraData without CharFmt" );
        pFnt = new SwFont( &pFmt->GetAttrSet(), pFrm->GetTxtNode()->getIDocumentSettingAccess() );
        pFnt->Invalidate();
        pFnt->ChgPhysFnt( pSh, *pSh->GetOut() );
        pFnt->SetVertical( 0, pFrm->IsVertical() );
        nLineNr += pFrm->GetAllLines() - pFrm->GetThisLines();
        LineNumberPosition ePos = rLineInf.GetPos();
        if( ePos != LINENUMBER_POS_LEFT && ePos != LINENUMBER_POS_RIGHT )
        {
            if( pFrm->FindPageFrm()->OnRightPage() )
            {
                nVirtPageNum = 1;
                ePos = ePos == LINENUMBER_POS_INSIDE ?
                        LINENUMBER_POS_LEFT : LINENUMBER_POS_RIGHT;
            }
            else
            {
                nVirtPageNum = 2;
                ePos = ePos == LINENUMBER_POS_OUTSIDE ?
                        LINENUMBER_POS_LEFT : LINENUMBER_POS_RIGHT;
            }
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
    if( eHor != text::HoriOrientation::NONE )
    {
        if( text::HoriOrientation::INSIDE == eHor || text::HoriOrientation::OUTSIDE == eHor )
        {
            if( !nVirtPageNum )
                nVirtPageNum = pFrm->FindPageFrm()->OnRightPage() ? 1 : 2;
            if( nVirtPageNum % 2 )
                eHor = eHor == text::HoriOrientation::INSIDE ? text::HoriOrientation::LEFT : text::HoriOrientation::RIGHT;
            else
                eHor = eHor == text::HoriOrientation::OUTSIDE ? text::HoriOrientation::LEFT : text::HoriOrientation::RIGHT;
        }
        const SwFrm* pTmpFrm = pFrm->FindTabFrm();
        if( !pTmpFrm )
            pTmpFrm = pFrm;
        nRedX = text::HoriOrientation::LEFT == eHor ? pTmpFrm->Frm().Left() - REDLINE_DISTANCE :
            pTmpFrm->Frm().Right() + REDLINE_DISTANCE;
    }
}

/*************************************************************************
 * SwExtraPainter::PaintExtra()
 **************************************************************************/

void SwExtraPainter::PaintExtra( SwTwips nY, long nAsc, long nMax, sal_Bool bRed )
{
    //Zeilennummer ist staerker als der Teiler
    const XubString aTmp( HasNumber() ? rLineInf.GetNumType().GetNumStr( nLineNr )
                                : rLineInf.GetDivider() );

    // get script type of line numbering:
    pFnt->SetActual( SwScriptInfo::WhichFont( 0, &aTmp, 0 ) );

    SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), 0, aTmp, 0, aTmp.Len() );
    aDrawInf.SetSpace( 0 );
    aDrawInf.SetWrong( NULL );
    aDrawInf.SetGrammarCheck( NULL );
    aDrawInf.SetSmartTags( NULL ); // SMARTTAGS
    aDrawInf.SetLeft( 0 );
    aDrawInf.SetRight( LONG_MAX );
    aDrawInf.SetFrm( pTxtFrm );
    aDrawInf.SetFont( pFnt );
    aDrawInf.SetSnapToGrid( sal_False );
    aDrawInf.SetIgnoreFrmRTL( sal_True );

    sal_Bool bTooBig = pFnt->GetSize( pFnt->GetActual() ).Height() > nMax &&
                pFnt->GetHeight( pSh, *pSh->GetOut() ) > nMax;
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
    Point aTmpPos( nX, nY );
    aTmpPos.Y() += nAsc;
    sal_Bool bPaint = sal_True;
    if( !IsClipChg() )
    {
        Size aSize = pTmpFnt->_GetTxtSize( aDrawInf );
        if( bGoLeft )
            aTmpPos.X() -= aSize.Width();
        // calculate rectangle containing the line number
        SwRect aRct( Point( aTmpPos.X(),
                         aTmpPos.Y() - pTmpFnt->GetAscent( pSh, *pSh->GetOut() )
                          ), aSize );
        if( !aRect.IsInside( aRct ) )
        {
            if( aRct.Intersection( aRect ).IsEmpty() )
                bPaint = sal_False;
            else
                aClip.ChgClip( aRect, pTxtFrm );
        }
    }
    else if( bGoLeft )
        aTmpPos.X() -= pTmpFnt->_GetTxtSize( aDrawInf ).Width();
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
            aClip.ChgClip( aRect, pTxtFrm );
        }
    }
    const Color aOldCol( pSh->GetOut()->GetLineColor() );
    pSh->GetOut()->SetLineColor( SW_MOD()->GetRedlineMarkColor() );

    if ( pTxtFrm->IsVertical() )
    {
        pTxtFrm->SwitchHorizontalToVertical( aStart );
        pTxtFrm->SwitchHorizontalToVertical( aEnd );
    }

    pSh->GetOut()->DrawLine( aStart, aEnd );
    pSh->GetOut()->SetLineColor( aOldCol );
}

void SwTxtFrm::PaintExtraData( const SwRect &rRect ) const
{
    if( Frm().Top() > rRect.Bottom() || Frm().Bottom() < rRect.Top() )
        return;

    const SwTxtNode& rTxtNode = *GetTxtNode();
    const IDocumentRedlineAccess* pIDRA = rTxtNode.getIDocumentRedlineAccess();
    const SwLineNumberInfo &rLineInf = rTxtNode.getIDocumentLineNumberAccess()->GetLineNumberInfo();
    const SwFmtLineNumber &rLineNum = GetAttrSet()->GetLineNumber();
    sal_Bool bLineNum = !IsInTab() && rLineInf.IsPaintLineNumbers() &&
               ( !IsInFly() || rLineInf.IsCountInFlys() ) && rLineNum.IsCount();
    sal_Int16 eHor = (sal_Int16)SW_MOD()->GetRedlineMarkPos();
    if( eHor != text::HoriOrientation::NONE && !IDocumentRedlineAccess::IsShowChanges( pIDRA->GetRedlineMode() ) )
        eHor = text::HoriOrientation::NONE;
    sal_Bool bRedLine = eHor != text::HoriOrientation::NONE;
    if ( bLineNum || bRedLine )
    {
        if( IsLocked() || IsHiddenNow() || !Prt().Height() )
            return;
        ViewShell *pSh = GetShell();

        SWAP_IF_NOT_SWAPPED( this )
        SwRect rOldRect( rRect );

        if ( IsVertical() )
            SwitchVerticalToHorizontal( (SwRect&)rRect );

        SwLayoutModeModifier aLayoutModeModifier( *pSh->GetOut() );
        aLayoutModeModifier.Modify( sal_False );

        // --> FME 2004-06-24 #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pSh->GetOut() );
        // <--

        SwExtraPainter aExtra( this, pSh, rLineInf, rRect, eHor, bLineNum );

        if( HasPara() )
        {
            SwTxtFrmLocker aLock((SwTxtFrm*)this);

            SwTxtLineAccess aAccess( (SwTxtFrm*)this );
            aAccess.GetPara();

            SwTxtPaintInfo aInf( (SwTxtFrm*)this, rRect );

            aLayoutModeModifier.Modify( sal_False );

            SwTxtPainter  aLine( (SwTxtFrm*)this, &aInf );
            sal_Bool bNoDummy = !aLine.GetNext(); // Nur eine Leerzeile!

            while( aLine.Y() + aLine.GetLineHeight() <= rRect.Top() )
            {
                if( !aLine.GetCurr()->IsDummy() &&
                    ( rLineInf.IsCountBlankLines() ||
                      aLine.GetCurr()->HasCntnt() ) )
                    aExtra.IncLineNr();
                if( !aLine.Next() )
                {
                    (SwRect&)rRect = rOldRect;
                    UNDO_SWAP( this )
                    return;
                }
            }

            long nBottom = rRect.Bottom();

            sal_Bool bNoPrtLine = 0 == GetMinPrtLine();
            if( !bNoPrtLine )
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
            bRedLine &= ( MSHRT_MAX!= pIDRA->GetRedlinePos(rTxtNode, USHRT_MAX) );

            if( bLineNum && rLineInf.IsCountBlankLines() &&
                ( aExtra.HasNumber() || aExtra.HasDivider() ) )
            {
                aExtra.PaintExtra( Frm().Top()+Prt().Top(), aExtra.GetFont()
                    ->GetAscent( pSh, *pSh->GetOut() ), Prt().Height(), bRedLine );
            }
            else if( bRedLine )
                aExtra.PaintRedline( Frm().Top()+Prt().Top(), Prt().Height() );
        }

        (SwRect&)rRect = rOldRect;
        UNDO_SWAP( this )
    }
}

/*************************************************************************
 *                      SwTxtFrm::Paint()
 *************************************************************************/

SwRect SwTxtFrm::Paint()
{
#if OSL_DEBUG_LEVEL > 1
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
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
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        if ( IsVertLR() ) // mba: the following line was added, but we don't need it for the existing directions; kept for IsVertLR(), but should be checked
            pRepaint->Chg( ( GetUpper()->Frm() ).Pos() + ( GetUpper()->Prt() ).Pos(), ( GetUpper()->Prt() ).SSize() );

        if( pRepaint->GetOfst() )
            pRepaint->Left( pRepaint->GetOfst() );

        l = pRepaint->GetRightOfst();
        if( l && ( pRepaint->GetOfst() || l > pRepaint->Right() ) )
             pRepaint->Right( l );
        pRepaint->SetOfst( 0 );
        aRet = *pRepaint;

        if ( IsRightToLeft() )
            SwitchLTRtoRTL( aRet );

        if ( IsVertical() )
            SwitchHorizontalToVertical( aRet );
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
        else if( pSh->GetWin() )
        {
            SwFont *pFnt;
            const SwTxtNode& rTxtNode = *GetTxtNode();
            if ( rTxtNode.HasSwAttrSet() )
            {
                const SwAttrSet *pAttrSet = &( rTxtNode.GetSwAttrSet() );
                pFnt = new SwFont( pAttrSet, rTxtNode.getIDocumentSettingAccess() );
            }
            else
            {
                SwFontAccess aFontAccess( &rTxtNode.GetAnyFmtColl(), pSh );
                pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
            }

            const IDocumentRedlineAccess* pIDRA = rTxtNode.getIDocumentRedlineAccess();
            if( IDocumentRedlineAccess::IsShowChanges( pIDRA->GetRedlineMode() ) )
            {
                MSHORT nRedlPos = pIDRA->GetRedlinePos( rTxtNode, USHRT_MAX );
                if( MSHRT_MAX != nRedlPos )
                {
                    SwAttrHandler aAttrHandler;
                    aAttrHandler.Init(  rTxtNode.GetSwAttrSet(),
                                       *rTxtNode.getIDocumentSettingAccess(), NULL );
                    SwRedlineItr aRedln( rTxtNode, *pFnt, aAttrHandler, nRedlPos, sal_True );
                }
            }

            if( pSh->GetViewOptions()->IsParagraph() && Prt().Height() )
            {
                if( RTL_TEXTENCODING_SYMBOL == pFnt->GetCharSet( SW_LATIN ) &&
                    pFnt->GetName( SW_LATIN ) != numfunc::GetDefBulletFontname() )
                {
                    pFnt->SetFamily( FAMILY_DONTKNOW, SW_LATIN );
                    pFnt->SetName( numfunc::GetDefBulletFontname(), SW_LATIN );
                    pFnt->SetStyleName( aEmptyStr, SW_LATIN );
                    pFnt->SetCharSet( RTL_TEXTENCODING_SYMBOL, SW_LATIN );
                }
                pFnt->SetVertical( 0, IsVertical() );
                SwFrmSwapper aSwapper( this, sal_True );
                SwLayoutModeModifier aLayoutModeModifier( *pSh->GetOut() );
                aLayoutModeModifier.Modify( IsRightToLeft() );

                pFnt->Invalidate();
                pFnt->ChgPhysFnt( pSh, *pSh->GetOut() );
                Point aPos = Frm().Pos() + Prt().Pos();

                const SvxLRSpaceItem &rSpace =
                    GetTxtNode()->GetSwAttrSet().GetLRSpace();

                if ( rSpace.GetTxtFirstLineOfst() > 0 )
                    aPos.X() += rSpace.GetTxtFirstLineOfst();

                SwSaveClip *pClip;
                if( IsUndersized() )
                {
                    pClip = new SwSaveClip( pSh->GetOut() );
                    pClip->ChgClip( rRect );
                }
                else
                    pClip = NULL;

                aPos.Y() += pFnt->GetAscent( pSh, *pSh->GetOut() );

                if ( GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() &&
                     IsInDocBody() )
                {
                    GETGRID( FindPageFrm() )
                    if ( pGrid )
                    {
                        // center character in grid line
                        aPos.Y() += ( pGrid->GetBaseHeight() -
                                      pFnt->GetHeight( pSh, *pSh->GetOut() ) ) / 2;

                        if ( ! pGrid->GetRubyTextBelow() )
                            aPos.Y() += pGrid->GetRubyHeight();
                    }
                }

                const XubString aTmp( CH_PAR );
                SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), 0, aTmp, 0, 1 );
                aDrawInf.SetLeft( rRect.Left() );
                aDrawInf.SetRight( rRect.Right() );
                aDrawInf.SetPos( aPos );
                aDrawInf.SetSpace( 0 );
                aDrawInf.SetKanaComp( 0 );
                aDrawInf.SetWrong( NULL );
                aDrawInf.SetGrammarCheck( NULL );
                aDrawInf.SetSmartTags( NULL ); // SMARTTAGS
                aDrawInf.SetFrm( this );
                aDrawInf.SetFont( pFnt );
                aDrawInf.SetSnapToGrid( sal_False );

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

void SwTxtFrm::Paint(SwRect const& rRect, SwPrintData const*const) const
{
    ResetRepaint();

    // --> FME 2004-06-24 #i16816# tagged pdf support
    ViewShell *pSh = GetShell();

    Num_Info aNumInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelperNumbering( &aNumInfo, 0, 0, *pSh->GetOut() );

    Frm_Info aFrmInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelperParagraph( 0, &aFrmInfo, 0, *pSh->GetOut() );
    // <--

    DBG_LOOP_RESET;
    if( !IsEmpty() || !PaintEmpty( rRect, sal_True ) )
    {
#if OSL_DEBUG_LEVEL > 1
        const SwTwips nDbgY = Frm().Top();
        (void)nDbgY;
#endif

#ifdef DBGTXT
        if( IsDbg( this ) )
            DBTXTFRM << "Paint()" << endl;
#endif
        if( IsLocked() || IsHiddenNow() || ! Prt().HasArea() )
            return;

        //Kann gut sein, dass mir der IdleCollector mir die gecachten
        //Informationen entzogen hat.
        if( !HasPara() )
        {
            ASSERT( GetValidPosFlag(), "+SwTxtFrm::Paint: no Calc()" );

            // --> FME 2004-10-29 #i29062# pass info that we are currently
            // painting.
            ((SwTxtFrm*)this)->GetFormatted( true );
            // <--
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

        // Rectangle
        ASSERT( ! IsSwapped(), "A frame is swapped before Paint" );
        SwRect aOldRect( rRect );

        SWAP_IF_NOT_SWAPPED( this )

        if ( IsVertical() )
            SwitchVerticalToHorizontal( (SwRect&)rRect );

        if ( IsRightToLeft() )
            SwitchRTLtoLTR( (SwRect&)rRect );

        SwTxtPaintInfo aInf( (SwTxtFrm*)this, rRect );
        aInf.SetWrongList( ( (SwTxtNode*)GetTxtNode() )->GetWrong() );
        aInf.SetGrammarCheckList( ( (SwTxtNode*)GetTxtNode() )->GetGrammarCheck() );
        aInf.SetSmartTags( ( (SwTxtNode*)GetTxtNode() )->GetSmartTags() );  // SMARTTAGS
        aInf.GetTxtFly()->SetTopRule();

        SwTxtPainter  aLine( (SwTxtFrm*)this, &aInf );
        // Eine Optimierung, die sich lohnt: wenn kein freifliegender Frame
        // in unsere Zeile ragt, schaltet sich der SwTxtFly einfach ab:
        aInf.GetTxtFly()->Relax();

        OutputDevice* pOut = aInf.GetOut();
        const sal_Bool bOnWin = pSh->GetWin() != 0;

        SwSaveClip aClip( bOnWin || IsUndersized() ? pOut : 0 );

        // Ausgabeschleife: Fuer jede Zeile ... (die noch zu sehen ist) ...
        // rRect muss angepasst werden (Top+1, Bottom-1), weil der Iterator
        // die Zeilen nahtlos aneinanderfuegt.
        aLine.TwipsToLine( rRect.Top() + 1 );
        long nBottom = rRect.Bottom();

        sal_Bool bNoPrtLine = 0 == GetMinPrtLine();
        if( !bNoPrtLine )
        {
            while ( aLine.Y() < GetMinPrtLine() && aLine.Next() )
                ;
            bNoPrtLine = aLine.Y() >= GetMinPrtLine();
        }
        if( bNoPrtLine )
        {
            do
            {
                //DBG_LOOP; shadows declaration above.
                //resolved into:
#if  OSL_DEBUG_LEVEL > 1
#ifdef DBG_UTIL
                DbgLoop aDbgLoop2( (const void*) this );
#endif
#endif
                aLine.DrawTextLine( rRect, aClip, IsUndersized() );

            } while( aLine.Next() && aLine.Y() <= nBottom );
        }

        // Einmal reicht:
        if( aLine.IsPaintDrop() )
            aLine.PaintDropPortion();

        if( rRepaint.HasArea() )
            rRepaint.Clear();

        UNDO_SWAP( this )
        (SwRect&)rRect = aOldRect;

        ASSERT( ! IsSwapped(), "A frame is swapped after Paint" );
    }
}

