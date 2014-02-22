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


#include "hintids.hxx"
#include <comphelper/string.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <rtl/ustrbuf.hxx>
#include <IDocumentSettingAccess.hxx>
#include <frmatr.hxx>
#include <SwPortionHandler.hxx>

#include "viewopt.hxx"
#include "portab.hxx"
#include "inftxt.hxx"
#include "itrform2.hxx"
#include "txtfrm.hxx"
#include <numrule.hxx>

#include <porfld.hxx>


/*************************************************************************
 *                    SwLineInfo::GetTabStop()
 *************************************************************************/


/* Return the first tab stop that is > nSearchPos.
 * If the tab stop is outside the print area, we
 * return 0 if it is not the first tab stop.*/
const SvxTabStop *SwLineInfo::GetTabStop( const SwTwips nSearchPos,
                                         const SwTwips nRight ) const
{
    for( MSHORT i = 0; i < pRuler->Count(); ++i )
    {
        const SvxTabStop &rTabStop = pRuler->operator[](i);
        if( rTabStop.GetTabPos() > SwTwips(nRight) )
            return i ? 0 : &rTabStop;

        if( rTabStop.GetTabPos() > nSearchPos )
            return &rTabStop;
    }
    return 0;
}

/*************************************************************************
 *                    SwLineInfo::NumberOfTabStops()
 *************************************************************************/

sal_uInt16 SwLineInfo::NumberOfTabStops() const
{
    return pRuler->Count();
}

/*************************************************************************
 *                      SwTxtFormatter::NewTabPortion()
 *************************************************************************/
