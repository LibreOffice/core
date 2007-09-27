/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: porrst.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:18:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_PGRDITEM_HXX
#include <svx/pgrditem.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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
#ifndef _SW_PORTIONHANDLER_HXX
#include <SwPortionHandler.hxx>
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
#ifndef _SWFNTCCH_HXX
#include <swfntcch.hxx> // SwFontAccess
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
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
#ifndef _ATRHNDL_HXX
#include <atrhndl.hxx>
#endif

#include <IDocumentRedlineAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>

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
        const XubString aTmp( CH_PAR );
        rInf.DrawText( aTmp, *this );
    }
}

/*************************************************************************
 *                      class SwBreakPortion
 *************************************************************************/
SwBreakPortion::SwBreakPortion( const SwLinePortion &rPortion )
    : SwLinePortion( rPortion )
{
    nLineLength = 1;
    SetWhichPor( POR_BRK );
}

xub_StrLen SwBreakPortion::GetCrsrOfst( const KSHORT ) const
{ return 0; }

KSHORT SwBreakPortion::GetViewWidth( const SwTxtSizeInfo & ) const
{ return 0; }

SwLinePortion *SwBreakPortion::Compress()
{ return (GetPortion() && GetPortion()->InTxtGrp() ? 0 : this); }

void SwBreakPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsLineBreak() )
        rInf.DrawLineBreak( *this );
}

/*************************************************************************
 *                 virtual SwBreakPortion::Format()
 *************************************************************************/

sal_Bool SwBreakPortion::Format( SwTxtFormatInfo &rInf )
{
    const SwLinePortion *pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    SetAscent( pRoot->GetAscent() );
    if ( rInf.GetIdx()+1 == rInf.GetTxt().Len() )
        rInf.SetNewLine( sal_True );
    return sal_True;
}

/*************************************************************************
 *              virtual SwBreakPortion::HandlePortion()
 *************************************************************************/

void SwBreakPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}


SwKernPortion::SwKernPortion( SwLinePortion &rPortion, short nKrn,
                              sal_Bool bBG, sal_Bool bGK ) :
    nKern( nKrn ), bBackground( bBG ), bGridKern( bGK )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );
    nLineLength = 0;
    SetWhichPor( POR_KERN );
    if( nKern > 0 )
        Width( nKern );
     rPortion.Insert( this );
}

SwKernPortion::SwKernPortion( const SwLinePortion& rPortion ) :
    nKern( 0 ), bBackground( sal_False ), bGridKern( sal_True )
{
    Height( rPortion.Height() );
    SetAscent( rPortion.GetAscent() );

    nLineLength = 0;
    SetWhichPor( POR_KERN );
}

void SwKernPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        // bBackground is set for Kerning Portions between two fields
        if ( bBackground )
            rInf.DrawViewOpt( *this, POR_FLD );

        rInf.DrawBackBrush( *this );

        // do we have to repaint a post it portion?
        if( rInf.OnWin() && pPortion && !pPortion->Width() )
            pPortion->PrePaint( rInf, this );

        if( rInf.GetFont()->IsPaintBlank() )
        {
            static sal_Char __READONLY_DATA sDoubleSpace[] = "  ";
            XubString aTxtDouble( sDoubleSpace, RTL_TEXTENCODING_MS_1252 );
            // --> FME 2006-07-12 #b6439097#
            SwRect aClipRect;
            rInf.CalcRect( *this, &aClipRect, 0 );
            SwSaveClip aClip( (OutputDevice*)rInf.GetOut() );
            aClip.ChgClip( aClipRect, 0 );
            // <--
            rInf.DrawText( aTxtDouble, *this, 0, 2, sal_True );
        }
    }
}

void SwKernPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    if ( bGridKern )
        return;

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
    Height( (USHORT)(rInf.GetTxtFrm()->Prt().Height()) );
    aPos.X() = rInf.GetTxtFrm()->Frm().Left() +
               rInf.GetTxtFrm()->Prt().Right();
    aPos.Y() = rInf.GetTxtFrm()->Frm().Top() +
               rInf.GetTxtFrm()->Prt().Bottom();
}

void SwArrowPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    ((SwArrowPortion*)this)->aPos = rInf.GetPos();
}

SwLinePortion *SwArrowPortion::Compress() { return this; }

