/*************************************************************************
 *
 *  $RCSfile: viewopt.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:42:48 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _MySVXACORR_HXX //autogen
#include <svx/svxacorr.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _REGION_HXX //autogen
#include <vcl/region.hxx>
#endif
#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#include <svtools/accessibilityoptions.hxx>
#endif


#ifndef PRODUCT
BOOL   SwViewOption::bTest9 = FALSE;        //DrawingLayerNotLoading
#endif
Color SwViewOption::aDocBoundColor(COL_LIGHTGRAY);
Color SwViewOption::aObjectBoundColor(COL_LIGHTGRAY);
Color SwViewOption::aDocColor(COL_LIGHTGRAY);
Color SwViewOption::aAppBackgroundColor(COL_LIGHTGRAY);
Color SwViewOption::aTableBoundColor(COL_LIGHTGRAY);
Color SwViewOption::aIndexShadingsColor(COL_LIGHTGRAY);
Color SwViewOption::aLinksColor(COL_BLUE);
Color SwViewOption::aVisitedLinksColor(COL_RED);
Color SwViewOption::aDirectCursorColor(COL_BLUE);
Color SwViewOption::aTextGridColor(COL_LIGHTGRAY);
Color SwViewOption::aSpellColor(COL_LIGHTRED);
Color SwViewOption::aFontColor(COL_BLACK);
Color SwViewOption::aFieldShadingsColor(COL_LIGHTGRAY);
Color SwViewOption::aSectionBoundColor(COL_LIGHTGRAY);
Color SwViewOption::aPageBreakColor(COL_BLUE);
Color SwViewOption::aNotesIndicatorColor(COL_YELLOW);
Color SwViewOption::aScriptIndicatorColor(COL_GREEN);

sal_Int32 SwViewOption::nAppearanceFlags = VIEWOPT_DOC_BOUNDARIES|VIEWOPT_OBJECT_BOUNDARIES;
USHORT SwViewOption::nPixelTwips = 0;   //ein Pixel auf dem Bildschirm


#define LINEBREAK_SIZE 12, 8
#define TAB_SIZE 12, 6

#define MIN_BLANKWIDTH       40
#define MIN_BLANKHEIGHT      40
#define MIN_TABWIDTH        120
#define MIN_TABHEIGHT       200

static const char __FAR_DATA aPostItStr[] = "  ";

/*************************************************************************
 *                    SwViewOption::IsEqualFlags()
 *************************************************************************/

BOOL SwViewOption::IsEqualFlags( const SwViewOption &rOpt ) const
{
    return  nCoreOptions == rOpt.nCoreOptions
            && nCore2Options == rOpt.nCore2Options
            && aSnapSize    == rOpt.aSnapSize
            && nDivisionX   == rOpt.GetDivisionX()
            && nDivisionY   == rOpt.GetDivisionY()
            && nPagePrevRow == rOpt.GetPagePrevRow()
            && nPagePrevCol == rOpt.GetPagePrevCol()
            && aRetoucheColor == rOpt.GetRetoucheColor()
#ifndef PRODUCT
            // korrespondieren zu den Angaben in ui/config/cfgvw.src
            && bTest1 == rOpt.IsTest1()
            && bTest2 == rOpt.IsTest2()
            && bTest3 == rOpt.IsTest3()
            && bTest4 == rOpt.IsTest4()
            && bTest5 == rOpt.IsTest5()
            && bTest6 == rOpt.IsTest6()
            && bTest7 == rOpt.IsTest7()
            && bTest8 == rOpt.IsTest8()
            && bTest10 == rOpt.IsTest10()
#endif
            ;
}

/*************************************************************************
 *                    class SwPxlToTwips
 *************************************************************************/

class SwPxlToTwips
{
    OutputDevice *pOut;
    Color   aLineColor;
    BOOL   bClip;
    Region aClip;
    Point aStart;
    SwRect aRect;
public:
    SwPxlToTwips( OutputDevice *pOut, const SwRect &rRect, const Size &rSize );
    ~SwPxlToTwips();
    inline const Point &GetStartPoint() const { return aStart; }
    inline BOOL  IsTooSmall() const {return 0 == pOut;}
    inline const SwRect &GetRect() const { return aRect; }
    void DrawLine( const Point &rStart, const Point &rEnd );
};