SwTabPortion *SwTxtFormatter::NewTabPortion( SwTxtFormatInfo &rInf, bool bAuto ) const
{
    sal_Unicode cFill = 0;
    sal_Unicode cDec = 0;
    SvxTabAdjust eAdj;

    KSHORT nNewTabPos;
    bool bAutoTabStop = true;
    {
        const bool bRTL = pFrm->IsRightToLeft();
        
        
        //
        
        const bool bTabsRelativeToIndent =
            pFrm->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT);
        const SwTwips nTabLeft = bRTL
                                 ? pFrm->Frm().Right() -
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 )
                                 : pFrm->Frm().Left() +
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 );

        //
        
        //
        SwTwips nLinePos = GetLeftMargin();
        if ( bRTL )
        {
            Point aPoint( nLinePos, 0 );
            pFrm->SwitchLTRtoRTL( aPoint );
            nLinePos = aPoint.X();
        }

        //
        
        //
        SwTwips nTabPos = rInf.GetLastTab() ? rInf.GetLastTab()->GetTabPos() : 0;
        if( nTabPos < rInf.X() )
        {
            nTabPos = rInf.X();
        }

        //
        
        //
        const SwTwips nCurrentAbsPos = bRTL ?
                                       nLinePos - nTabPos :
                                       nLinePos + nTabPos;

       
        SwTwips nMyRight;
        if ( pFrm->IsVertLR() )
           nMyRight = Left();
        else
           nMyRight = Right();

        if ( pFrm->IsVertical() )
        {
            Point aRightTop( nMyRight, pFrm->Frm().Top() );
            pFrm->SwitchHorizontalToVertical( aRightTop );
            nMyRight = aRightTop.Y();
        }

        SwTwips nNextPos = 0;

        
        
        //
        const SwTwips nSearchPos = bRTL ?
                                   nTabLeft - nCurrentAbsPos :
                                   nCurrentAbsPos - nTabLeft;

        //
        
        
        
        
        //
        const SvxTabStop* pTabStop = aLineInf.GetTabStop( nSearchPos, nMyRight );
        if ( pTabStop )
        {
            cFill = ' ' != pTabStop->GetFill() ? pTabStop->GetFill() : 0;
            cDec = pTabStop->GetDecimal();
            eAdj = pTabStop->GetAdjustment();
            nNextPos = pTabStop->GetTabPos();
            if(!bTabsRelativeToIndent && eAdj == SVX_TAB_ADJUST_DEFAULT && nSearchPos < 0)
            {
                
                nNextPos = ( nSearchPos / nNextPos ) * nNextPos;
            }
            bAutoTabStop = false;
        }
        else
        {
            KSHORT nDefTabDist = aLineInf.GetDefTabStop();
            if( KSHRT_MAX == nDefTabDist )
            {
                const SvxTabStopItem& rTab =
                    (const SvxTabStopItem &)pFrm->GetAttrSet()->
                    GetPool()->GetDefaultItem( RES_PARATR_TABSTOP );
                if( rTab.Count() )
                    nDefTabDist = (KSHORT)rTab[0].GetTabPos();
                else
                    nDefTabDist = SVX_TAB_DEFDIST;
                aLineInf.SetDefTabStop( nDefTabDist );
            }
            SwTwips nCount = nSearchPos;

            
            if (nDefTabDist <= 0)
                nDefTabDist = 1;

            nCount /= nDefTabDist;
            nNextPos = ( nCount < 0 || ( !nCount && nSearchPos <= 0 ) )
                       ? ( nCount * nDefTabDist )
                       : ( ( nCount + 1 ) * nDefTabDist );
            
            const SwTwips nMinimumTabWidth = pFrm->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT) ? 0 : 50;
            if( (  bRTL && nTabLeft - nNextPos >= nCurrentAbsPos - nMinimumTabWidth ) ||
                 ( !bRTL && nNextPos + nTabLeft <= nCurrentAbsPos + nMinimumTabWidth  ) )
            {
                nNextPos += nDefTabDist;
            }
            cFill = 0;
            eAdj = SVX_TAB_ADJUST_LEFT;
        }

        
        
        
        
        
        
        
        {
            long nLeftMarginTabPos = 0;
            {
                if ( !bTabsRelativeToIndent )
                {
                    if ( bRTL )
                    {
                        Point aPoint( Left(), 0 );
                        pFrm->SwitchLTRtoRTL( aPoint );
                        nLeftMarginTabPos = pFrm->Frm().Right() - aPoint.X();
                    }
                    else
                    {
                        nLeftMarginTabPos = Left() - pFrm->Frm().Left();
                    }
                }
                if( pCurr->HasForcedLeftMargin() )
                {
                    SwLinePortion* pPor = pCurr->GetPortion();
                    while( pPor && !pPor->IsFlyPortion() )
                    {
                        pPor = pPor->GetPortion();
                    }
                    if ( pPor )
                    {
                        nLeftMarginTabPos += pPor->Width();
                    }
                }
            }
            const bool bNewTabPortionInsideHangingIndent =
                        bRTL ? nCurrentAbsPos > nTabLeft - nLeftMarginTabPos
                             : nCurrentAbsPos < nTabLeft + nLeftMarginTabPos;
            if ( bNewTabPortionInsideHangingIndent )
            {
                
                
                
                
                
                const bool bTabAtLeftMarginAllowed =
                    ( !aLineInf.IsListTabStopIncluded() ||
                      !pTabStop ||
                      nNextPos != aLineInf.GetListTabStopPosition() ) ||
                    
                    pFrm->GetTxtNode()->getIDocumentSettingAccess()->
                        get(IDocumentSettingAccess::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST);
                if ( bTabAtLeftMarginAllowed )
                {
                    if ( !pTabStop || eAdj == SVX_TAB_ADJUST_DEFAULT ||
                         ( nNextPos > nLeftMarginTabPos ) )
                    {
                        eAdj = SVX_TAB_ADJUST_DEFAULT;
                        cFill = 0;
                        nNextPos = nLeftMarginTabPos;
                    }
                }
            }
        }

        nNextPos += bRTL ? nLinePos - nTabLeft : nTabLeft - nLinePos;
        OSL_ENSURE( nNextPos >= 0, "GetTabStop: Don't go back!" );
        nNewTabPos = KSHORT(nNextPos);
    }

    SwTabPortion *pTabPor = 0;
    if ( bAuto )
    {
        if ( SVX_TAB_ADJUST_DECIMAL == eAdj &&
             
             1 == aLineInf.NumberOfTabStops() )
            pTabPor = new SwAutoTabDecimalPortion( nNewTabPos, cDec, cFill );
    }
    else
    {
        switch( eAdj )
        {
        case SVX_TAB_ADJUST_RIGHT :
            {
                pTabPor = new SwTabRightPortion( nNewTabPos, cFill );
                break;
            }
        case SVX_TAB_ADJUST_CENTER :
            {
                pTabPor = new SwTabCenterPortion( nNewTabPos, cFill );
                break;
            }
        case SVX_TAB_ADJUST_DECIMAL :
            {
                pTabPor = new SwTabDecimalPortion( nNewTabPos, cDec, cFill );
                break;
            }
        default:
            {
                OSL_ENSURE( SVX_TAB_ADJUST_LEFT == eAdj || SVX_TAB_ADJUST_DEFAULT == eAdj,
                    "+SwTxtFormatter::NewTabPortion: unknown adjustment" );
                pTabPor = new SwTabLeftPortion( nNewTabPos, cFill, bAutoTabStop );
                break;
            }
        }
    }

    return pTabPor;
}

