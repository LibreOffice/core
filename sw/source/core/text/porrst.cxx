/*************************************************************************
 *
 *  $RCSfile: porrst.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:26 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_TEXTCONV_HXX //autogen
#include <svx/textconv.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TEXTCONV_HXX //autogen
#include <svx/textconv.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>    // SwTxtNode
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>  // SwPageFrm
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>   // ASSERT
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>
#endif
#ifndef _TXTPAINT_HXX
#include <txtpaint.hxx> // ClipVout
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>   // SwTxtFrm
#endif
#ifndef _SWFNTCCH_HXX
#include <swfntcch.hxx> // SwFontAccess
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _REDLNITR_HXX
#include <redlnitr.hxx> // SwRedlineItr
#endif
#ifndef _PORFLY_HXX
#include <porfly.hxx>   // SwFlyPortion
#endif

/*************************************************************************
 *                      class SwTmpEndPortion
 *************************************************************************/

SwTmpEndPortion::SwTmpEndPortion( const SwLinePortion &rPortion )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    SetWhichPor( POR_TMPEND );
}

/*************************************************************************
 *                 virtual SwTmpEndPortion::Paint()
 *************************************************************************/

void SwTmpEndPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsParagraph() )
    {
        SwDefFontSave aSave( rInf );
        const XubString aTmp( sal_Char(aSave.IsAlter() ? CH_PAR_ALTER : CH_PAR),
            RTL_TEXTENCODING_MS_1252 );
        rInf.DrawText( aTmp, *this );
    }
}

/*************************************************************************
 *                      class SwBreakPortion
 *************************************************************************/

SwBreakPortion::SwBreakPortion( const SwLinePortion &rPortion )
    : SwLinePortion( rPortion ), nViewWidth( 0 ), nRestWidth( 0 )
{
    nLineLength = 1;
    SetWhichPor( POR_BRK );
}

xub_StrLen SwBreakPortion::GetCrsrOfst( const KSHORT ) const
{ return 0; }

KSHORT SwBreakPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{ return 0; }

SwLinePortion *SwBreakPortion::Compress()
{ return (GetPortion() && GetPortion()->InTxtGrp() ? 0 : this); }

void SwBreakPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsLineBreak() )
    {
        ((SwBreakPortion*)this)->CalcViewWidth( rInf );
        if( nViewWidth && nViewWidth <= nRestWidth )
            rInf.DrawLineBreak( *this );
    }
}

/*************************************************************************
 *                  SwBreakPortion::CalcViewWidth()
 *************************************************************************/

void SwBreakPortion::CalcViewWidth( const SwTxtSizeInfo &rInf )
{
    ASSERT( rInf.GetOpt().IsLineBreak(), "SwBreakPortion::CalcViewWidth: zombie" );
    // Im Mormalfall folgt auf ein Break keine weitere Portion, nur wenn im Blocksatz
    // auch die letzte Zeile im Blocksatz ist, folgt eine Marginportion der Breite 0,
    // ist die Zeile zentriert, so folgt eine Marginportion mit Breite > 0.
    if( GetPortion() )
    {
        if( GetPortion()->IsFlyPortion() )
        {
            short nTmp = ((SwFlyPortion*)GetPortion())->GetPrtGlue();
            nRestWidth = nTmp > 0 ? nTmp : 0;
        }
        else
            nRestWidth = GetPortion()->Width();
    }
    if( rInf.GetWin() && nRestWidth )
    {
        if( !nViewWidth )
            nViewWidth = (KSHORT)rInf.GetOpt().GetLineBreakWidth( rInf.GetWin() );
    }
    else
        nViewWidth = 0;
}

/*************************************************************************
 *                 virtual SwBreakPortion::Format()
 *************************************************************************/

sal_Bool SwBreakPortion::Format( SwTxtFormatInfo &rInf )
{
    nRestWidth = rInf.Width() - rInf.X();
    register const SwLinePortion *pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    SetAscent( pRoot->GetAscent() );
    if ( rInf.GetIdx()+1 == rInf.GetTxt().Len() )
        rInf.SetNewLine( sal_True );
    return sal_True;
}

SwKernPortion::SwKernPortion( SwLinePortion &rPortion, short nKrn ) :
    nKern( nKrn )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    nLineLength = 0;
    SetWhichPor( POR_KERN );
    if( nKern > 0 )
        Width( nKern );
    rPortion.Insert( this );
}

void SwKernPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawBackBrush( *this );
        if( rInf.GetFont()->IsPaintBlank() )
        {
static sal_Char __READONLY_DATA sDoubleSpace[] = "  ";
            // Tabs mit Fuellung
            if( rInf.OnWin() ) // Keine Luecken am Bildschirm
                ((SwKernPortion*)this)->Width( Width() + 12 );
            XubString aTxt( sDoubleSpace, RTL_TEXTENCODING_MS_1252 );
            rInf.DrawText( aTxt, *this, 0, 2, sal_True );
            if( rInf.OnWin() )
                ((SwKernPortion*)this)->Width( Width() - 12 );
        }
    }
}

void SwKernPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    if( rInf.GetLast() == this )
        rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );
    if( nKern < 0 )
        Width( -nKern );
    else
        Width( 0 );
    rInf.GetLast()->FormatEOL( rInf );
}

SwArrowPortion::SwArrowPortion( const SwLinePortion &rPortion ) :
    bLeft( sal_True )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    nLineLength = 0;
    SetWhichPor( POR_ARROW );
}

SwArrowPortion::SwArrowPortion( const SwTxtPaintInfo &rInf )
    : bLeft( sal_False )
{
    Height( rInf.GetTxtFrm()->Prt().Height() );
    aPos.X() = rInf.GetTxtFrm()->Frm().Left() +
               rInf.GetTxtFrm()->Prt().Right();
    aPos.Y() = rInf.GetTxtFrm()->Frm().Top() +
               rInf.GetTxtFrm()->Prt().Bottom();
}

void SwArrowPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    ((SwArrowPortion*)this)->aPos = rInf.GetPos();
}

void SwArrowPortion::PaintIt( OutputDevice *pOut ) const
{
    Size aSize( 6, 12 );
    aSize = pOut->PixelToLogic( aSize );
    SwRect aRect( GetPos(), aSize );
    if( bLeft )
    {
        aRect.Pos().Y() += 20 - GetAscent();
        aRect.Pos().X() += 20;
        if( aSize.Height() > Height() )
            aRect.Height( Height() );
    }
    else
    {
        if( aSize.Height() > Height() )
            aRect.Height( Height() );
        aRect.Pos().Y() -= aRect.Height() + 20;
        aRect.Pos().X() -= aRect.Width() + 20;
    }

    Color aCol( COL_LIGHTRED );
    SvxFont::DrawArrow( *pOut, aRect.SVRect(), aSize, aCol, bLeft );
}

SwLinePortion *SwArrowPortion::Compress() { return this; }


SwTwips SwTxtFrm::EmptyHeight() const
{
    SwFont *pFnt;
    const SwTxtNode& rTxtNode = *GetTxtNode();
    ViewShell *pSh = GetShell();
    if ( rTxtNode.HasSwAttrSet() )
    {
        const SwAttrSet *pAttrSet = &( rTxtNode.GetSwAttrSet() );
        pFnt = new SwFont( pAttrSet );
    }
    else
    {
        SwFontAccess aFontAccess( &rTxtNode.GetAnyFmtColl(), pSh);
        pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
        pFnt->ChkMagic( pSh, pFnt->GetActual() );
    }
    OutputDevice *pOut = pSh ? pSh->GetOut() : 0;
    if ( !pOut || !rTxtNode.GetDoc()->IsBrowseMode() ||
         ( pSh->GetViewOptions()->IsPrtFormat() ) )
    {
        Printer *pPrt = rTxtNode.GetDoc()->GetPrt();
        if( !pOut || ( pPrt && pPrt->IsValid() ) )
            pOut = pPrt;
    }

    const SwDoc* pDoc = rTxtNode.GetDoc();
    if( ::IsShowChanges( pDoc->GetRedlineMode() ) )
    {
        MSHORT nRedlPos = pDoc->GetRedlinePos( rTxtNode );
        if( MSHRT_MAX != nRedlPos )
            SwRedlineItr aRedln( rTxtNode, *pFnt, nRedlPos, sal_True );
    }

    SwTwips nRet;
    if( !pOut )
        nRet = Prt().SSize().Height() + 1;
    else
    {
        pFnt->SetFntChg( sal_True );
        pFnt->ChgPhysFnt( pSh, pOut );
        nRet = pFnt->GetHeight( pSh, pOut );
    }
    delete pFnt;
    return nRet;
}

/*************************************************************************
 *                      SwTxtFrm::FormatEmpty()
 *************************************************************************/

sal_Bool SwTxtFrm::FormatEmpty()
{
    if ( HasFollow() || GetTxtNode()->GetpSwpHints() ||
        0 != GetTxtNode()->GetNumRule() ||
        0 != GetTxtNode()->GetOutlineNum() ||
         IsInFtn() || ( HasPara() && GetPara()->IsPrepMustFit() ) )
        return sal_False;
    const SwAttrSet& aSet = GetTxtNode()->GetSwAttrSet();
    if( SVX_ADJUST_LEFT != aSet.GetAdjust().GetAdjust()
        || aSet.GetRegister().GetValue() )
        return sal_False;
    const SvxLineSpacingItem &rSpacing = aSet.GetLineSpacing();
    if( SVX_LINE_SPACE_MIN == rSpacing.GetLineSpaceRule() ||
        SVX_LINE_SPACE_FIX == rSpacing.GetLineSpaceRule() ||
        aSet.GetLRSpace().IsAutoFirst() )
        return sal_False;
    else
    {
        SwTxtFly aTxtFly( this );
        SwRect aRect;
        sal_Bool bFirstFlyCheck = 0 != Prt().Height();
        if ( bFirstFlyCheck &&
             aTxtFly.IsOn() && aTxtFly.IsAnyObj( aRect ) )
            return sal_False;
        else
        {
            SwTwips nHeight = EmptyHeight();

            const SwTwips nChg = nHeight - Prt().SSize().Height();
            if( !nChg )
                SetUndersized( sal_False );
            AdjustFrm( nChg );

            if( HasBlinkPor() )
            {
                ClearPara();
                ResetBlinkPor();
            }
            SetCacheIdx( MSHRT_MAX );
            if( !IsEmpty() )
            {
                SetEmpty( sal_True );
                SetCompletePaint();
            }
            if( !bFirstFlyCheck &&
                 aTxtFly.IsOn() && aTxtFly.IsAnyObj( aRect ) )
                 return sal_False;
            return sal_True;
        }
    }
}