/*************************************************************************
 *                    SwPxlToTwips::CTOR
 *************************************************************************/

SwPxlToTwips::SwPxlToTwips( OutputDevice *pOutDev,
                            const SwRect &rRect, const Size &rSize )
    : pOut( pOutDev )
{
    aRect = pOut->LogicToPixel( rRect.SVRect() );

    // Wenn der Tab nicht mehr darstellbar ist, geben wir auf.
    if( 3 > aRect.Width() )
    {
        pOut = NULL;
        return;
    }

    if( rSize.Height() - 1 > aRect.Height() )
    {
        pOut = NULL;
        return;
    }

    aStart = aRect.Pos();
    aStart.X() += (aRect.Width()  / 2) - (rSize.Width() / 2);
    aStart.Y() += (aRect.Height() / 2) - (rSize.Height() / 2);
    if ( aStart.X() < aRect.Left() )
        aStart.X() = aRect.Left();

    if ( pOut->GetConnectMetaFile() )
        pOut->Push();

    bClip = pOut->IsClipRegion();
    if ( bClip )
    {
        aClip = pOut->GetClipRegion();
        pOut->SetClipRegion();
    }

    aLineColor = pOut->GetLineColor( );
    pOut->SetLineColor( Color(COL_BLACK) );
}

/*************************************************************************
 *                    SwPxlToTwips::DTOR
 *************************************************************************/

SwPxlToTwips::~SwPxlToTwips()
{
    if( pOut )
    {
        if ( pOut->GetConnectMetaFile() )
            pOut->Pop();
        else
        {
            pOut->SetLineColor( aLineColor );
            if( bClip )
                pOut->SetClipRegion( aClip );
        }
    }
}

/*************************************************************************
 *                    SwPxlToTwips::DrawLine
 *************************************************************************/

void SwPxlToTwips::DrawLine( const Point &rStart, const Point &rEnd )
{
    if( pOut )
    {
        const Point aStart( pOut->PixelToLogic( rStart ) );
        const Point aEnd( pOut->PixelToLogic( rEnd ) );
        pOut->DrawLine( aStart, aEnd );
    }
}

/*************************************************************************
 *                    SwViewOption::DrawRect()
 *************************************************************************/

void SwViewOption::DrawRect( OutputDevice *pOut,
                             const SwRect &rRect, long nCol ) const
{
    if ( pOut->GetOutDevType() != OUTDEV_PRINTER )
    {
        const Color aCol( nCol );
        const Color aOldColor( pOut->GetFillColor() );
        pOut->SetFillColor( aCol );
        pOut->DrawRect( rRect.SVRect() );
        pOut->SetFillColor( aOldColor );
    }
    else
        DrawRectPrinter( pOut, rRect );
}

/*************************************************************************
 *                    SwViewOption::DrawRectPrinter()
 *************************************************************************/

void SwViewOption::DrawRectPrinter( OutputDevice *pOut,
                                    const SwRect &rRect ) const
{
    Color aOldColor(pOut->GetLineColor());
    Color aOldFillColor( pOut->GetFillColor() );
    pOut->SetLineColor( Color(COL_BLACK) );
    pOut->SetFillColor( Color(COL_TRANSPARENT ));
    pOut->DrawRect( rRect.SVRect() );
    pOut->SetFillColor( aOldFillColor );
    pOut->SetLineColor( aOldColor );
}

/*************************************************************************
 *                    SwViewOption::PaintTab()
 *************************************************************************/