/*************************************************************************
 *                SwTabPortion::SwTabPortion()
 *************************************************************************/




SwTabPortion::SwTabPortion( const KSHORT nTabPosition, const sal_Unicode cFillChar, const bool bAutoTab )
    : SwFixPortion( 0, 0 ), nTabPos(nTabPosition), cFill(cFillChar), bAutoTabStop( bAutoTab )
{
    nLineLength = 1;
    OSL_ENSURE(!IsFilled() || ' ' != cFill, "SwTabPortion::CTOR: blanks ?!");
    SetWhichPor( POR_TAB );
}

/*************************************************************************
 *                 virtual SwTabPortion::Format()
 *************************************************************************/



bool SwTabPortion::Format( SwTxtFormatInfo &rInf )
{
    SwTabPortion *pLastTab = rInf.GetLastTab();
    if( pLastTab == this )
        return PostFormat( rInf );
    if( pLastTab )
        pLastTab->PostFormat( rInf );
    return PreFormat( rInf );
}

/*************************************************************************
 *                 virtual SwTabPortion::FormatEOL()
 *************************************************************************/



void SwTabPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    if( rInf.GetLastTab() == this && !IsTabLeftPortion() )
        PostFormat( rInf );
}

/*************************************************************************
 *                    SwTabPortion::PreFormat()
 *************************************************************************/