SwTwips SwTxtFrm::EmptyHeight() const
{
    ASSERT( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::EmptyHeight with swapped frame" );

    SwFont *pFnt;
    const SwTxtNode& rTxtNode = *GetTxtNode();
    const IDocumentSettingAccess* pIDSA = rTxtNode.getIDocumentSettingAccess();
    ViewShell *pSh = GetShell();
    if ( rTxtNode.HasSwAttrSet() )
    {
        const SwAttrSet *pAttrSet = &( rTxtNode.GetSwAttrSet() );
        pFnt = new SwFont( pAttrSet, pIDSA );
    }
    else
    {
        SwFontAccess aFontAccess( &rTxtNode.GetAnyFmtColl(), pSh);
        pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
        pFnt->ChkMagic( pSh, pFnt->GetActual() );
    }

    if ( IsVertical() )
        pFnt->SetVertical( 2700 );

    OutputDevice* pOut = pSh ? pSh->GetOut() : 0;
    if ( !pOut || !pIDSA->get(IDocumentSettingAccess::BROWSE_MODE) ||
         ( pSh->GetViewOptions()->IsPrtFormat() ) )
    {
        pOut = rTxtNode.getIDocumentDeviceAccess()->getReferenceDevice(true);
    }

    const IDocumentRedlineAccess* pIDRA = rTxtNode.getIDocumentRedlineAccess();
    if( IDocumentRedlineAccess::IsShowChanges( pIDRA->GetRedlineMode() ) )
    {
        MSHORT nRedlPos = pIDRA->GetRedlinePos( rTxtNode, USHRT_MAX );
        if( MSHRT_MAX != nRedlPos )
        {
            SwAttrHandler aAttrHandler;
            aAttrHandler.Init(  GetTxtNode()->GetSwAttrSet(),
                               *GetTxtNode()->getIDocumentSettingAccess(), NULL );
            SwRedlineItr aRedln( rTxtNode, *pFnt, aAttrHandler,
                                 nRedlPos, sal_True );
        }
    }

    SwTwips nRet;
    if( !pOut )
        nRet = IsVertical() ?
               Prt().SSize().Width() + 1 :
               Prt().SSize().Height() + 1;
    else
    {
        pFnt->SetFntChg( sal_True );
        pFnt->ChgPhysFnt( pSh, *pOut );
        nRet = pFnt->GetHeight( pSh, *pOut );
    }
    delete pFnt;
    return nRet;
}

/*************************************************************************
 *                      SwTxtFrm::FormatEmpty()
 *************************************************************************/

sal_Bool SwTxtFrm::FormatEmpty()
{
    ASSERT( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::FormatEmpty with swapped frame" );

    if ( HasFollow() || GetTxtNode()->GetpSwpHints() ||
        0 != GetTxtNode()->GetNumRule() ||
        GetTxtNode()->HasHiddenCharAttribute( true ) ||
         IsInFtn() || ( HasPara() && GetPara()->IsPrepMustFit() ) )
        return sal_False;
    const SwAttrSet& aSet = GetTxtNode()->GetSwAttrSet();
    const SvxAdjust nAdjust = aSet.GetAdjust().GetAdjust();
    if( ( ( ! IsRightToLeft() && ( SVX_ADJUST_LEFT != nAdjust ) ) ||
          (   IsRightToLeft() && ( SVX_ADJUST_RIGHT != nAdjust ) ) ) ||
          aSet.GetRegister().GetValue() )
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

            if ( GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() &&
                 IsInDocBody() )
            {
                GETGRID( FindPageFrm() )
                if ( pGrid )
                    nHeight = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
            }

            SWRECTFN( this )
            const SwTwips nChg = nHeight - (Prt().*fnRect->fnGetHeight)();

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

            // --> OD 2004-11-17 #i35635# - call method <HideAndShowObjects()>
            // to assure that objects anchored at the empty paragraph are
            // correctly visible resp. invisible.
            HideAndShowObjects();
            // <--
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
        SWRECTFN( pFrm )
        rRegStart = (pFrm->*fnRect->fnGetPrtTop)();
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
                            if( !GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) ||
                                (pSh && pSh->GetViewOptions()->IsPrtFormat()) )
                                pOut = GetTxtNode()->getIDocumentDeviceAccess()->getReferenceDevice( true );

                            if( pSh && !pOut )
                                pOut = pSh->GetWin();

                            if( !pOut )
                                pOut = GetpApp()->GetDefaultDevice();

                            MapMode aOldMap( pOut->GetMapMode() );
                            pOut->SetMapMode( MapMode( MAP_TWIP ) );

                            aFnt.ChgFnt( pSh, *pOut );
                            rRegDiff = aFnt.GetHeight( pSh, *pOut );
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
                                    rRegDiff = rRegDiff + rSpace.GetInterLineSpace();
                                    nNettoHeight = rRegDiff;
                                    break;
                                }
                                default: ASSERT( sal_False, ": unknown InterLineSpaceRule" );
                            }
                            pDesc->SetRegHeight( rRegDiff );
                            pDesc->SetRegAscent( rRegDiff - nNettoHeight +
                                                 aFnt.GetAscent( pSh, *pOut ) );
                            pOut->SetMapMode( aOldMap );
                        }
                    }
                }
                const long nTmpDiff = pDesc->GetRegAscent() - rRegDiff;
                if ( bVert )
                    rRegStart -= nTmpDiff;
                else
                    rRegStart += nTmpDiff;
            }
        }
    }
    return ( 0 != rRegDiff );
}