#ifdef VERTICAL_LAYOUT
#else
void SwViewOption::PaintTab( OutputDevice *pOut, const SwRect &rRect ) const
{
    SwPxlToTwips aSave( pOut, rRect, Size( TAB_SIZE ) );

    if( aSave.IsTooSmall() )
        return;

    Point aStart( aSave.GetStartPoint() );
    const SwRect &rPaintRect = aSave.GetRect();

    // horizontale
    aStart.Y() += 2;
    aStart.X() += 1;
    Point aEnd( aStart );
    aEnd.X() += 10;
    if( aEnd.X() >= rPaintRect.Right() )
        aEnd.X() = rPaintRect.Right() - 1;
    long nDiff = aEnd.X() - aStart.X();
    aSave.DrawLine( aStart, aEnd );

    // Pfeil
    aEnd.X() -= 1;
    aStart.X() = aEnd.X();
    aStart.Y() -= 1;
    aEnd.Y() += 1;
    aSave.DrawLine( aStart, aEnd );

    if( nDiff > 1 && rPaintRect.Height() > 8 )
    {
        aStart.X() -= 1;
        aStart.Y() -= 1;
        aEnd.X() = aStart.X();
        aEnd.Y() += 1;
        aSave.DrawLine( aStart, aEnd );
        if( nDiff > 2 && rPaintRect.Height() > 12 )
        {
            aStart.X() -= 1;
            aEnd.X() -= 1;
            aSave.DrawLine( aStart, aEnd );
        }
    }
}
#endif
/*************************************************************************
 *                    SwViewOption::GetLineBreakWidth()
 *************************************************************************/

USHORT SwViewOption::GetLineBreakWidth( const OutputDevice *pOut ) const
{
    ASSERT( pOut, "no Outdev" );
    const Size aSz( LINEBREAK_SIZE );
    return USHORT(pOut->PixelToLogic( aSz ).Width());
}

/*************************************************************************
 *                    SwViewOption::PaintLineBreak()
 *************************************************************************/
#ifdef VERTICAL_LAYOUT
#else
void SwViewOption::PaintLineBreak( OutputDevice *pOut, const SwRect &rRect ) const
{
    const Size aSz( LINEBREAK_SIZE );
    SwPxlToTwips aSave( pOut, rRect, aSz );
    if( aSave.IsTooSmall() )
        return;
    Point aStart( aSave.GetStartPoint() );
    const SwRect &rPaintRect = aSave.GetRect();

    // horizontale
    aStart.Y() += 4;
    aStart.X() += 1;
    Point aEnd( aStart );
    aEnd.X() += 8;
    if( aEnd.X() >= rPaintRect.Right() - 1 )
        aEnd.X() = rPaintRect.Right() - 2;
    aSave.DrawLine( aStart, aEnd );

    // Pfeil
    aStart.Y() -= 1;
    aStart.X() += 1;
    aEnd.Y()   = aStart.Y() + 2;
    aEnd.X()   = aStart.X();
    aSave.DrawLine( aStart, aEnd );

    // Pfeil
    aStart.Y() -= 1;
    aStart.X() += 1;
    aEnd.Y()   += 1;
    aEnd.X()   += 1;
    aSave.DrawLine( aStart, aEnd );

    // Pfeil
    aStart.X() += 1;
    aEnd.X()   += 1;
    aSave.DrawLine( aStart, aEnd );

    // vertikale
    aStart.Y() -= 2;
    aStart.X() += 6;
    if( aStart.X() >= rPaintRect.Right() )
        aStart.X() = rPaintRect.Right() - 1;
    aEnd.X() = aStart.X();
    aEnd.Y() = aStart.Y() + 3;
    aSave.DrawLine( aStart, aEnd );
}
#endif
/*************************************************************************
 *                    SwViewOption::GetPostItsWidth()
 *************************************************************************/

USHORT SwViewOption::GetPostItsWidth( const OutputDevice *pOut ) const
{
    ASSERT( pOut, "no Outdev" );
    return USHORT(pOut->GetTextWidth( String::CreateFromAscii(aPostItStr )));
}

/*************************************************************************
 *                    SwViewOption::PaintPostIts()
 *************************************************************************/