bool SwTabPortion::PreFormat( SwTxtFormatInfo &rInf )
{
    OSL_ENSURE( rInf.X() <= GetTabPos(), "SwTabPortion::PreFormat: rush hour" );

    
    Fix( static_cast<sal_uInt16>(rInf.X()) );

    const bool bTabCompat = rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);
    const bool bTabOverflow = rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_OVERFLOW);

    
    
    
    sal_uInt16 nMinimumTabWidth = 1;
    if ( !bTabCompat )
    {
        
        
        
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr< SwFontSave > pSave( 0 );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if ( GetLen() == 0 &&
             rInf.GetLast() && rInf.GetLast()->InNumberGrp() &&
             static_cast<SwNumberPortion*>(rInf.GetLast())->HasFont() )
        {
            const SwFont* pNumberPortionFont =
                    static_cast<SwNumberPortion*>(rInf.GetLast())->GetFont();
            pSave.reset( new SwFontSave( rInf, const_cast<SwFont*>(pNumberPortionFont) ) );
        }
        OUString aTmp( ' ' );
        SwTxtSizeInfo aInf( rInf, &aTmp );
        nMinimumTabWidth = aInf.GetTxtSize().Width();
    }
    PrtWidth( nMinimumTabWidth );

    
    
    
    bool bFull = ( bTabCompat && rInf.IsUnderFlow() ) ||
                     ( rInf.Width() <= rInf.X() + PrtWidth() && rInf.X() <= rInf.Width() ) ;

    
    const sal_uInt16 nDir = rInf.GetFont()->GetOrientation( rInf.GetTxtFrm()->IsVertical() );

    if( ! bFull && 0 == nDir )
    {
        const MSHORT nWhich = GetWhichPor();
        switch( nWhich )
        {
            case POR_TABRIGHT:
            case POR_TABDECIMAL:
            case POR_TABCENTER:
            {
                if( POR_TABDECIMAL == nWhich )
                    rInf.SetTabDecimal(
                        ((SwTabDecimalPortion*)this)->GetTabDecimal());
                rInf.SetLastTab( this );
                break;
            }
            case POR_TABLEFT:
            {
                PrtWidth( static_cast<sal_uInt16>(GetTabPos() - rInf.X()) );
                bFull = rInf.Width() <= rInf.X() + PrtWidth();

                
                
                
                bool bAtParaEnd = rInf.GetIdx() + GetLen() == rInf.GetTxt().getLength();
                if ( bFull && bTabCompat &&
                     ( ( bTabOverflow && ( rInf.IsTabOverflow() || !IsAutoTabStop() ) ) || bAtParaEnd ) &&
                     GetTabPos() >= rInf.GetTxtFrm()->Frm().Width() )
                {
                    bFull = false;
                    if ( bTabOverflow && !IsAutoTabStop() )
                        rInf.SetTabOverflow( true );
                }

                break;
            }
            default: OSL_ENSURE( !this, "SwTabPortion::PreFormat: unknown adjustment" );
        }
    }

    if( bFull )
    {
        
        if( rInf.GetIdx() == rInf.GetLineStart() &&
            
            
            !rInf.GetFly() )
        {
            PrtWidth( static_cast<sal_uInt16>(rInf.Width() - rInf.X()) );
            SetFixWidth( PrtWidth() );
        }
        else
        {
            Height( 0 );
            Width( 0 );
            SetLen( 0 );
            SetAscent( 0 );
            SetPortion( NULL ); 
        }
        return true;
    }
    else
    {
        
        
        SetFixWidth( PrtWidth() );
        return false;
    }
}

/*************************************************************************
 *                      SwTabPortion::PostFormat()
 *************************************************************************/



bool SwTabPortion::PostFormat( SwTxtFormatInfo &rInf )
{
    const bool bTabOverMargin = rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_OVER_MARGIN);
    
    
    const KSHORT nRight = bTabOverMargin ? GetTabPos() : std::min(GetTabPos(), rInf.Width());
    const SwLinePortion *pPor = GetPortion();

    KSHORT nPorWidth = 0;
    while( pPor )
    {
        nPorWidth = nPorWidth + pPor->Width();
        pPor = pPor->GetPortion();
    }

    const MSHORT nWhich = GetWhichPor();
    OSL_ENSURE( POR_TABLEFT != nWhich, "SwTabPortion::PostFormat: already formatted" );
    const bool bTabCompat = rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);

    
    if ( bTabCompat && POR_TABDECIMAL == nWhich )
    {
        KSHORT nPrePorWidth = static_cast<const SwTabDecimalPortion*>(this)->GetWidthOfPortionsUpToDecimalPosition();

        
        if ( USHRT_MAX != nPrePorWidth )
        {
            if ( nPrePorWidth && nPorWidth - nPrePorWidth > rInf.Width() - nRight )
            {
                nPrePorWidth += nPorWidth - nPrePorWidth - ( rInf.Width() - nRight );
            }

            nPorWidth = nPrePorWidth - 1;
        }
    }

    if( POR_TABCENTER == nWhich )
    {
        
        
        KSHORT nNewWidth = nPorWidth /2;
        if( nNewWidth > rInf.Width() - nRight )
            nNewWidth = nPorWidth - (rInf.Width() - nRight);
        nPorWidth = nNewWidth;
    }

    const KSHORT nDiffWidth = nRight - Fix();

    if( nDiffWidth > nPorWidth )
    {
        const KSHORT nOldWidth = GetFixWidth();
        const KSHORT nAdjDiff = nDiffWidth - nPorWidth;
        if( nAdjDiff > GetFixWidth() )
            PrtWidth( nAdjDiff );
        
        
        
        rInf.X( rInf.X() + PrtWidth() - nOldWidth );
    }
    SetFixWidth( PrtWidth() );
    
    rInf.SetLastTab(0);
    if( POR_TABDECIMAL == nWhich )
        rInf.SetTabDecimal(0);

    return rInf.Width() <= rInf.X();
}

