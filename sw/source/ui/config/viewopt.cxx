/*************************************************************************
 *
 *  $RCSfile: viewopt.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:07:00 $
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


#ifndef PRODUCT
BOOL   SwViewOption::bTest9 = FALSE;        //DrawingLayerNotLoading
#endif
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
                                 long nCol ) const
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
        DrawRect( pOut, aRect, nCol );
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
    aRetoucheColor( COL_TRANSPARENT ),
    aShdwCrsrCol( COL_BLUE ),
    aIdxBackgrndCol( COL_LIGHTGRAY ),
    nShdwCrsrFillMode( FILL_TAB ),
    bStarOneSetting(FALSE)
{
    // Initialisierung ist jetzt etwas einfacher
    // alle Bits auf 0
    nCoreOptions =  VIEWOPT_1_IDLE | VIEWOPT_1_HARDBLANK | VIEWOPT_1_SOFTHYPH |
                    VIEWOPT_1_TOX | VIEWOPT_1_REF | VIEWOPT_1_FIELD |
                    VIEWOPT_1_FOOTNOTE | VIEWOPT_1_SUBSLINES | VIEWOPT_1_GRAPHIC |
                    VIEWOPT_1_TABLE    | VIEWOPT_1_DRAW | VIEWOPT_1_CONTROL |
                    VIEWOPT_1_SUBSTABLE| VIEWOPT_1_PAGEBACK |
                    VIEWOPT_1_SOLIDMARKHDL | VIEWOPT_1_POSTITS;
    nCore2Options = VIEWOPT_CORE2_BLACKFONT | VIEWOPT_CORE2_HIDDENPARA|
                    VIEWOPT_CORE2_INDEX_BACKGROUND | VIEWOPT_CORE2_SECTION_BOUNDS;
    nUIOptions    = VIEWOPT_2_MODIFIED | VIEWOPT_2_EXECHYPERLINKS;

    MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
    if(MEASURE_METRIC != GetAppLocaleData().getMeasurementSystemEnum())
        aSnapSize.Width() = aSnapSize.Height() = 720;   // 1/2"
    else
        aSnapSize.Width() = aSnapSize.Height() = 567;   // 1 cm
    nDivisionX = nDivisionY = 1;


#ifndef PRODUCT
    // korrespondieren zu den Angaben in ui/config/cfgvw.src
    bTest1 = bTest2 = bTest3 = bTest4 =
             bTest5 = bTest6 = bTest7 = bTest8 = bTest10 = FALSE;
#endif
}


SwViewOption::SwViewOption(const SwViewOption& rVOpt)
{
    bReadonly = FALSE;
    nZoom           = rVOpt.nZoom       ;
    aSnapSize       = rVOpt.aSnapSize   ;
    nDivisionX      = rVOpt.nDivisionX  ;
    nDivisionY      = rVOpt.nDivisionY  ;
    nPagePrevRow    = rVOpt.nPagePrevRow;
    nPagePrevCol    = rVOpt.nPagePrevCol;
    eZoom           = rVOpt.eZoom       ;
    nTblDest        = rVOpt.nTblDest    ;
    nUIOptions      = rVOpt.nUIOptions  ;
    nCoreOptions    = rVOpt.nCoreOptions  ;
    nCore2Options   = rVOpt.nCore2Options  ;
    aRetoucheColor  = rVOpt.GetRetoucheColor();
    sSymbolFont     = rVOpt.sSymbolFont;
    aShdwCrsrCol    = rVOpt.aShdwCrsrCol;
    aIdxBackgrndCol = rVOpt.aIdxBackgrndCol;
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
    eZoom           = rVOpt.eZoom       ;
    nTblDest        = rVOpt.nTblDest    ;
    nUIOptions      = rVOpt.nUIOptions  ;
    nCoreOptions    = rVOpt.nCoreOptions;
    nCore2Options   = rVOpt.nCore2Options;
    aRetoucheColor  = rVOpt.GetRetoucheColor();
    sSymbolFont     = rVOpt.sSymbolFont;
    aShdwCrsrCol    = rVOpt.aShdwCrsrCol;
    aIdxBackgrndCol = rVOpt.aIdxBackgrndCol;
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
            case HTML_CFG_NS30:
                nRet |= HTMLMODE_BLINK|HTMLMODE_FRM_COLUMNS|HTMLMODE_FIRSTLINE;
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


/************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:33  hr
      initial import

      Revision 1.57  2000/09/18 16:05:18  willem.vandorp
      OpenOffice header added.

      Revision 1.56  2000/04/11 08:02:24  os
      UNICODE

      Revision 1.55  2000/03/03 15:16:59  os
      StarView remainders removed

      Revision 1.54  2000/01/19 18:24:09  jp
      Bug #72118#: change default of view sectionboundaries

      Revision 1.53  2000/01/13 21:28:44  jp
      Task #71894#: new Options for SW-AutoComplete

      Revision 1.52  1999/09/20 10:35:00  os
      Color changes

      Revision 1.51  1999/07/28 11:03:20  OS
      index background in lighter gray


      Rev 1.50   28 Jul 1999 13:03:20   OS
   index background in lighter gray

      Rev 1.49   13 Jul 1999 08:47:52   OS
   #67584# Scrollbar settings via StarOne

      Rev 1.48   09 Jun 1999 13:22:34   OS
   index background

      Rev 1.47   17 Mar 1999 11:24:10   JP
   Task #63576#: IsAutoCompleteWords - das Flag von der OffApp erfragen

      Rev 1.46   15 Mar 1999 09:47:52   MA
   #63047# neue Defaults

      Rev 1.45   09 Mar 1999 19:34:34   JP
   Task #61405#: AutoCompletion von Woertern

      Rev 1.44   19 Jan 1999 08:47:36   MIB
   #60957#: Kapitaelchen auch fuer IE4

      Rev 1.43   29 Apr 1998 09:27:06   MA
   BackgroundBrush -> RetoucheColor

      Rev 1.42   20 Apr 1998 09:10:42   OS
   IE3 entfaellt

      Rev 1.41   03 Apr 1998 14:42:22   OS
   HTMLMODE_SOME/FULL_ABS_POS

      Rev 1.40   05 Mar 1998 14:34:12   OM
   Redline-Attribute in Module-Cfg speichern

      Rev 1.39   23 Feb 1998 12:40:46   OM
   Redlining-Optionen

      Rev 1.38   04 Feb 1998 17:53:06   MA
   chg: Notiz wieder per default an

      Rev 1.37   09 Jan 1998 17:15:14   AMA
   Fix #46523#: Colors sollten als long durchgereicht werden...

      Rev 1.36   28 Nov 1997 15:24:22   MA
   includes

      Rev 1.35   24 Nov 1997 17:58:48   MA
   include

      Rev 1.34   03 Nov 1997 16:12:36   JP
   neu: Optionen/-Page/Basic-Schnittst. fuer ShadowCursor

      Rev 1.33   01 Sep 1997 13:16:58   OS
   DLL-Umstellung

      Rev 1.32   08 Aug 1997 17:37:24   OM
   Headerfile-Umstellung

      Rev 1.31   05 Aug 1997 14:52:06   OS
   Option fuer Grafikhintergrund #41663#

      Rev 1.30   02 Jul 1997 12:46:00   MA
   inlines

      Rev 1.29   17 Jun 1997 17:24:06   MIB
   HTML-Modus-Flgas fuer Netacpe 4.0

      Rev 1.28   06 Jun 1997 12:44:24   MA
   chg: versteckte Absaetze ausblenden

      Rev 1.27   05 May 1997 10:56:32   AMA
   Fix #39418#: Tabulatorsymbolgroesse jetzt zoomabhaengig.

      Rev 1.26   25 Feb 1997 09:11:14   MA
   chg: Option fuer SolidHdl

      Rev 1.25   13 Feb 1997 18:57:16   AMA
   New: JavaScript-Felder werden gruen dargestellt.

      Rev 1.24   07 Feb 1997 18:25:30   OS
   HtmlMode auch fuer pDocSh == 0; Flags berichtigt

      Rev 1.23   05 Feb 1997 13:44:18   OS
   HTML-Modi erweitert

      Rev 1.22   27 Jan 1997 16:33:52   OS
   GetHtmlMode wird mit der DocShell ermittelt

      Rev 1.21   08 Jan 1997 10:47:10   OS
   neu: ::GetHtmlMode()

      Rev 1.20   13 Dec 1996 14:33:32   OS
   UndoCount wird aus der SfxApp besorgt

      Rev 1.19   10 Dec 1996 16:58:46   OS
   TabDist ab sofort in der OFA

      Rev 1.18   28 Nov 1996 15:20:24   OS
   neu: Schwarz drucken

      Rev 1.17   25 Sep 1996 14:56:18   OS
   Linealabfragen nicht mehr inline

      Rev 1.16   19 Sep 1996 18:40:54   OS
   SetUIOptions enthaelt auch nTblDest

      Rev 1.15   06 Sep 1996 14:32:20   OS
   UsrPrefs wieder vereinheitlicht

      Rev 1.14   30 Aug 1996 08:44:04   OS
   neu: sSymbolFont

      Rev 1.13   27 Aug 1996 10:11:44   OS
   IsEqualFlags: Brush vergleichen

      Rev 1.12   27 Aug 1996 09:44:46   OS
   operator=

*************************************************************************/