void SwViewOption::PaintPostIts( OutputDevice *pOut, const SwRect &rRect,
                                 sal_Bool bIsScript ) const
{
    if( pOut )
    {
        Color aOldLineColor( pOut->GetLineColor() );
        pOut->SetLineColor( Color(COL_GRAY ) );
        // Wir ziehen ueberall zwei Pixel ab, damit es schick aussieht
        USHORT nPix = GetPixelTwips() * 2;
        if( rRect.Width() <= 2 * nPix || rRect.Height() <= 2 * nPix )
            nPix = 0;
        const Point aTopLeft(  rRect.Left()  + nPix, rRect.Top()    + nPix );
        const Point aBotRight( rRect.Right() - nPix, rRect.Bottom() - nPix );
        const SwRect aRect( aTopLeft, aBotRight );
        sal_Int32 nColor = bIsScript ? aScriptIndicatorColor.GetColor() : aNotesIndicatorColor.GetColor();
        DrawRect( pOut, aRect, nColor );
        pOut->SetLineColor( aOldLineColor );
    }
}


/*************************************************************************
|*
|*  ViewOption::ViewOption()
|*
|*  Letzte Aenderung    MA 04. Aug. 93
|*
|*************************************************************************/

SwViewOption::SwViewOption() :
    nZoom( 100 ),
    nPagePrevRow( 1 ),
    nPagePrevCol( 2 ),
    eZoom( 0 ),
    nTblDest(TBL_DEST_CELL),
    bReadonly(FALSE),
    bSelectionInReadonly(FALSE),
    aRetoucheColor( COL_TRANSPARENT ),
    nShdwCrsrFillMode( FILL_TAB ),
    bStarOneSetting(FALSE),
    bIsPagePreview(FALSE)
{
    // Initialisierung ist jetzt etwas einfacher
    // alle Bits auf 0
    nCoreOptions =  VIEWOPT_1_IDLE | VIEWOPT_1_HARDBLANK | VIEWOPT_1_SOFTHYPH |
                    VIEWOPT_1_REF |
                    VIEWOPT_1_GRAPHIC |
                    VIEWOPT_1_TABLE    | VIEWOPT_1_DRAW | VIEWOPT_1_CONTROL |
                    VIEWOPT_1_PAGEBACK |
                    VIEWOPT_1_SOLIDMARKHDL | VIEWOPT_1_POSTITS;
    nCore2Options = VIEWOPT_CORE2_BLACKFONT | VIEWOPT_CORE2_HIDDENPARA;
    nUIOptions    = VIEWOPT_2_MODIFIED | VIEWOPT_2_EXECHYPERLINKS | VIEWOPT_2_GRFKEEPZOOM |VIEWOPT_2_ANY_RULER;

    if(MEASURE_METRIC != GetAppLocaleData().getMeasurementSystemEnum())
        aSnapSize.Width() = aSnapSize.Height() = 720;   // 1/2"
    else
        aSnapSize.Width() = aSnapSize.Height() = 567;   // 1 cm
    nDivisionX = nDivisionY = 1;

    bSelectionInReadonly = SW_MOD()->GetAccessibilityOptions().IsSelectionInReadonly();

#ifndef PRODUCT
    // korrespondieren zu den Angaben in ui/config/cfgvw.src
    bTest1 = bTest2 = bTest3 = bTest4 =
             bTest5 = bTest6 = bTest7 = bTest8 = bTest10 = FALSE;
#endif
}


SwViewOption::SwViewOption(const SwViewOption& rVOpt)
{
    bReadonly = FALSE;
    bSelectionInReadonly = FALSE;
    nZoom           = rVOpt.nZoom       ;
    aSnapSize       = rVOpt.aSnapSize   ;
    nDivisionX      = rVOpt.nDivisionX  ;
    nDivisionY      = rVOpt.nDivisionY  ;
    nPagePrevRow    = rVOpt.nPagePrevRow;
    nPagePrevCol    = rVOpt.nPagePrevCol;
    bIsPagePreview  = rVOpt.bIsPagePreview;
    eZoom           = rVOpt.eZoom       ;
    nTblDest        = rVOpt.nTblDest    ;
    nUIOptions      = rVOpt.nUIOptions  ;
    nCoreOptions    = rVOpt.nCoreOptions  ;
    nCore2Options   = rVOpt.nCore2Options  ;
    aRetoucheColor  = rVOpt.GetRetoucheColor();
    sSymbolFont     = rVOpt.sSymbolFont;
    nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
    bStarOneSetting = rVOpt.bStarOneSetting;

#ifndef PRODUCT
    bTest1          = rVOpt.bTest1      ;
    bTest2          = rVOpt.bTest2      ;
    bTest3          = rVOpt.bTest3      ;
    bTest4          = rVOpt.bTest4      ;
    bTest5          = rVOpt.bTest5      ;
    bTest6          = rVOpt.bTest6      ;
    bTest7          = rVOpt.bTest7      ;
    bTest8          = rVOpt.bTest8      ;
    bTest10         = rVOpt.bTest10     ;
#endif
}