/*************************************************************************
 *              virtual SwHiddenTextPortion::Paint()
 *************************************************************************/

void SwHiddenTextPortion::Paint( const SwTxtPaintInfo & rInf) const
{
    (void)rInf;
#if OSL_DEBUG_LEVEL > 1
    OutputDevice* pOut = (OutputDevice*)rInf.GetOut();
    Color aCol( SwViewOption::GetFieldShadingsColor() );
    Color aOldColor( pOut->GetFillColor() );
    pOut->SetFillColor( aCol );
    Point aPos( rInf.GetPos() );
    aPos.Y() -= 150;
    aPos.X() -= 25;
    SwRect aRect( aPos, Size( 100, 200 ) );
    ((OutputDevice*)pOut)->DrawRect( aRect.SVRect() );
    pOut->SetFillColor( aOldColor );
#endif
}

/*************************************************************************
 *              virtual SwHiddenTextPortion::Format()
 *************************************************************************/

sal_Bool SwHiddenTextPortion::Format( SwTxtFormatInfo &rInf )
{
    Width( 0 );
    rInf.GetTxtFrm()->HideFootnotes( rInf.GetIdx(), rInf.GetIdx() + GetLen() );

    return sal_False;
};

/*************************************************************************
 *              virtual SwControlCharPortion::Paint()
 *************************************************************************/

void SwControlCharPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if ( Width() )  // is only set during prepaint mode
    {
        rInf.DrawViewOpt( *this, POR_CONTROLCHAR );

        if ( !rInf.GetOpt().IsPagePreview() &&
             !rInf.GetOpt().IsReadonly() &&
              SwViewOption::IsFieldShadings() &&
              CHAR_ZWNBSP != mcChar )
        {
            SwFont aTmpFont( *rInf.GetFont() );
            aTmpFont.SetEscapement( CHAR_ZWSP == mcChar ? DFLT_ESC_AUTO_SUB : -25 );
            const USHORT nProp = 40;
            aTmpFont.SetProportion( nProp );  // a smaller font
            SwFontSave aFontSave( rInf, &aTmpFont );

            String aOutString;

            switch ( mcChar )
            {
                case CHAR_ZWSP :
                    aOutString = '/'; break;
//                case CHAR_LRM :
//                    rTxt = sal_Unicode(0x2514); break;
//                case CHAR_RLM :
//                    rTxt = sal_Unicode(0x2518); break;
            }

            if ( !mnHalfCharWidth )
                mnHalfCharWidth = rInf.GetTxtSize( aOutString ).Width() / 2;

            Point aOldPos = rInf.GetPos();
            Point aNewPos( aOldPos );
            aNewPos.X() = aNewPos.X() + ( Width() / 2 ) - mnHalfCharWidth;
            const_cast< SwTxtPaintInfo& >( rInf ).SetPos( aNewPos );

            rInf.DrawText( aOutString, *this );

            const_cast< SwTxtPaintInfo& >( rInf ).SetPos( aOldPos );
        }
    }
}

/*************************************************************************
 *              virtual SwControlCharPortion::Format()
 *************************************************************************/

sal_Bool SwControlCharPortion::Format( SwTxtFormatInfo &rInf )
{
    const SwLinePortion* pRoot = rInf.GetRoot();
    Width( 0 );
    Height( pRoot->Height() );
    SetAscent( pRoot->GetAscent() );

    return sal_False;
}

/*************************************************************************
 *              virtual SwControlCharPortion::GetViewWidth()
 *************************************************************************/

KSHORT SwControlCharPortion::GetViewWidth( const SwTxtSizeInfo& rInf ) const
{
    if( !mnViewWidth )
        mnViewWidth = rInf.GetTxtSize( ' ' ).Width();

    return mnViewWidth;
}
