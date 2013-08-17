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


#include <com/sun/star/text/HoriOrientation.hpp>
#include <hintids.hxx>
#include <tools/shl.hxx> // SW_MOD
#include <editeng/pgrditem.hxx>
#include <editeng/lrspitem.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <paratr.hxx>

#include <fmtline.hxx>
#include <lineinfo.hxx>
#include <charfmt.hxx>
#include "rootfrm.hxx"
#include <pagefrm.hxx>
#include <viewsh.hxx>   // ViewShell
#include <viewimp.hxx>  // SwViewImp
#include <viewopt.hxx>  // SwViewOption
#include <frmtool.hxx>  // DrawGraphic
#include <txtfrm.hxx>   // SwTxtFrm
#include <itrpaint.hxx> // SwTxtPainter
#include <txtpaint.hxx> // SwSaveClip
#include <txtcache.hxx> // SwTxtLineAccess
#include <flyfrm.hxx>   // SwFlyFrm
#include <redlnitr.hxx> // SwRedlineItr
#include <swmodule.hxx> // SW_MOD
#include <tabfrm.hxx>   // SwTabFrm (Redlining)
#include <SwGrammarMarkUp.hxx>

// #i12836# enhanced pdf export
#include <EnhancedPDFExportHelper.hxx>

#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentLineNumberAccess.hxx>

// variable moved to class <numfunc:GetDefBulletConfig>
//extern const sal_Char sBulletFntName[];
namespace numfunc
{
    extern const String& GetDefBulletFontname();
    extern bool IsDefBulletFontUserDefined();
}


#define REDLINE_DISTANCE 567/4
#define REDLINE_MINDIST  567/10

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////

bool bInitFont = true;

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
    bool bGoLeft;
    bool bLineNum;
    inline bool IsClipChg() { return aClip.IsChg(); }
public:
    SwExtraPainter( const SwTxtFrm *pFrm, ViewShell *pVwSh,
        const SwLineNumberInfo &rLnInf, const SwRect &rRct,
        sal_Int16 eHor, bool bLnNm );
    ~SwExtraPainter() { delete pFnt; }
    inline SwFont* GetFont() const { return pFnt; }
    inline void IncLineNr() { ++nLineNr; }
    inline bool HasNumber() { return !( nLineNr % rLineInf.GetCountBy() ); }
    inline bool HasDivider() { if( !nDivider ) return false;
        return !(nLineNr % rLineInf.GetDividerCountBy()); }

    void PaintExtra( SwTwips nY, long nAsc, long nMax, bool bRed );
    void PaintRedline( SwTwips nY, long nMax );
};