SwViewOption& SwViewOption::operator=( const SwViewOption &rVOpt )
{
    nZoom           = rVOpt.nZoom       ;
    aSnapSize       = rVOpt.aSnapSize   ;
    nDivisionX      = rVOpt.nDivisionX  ;
    nDivisionY      = rVOpt.nDivisionY  ;
    nPagePrevRow    = rVOpt.nPagePrevRow;
    nPagePrevCol    = rVOpt.nPagePrevCol;
    bIsPagePreview  = rVOpt.bIsPagePreview;
    eZoom           = rVOpt.eZoom       ;
    nTblDest        = rVOpt.nTblDest    ;
    nUIOptions      = rVOpt.nUIOptions  ;
    nCoreOptions    = rVOpt.nCoreOptions;
    nCore2Options   = rVOpt.nCore2Options;
    aRetoucheColor  = rVOpt.GetRetoucheColor();
    sSymbolFont     = rVOpt.sSymbolFont;
    nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
    bStarOneSetting = rVOpt.bStarOneSetting;

#ifndef PRODUCT
    bTest1          = rVOpt.bTest1      ;
    bTest2          = rVOpt.bTest2      ;
    bTest3          = rVOpt.bTest3      ;
    bTest4          = rVOpt.bTest4      ;
    bTest5          = rVOpt.bTest5      ;
    bTest6          = rVOpt.bTest6      ;
    bTest7          = rVOpt.bTest7      ;
    bTest8          = rVOpt.bTest8      ;
    bTest10         = rVOpt.bTest10     ;
#endif
    return *this;
}


SwViewOption::~SwViewOption()
{
}

/*************************************************************************
|*
|*  ViewOption::Init()
|*
|*  Letzte Aenderung    MA 04. Aug. 93
|*
|*************************************************************************/

void SwViewOption::Init( Window *pWin )
{
    if( !nPixelTwips && pWin )
    {
        nPixelTwips = (USHORT)pWin->PixelToLogic( Size(1,1) ).Height();
    }
}

BOOL SwViewOption::IsAutoCompleteWords() const
{
    const SvxSwAutoFmtFlags& rFlags = OFF_APP()->GetAutoCorrect()->GetSwFlags();
    return /*rFlags.bAutoCompleteWords &&*/ rFlags.bAutoCmpltCollectWords;
}

/*************************************************************************/
/*
/*************************************************************************/

AuthorCharAttr::AuthorCharAttr() :
    nItemId (SID_ATTR_CHAR_UNDERLINE),
    nAttr   (UNDERLINE_SINGLE),
    nColor  (COL_TRANSPARENT)
{
}

/*-----------------07.01.97 13.50-------------------

--------------------------------------------------*/