sal_Bool SwTxtFrm::FillRegister( SwTwips& rRegStart, KSHORT& rRegDiff )
{
    const SwFrm *pFrm = this;
    rRegDiff = 0;
    while( !( ( FRM_BODY | FRM_FLY )
           & pFrm->GetType() ) && pFrm->GetUpper() )
        pFrm = pFrm->GetUpper();
    if( ( FRM_BODY| FRM_FLY ) & pFrm->GetType() )
    {
        rRegStart = pFrm->Frm().Top() + pFrm->Prt().Top();
        pFrm = pFrm->FindPageFrm();
        if( pFrm->IsPageFrm() )
        {
            SwPageDesc* pDesc = ((SwPageFrm*)pFrm)->FindPageDesc();
            if( pDesc )
            {
                rRegDiff = pDesc->GetRegHeight();
                if( !rRegDiff )
                {
                    const SwTxtFmtColl *pFmt = pDesc->GetRegisterFmtColl();
                    if( pFmt )
                    {
                        const SvxLineSpacingItem &rSpace = pFmt->GetLineSpacing();
                        if( SVX_LINE_SPACE_FIX == rSpace.GetLineSpaceRule() )
                        {
                            rRegDiff = rSpace.GetLineHeight();
                            pDesc->SetRegHeight( rRegDiff );
                            pDesc->SetRegAscent( ( 4 * rRegDiff ) / 5 );
                        }
                        else
                        {
                            ViewShell *pSh = GetShell();
                            SwFontAccess aFontAccess( pFmt, pSh );
                            SwFont aFnt( *aFontAccess.Get()->GetFont() );
                            OutputDevice *pOut = 0;
                            if( !GetTxtNode()->GetDoc()->IsBrowseMode() ||
                                (pSh && pSh->GetViewOptions()->IsPrtFormat()) )
                                pOut = GetTxtNode()->GetDoc()->GetPrt();
                            if( (!pOut || !((Printer*)pOut)->IsValid()) && pSh )
                                pOut = pSh->GetWin();
                            if( !pOut )
                                pOut = GetpApp()->GetDefaultDevice();
                            MapMode aOldMap( pOut->GetMapMode() );
                            pOut->SetMapMode( MapMode( MAP_TWIP ) );
                            aFnt.ChgFnt( pSh, pOut );
                            rRegDiff = aFnt.GetHeight( pSh, pOut );
                            KSHORT nNettoHeight = rRegDiff;
                            switch( rSpace.GetLineSpaceRule() )
                            {
                                case SVX_LINE_SPACE_AUTO:
                                break;
                                case SVX_LINE_SPACE_MIN:
                                {
                                    if( rRegDiff < KSHORT( rSpace.GetLineHeight() ) )
                                        rRegDiff = rSpace.GetLineHeight();
                                    break;
                                }
                                default: ASSERT(
                                sal_False, ": unknown LineSpaceRule" );
                            }
                            switch( rSpace.GetInterLineSpaceRule() )
                            {
                                case SVX_INTER_LINE_SPACE_OFF:
                                break;
                                case SVX_INTER_LINE_SPACE_PROP:
                                {
                                    long nTmp = rSpace.GetPropLineSpace();
                                    if( nTmp < 50 )
                                        nTmp = nTmp ? 50 : 100;
                                    nTmp *= rRegDiff;
                                    nTmp /= 100;
                                    if( !nTmp )
                                        ++nTmp;
                                    rRegDiff = (KSHORT)nTmp;
                                    nNettoHeight = rRegDiff;
                                    break;
                                }
                                case SVX_INTER_LINE_SPACE_FIX:
                                {
                                    rRegDiff += rSpace.GetInterLineSpace();
                                    nNettoHeight = rRegDiff;
                                    break;
                                }
                                default: ASSERT( sal_False, ": unknown InterLineSpaceRule" );
                            }
                            pDesc->SetRegHeight( rRegDiff );
                            pDesc->SetRegAscent( rRegDiff - nNettoHeight +
                                                 aFnt.GetAscent( pSh, pOut ) );
                            pOut->SetMapMode( aOldMap );
                        }
                    }
                }
                rRegStart += pDesc->GetRegAscent() - rRegDiff;
            }
        }
    }
    return ( 0 != rRegDiff );
}