SwExtraPainter::SwExtraPainter( const SwTxtFrm *pFrm, ViewShell *pVwSh,
    const SwLineNumberInfo &rLnInf, const SwRect &rRct,
    sal_Int16 eHor, bool bLnNm )
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
        {/* Initializes the Members necessary for line numbering:

            nDivider,   how often do we want a substring; 0 == never
            nX,         line number's x position
            pFnt,       line number's font
            nLineNr,    the first line number
            bLineNum is set back to sal_False if the numbering is completely
            outside of the paint rect */
        nDivider = !rLineInf.GetDivider().isEmpty() ? rLineInf.GetDividerCountBy() : 0;
        nX = pFrm->Frm().Left();
        SwCharFmt* pFmt = rLineInf.GetCharFmt( const_cast<IDocumentStylePoolAccess&>(*pFrm->GetNode()->getIDocumentStylePoolAccess()) );
        OSL_ENSURE( pFmt, "PaintExtraData without CharFmt" );
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
            bGoLeft = true;
            nX -= rLineInf.GetPosFromLeft();
            if( nX < aRect.Left() )
                bLineNum = false;
        }
        else
        {
            bGoLeft = false;
            nX += pFrm->Frm().Width() + rLineInf.GetPosFromLeft();
            if( nX > aRect.Right() )
                bLineNum = false;
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

void SwExtraPainter::PaintExtra( SwTwips nY, long nAsc, long nMax, bool bRed )
{
  // Line number is stronger than the divider
    const OUString aTmp( HasNumber() ? rLineInf.GetNumType().GetNumStr( nLineNr )
                                : rLineInf.GetDivider() );

    // Get script type of line numbering:
    pFnt->SetActual( SwScriptInfo::WhichFont( 0, &aTmp, 0 ) );

    SwDrawTextInfo aDrawInf( pSh, *pSh->GetOut(), 0, aTmp, 0, aTmp.getLength() );
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

    bool bTooBig = pFnt->GetSize( pFnt->GetActual() ).Height() > nMax &&
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
    bool bPaint = true;
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
                bPaint = false;
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
    bool bLineNum = !IsInTab() && rLineInf.IsPaintLineNumbers() &&
               ( !IsInFly() || rLineInf.IsCountInFlys() ) && rLineNum.IsCount();
    sal_Int16 eHor = (sal_Int16)SW_MOD()->GetRedlineMarkPos();
    if( eHor != text::HoriOrientation::NONE && !IDocumentRedlineAccess::IsShowChanges( pIDRA->GetRedlineMode() ) )
        eHor = text::HoriOrientation::NONE;
    bool bRedLine = eHor != text::HoriOrientation::NONE;
    if ( bLineNum || bRedLine )
    {
        if( IsLocked() || IsHiddenNow() || !Prt().Height() )
            return;
        ViewShell *pSh = getRootFrm()->GetCurrShell();

        SWAP_IF_NOT_SWAPPED( this )
        SwRect rOldRect( rRect );

        if ( IsVertical() )
            SwitchVerticalToHorizontal( (SwRect&)rRect );

        SwLayoutModeModifier aLayoutModeModifier( *pSh->GetOut() );
        aLayoutModeModifier.Modify( sal_False );

        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pSh->GetOut() );

        SwExtraPainter aExtra( this, pSh, rLineInf, rRect, eHor, bLineNum );

        if( HasPara() )
        {
            SwTxtFrmLocker aLock((SwTxtFrm*)this);

            SwTxtLineAccess aAccess( (SwTxtFrm*)this );
            aAccess.GetPara();

            SwTxtPaintInfo aInf( (SwTxtFrm*)this, rRect );

            aLayoutModeModifier.Modify( sal_False );

            SwTxtPainter  aLine( (SwTxtFrm*)this, &aInf );
            bool bNoDummy = !aLine.GetNext(); // Only one empty line!

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

            bool bNoPrtLine = 0 == GetMinPrtLine();
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
                        bool bRed = bRedLine && aLine.GetCurr()->HasRedline();
                        if( rLineInf.IsCountBlankLines() || aLine.GetCurr()->HasCntnt() )
                        {
                            if( bLineNum &&
                                ( aExtra.HasNumber() || aExtra.HasDivider() ) )
                            {
                                KSHORT nTmpHeight, nTmpAscent;
                                aLine.CalcAscentAndHeight( nTmpAscent, nTmpHeight );
                                aExtra.PaintExtra( aLine.Y(), nTmpAscent,
                                    nTmpHeight, bRed );
                                bRed = false;
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
            if ( MSHRT_MAX == pIDRA->GetRedlinePos(rTxtNode, USHRT_MAX) )
                bRedLine = false;

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

SwRect SwTxtFrm::Paint()
{
#if OSL_DEBUG_LEVEL > 1
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
#endif

    // finger layout
    OSL_ENSURE( GetValidPosFlag(), "+SwTxtFrm::Paint: no Calc()" );

    SwRect aRet( Prt() );
    if ( IsEmpty() || !HasPara() )
        aRet += Frm().Pos();
    else
    {
        // We return the right paint rect. Use the calculated PaintOfst as the
        // left margin
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

sal_Bool SwTxtFrm::PaintEmpty( const SwRect &rRect, sal_Bool bCheck ) const
{
    ViewShell *pSh = getRootFrm()->GetCurrShell();
    if( pSh && ( pSh->GetViewOptions()->IsParagraph() || bInitFont ) )
    {
        bInitFont = false;
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

                // Don't show the paragraph mark for collapsed paragraphs, when they are hidden
                if ( EmptyHeight( ) > 1 )
                {
                    const OUString aTmp( CH_PAR );
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
                }
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

void SwTxtFrm::Paint(SwRect const& rRect, SwPrintData const*const) const
{
    ResetRepaint();

    // #i16816# tagged pdf support
    ViewShell *pSh = getRootFrm()->GetCurrShell();

    Num_Info aNumInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelperNumbering( &aNumInfo, 0, 0, *pSh->GetOut() );

    Frm_Info aFrmInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelperParagraph( 0, &aFrmInfo, 0, *pSh->GetOut() );

    if( !IsEmpty() || !PaintEmpty( rRect, sal_True ) )
    {
#if OSL_DEBUG_LEVEL > 1
        const SwTwips nDbgY = Frm().Top();
        (void)nDbgY;
#endif

        if( IsLocked() || IsHiddenNow() || ! Prt().HasArea() )
            return;

        // It can happen that the IdleCollector withdrew my cached information
        if( !HasPara() )
        {
            OSL_ENSURE( GetValidPosFlag(), "+SwTxtFrm::Paint: no Calc()" );

            // #i29062# pass info that we are currently
            // painting.
            ((SwTxtFrm*)this)->GetFormatted( true );
            if( IsEmpty() )
            {
                PaintEmpty( rRect, sal_False );
                return;
            }
            if( !HasPara() )
            {
                OSL_ENSURE( !this, "+SwTxtFrm::Paint: missing format information" );
                return;
            }
        }

        // We don't want to be interrupted while painting.
        // Do that after thr Format()!
        SwTxtFrmLocker aLock((SwTxtFrm*)this);

        // We only paint the part of the TxtFrm which changed, is within the
        // range and was requested to paint.
        // One could think that the area rRect _needs_ to be painted, although
        // rRepaint is set. Indeed, we cannot avoid this problem from a formal
        // perspective. Luckily we can assume rRepaint to be empty when we need
        // paint the while Frm.
        SwTxtLineAccess aAccess( (SwTxtFrm*)this );
        SwParaPortion *pPara = aAccess.GetPara();

        SwRepaint &rRepaint = *(pPara->GetRepaint());

        // Switch off recycling when in the FlyCntFrm.
        // A DrawRect is called for repainting the line anyways.
        if( rRepaint.GetOfst() )
        {
            const SwFlyFrm *pFly = FindFlyFrm();
            if( pFly && pFly->IsFlyInCntFrm() )
                rRepaint.SetOfst( 0 );
        }

        // Ge the String for painting. The length is of special interest.

        // Rectangle
        OSL_ENSURE( ! IsSwapped(), "A frame is swapped before Paint" );
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
        // Optimization: if no free flying Frm overlaps into our line, the
        // SwTxtFly just switches off
        aInf.GetTxtFly()->Relax();

        OutputDevice* pOut = aInf.GetOut();
        const bool bOnWin = pSh->GetWin() != 0;

        SwSaveClip aClip( bOnWin || IsUndersized() ? pOut : 0 );

        // Output loop: For each Line ... (which is still visible) ...
        //   adapt rRect (Top + 1, Bottom - 1)
        // Because the Iterator attaches the Lines without a gap to each other
        aLine.TwipsToLine( rRect.Top() + 1 );
        long nBottom = rRect.Bottom();

        bool bNoPrtLine = 0 == GetMinPrtLine();
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
                aLine.DrawTextLine( rRect, aClip, IsUndersized() );

            } while( aLine.Next() && aLine.Y() <= nBottom );
        }

        // Once is enough:
        if( aLine.IsPaintDrop() )
            aLine.PaintDropPortion();

        if( rRepaint.HasArea() )
            rRepaint.Clear();

        UNDO_SWAP( this )
        (SwRect&)rRect = aOldRect;

        OSL_ENSURE( ! IsSwapped(), "A frame is swapped after Paint" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