USHORT      GetHtmlMode(const SwDocShell* pShell)
{
    USHORT nRet = 0;
    if(!pShell || PTR_CAST(SwWebDocShell, pShell))
    {
        nRet = HTMLMODE_ON;
        OfaHtmlOptions* pHtmlOpt = OFF_APP()->GetHtmlOptions();
        switch ( pHtmlOpt->GetExportMode() )
        {
            case HTML_CFG_MSIE_40:
                nRet |= HTMLMODE_PARA_BORDER|HTMLMODE_SMALL_CAPS|
                        HTMLMODE_SOME_STYLES|
                        HTMLMODE_FULL_STYLES|HTMLMODE_GRAPH_POS|
                        HTMLMODE_FULL_ABS_POS|HTMLMODE_SOME_ABS_POS;
            break;
            case HTML_CFG_NS40:
                nRet |= HTMLMODE_PARA_BORDER|HTMLMODE_SOME_STYLES|
                        HTMLMODE_FRM_COLUMNS|HTMLMODE_BLINK|HTMLMODE_GRAPH_POS|
                        HTMLMODE_SOME_ABS_POS;
            break;
            case HTML_CFG_WRITER:
                nRet |= HTMLMODE_PARA_BORDER|HTMLMODE_SMALL_CAPS|
                        HTMLMODE_SOME_STYLES|
                        HTMLMODE_FRM_COLUMNS|HTMLMODE_FULL_STYLES|
                        HTMLMODE_BLINK|HTMLMODE_DROPCAPS|HTMLMODE_GRAPH_POS|
                        HTMLMODE_FULL_ABS_POS|HTMLMODE_SOME_ABS_POS;
            break;
            case HTML_CFG_HTML32:
            break;
        }
    }
    return nRet;
}
/* -----------------------------24.04.2002 10:20------------------------------

 ---------------------------------------------------------------------------*/
Color&   SwViewOption::GetDocColor()
{
    return aDocColor;
}
/* -----------------------------23.04.2002 17:18------------------------------

 ---------------------------------------------------------------------------*/
Color&   SwViewOption::GetDocBoundariesColor()
{
    return aDocBoundColor;
}
/* -----------------------------23.04.2002 17:53------------------------------

 ---------------------------------------------------------------------------*/
Color&   SwViewOption::GetObjectBoundariesColor()
{
    return aObjectBoundColor;
}
/* -----------------------------24.04.2002 10:41------------------------------

 ---------------------------------------------------------------------------*/
Color& SwViewOption::GetAppBackgroundColor()
{
    return aAppBackgroundColor;
}
/*-- 24.04.2002 10:50:11---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetTableBoundariesColor()
{
    return aTableBoundColor;
}
/*-- 24.04.2002 10:50:12---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetIndexShadingsColor()
{
    return aIndexShadingsColor;
}
/*-- 24.04.2002 10:50:12---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetLinksColor()
{
    return aLinksColor;
}
/*-- 24.04.2002 10:50:13---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetVisitedLinksColor()
{
    return aVisitedLinksColor;
}
/*-- 24.04.2002 10:50:13---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetDirectCursorColor()
{
    return aDirectCursorColor;
}
/*-- 24.04.2002 10:50:14---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetTextGridColor()
{
    return aTextGridColor;
}
/*-- 24.04.2002 10:50:14---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetSpellColor()
{
    return aSpellColor;
}
/*-- 06.12.2002 10:50:11---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetFontColor()
{
    return aFontColor;
}
/*-- 24.04.2002 10:50:15---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetFieldShadingsColor()
{
    return aFieldShadingsColor;
}
/*-- 24.04.2002 10:50:15---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetSectionBoundColor()
{
    return aSectionBoundColor;
}
/* -----------------------------2002/07/31 14:00------------------------------

 ---------------------------------------------------------------------------*/
Color& SwViewOption::GetPageBreakColor()
{
    return aPageBreakColor;
}
/*-- 24.04.2002 10:50:15---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetNotesIndicatorColor()
{
    return aNotesIndicatorColor;
}
/*-- 24.04.2002 10:50:15---------------------------------------------------

  -----------------------------------------------------------------------*/
Color&   SwViewOption::GetScriptIndicatorColor()
{
    return aScriptIndicatorColor;
}
/* -----------------------------23.04.2002 17:41------------------------------

 ---------------------------------------------------------------------------*/