/*************************************************************************
 *                virtual SwTabPortion::Paint()
 *
 * Ex: LineIter::DrawTab()
 *************************************************************************/

void SwTabPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
#if OSL_DEBUG_LEVEL > 1
    
    if( rInf.OnWin() && OPTDBG( rInf ) &&
        !rInf.GetOpt().IsPagePreview() && \
        !rInf.GetOpt().IsReadonly() && \
        SwViewOption::IsFieldShadings()    )
    {
        const KSHORT nTmpWidth = PrtWidth();
        ((SwTabPortion*)this)->PrtWidth( GetFixWidth() );
        rInf.DrawViewOpt( *this, POR_TAB );
        ((SwTabPortion*)this)->PrtWidth( nTmpWidth );
    }
#endif

    
    
    
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< SwFontSave > pSave( 0 );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    bool bAfterNumbering = false;
    if ( GetLen() == 0 )
    {
        const SwLinePortion* pPrevPortion =
            const_cast<SwTabPortion*>(this)->FindPrevPortion( rInf.GetParaPortion() );
        if ( pPrevPortion &&
             pPrevPortion->InNumberGrp() &&
             static_cast<const SwNumberPortion*>(pPrevPortion)->HasFont() )
        {
            const SwFont* pNumberPortionFont =
                    static_cast<const SwNumberPortion*>(pPrevPortion)->GetFont();
            pSave.reset( new SwFontSave( rInf, const_cast<SwFont*>(pNumberPortionFont) ) );
            bAfterNumbering = true;
        }
    }
    rInf.DrawBackBrush( *this );
    if( !bAfterNumbering )
        rInf.DrawBorder( *this );

    
    if( rInf.OnWin() && pPortion && !pPortion->Width() )
        pPortion->PrePaint( rInf, this );

    
    if( rInf.OnWin() && rInf.GetOpt().IsTab() )
    {
        
        if( IsFilled() )
            rInf.DrawViewOpt( *this, POR_TAB );
        else
            rInf.DrawTab( *this );
    }

    
    if( rInf.GetFont()->IsPaintBlank() )
    {
        
        const KSHORT nCharWidth = rInf.GetTxtSize(OUString(' ')).Width();
        
        if( nCharWidth )
        {
            
            KSHORT nChar = Width() / nCharWidth;
            OUStringBuffer aBuf;
            comphelper::string::padToLength(aBuf, nChar, ' ');
            rInf.DrawText(aBuf.makeStringAndClear(), *this, 0, nChar, true);
        }
    }

    
    if( IsFilled() )
    {
        
        const KSHORT nCharWidth = rInf.GetTxtSize(OUString(cFill)).Width();
        OSL_ENSURE( nCharWidth, "!SwTabPortion::Paint: sophisticated tabchar" );
        
        if( nCharWidth )
        {
            
            KSHORT nChar = Width() / nCharWidth;
            if ( cFill == '_' )
                ++nChar; 
            OUStringBuffer aBuf;
            comphelper::string::padToLength(aBuf, nChar, cFill);
            rInf.DrawText(aBuf.makeStringAndClear(), *this, 0, nChar, true);
        }
    }
}

/*************************************************************************
 *                virtual SwAutoTabDecimalPortion::Paint()
 *************************************************************************/

void SwAutoTabDecimalPortion::Paint( const SwTxtPaintInfo & ) const
{
}

/*************************************************************************
 *              virtual SwTabPortion::HandlePortion()
 *************************************************************************/

void SwTabPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor(), Height(), Width() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
