/*************************************************************************
 *
 *  $RCSfile: inftxt.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-06 14:15:15 $
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
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_HNGPNCTITEM_HXX
#include <svx/hngpnctitem.hxx>
#endif
#ifndef _SVX_SRIPTSPACEITEM_HXX
#include <svx/scriptspaceitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_SPLWRAP_HXX
#include <svx/splwrap.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _SVX_FORBIDDENRULEITEM_HXX
#include <svx/forbiddenruleitem.hxx>
#endif

#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>  // SwViewOptions
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>  // DrawGraphic
#endif
#ifndef _DOC_HXX
#include <doc.hxx>      // SwDoc
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>   // SwFmtDrop
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>  // SwRootFrm
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>   // SwTxtInfo
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>   // SwFont
#endif
#ifndef _TXTFLY_HXX
#include <txtfly.hxx>   // SwTxtPaintInfo
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>    // SwBlink
#endif
#ifndef _NOTEURL_HXX
#include <noteurl.hxx>  // SwNoteURL
#endif
#ifndef _DRAWFONT_HXX
#include <drawfont.hxx> // SwDrawTextInfo
#endif
#ifndef _PORFTN_HXX
#include <porftn.hxx>   // SwFtnPortion
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>       // SwHangingPortion
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _ITRATR_HXX
#include <itratr.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
#define DARK_COLOR 153

// steht im number.cxx
extern const sal_Char __FAR_DATA sBulletFntName[];

extern void MA_FASTCALL SwAlignRect( SwRect &rRect, ViewShell *pSh );

#ifndef PRODUCT
// Test2: WYSIWYG++
// Test4: WYSIWYG debug
static sal_Bool bDbgLow = sal_False;
#endif

#ifndef PRODUCT

sal_Bool SwTxtSizeInfo::IsOptCalm() const { return !GetOpt().IsTest3(); }

sal_Bool SwTxtSizeInfo::IsOptLow() const { return bDbgLow; }

sal_Bool SwTxtSizeInfo::IsOptDbg() const { return GetOpt().IsTest4(); }

sal_Bool SwTxtSizeInfo::IsOptTest1() const { return GetOpt().IsTest1(); }

sal_Bool SwTxtSizeInfo::IsOptTest2() const { return GetOpt().IsTest2(); }

sal_Bool SwTxtSizeInfo::IsOptTest3() const { return GetOpt().IsTest3(); }

sal_Bool SwTxtSizeInfo::IsOptTest4() const { return GetOpt().IsTest4(); }

sal_Bool SwTxtSizeInfo::IsOptTest5() const { return GetOpt().IsTest5(); }

sal_Bool SwTxtSizeInfo::IsOptTest6() const { return GetOpt().IsTest6(); }

sal_Bool SwTxtSizeInfo::IsOptTest7() const { return GetOpt().IsTest7(); }

sal_Bool SwTxtSizeInfo::IsOptTest8() const { return GetOpt().IsTest8(); }

#endif

/*************************************************************************
 *                      SwLineInfo::SwLineInfo()
 *************************************************************************/

void SwLineInfo::CtorInit( const SwAttrSet& rAttrSet )
{
    pRuler = &rAttrSet.GetTabStops();
    pSpace = &rAttrSet.GetLineSpacing();
    nDefTabStop = MSHRT_MAX;
}

/*************************************************************************
 *                      SwTxtInfo::CtorInit()
 *************************************************************************/

void SwTxtInfo::CtorInit( SwTxtFrm *pFrm )
{
    pPara = pFrm->GetPara();
    nTxtStart = pFrm->GetOfst();
    if( !pPara )
    {
        ASSERT( pPara, "+SwTxtInfo::CTOR: missing paragraph information" );
        pFrm->Format();
        pPara = pFrm->GetPara();
    }
}

SwTxtInfo::SwTxtInfo( const SwTxtInfo &rInf )
    : pPara( ((SwTxtInfo&)rInf).GetParaPortion() ),
      nTxtStart( rInf.GetTxtStart() )
{ }

#ifndef PRODUCT
/*************************************************************************
 *                      ChkOutDev()
 *************************************************************************/
// Sonderbehandlung: wenn pVsh->GetPrt() null ist, ist alles erlaubt

void ChkOutDev( const SwTxtSizeInfo &rInf )
{
    const OutputDevice *pOut = rInf.GetOut();
    const OutputDevice *pWin = rInf.GetWin();
    const OutputDevice *pPrt = rInf.GetPrt();
    ASSERT( pOut &&
            ( !(rInf.GetVsh() && rInf.GetVsh()->GetPrt()) ||
              (!pWin || OUTDEV_WINDOW  == pWin->GetOutDevType()
                     || OUTDEV_VIRDEV  == pWin->GetOutDevType() ) &&
              (!pPrt || pPrt == pWin || OUTDEV_PRINTER == pPrt->GetOutDevType())
                     || ( OUTDEV_VIRDEV == pOut->GetOutDevType() ) ),
            "ChkOutDev: invalid output device" );
}
#endif  // PRODUCT

inline xub_StrLen GetMinLen( const SwTxtSizeInfo &rInf )
{
    const xub_StrLen nInfLen = rInf.GetIdx() + rInf.GetLen();
    return Min( rInf.GetTxt().Len(), nInfLen );
}

SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew )
    : SwTxtInfo( rNew ),
      pVsh(((SwTxtSizeInfo&)rNew).GetVsh()),
      pOut(((SwTxtSizeInfo&)rNew).GetOut()),
      pWin(((SwTxtSizeInfo&)rNew).GetWin()),
      pPrt(((SwTxtSizeInfo&)rNew).GetPrt()),
      pFnt(((SwTxtSizeInfo&)rNew).GetFont()),
      pFrm(rNew.pFrm),
      pOpt(&rNew.GetOpt()),
      pTxt(&rNew.GetTxt()),
      nIdx(rNew.GetIdx()),
      nLen(rNew.GetLen()),
      bOnWin( rNew.OnWin() ),
      bNotEOL( rNew.NotEOL() ),
      bURLNotify( rNew.URLNotify() ),
      bStopUnderFlow( rNew.StopUnderFlow() ),
      bMulti( rNew.IsMulti() ),
      bFirstMulti( rNew.IsFirstMulti() ),
      bRuby( rNew.IsRuby() ),
      bHanging( rNew.IsHanging() ),
      bScriptSpace( rNew.HasScriptSpace() ),
      bForbiddenChars( rNew.HasForbiddenChars() ),
      nDirection( rNew.GetDirection() )
{
#ifndef PRODUCT
    ChkOutDev( *this );
#endif
}

void SwTxtSizeInfo::_SelectOut()
{
    ASSERT( pVsh, "Where's my ViewShell?" );
    pOut = pVsh->GetOut();
    if( bOnWin )
        pWin = pOut;
}

void SwTxtSizeInfo::CtorInit( SwTxtFrm *pFrame, SwFont *pNewFnt,
                   const xub_StrLen nNewIdx, const xub_StrLen nNewLen )
{
    pFrm = pFrame;
    SwTxtInfo::CtorInit( pFrm );
    const SwTxtNode *pNd = pFrm->GetTxtNode();
    pVsh = pFrm->GetShell();
    if ( pVsh )
        pOut = pVsh->GetOut();
    else
    {
        //Zugriff ueber StarONE, es muss keine Shell existieren oder aktiv sein.
        if ( pNd->GetDoc()->IsBrowseMode() ) //?!?!?!?
            //in Ermangelung eines Besseren kann hier ja wohl nur noch das
            //AppWin genommen werden?
            pOut = GetpApp()->GetDefaultDevice();
        else
            pOut = pNd->GetDoc()->GetPrt(); //Muss es geben (oder sal_True uebergeben?)
    }
    pOpt = pVsh ? pVsh->GetViewOptions() : SW_MOD()->GetViewOption(pNd->GetDoc()->IsHTMLMode());//Options vom Module wg. StarONE
    //Hier auf GetWin() abfragen nicht auf GetOut != PRINTER (SwFlyFrmFmt::MakeGraphic)
    bOnWin = pVsh && ( pVsh->GetWin() ||
        ( pOut && OUTDEV_PRINTER != pOut->GetOutDevType() && pOpt->IsPrtFormat() ) );
    pWin = bOnWin ? pOut : 0;

    // bURLNotify wird gesetzt, wenn MakeGraphic dies vorbereitet
    bURLNotify = pNoteURL && !bOnWin
        && (pOut && OUTDEV_PRINTER != pOut->GetOutDevType());


    pFnt = pNewFnt;

    if( 0 == ( pPrt = pVsh ? pVsh->GetReferenzDevice():0 ) )
        pPrt = pNd->GetDoc()->GetPrt();
    else
    {
        ASSERT( !bOnWin, "SwTxtSizeInfo: Funny ReferenzDevice" );
        if( ((Printer*)pPrt)->IsValid() )
            pOut = pPrt;
    }
    if ( pPrt && !((Printer*)pPrt)->IsValid() )
        pPrt = 0;
    pTxt = &pNd->GetTxt();
#ifndef PRODUCT
    ChkOutDev( *this );
#endif
    if( pVsh && pNd->GetDoc()->IsBrowseMode() &&
        !pVsh->GetViewOptions()->IsPrtFormat() )
        pPrt = pOut;
    nIdx = nNewIdx;
    nLen = nNewLen;
    bNotEOL = sal_False;
    bStopUnderFlow = sal_False;
    bSpecialUnderline = sal_False;
    bMulti = bFirstMulti = bRuby = bHanging = bScriptSpace =
        bForbiddenChars = sal_False;
    nDirection = DIR_LEFT2RIGHT;
    SetLen( GetMinLen( *this ) );
}

SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew, const XubString &rTxt,
                              const xub_StrLen nIdx, const xub_StrLen nLen )
    : SwTxtInfo( rNew ),
      pVsh(((SwTxtSizeInfo&)rNew).GetVsh()),
      pOut(((SwTxtSizeInfo&)rNew).GetOut()),
      pWin(((SwTxtSizeInfo&)rNew).GetWin()),
      pPrt(((SwTxtSizeInfo&)rNew).GetPrt()),
      pFnt(((SwTxtSizeInfo&)rNew).GetFont()),
      pFrm( rNew.pFrm ),
      pOpt(&rNew.GetOpt()),
      pTxt(&rTxt),
      nIdx(nIdx),
      nLen(nLen),
      bOnWin( rNew.OnWin() ),
      bNotEOL( rNew.NotEOL() ),
      bURLNotify( rNew.URLNotify() ),
      bStopUnderFlow( rNew.StopUnderFlow() ),
      bMulti( rNew.IsMulti() ),
      bFirstMulti( rNew.IsFirstMulti() ),
      bRuby( rNew.IsRuby() ),
      bHanging( rNew.IsHanging() ),
      bScriptSpace( rNew.HasScriptSpace() ),
      bForbiddenChars( rNew.HasForbiddenChars() ),
      nDirection( rNew.GetDirection() )
{
#ifndef PRODUCT
    ChkOutDev( *this );
#endif
    SetLen( GetMinLen( *this ) );
}