void SwViewOption::ApplyColorConfigValues(const svtools::ColorConfig& rConfig )
{
    aDocColor.SetColor(rConfig.GetColorValue(svtools::DOCCOLOR).nColor);

    svtools::ColorConfigValue aValue = rConfig.GetColorValue(svtools::DOCBOUNDARIES);
    aDocBoundColor.SetColor(aValue.nColor);
    nAppearanceFlags = 0;
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_DOC_BOUNDARIES;

    aAppBackgroundColor.SetColor(rConfig.GetColorValue(svtools::APPBACKGROUND).nColor);

    aValue = rConfig.GetColorValue(svtools::OBJECTBOUNDARIES);
    aObjectBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_OBJECT_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::TABLEBOUNDARIES);
    aTableBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_TABLE_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::WRITERIDXSHADINGS);
    aIndexShadingsColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_INDEX_SHADINGS;

    aValue = rConfig.GetColorValue(svtools::LINKS);
    aLinksColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_LINKS;

    aValue = rConfig.GetColorValue(svtools::LINKSVISITED);
    aVisitedLinksColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_VISITED_LINKS;

    aDirectCursorColor.SetColor(rConfig.GetColorValue(svtools::WRITERDIRECTCURSOR).nColor);

    aTextGridColor.SetColor(rConfig.GetColorValue(svtools::WRITERTEXTGRID).nColor);

    aSpellColor.SetColor(rConfig.GetColorValue(svtools::SPELL).nColor);

    aFontColor.SetColor(rConfig.GetColorValue(svtools::FONTCOLOR).nColor);

    aValue = rConfig.GetColorValue(svtools::WRITERFIELDSHADINGS);
    aFieldShadingsColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_FIELD_SHADINGS;

    aValue = rConfig.GetColorValue(svtools::WRITERSECTIONBOUNDARIES);
    aSectionBoundColor.SetColor(aValue.nColor);
    if(aValue.bIsVisible)
        nAppearanceFlags |= VIEWOPT_SECTION_BOUNDARIES;

    aValue = rConfig.GetColorValue(svtools::WRITERPAGEBREAKS);
    aPageBreakColor.SetColor(aValue.nColor);

    aNotesIndicatorColor.SetColor(rConfig.GetColorValue(svtools::WRITERNOTESINDICATOR).nColor);
    aScriptIndicatorColor.SetColor(rConfig.GetColorValue(svtools::WRITERSCRIPTINDICATOR).nColor);
}
/* -----------------------------23.04.2002 17:48------------------------------

 ---------------------------------------------------------------------------*/
void SwViewOption::SetAppearanceFlag(sal_Int32 nFlag, BOOL bSet, BOOL bSaveInConfig )
{
    if(bSet)
        nAppearanceFlags |= nFlag;
    else
        nAppearanceFlags &= ~nFlag;
    if(bSaveInConfig)
    {
        //create an editable svtools::ColorConfig and store the change
        svtools::EditableColorConfig aEditableConfig;
        struct FlagToConfig_Impl
        {
            sal_Int32               nFlag;
            svtools::ColorConfigEntry   eEntry;
        };
        static const FlagToConfig_Impl aFlags[] =
        {
            VIEWOPT_DOC_BOUNDARIES     ,   svtools::DOCBOUNDARIES     ,
            VIEWOPT_OBJECT_BOUNDARIES  ,   svtools::OBJECTBOUNDARIES  ,
            VIEWOPT_TABLE_BOUNDARIES   ,   svtools::TABLEBOUNDARIES   ,
            VIEWOPT_INDEX_SHADINGS     ,   svtools::WRITERIDXSHADINGS     ,
            VIEWOPT_LINKS              ,   svtools::LINKS              ,
            VIEWOPT_VISITED_LINKS      ,   svtools::LINKSVISITED      ,
            VIEWOPT_FIELD_SHADINGS     ,   svtools::WRITERFIELDSHADINGS     ,
            VIEWOPT_SECTION_BOUNDARIES ,   svtools::WRITERSECTIONBOUNDARIES ,
            0                          ,   svtools::ColorConfigEntryCount
        };
        sal_uInt16 nPos = 0;
        while(aFlags[nPos].nFlag)
        {
            if(0 != (nFlag&aFlags[nPos].nFlag))
            {
                svtools::ColorConfigValue aValue = aEditableConfig.GetColorValue(aFlags[nPos].eEntry);
                aValue.bIsVisible = bSet;
                aEditableConfig.SetColorValue(aFlags[nPos].eEntry, aValue);
            }
            nPos++;
        }
    }
}
/* -----------------------------24.04.2002 10:42------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwViewOption::IsAppearanceFlag(sal_Int32 nFlag)
{
    return 0 != (nAppearanceFlags & nFlag);
}