/*************************************************************************
 *                      SwTxtSizeInfo::SelectFont()
 *************************************************************************/

void SwTxtSizeInfo::SelectFont()
{
    // 8731: Der Weg muss ueber ChgPhysFnt gehen, sonst geraet
    // der FontMetricCache durcheinander. In diesem Fall steht pLastMet
    // auf dem alten Wert.
    // Falsch: GetOut()->SetFont( GetFont()->GetFnt() );
    GetFont()->Invalidate();
    GetFont()->ChgPhysFnt( pVsh, GetOut() );
}

/*************************************************************************
 *                      SwTxtSizeInfo::_NoteAnimation()
 *************************************************************************/

void SwTxtSizeInfo::_NoteAnimation()
{
    ASSERT( bOnWin, "NoteAnimation: Wrong Call" );
    if( SwRootFrm::FlushVout() )
    {
        pOut = pVsh->GetOut();
        pWin = pOut;
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::CtorInit()
 *************************************************************************/

void SwTxtPaintInfo::CtorInit( SwTxtFrm *pFrame, const SwRect &rPaint )
{
    SwTxtSizeInfo::CtorInit( pFrame );
    aTxtFly.CtorInit( pFrame ),
    aPaintRect = rPaint;
    nSpaceIdx = 0;
    pSpaceAdd = NULL;
    pWrongList = NULL;
#ifdef PRODUCT
    pBrushItem = 0;
#else
    pBrushItem = ((SvxBrushItem*)-1);
#endif
}

SwTxtPaintInfo::SwTxtPaintInfo( const SwTxtPaintInfo &rInf, const XubString &rTxt )
    : SwTxtSizeInfo( rInf, rTxt ),
      aTxtFly( *rInf.GetTxtFly() ),
      aPos( rInf.GetPos() ),
      aPaintRect( rInf.GetPaintRect() ),
      nSpaceIdx( rInf.GetSpaceIdx() ),
      pSpaceAdd( rInf.GetpSpaceAdd() ),
      pWrongList( rInf.GetpWrongList() ),
      pBrushItem( rInf.GetBrushItem() )
{ }

SwTxtPaintInfo::SwTxtPaintInfo( const SwTxtPaintInfo &rInf )
    : SwTxtSizeInfo( rInf ),
      aTxtFly( *rInf.GetTxtFly() ),
      aPos( rInf.GetPos() ),
      aPaintRect( rInf.GetPaintRect() ),
      nSpaceIdx( rInf.GetSpaceIdx() ),
      pSpaceAdd( rInf.GetpSpaceAdd() ),
      pWrongList( rInf.GetpWrongList() ),
      pBrushItem( rInf.GetBrushItem() )
{ }

/*************************************************************************
 *                     SwTxtPaintInfo::_DrawText()
 *************************************************************************/

void SwTxtPaintInfo::_DrawText( const XubString &rText, const SwLinePortion &rPor,
                            const xub_StrLen nStart, const xub_StrLen nLen,
                            const sal_Bool bKern, const sal_Bool bWrong )
{
    if( !nLen )
        return;
    if( GetFont()->IsBlink() && OnWin() && rPor.Width() )
    {
        if( !pBlink )
            pBlink = new SwBlink();
        pBlink->Insert( &rPor, aPos, GetTxtFrm() );
        if( !pBlink->IsVisible() )
            return;
    }
    Color *pOldCol;
    if( GetFont()->IsAutomaticCol() )
    {
        pOldCol = new Color( GetFont()->GetColor() );
        const Color* pCol = GetFont()->GetBackColor();
        BOOL bBlack = TRUE;
        if( !pCol )
        {
            const SvxBrushItem* pItem;
            SwRect aOrigBackRect;
            if( GetTxtFrm()->GetBackgroundBrush( pItem, pCol, aOrigBackRect,
                FALSE ) && !pItem->GetColor().GetTransparency() )
                pCol = &pItem->GetColor();
        }
        if( pCol &&
            DARK_COLOR < pCol->GetRed() + pCol->GetGreen() + pCol->GetBlue() )
            bBlack = FALSE;
        Color aCol( bBlack ? COL_BLACK : COL_WHITE );
        GetFont()->SetColor( aCol );
    }
    else
        pOldCol = NULL;

    short nSpaceAdd = ( rPor.IsBlankPortion() || rPor.IsDropPortion() ||
                        rPor.InNumberGrp() ) ? 0 : GetSpaceAdd();

    const sal_Bool bBullet = OnWin() && GetOpt().IsBlank() && IsNoSymbol();
    sal_Bool bTmpWrong = bWrong && OnWin() && GetOpt().IsOnlineSpell()
                             && !GetOpt().IsHideSpell();
    SwDrawTextInfo aDrawInf( pFrm->GetShell(), *pOut,
                             rText, nStart, nLen, rPor.Width(), bBullet );
    aDrawInf.SetLeft( GetPaintRect().Left() );
    aDrawInf.SetRight( GetPaintRect().Right() );
    aDrawInf.SetSpecialUnderline( bSpecialUnderline );
    aDrawInf.SetSpace( nSpaceAdd );

    if( GetTxtFly()->IsOn() )
    {
        // aPos muss als TopLeft vorliegen, weil die ClipRects sonst
        // nicht berechnet werden koennen.
        const Point aPoint( aPos.X(), aPos.Y() - rPor.GetAscent() );
        const Size aSize( rPor.Width(), rPor.Height() );
        aDrawInf.SetFont( pFnt );
        aDrawInf.SetPos( aPoint );
        aDrawInf.SetSize( aSize );
        aDrawInf.SetAscent( rPor.GetAscent() );
        aDrawInf.SetKern( bKern ? rPor.Width() : 0 );
        aDrawInf.SetSpace( nSpaceAdd );
        aDrawInf.SetWrong( bTmpWrong ? pWrongList : NULL );
        GetTxtFly()->DrawTextOpaque( aDrawInf );
    }
    else
    {
        aDrawInf.SetPos( aPos );
        if( bKern )
            pFnt->_DrawStretchText( aDrawInf );
        else
        {
            aDrawInf.SetWrong( bTmpWrong ? pWrongList : NULL );
            aDrawInf.SetSpace( nSpaceAdd );
            pFnt->_DrawText( aDrawInf );
        }
    }
    if( pOldCol )
    {
        GetFont()->SetColor( *pOldCol );
        delete pOldCol;
    }
}

/*************************************************************************
 *                          lcl_CalcRect()
 *************************************************************************/

SwRect lcl_CalcRect( const SwTxtPaintInfo *pInf, const SwLinePortion &rPor )
{
    Size aSize( rPor.Width(), rPor.Height() );
    if( rPor.IsHangingPortion() )
        aSize.Width() = ((SwHangingPortion&)rPor).GetInnerWidth();
    if( rPor.InSpaceGrp() && pInf->GetSpaceAdd() )
    {
        SwTwips nAdd = rPor.CalcSpacing( pInf->GetSpaceAdd(), *pInf );
        if( rPor.InFldGrp() && pInf->GetSpaceAdd() < 0 && nAdd )
            nAdd += pInf->GetSpaceAdd();
        aSize.Width() += nAdd;
    }

    Point aPoint;

    if( pInf->IsRotated() )
    {
        long nTmp = aSize.Width();
        aSize.Width() = aSize.Height();
        aSize.Height() = nTmp;
        aPoint.A() = pInf->X() - rPor.GetAscent();
        aPoint.B() = pInf->Y() - aSize.Height();
    }
    else
    {
        aPoint.A() = pInf->X();
        aPoint.B() = pInf->Y() - rPor.GetAscent();
    }

    SwRect aRect( aPoint, aSize );
    if( aRect.HasArea() )
    {
        ::SwAlignRect( aRect, (ViewShell*)pInf->GetVsh() );

        if ( pInf->GetOut()->IsClipRegion() )
        {
            SwRect aClip( pInf->GetOut()->GetClipRegion().GetBoundRect() );
            aRect.Intersection( aClip );
        }
    }
    return aRect;
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawRect()
 *************************************************************************/

void SwTxtPaintInfo::DrawRect( const SwRect &rRect, sal_Bool bNoGraphic,
                               sal_Bool bRetouche ) const
{
    if ( OnWin() || !bRetouche )
    {
        if( aTxtFly.IsOn() )
            ((SwTxtPaintInfo*)this)->GetTxtFly()->
                DrawFlyRect( pOut, rRect, *this, bNoGraphic );
        else if ( bNoGraphic )
            pOut->DrawRect( rRect.SVRect() );
        else
        {
            ASSERT( ((SvxBrushItem*)-1) != pBrushItem, "DrawRect: Uninitialized BrushItem!" );
            ::DrawGraphic( pBrushItem, pOut, aItemRect, rRect );
        }
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawTab()
 *************************************************************************/

void SwTxtPaintInfo::DrawTab( const SwLinePortion &rPor ) const
{
    if( OnWin() )
    {
        const SwRect aRect( lcl_CalcRect( this, rPor ) );
#ifndef PRODUCT
#ifdef DEBUG
        if( IsOptDbg() )
            pWin->DrawRect( aRect.SVRect() );
#endif
#endif
        pOpt->PaintTab( pWin, aRect );
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawLineBreak()
 *************************************************************************/

void SwTxtPaintInfo::DrawLineBreak( const SwLinePortion &rPor ) const
{
    if( OnWin() )
    {
        KSHORT nOldWidth = rPor.Width();
        ((SwLinePortion&)rPor).Width( pOpt->GetLineBreakWidth( pWin ) );
        const SwRect aRect( lcl_CalcRect( this, rPor ) );
        ((SwLinePortion&)rPor).Width( nOldWidth );
        if( aRect.HasArea() )
             pOpt->PaintLineBreak( pWin, aRect.SVRect() );
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawPostIts()
 *************************************************************************/

void SwTxtPaintInfo::DrawPostIts( const SwLinePortion &rPor, sal_Bool bScript ) const
{
    if( OnWin() && pOpt->IsPostIts() )
    {
        const Size aSize( pOpt->GetPostItsWidth( pWin ),
            pFnt->GetHeight( pVsh, GetOut() ) );
        const Point aTmp( aPos.X(), aPos.Y() - pFnt->GetAscent( pVsh, GetOut() ) );
        const Rectangle aRect( aTmp, aSize );
        pOpt->PaintPostIts( pWin, aRect, bScript ? COL_LIGHTGREEN : COL_YELLOW );
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawBackGround()
 *************************************************************************/

void SwTxtPaintInfo::DrawBackground( const SwLinePortion &rPor ) const
{
    ASSERT( OnWin(), "SwTxtPaintInfo::DrawBackground: printer polution ?" );

    SwRect aRect = lcl_CalcRect( this, rPor );

    if ( aRect.HasArea() )
    {
        OutputDevice *pOut = (OutputDevice*)GetOut();
        Color aCol( COL_LIGHTGRAY );
        const Color aOldColor( pOut->GetFillColor() );
        sal_Bool bChgBrsh;
        if( 0 != (bChgBrsh = aOldColor != aCol) )
            pOut->SetFillColor( aCol );

        DrawRect( aRect, sal_True );

        if ( bChgBrsh )
            pOut->SetFillColor( aOldColor );
    }
}

void SwTxtPaintInfo::_DrawBackBrush( const SwLinePortion &rPor ) const
{
    ASSERT( pFnt->GetBackColor(), "DrawBackBrush: Lost Color" );
    SwRect aRect = lcl_CalcRect( this, rPor );

    if( GetSpaceAdd() < 0 )
    {
        if( !rPor.GetPortion() || rPor.GetPortion()->InFixMargGrp() )
            aRect.Width( aRect.Width() + GetSpaceAdd() );
    }

    if ( aRect.HasArea() )
    {
        OutputDevice *pOut = (OutputDevice*)GetOut();
        const Color aOldColor( pOut->GetFillColor() );
        sal_Bool bChgColor;
        if( 0 != ( bChgColor = aOldColor != *pFnt->GetBackColor() ) )
            pOut->SetFillColor( *pFnt->GetBackColor() );
        DrawRect( aRect, sal_True, sal_False );
        if( bChgColor )
            pOut->SetFillColor( aOldColor );
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::DrawViewOpt()
 *************************************************************************/

void SwTxtPaintInfo::DrawViewOpt( const SwLinePortion &rPor,
                                  const MSHORT nWhich ) const
{
    if( OnWin() && !IsMulti() )
    {
        sal_Bool bDraw = sal_False;
        switch( nWhich )
        {
            case POR_FTN:       if ( GetOpt().IsFootNote() )bDraw = sal_True; break;
            case POR_TOX:       if ( GetOpt().IsTox() )     bDraw = sal_True; break;
            case POR_REF:       if ( GetOpt().IsRef() )     bDraw = sal_True; break;
            case POR_QUOVADIS:
            case POR_NUMBER:
            case POR_FLD:
            case POR_URL:
            case POR_HIDDEN:    if ( GetOpt().IsField() )   bDraw = sal_True; break;
            case POR_TAB:       if ( GetOpt().IsTab() )     bDraw = sal_True; break;
            case POR_SOFTHYPH:  if ( GetOpt().IsSoftHyph() )bDraw = sal_True; break;
            case POR_BLANK:     if ( GetOpt().IsHardBlank())bDraw = sal_True; break;
            default:
            {
                ASSERT( !this, "SwTxtPaintInfo::DrawViewOpt: don't know how to draw this" );
                break;
            }
        }
        if ( bDraw )
            DrawBackground( rPor );
    }
}

/*************************************************************************
 *                     SwTxtPaintInfo::_NotifyURL()
 *************************************************************************/

void SwTxtPaintInfo::_NotifyURL( const SwLinePortion &rPor ) const
{
    ASSERT( pNoteURL, "NotifyURL: pNoteURL gone with the wind!" );
    SwRect aRect = lcl_CalcRect( this, rPor );
    if( aRect.HasArea() )
    {
        SwTxtNode *pNd = (SwTxtNode*)GetTxtFrm()->GetTxtNode();
        SwIndex aIndex( pNd, GetIdx() );
        SwTxtAttr *pAttr = pNd->GetTxtAttr( aIndex, RES_TXTATR_INETFMT );
        if( pAttr )
        {
            const SwFmtINetFmt& rFmt = pAttr->GetINetFmt();
            pNoteURL->InsertURLNote( rFmt.GetValue(), rFmt.GetTargetFrame(),
                aRect );
        }
    }
}

/*************************************************************************
 *                  SwTxtFormatInfo::InitHyph()
 *************************************************************************/

sal_Bool SwTxtFormatInfo::InitHyph( const sal_Bool bAutoHyph )
{
    const SwAttrSet& rAttrSet = GetTxtFrm()->GetTxtNode()->GetSwAttrSet();
    SetHanging( rAttrSet.GetHangingPunctuation().GetValue() );
    SetScriptSpace( rAttrSet.GetScriptSpace().GetValue() );
    SetForbiddenChars( rAttrSet.GetForbiddenRule().GetValue() );
    const SvxHyphenZoneItem &rAttr = rAttrSet.GetHyphenZone();
    MaxHyph() = rAttr.GetMaxHyphens();
    sal_Bool bAuto = bAutoHyph || rAttr.IsHyphen();
    if( bAuto || bInterHyph )
    {
        uno::Reference< beans::XPropertySet > xProp( ::GetLinguPropertySet() );
        nHyphStart = nHyphWrdStart = STRING_LEN;
        nHyphWrdLen = 0;

        // save hyphenation relevant Lingu properties for later restoration
        // before changing them
        if( xProp.is() && !bRestoreHyphOptions )
        {
            // paraex.hxx zeigt, dass im Attribut nMinLead = nMinTrail = 0
            // initialisiert wird.

            sal_Int16 nTemp;
            xProp->getPropertyValue( C2U(UPN_HYPH_MIN_LEADING) ) >>= nTemp;
            nMinLeading = nTemp;
            xProp->getPropertyValue( C2U(UPN_HYPH_MIN_LEADING) ) >>= nTemp;
            nMinTrailing = nTemp;
            // nMinWordLength = ; noch nicht am Absatz verwendet
            INT16 nNewMinLeading  = Max(rAttr.GetMinLead(), sal_uInt8(2));
            INT16 nNewMinTrailing = rAttr.GetMinTrail();
            bRestoreHyphOptions = (nMinLeading  != nNewMinLeading) ||
                                  (nMinTrailing != nNewMinTrailing);

            if (bRestoreHyphOptions)
            {
                uno::Any aTemp;
                aTemp <<= nNewMinLeading;
                xProp->setPropertyValue( C2U(UPN_HYPH_MIN_LEADING), aTemp );
                aTemp <<= nNewMinTrailing;
                xProp->setPropertyValue( C2U(UPN_HYPH_MIN_TRAILING), aTemp );
            }
        }
    }
    return bAuto;
}
/*************************************************************************
 *                  SwTxtFormatInfo::RestoreHyphOptions()
 *************************************************************************/

void SwTxtFormatInfo::RestoreHyphOptions()
{
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    if (xProp.is())
    {
        uno::Any aVal;
        aVal <<= (sal_Int16)nMinLeading;
        xProp->setPropertyValue( C2U(UPN_HYPH_MIN_LEADING), aVal );
        aVal <<= (sal_Int16)nMinTrailing;
        xProp->setPropertyValue( C2U(UPN_HYPH_MIN_TRAILING), aVal );
        //xProp->setPropertyValue( C2U(UPN_HYPH_MIN_WORDLENGTH),  );
    }
}

/*************************************************************************
 *                  SwTxtFormatInfo::CtorInit()
 *************************************************************************/

void SwTxtFormatInfo::CtorInit( SwTxtFrm *pNewFrm, const sal_Bool bNewInterHyph,
                                const sal_Bool bNewQuick, const sal_Bool bTst )
{
    SwTxtPaintInfo::CtorInit( pNewFrm, SwRect() );
    bQuick = bNewQuick;
    bInterHyph = bNewInterHyph;

    //! needs to be done in this order
    nMinLeading     = 2;
    nMinTrailing    = 2;
    nMinWordLength  = 0;
    bRestoreHyphOptions = sal_False;
    bAutoHyph = InitHyph();

    bIgnoreFly = sal_False;
    bShift = sal_False;
    bDropInit = sal_False;
    bTestFormat = bTst;
    nLeft = 0;
    nRight = 0;
    nFirst = 0;
    nRealWidth = 0;
    nForcedLeftMargin = 0;
    pRest = 0;
    nLineHeight = 0;
    SetLineStart(0);
    Init();
}

/*************************************************************************
 *                  SwTxtFormatInfo::IsHyphenate()
 *************************************************************************/
// Trennen oder nicht trennen, das ist hier die Frage:
// - in keinem Fall trennen, wenn der Hyphenator ERROR zurueckliefert,
//   oder wenn als Sprache NOLANGUAGE eingestellt ist.
// - ansonsten immer trennen, wenn interaktive Trennung vorliegt
// - wenn keine interakt. Trennung, dann nur trennen, wenn im ParaFmt
//   automatische Trennung eingestellt ist.

sal_Bool SwTxtFormatInfo::IsHyphenate() const
{
    if( !bInterHyph && !bAutoHyph )
        return sal_False;

    LanguageType eTmp = GetFont()->GetLanguage();
    if( LANGUAGE_DONTKNOW == eTmp || LANGUAGE_NONE == eTmp )
        return sal_False;

    uno::Reference< XHyphenator > xHyph = ::GetHyphenator();
    if (bInterHyph && xHyph.is())
        SvxSpellWrapper::CheckHyphLang( xHyph, eTmp );

    if( !xHyph.is() || !xHyph->hasLocale( pBreakIt->GetLocale(eTmp) ) )
        return sal_False;
    return sal_True;
}

/*************************************************************************
 *                  SwTxtFormatInfo::GetDropFmt()
 *************************************************************************/

// Dropcaps vom SwTxtFormatter::CTOR gerufen.
const SwFmtDrop *SwTxtFormatInfo::GetDropFmt() const
{
    const SwFmtDrop *pDrop = &GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetDrop();
    if( 1 >= pDrop->GetLines() ||
        ( !pDrop->GetChars() && !pDrop->GetWholeWord() ) )
        pDrop = 0;
    return pDrop;
}

/*************************************************************************
 *                      SwTxtFormatInfo::Init()
 *************************************************************************/

void SwTxtFormatInfo::Init()
{
    // Nicht initialisieren: pRest, nLeft, nRight, nFirst, nRealWidth
    X(0);
    bArrowDone = bFull = bFtnDone = bErgoDone = bNumDone = bNoEndHyph =
        bNoMidHyph = bStop = bNewLine = bUnderFlow = sal_False;
    pRoot = 0;
    pLast = 0;
    pFly = 0;
    pLastFld = 0;
    pLastTab = 0;
    pUnderFlow = 0;
    cTabDecimal = 0;
    nWidth = nRealWidth;
    nForcedLeftMargin = 0;
    nSoftHyphPos = 0;
    cHookChar = 0;
    SetIdx(0);
    SetLen( GetTxt().Len() );
    SetPaintOfst(0);
}

/*-----------------16.10.00 11:39-------------------
 * There are a few differences between a copy constructor
 * and the following constructor for multi-line formatting.
 * The root is the first line inside the multi-portion,
 * the line start is the actual position in the text,
 * the line width is the rest width from the surrounding line
 * and the bMulti and bFirstMulti-flag has to be set correctly.
 * --------------------------------------------------*/

SwTxtFormatInfo::SwTxtFormatInfo( const SwTxtFormatInfo& rInf,
    SwLineLayout& rLay, SwTwips nActWidth ) : SwTxtPaintInfo( rInf )
{
    pRoot = &rLay;
    pLast = &rLay;
    pFly = NULL;
    pLastFld = NULL;
    pUnderFlow = NULL;
    pRest = NULL;
    pLastTab = NULL;

    nSoftHyphPos = 0;
    nHyphStart = 0;
    nHyphWrdStart = 0;
    nHyphWrdLen = 0;
    nLineStart = rInf.GetIdx();
    nLeft = rInf.nLeft;
    nRight = rInf.nRight;
    nFirst = rInf.nLeft;
    nRealWidth = KSHORT(nActWidth);
    nWidth = nRealWidth;
    nLineHeight = 0;
    nForcedLeftMargin = 0;

    nMinLeading = 0;
    nMinTrailing = 0;
    nMinWordLength = 0;
    bRestoreHyphOptions = FALSE;
    bFull = FALSE;
    bFtnDone = TRUE;
    bErgoDone = TRUE;
    bNumDone = TRUE;
    bStop = FALSE;
    bNewLine = TRUE;
    bShift  = FALSE;
    bUnderFlow = FALSE;
    bInterHyph = FALSE;
    bAutoHyph = FALSE;
    bDropInit = FALSE;
    bQuick  = rInf.bQuick;
    bNoEndHyph  = FALSE;
    bNoMidHyph  = FALSE;
    bIgnoreFly = FALSE;

    cTabDecimal = 0;
    cHookChar = 0;
    nMaxHyph = 0;
    bTestFormat = rInf.bTestFormat;
    SetMulti( sal_True );
    SetFirstMulti( rInf.IsMulti() ? rInf.IsFirstMulti() :
                   nLineStart == rInf.GetLineStart() );
}

/*************************************************************************
 *                 SwTxtFormatInfo::_CheckFtnPortion()
 *************************************************************************/

sal_Bool SwTxtFormatInfo::_CheckFtnPortion( SwLineLayout* pCurr )
{
    KSHORT nHeight = pCurr->GetRealHeight();
    SwLinePortion *pPor = pCurr->GetPortion();
    sal_Bool bRet = sal_False;
    while( pPor )
    {
        if( pPor->IsFtnPortion() && nHeight > ((SwFtnPortion*)pPor)->Orig() )
        {
            bRet = sal_True;
            SetLineHeight( nHeight );
            break;
        }
        pPor = pPor->GetPortion();
    }
    return bRet;
}




/*************************************************************************
 *                 SwTxtFormatInfo::ScanPortionEnd()
 *************************************************************************/
xub_StrLen SwTxtFormatInfo::ScanPortionEnd( const xub_StrLen nStart,
                                            const xub_StrLen nEnd,
                                            sal_Bool bSkip )
{
    cHookChar = 0;
    const xub_Unicode cTabDec = GetLastTab() ? (sal_Unicode)GetTabDecimal() : 0;
    xub_StrLen i = nStart;
    if ( bSkip && i < nEnd )
        ++i;

    for( ; i < nEnd; ++i )
    {
        const xub_Unicode cPos = GetChar( i );
        switch( cPos )
        {
        case CH_TXTATR_BREAKWORD:
        case CH_TXTATR_INWORD:
            if( !HasHint( i ))
                break;
            // no break;

        case CHAR_SOFTHYPHEN:
        case CHAR_HARDHYPHEN:
        case CHAR_HARDBLANK:
        case CH_TAB:
        case CH_BREAK:
            cHookChar = cPos;
            return i;

        default:
            if( cTabDec == cPos )
            {
                ASSERT( cPos, "Unexspected end of string" );
                if( cPos ) // robust
                {
                    cHookChar = cPos;
                    return i;
                }
            }
        }
    }
/*
JP 07.08.00: old
    if ( cTabDec )
        for( ; i < nEnd; ++i )
        {
            const xub_Unicode cPos = GetChar( i );
            if( cTabDec == cPos || CH_TAB == cPos || CH_BREAK == cPos ||
                (( CH_TXTATR_BREAKWORD == cPos || CH_TXTATR_INWORD == cPos )
                    && HasHint( i ) ) )
            {
                cHookChar = cPos;
                return i;
            }
        }
    else
        for( ; i < nEnd; ++i )
        {
            const xub_Unicode cPos = GetChar( i );
            if( CH_TAB == cPos || CH_BREAK == cPos ||
                ( ( CH_TXTATR_BREAKWORD == cPos || CH_TXTATR_INWORD == cPos )
                    && HasHint( i ) ) )
            {
                cHookChar = cPos;
                return i;
            }
        }
*/
    return i;
}

/*************************************************************************
 *                      class SwTxtSlot
 *************************************************************************/

SwTxtSlot::SwTxtSlot( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor )
{
    bOn = pPor->GetExpTxt( *pNew, aTxt );

    // Der Text wird ausgetauscht...
    if( bOn )
    {
        pInf = (SwTxtSizeInfo*)pNew;
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pInf->SetLen( pPor->GetLen() );
        pOldTxt = &(pInf->GetTxt());
        pInf->SetTxt( aTxt );
        pInf->SetIdx( 0 );
    }
}

/*************************************************************************
 *                       SwTxtSlot::~SwTxtSlot()
 *************************************************************************/

SwTxtSlot::~SwTxtSlot()
{
    if( bOn )
    {
        pInf->SetTxt( *pOldTxt );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );
    }
}

/*************************************************************************
 *                      class SwTxtSlotLen
 *************************************************************************/

SwTxtSlotLen::SwTxtSlotLen( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor,
    const sal_Char *pCh )
{
    if( pCh )
    {
        aTxt = XubString( pCh, RTL_TEXTENCODING_MS_1252 );
        bOn = sal_True;
    }
    else
        bOn = pPor->GetExpTxt( *pNew, aTxt );

    // Der Text wird ausgetauscht...
    if( bOn )
    {
        pInf = (SwTxtSizeInfo*)pNew;
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pOldTxt = &(pInf->GetTxt());
        pInf->SetTxt( aTxt );
        pInf->SetIdx( 0 );
        pInf->SetLen( pInf->GetTxt().Len() );
    }
}

/*************************************************************************
 *                       SwTxtSlotLen::~SwTxtSlotLen()
 *************************************************************************/

SwTxtSlotLen::~SwTxtSlotLen()
{
    if( bOn )
    {
        pInf->SetTxt( *pOldTxt );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );
    }
}

/*************************************************************************
 *                     SwFontSave::SwFontSave()
 *************************************************************************/

SwFontSave::SwFontSave( const SwTxtSizeInfo &rInf, SwFont *pNew,
        SwAttrIter* pItr )
        : pFnt( pNew ? ((SwTxtSizeInfo&)rInf).GetFont() : 0 )
{
    if( pFnt )
    {
        pInf = &((SwTxtSizeInfo&)rInf);
        if( pFnt->DifferentMagic( pNew, pFnt->GetActual() ) ||
            pNew->GetActual() != pFnt->GetActual() )
        {
            pNew->SetTransparent( sal_True );
            pNew->SetAlign( ALIGN_BASELINE );
            pInf->SetFont( pNew );
        }
        else
            pFnt = 0;
        pNew->Invalidate();
        pNew->ChgPhysFnt( pInf->GetVsh(), pInf->GetOut() );
        if( pItr && pItr->GetFnt() == pFnt )
        {
            pIter = pItr;
            pIter->SetFnt( pNew );
        }
        else
            pIter = NULL;
    }
}

/*************************************************************************
 *                     SwFontSave::~SwFontSave()
 *************************************************************************/

SwFontSave::~SwFontSave()
{
    if( pFnt )
    {
        // SwFont zurueckstellen
        pFnt->Invalidate();
        pInf->SetFont( pFnt );
        if( pIter )
        {
            pIter->SetFnt( pFnt );
            pIter->nPos = STRING_LEN;
        }
    }
}

/*************************************************************************
 *                     SwDefFontSave::SwDefFontSave()
 *************************************************************************/

SwDefFontSave::SwDefFontSave( const SwTxtSizeInfo &rInf )
        : pFnt( ((SwTxtSizeInfo&)rInf).GetFont()  )
{
    bAlter = pFnt->GetFixKerning() ||
             ( RTL_TEXTENCODING_SYMBOL == pFnt->GetCharSet(pFnt->GetActual()) )
#if defined( PM2 )
             || ( System::GetCharSet() != CHARSET_IBMPC_850 )
#endif
        ;

    if( bAlter && COMPARE_EQUAL !=
        pFnt->GetName( pFnt->GetActual() ).CompareToAscii( sBulletFntName ) )
    {
        pNewFnt = new SwFont( *pFnt );
        pNewFnt->SetFamily( FAMILY_DONTKNOW, pFnt->GetActual() );
        pNewFnt->SetName( XubString( sBulletFntName,
                          RTL_TEXTENCODING_MS_1252 ), pFnt->GetActual() );
        pNewFnt->SetStyleName( aEmptyStr, pFnt->GetActual() );
        pNewFnt->SetCharSet( RTL_TEXTENCODING_SYMBOL, pFnt->GetActual() );
        pNewFnt->SetFixKerning( 0 );
        pInf = &((SwTxtSizeInfo&)rInf);
        pNewFnt->Invalidate();
        pInf->SetFont( pNewFnt );
    }
    else
    {
        pFnt = 0;
        pNewFnt = 0;
    }
}

/*************************************************************************
 *                     SwDefFontSave::~SwDefFontSave()
 *************************************************************************/

SwDefFontSave::~SwDefFontSave()
{
    if( pFnt )
    {
        delete pNewFnt;
        // SwFont zurueckstellen
        pFnt->Invalidate();
        pInf->SetFont( pFnt );
    }
}

/*************************************************************************
 *                     SwFtnSave::~SwFtnSave()
 *************************************************************************/

SwFtnSave::~SwFtnSave()
{
    if( pFnt )
    {
        // SwFont zurueckstellen
        *pFnt = *pOld;
        pFnt->GetTox() = pOld->GetTox();
        pFnt->ChgPhysFnt( pInf->GetVsh(), pInf->GetOut() );
        delete pOld;
    }
}

/*************************************************************************
 *                  SwTxtFormatInfo::ChgHyph()
 *************************************************************************/

sal_Bool SwTxtFormatInfo::ChgHyph( const sal_Bool bNew )
{
    const sal_Bool bOld = bAutoHyph;
    if( bAutoHyph != bNew )
    {
        bAutoHyph = bNew;
        InitHyph( bNew );
        // 5744: Sprache am Hyphenator einstellen.
        if( pFnt )
            pFnt->ChgPhysFnt( pVsh, pOut );
    }
    return bOld;
}


