/*************************************************************************
 *
 *  $RCSfile: common_gfx.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:59:06 $
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

#ifndef _PSPRINT_PRINTERGFX_HXX_
#include <psprint/printergfx.hxx>
#endif
#ifndef _PSPRINT_PRINTERJOB_HXX_
#include <psprint/printerjob.hxx>
#endif
#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifndef _PSPRINT_PRINTERUTIL_HXX_
#include <psputil.hxx>
#endif
#ifndef _PSPRINT_GLYPHSET_HXX_
#include <glyphset.hxx>
#endif
#ifndef _PSPRINT_STRHELPER_HXX_
#include <psprint/strhelper.hxx>
#endif
#ifndef _PSPRINT_PRINTERINFOMANAGER_HXX_
#include <psprint/printerinfomanager.hxx>
#endif

#include <tools/debug.hxx>
#include <tools/color.hxx>
#ifndef _POLY_HXX
#include <tools/poly.hxx>
#endif

using namespace psp ;

static const sal_Int32 nMaxTextColumn = 80;

GraphicsStatus::GraphicsStatus() :
        mnTextHeight( 0 ),
        mnTextWidth( 0 ),
        mfLineWidth( -1 ),
        mbArtItalic( false ),
        mbArtBold( false )
{
}

/*
 * non graphics graphics routines
 */

sal_Bool
PrinterGfx::Init (PrinterJob &rPrinterJob)
{
    mpPageHeader = rPrinterJob.GetCurrentPageHeader ();
    mpPageBody   = rPrinterJob.GetCurrentPageBody ();
    mnDepth      = rPrinterJob.GetDepth ();
    mnPSLevel    = rPrinterJob.GetPostscriptLevel ();
    mbColor      = rPrinterJob.IsColorPrinter ();

    mnDpi = rPrinterJob.GetResolution();
    rPrinterJob.GetScale (mfScaleX, mfScaleY);
    const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( rPrinterJob.GetPrinterName() ) );
    if( mpFontSubstitutes )
        delete const_cast< ::std::hash_map<fontID,fontID>* >(mpFontSubstitutes);
    if( rInfo.m_bPerformFontSubstitution )
        mpFontSubstitutes = new ::std::hash_map< fontID, fontID >( rInfo.m_aFontSubstitutions );
    else
        mpFontSubstitutes = NULL;
    mbUploadPS42Fonts = rInfo.m_pParser ? ( rInfo.m_pParser->isType42Capable() ? sal_True : sal_False ) : sal_False;

    return sal_True;
}

sal_Bool
PrinterGfx::Init (const JobData& rData)
{
    mpPageHeader    = NULL;
    mpPageBody      = NULL;
    mnDepth         = rData.m_nColorDepth;
    mnPSLevel       = rData.m_nPSLevel ? rData.m_nPSLevel : rData.m_pParser->getLanguageLevel();
    mbColor         = rData.m_nColorDevice ? ( rData.m_nColorDevice == -1 ? sal_False : sal_True ) : ( rData.m_pParser->isColorDevice() ? sal_True : sal_False );
    int nRes = rData.m_aContext.getRenderResolution();
    mnDpi           = nRes;
    mfScaleX        = (double)72.0 / (double)mnDpi;
    mfScaleY        = (double)72.0 / (double)mnDpi;
    const PrinterInfo& rInfo( PrinterInfoManager::get().getPrinterInfo( rData.m_aPrinterName ) );
    if( mpFontSubstitutes )
        delete const_cast< ::std::hash_map<fontID,fontID>* >(mpFontSubstitutes);
    if( rInfo.m_bPerformFontSubstitution )
        mpFontSubstitutes = new ::std::hash_map< fontID, fontID >( rInfo.m_aFontSubstitutions );
    else
        mpFontSubstitutes = NULL;
    mbUploadPS42Fonts = rInfo.m_pParser ? ( rInfo.m_pParser->isType42Capable() ? sal_True : sal_False ) : sal_False;

    return sal_True;
}

void
PrinterGfx::GetResolution (sal_Int32 &rDpiX, sal_Int32 &rDpiY) const
{
    rDpiX = mnDpi;
    rDpiY = mnDpi;
}

void
PrinterGfx::GetScreenFontResolution (sal_Int32 &rDpiX, sal_Int32 &rDpiY) const
{
    rDpiX = mnDpi;
    rDpiY = mnDpi;
}

sal_uInt16
PrinterGfx::GetBitCount ()
{
    return mnDepth;
}

PrinterGfx::PrinterGfx() :
        mpPageHeader (NULL),
        mpPageBody (NULL),
        mnFontID (0),
        mnFallbackID (0),
        maLineColor (0, 0xff, 0),
        maFillColor (0xff,0,0),
        maTextColor (0,0,0),
        mbTextVertical (false),
        mrFontMgr (PrintFontManager::get()),
        mbCompressBmp (sal_True),
        mpFontSubstitutes( NULL )
{
    maVirtualStatus.mfLineWidth = 1.0;
    maVirtualStatus.mnTextHeight = 12;
    maVirtualStatus.mnTextWidth = 0;

    maGraphicsStack.push_back( GraphicsStatus() );
}

PrinterGfx::~PrinterGfx()
{
    /*
     *  #95810# the original reasoning why mpFontSubstitutes is a pointer was
     *  that applications should release all PrinterGfx when printers change
     *  because they are really invalid; the corresponding printers may have
     *  changed their settings or even not exist anymore.
     *
     *  Alas, this is not always done real time. So we keep a local copy of
     *  the font substitutes now in case of bad timing.
     */
    delete const_cast< ::std::hash_map<fontID,fontID>* >(mpFontSubstitutes);
}

void
PrinterGfx::Clear()
{
    mpPageHeader                    = NULL;
    mpPageBody                      = NULL;
    mnFontID                        = 0;
    maVirtualStatus                 = GraphicsStatus();
    maVirtualStatus.mnTextHeight    = 12;
    maVirtualStatus.mnTextWidth     = 0;
    maVirtualStatus.mfLineWidth     = 1.0;
    mbTextVertical                  = false;
    maLineColor                     = PrinterColor();
    maFillColor                     = PrinterColor();
    maTextColor                     = PrinterColor();
    mbCompressBmp                   = sal_True;
    mnDpi                           = 300;
    mnDepth                         = 24;
    mnPSLevel                       = 2;
    mbColor                         = sal_True;
    mnTextAngle                     = 0;

    maClipRegion.clear();
    maGraphicsStack.clear();
    maGraphicsStack.push_back( GraphicsStatus() );
}

/*
 * clip region handling
 */

void
PrinterGfx::ResetClipRegion()
{
    maClipRegion.clear();
    PSGRestore ();
    PSGSave (); // get "clean" clippath
}

void
PrinterGfx::BeginSetClipRegion( sal_uInt32 )
{
    maClipRegion.clear();
}

sal_Bool
PrinterGfx::UnionClipRegion (sal_Int32 nX,sal_Int32 nY,sal_Int32 nDX,sal_Int32 nDY)
{
    if( nDX && nDY )
        maClipRegion.push_back (Rectangle(Point(nX,nY ), Size(nDX,nDY)));
    return sal_True;
}

sal_Bool
PrinterGfx::JoinVerticalClipRectangles( std::list< Rectangle >::iterator& it,
                                        Point& rOldPoint, sal_Int32& rColumn )
{
    sal_Bool bSuccess = sal_False;

    std::list< Rectangle >::iterator tempit, nextit;
    nextit = it;
    ++nextit;
    std::list< Point > leftside, rightside;

    Rectangle aLastRect( *it );
    leftside.push_back( Point( it->Left(), it->Top() ) );
    rightside.push_back( Point( it->Right()+1, it->Top() ) );
    while( nextit != maClipRegion.end() )
    {
        tempit = nextit;
        ++tempit;
        if( nextit->Top() == aLastRect.Bottom()+1 )
        {
            if(
               ( nextit->Left() >= aLastRect.Left() && nextit->Left() <= aLastRect.Right() ) // left endpoint touches last rectangle
               ||
               ( nextit->Right() >= aLastRect.Left() && nextit->Right() <= aLastRect.Right() ) // right endpoint touches last rectangle
               ||
               ( nextit->Left() <= aLastRect.Left() && nextit->Right() >= aLastRect.Right() ) // whole line touches last rectangle
               )
            {
                if( aLastRect.GetHeight() > 1                           ||
                    abs( aLastRect.Left() - nextit->Left() ) > 2        ||
                    abs( aLastRect.Right() - nextit->Right() ) > 2
                    )
                {
                    leftside.push_back( Point( aLastRect.Left(), aLastRect.Bottom()+1 ) );
                    rightside.push_back( Point( aLastRect.Right()+1, aLastRect.Bottom()+1 ) );
                }
                aLastRect = *nextit;
                leftside.push_back( aLastRect.TopLeft() );
                rightside.push_back( aLastRect.TopRight() );
                maClipRegion.erase( nextit );
            }
        }
        nextit = tempit;
    }
    if( leftside.size() > 1 )
    {
        // push the last coordinates
        leftside.push_back( Point( aLastRect.Left(), aLastRect.Bottom()+1 ) );
        rightside.push_back( Point( aLastRect.Right()+1, aLastRect.Bottom()+1 ) );

        // cool, we can concatenate rectangles
        int nDX = -65536, nDY = 65536;
        int nNewDX = 0, nNewDY = 0;

        Point aLastPoint = leftside.front();
        PSBinMoveTo (aLastPoint, rOldPoint, rColumn);
        leftside.pop_front();
        while( leftside.begin() != leftside.end() )
        {
            Point aPoint (leftside.front());
            leftside.pop_front();
            // may have been the last one
            if( leftside.begin() != leftside.end() )
            {
                nNewDX = aPoint.X() - aLastPoint.X();
                nNewDY = aPoint.Y() - aLastPoint.Y();
                if( nNewDX == 0 && nDX == 0 )
                    continue;
                if( nDX != 0 && nNewDX != 0 &&
                    (double)nNewDY/(double)nNewDX == (double)nDY/(double)nDX )
                    continue;
            }
            PSBinLineTo (aPoint, rOldPoint, rColumn);
            aLastPoint = aPoint;
        }

        aLastPoint = rightside.back();
        nDX = -65536;
        nDY = 65536;
        PSBinLineTo (aLastPoint, rOldPoint, rColumn);
        rightside.pop_back();
        while( rightside.begin() != rightside.end() )
        {
            Point aPoint (rightside.back());
            rightside.pop_back();
            if( rightside.begin() != rightside.end() )
            {
                nNewDX = aPoint.X() - aLastPoint.X();
                nNewDY = aPoint.Y() - aLastPoint.Y();
                if( nNewDX == 0 && nDX == 0 )
                    continue;
                if( nDX != 0 && nNewDX != 0 &&
                    (double)nNewDY/(double)nNewDX == (double)nDY/(double)nDX )
                    continue;
            }
            PSBinLineTo (aPoint, rOldPoint, rColumn);
        }

        tempit = it;
        ++tempit;
        maClipRegion.erase( it );
        it = tempit;
        bSuccess = sal_True;
    }
    return bSuccess;
}

void
PrinterGfx::EndSetClipRegion()
{
    PSGRestore ();
    PSGSave (); // get "clean" clippath

    PSBinStartPath ();
    Point aOldPoint (0, 0);
    sal_Int32 nColumn = 0;

    std::list< Rectangle >::iterator it = maClipRegion.begin();
    while( it != maClipRegion.end() )
    {
        // try to concatenate adjacent rectangles
        // first try in y direction, then in x direction
        if( ! JoinVerticalClipRectangles( it, aOldPoint, nColumn ) )
        {
            // failed, so it is a single rectangle
            PSBinMoveTo (it->TopLeft(),                          aOldPoint, nColumn );
            PSBinLineTo (Point( it->Left(), it->Bottom()+1 ),    aOldPoint, nColumn );
            PSBinLineTo (Point( it->Right()+1, it->Bottom()+1 ), aOldPoint, nColumn );
            PSBinLineTo (Point( it->Right()+1, it->Top() ),      aOldPoint, nColumn );
            ++it;
        }
    }

    PSBinEndPath ();

    WritePS (mpPageBody, "closepath clip newpath\n");
    maClipRegion.clear();
}

/*
 * draw graphic primitives
 */

void
PrinterGfx::DrawRect (const Rectangle& rRectangle )
{
    char pRect [128];
    sal_Int32 nChar = 0;

    nChar  = psp::getValueOf (rRectangle.TopLeft().X(),     pRect);
    nChar += psp::appendStr (" ",                           pRect + nChar);
    nChar += psp::getValueOf (rRectangle.TopLeft().Y(),     pRect + nChar);
    nChar += psp::appendStr (" ",                           pRect + nChar);
    nChar += psp::getValueOf (rRectangle.GetWidth(),        pRect + nChar);
    nChar += psp::appendStr (" ",                           pRect + nChar);
    nChar += psp::getValueOf (rRectangle.GetHeight(),       pRect + nChar);
    nChar += psp::appendStr (" ",                           pRect + nChar);

    if( maFillColor.Is() )
    {
        PSSetColor (maFillColor);
        PSSetColor ();
        WritePS (mpPageBody, pRect, nChar);
        WritePS (mpPageBody, "rectfill\n");
    }
    if( maLineColor.Is() )
    {
        PSSetColor (maLineColor);
        PSSetColor ();
        PSSetLineWidth ();
        WritePS (mpPageBody, pRect, nChar);
        WritePS (mpPageBody, "rectstroke\n");
    }
}

void
PrinterGfx::DrawLine (const Point& rFrom, const Point& rTo)
{
    if( maLineColor.Is() )
    {
        PSSetColor (maLineColor);
        PSSetColor ();
        PSSetLineWidth ();

        PSMoveTo (rFrom);
        PSLineTo (rTo);
        WritePS (mpPageBody, "stroke\n" );
    }
}

void
PrinterGfx::DrawPixel (const Point& rPoint, const PrinterColor& rPixelColor)
{
    if( rPixelColor.Is() )
    {
        PSSetColor (rPixelColor);
        PSSetColor ();

        PSMoveTo (rPoint);
        PSLineTo (Point (rPoint.X ()+1, rPoint.Y ()));
        PSLineTo (Point (rPoint.X ()+1, rPoint.Y ()+1));
        PSLineTo (Point (rPoint.X (), rPoint.Y ()+1));
        WritePS (mpPageBody, "fill\n" );
    }
}

void
PrinterGfx::DrawPolyLine (sal_uInt32 nPoints, const Point* pPath)
{
    if( maLineColor.Is() && nPoints && pPath )
    {
        PSSetColor (maLineColor);
        PSSetColor ();
        PSSetLineWidth ();

        PSBinCurrentPath (nPoints, pPath);

        WritePS (mpPageBody, "stroke\n" );
    }
}

void
PrinterGfx::DrawPolygon (sal_uInt32 nPoints, const Point* pPath)
{
    // premature end of operation
    if (!(nPoints > 1) || (pPath == NULL) || !(maFillColor.Is() || maLineColor.Is()))
        return;

    // setup closed path
    Point aPoint( 0, 0 );
    sal_Int32 nColumn( 0 );

    PSBinStartPath();
    PSBinMoveTo( pPath[0], aPoint, nColumn );
    for( int n = 1; n < nPoints; n++ )
        PSBinLineTo( pPath[n], aPoint, nColumn );
    if( pPath[0] != pPath[nPoints-1] )
        PSBinLineTo( pPath[0], aPoint, nColumn );
    PSBinEndPath();

    // fill the polygon first, then draw the border, note that fill and
    // stroke reset the currentpath

    // if fill and stroke, save the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGSave();

    if (maFillColor.Is ())
    {
        PSSetColor (maFillColor);
        PSSetColor ();
        WritePS (mpPageBody, "eofill\n");
    }

    // restore the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGRestore();

    if (maLineColor.Is ())
    {
        PSSetColor (maLineColor);
        PSSetColor ();
        PSSetLineWidth ();
        WritePS (mpPageBody, "stroke\n");
    }
}

void
PrinterGfx::DrawPolyPolygon (sal_uInt32 nPoly, const sal_uInt32* pSizes, const Point** pPaths )
{
    // sanity check
    if ( !nPoly || !pPaths || !(maFillColor.Is() || maLineColor.Is()))
        return;


    // setup closed path
    for( int i = 0; i < nPoly; i++ )
    {
        Point aPoint( 0, 0 );
        sal_Int32 nColumn( 0 );

        PSBinStartPath();
        PSBinMoveTo( pPaths[i][0], aPoint, nColumn );
        for( int n = 1; n < pSizes[i]; n++ )
            PSBinLineTo( pPaths[i][n], aPoint, nColumn );
        if( pPaths[i][0] != pPaths[i][pSizes[i]-1] )
                PSBinLineTo( pPaths[i][0], aPoint, nColumn );
        PSBinEndPath();
    }

    // if eofill and stroke, save the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGSave();

    // first draw area
    if( maFillColor.Is() )
    {
        PSSetColor (maFillColor);
        PSSetColor ();
        WritePS (mpPageBody, "eofill\n");
    }

    // restore the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGRestore();

    // now draw outlines
    if( maLineColor.Is() )
    {
        PSSetColor (maLineColor);
        PSSetColor ();
        PSSetLineWidth ();
        WritePS (mpPageBody, "stroke\n");
    }
}

/*
 * Bezier Polygon Drawing methods.
 */

void
PrinterGfx::DrawPolyLineBezier (sal_uInt32 nPoints, const Point* pPath, const BYTE* pFlgAry)
{
    const sal_uInt32 nBezString = 1024;
    sal_Char pString[nBezString];

    if ( maLineColor.Is() && nPoints && pPath )
    {
        PSSetColor (maLineColor);
        PSSetColor ();
        PSSetLineWidth ();

        if (pFlgAry[0] != POLY_NORMAL) //There must be a starting point to moveto
        {
            return;
        }
        else
        {
            snprintf(pString, nBezString, "%i %i moveto\n", pPath[0].X(), pPath[0].Y());
            WritePS(mpPageBody, pString);
        }

        // Handle the drawing of mixed lines mixed with curves
        // - a normal point followed by a normal point is a line
        // - a normal point followed by 2 control points and a normal point is a curve
        for (int i=1; i<nPoints;)
        {
            if (pFlgAry[i+1] != POLY_CONTROL) //If the next point is a POLY_NORMAL, we're drawing a line
            {
                if (i+1 >= nPoints) return; //Make sure we don't pass the end of the array
                snprintf(pString, nBezString, "%i %i lineto\n", pPath[i].X(), pPath[i].Y());
                i++;
            }
            else //Otherwise we're drawing a spline
            {
                if (i+3 >= nPoints) return; //Make sure we don't pass the end of the array
                snprintf(pString, nBezString, "%i %i %i %i %i %i curveto\n",
                        pPath[i+1].X(), pPath[i+1].Y(),
                        pPath[i+2].X(), pPath[i+2].Y(),
                        pPath[i+3].X(), pPath[i+3].Y());
                i+=3;
            }
            WritePS(mpPageBody, pString);
        }
    }

    // if eofill and stroke, save the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGSave();

    // first draw area
    if( maFillColor.Is() )
    {
        PSSetColor (maFillColor);
        PSSetColor ();
        WritePS (mpPageBody, "eofill\n");
    }

    // restore the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGRestore();

    // now draw outlines
    if( maLineColor.Is() )
    {
        PSSetColor (maLineColor);
        PSSetColor ();
        PSSetLineWidth ();
        WritePS (mpPageBody, "stroke\n");
    }
}

void
PrinterGfx::DrawPolygonBezier (sal_uInt32 nPoints, const Point* pPath, const BYTE* pFlgAry)
{
    const sal_uInt32 nBezString = 1024;
    sal_Char pString[nBezString];
    // premature end of operation
    if (!(nPoints > 1) || (pPath == NULL) || !(maFillColor.Is() || maLineColor.Is()))
        return;

    snprintf(pString, nBezString, "%i %i moveto\n", pPath[0].X(), pPath[0].Y());
    WritePS(mpPageBody, pString); //Move to the starting point for the PolyPoygon
    for (int i=1; i < nPoints;)
    {
        if (pFlgAry[i] != POLY_CONTROL)
        {
            snprintf(pString, nBezString, "%i %i lineto\n", pPath[i].X(), pPath[i].Y());
            WritePS(mpPageBody, pString);
            i++;
        }
        else
        {
            if (i+2 >= nPoints)
                return; //Error: wrong sequence of contol/normal points somehow
            if ((pFlgAry[i] == POLY_CONTROL) && (pFlgAry[i+1] == POLY_CONTROL) &&
                    (pFlgAry[i+2] != POLY_CONTROL))
            {
                snprintf(pString, nBezString, "%i %i %i %i %i %i curveto\n",
                        pPath[i].X(), pPath[i].Y(),
                        pPath[i+1].X(), pPath[i+1].Y(),
                        pPath[i+2].X(), pPath[i+2].Y());
                WritePS(mpPageBody, pString);
            }
            else
            {
                fprintf(stderr, "Strange output\n");
            }
            i+=3;
        }
    }

    // if fill and stroke, save the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGSave();

    if (maFillColor.Is ())
    {
        PSSetColor (maFillColor);
        PSSetColor ();
        WritePS (mpPageBody, "eofill\n");
    }

    // restore the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGRestore();
}

void
PrinterGfx::DrawPolyPolygonBezier (sal_uInt32 nPoly, const sal_uInt32 * pPoints, const Point* const * pPtAry, const BYTE* const* pFlgAry)
{
    const sal_uInt32 nBezString = 1024;
    sal_Char pString[nBezString];
    if ( !nPoly || !pPtAry || !pPoints || !(maFillColor.Is() || maLineColor.Is()))
        return;


    for (int i=0; i<nPoly;i++)
    {
        sal_uInt32 nPoints = pPoints[i];
        snprintf(pString, nBezString, "%i %i moveto\n", pPtAry[i][0].X(), pPtAry[i][0].Y()); //Move to the starting point
        WritePS(mpPageBody, pString);
        for (int j=1; j < nPoints;)
        {
            if (pFlgAry[i][j] != POLY_CONTROL)
            {
                snprintf(pString, nBezString, "%i %i lineto\n", pPtAry[i][j].X(), pPtAry[i][j].Y());
                WritePS(mpPageBody, pString);
                j++;
            }
            else
            {
                if (j+2 >= nPoints)
                    return; //Error: wrong sequence of contol/normal points somehow
                if ((pFlgAry[i][j] == POLY_CONTROL) && (pFlgAry[i][j+1] == POLY_CONTROL) && (pFlgAry[i][j+2] != POLY_CONTROL))
                {
                    snprintf(pString, nBezString, "%i %i %i %i %i %i curveto\n",
                            pPtAry[i][j].X(), pPtAry[i][j].Y(),
                            pPtAry[i][j+1].X(), pPtAry[i][j+1].Y(),
                            pPtAry[i][j+2].X(), pPtAry[i][j+2].Y());
                    WritePS(mpPageBody, pString);
                }
                else
                {
                    fprintf(stderr, "Strange output\n");
                }
                j+=3;
            }
        }
    }

    // if fill and stroke, save the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGSave();

    if (maFillColor.Is ())
    {
        PSSetColor (maFillColor);
        PSSetColor ();
        WritePS (mpPageBody, "eofill\n");
    }

    // restore the current path
    if( maFillColor.Is() && maLineColor.Is())
        PSGRestore();
}


/*
 * postscript generating routines
 */
void
PrinterGfx::PSGSave ()
{
    WritePS (mpPageBody, "gsave\n" );
    GraphicsStatus aNewState;
    if( maGraphicsStack.begin() != maGraphicsStack.end() )
        aNewState = maGraphicsStack.front();
    maGraphicsStack.push_front( aNewState );
}

void
PrinterGfx::PSGRestore ()
{
    WritePS (mpPageBody, "grestore\n" );
    if( maGraphicsStack.begin() == maGraphicsStack.end() )
        WritePS (mpPageBody, "Error: too many grestores\n" );
    else
        maGraphicsStack.pop_front();
}

void
PrinterGfx::PSSetLineWidth ()
{
    if( currentState().mfLineWidth != maVirtualStatus.mfLineWidth )
    {
        char pBuffer[128];
        sal_Int32 nChar = 0;

        currentState().mfLineWidth = maVirtualStatus.mfLineWidth;
        nChar  = psp::getValueOfDouble (pBuffer, maVirtualStatus.mfLineWidth, 5);
        nChar += psp::appendStr (" setlinewidth\n", pBuffer + nChar);
        WritePS (mpPageBody, pBuffer, nChar);
    }
}

void
PrinterGfx::PSSetColor ()
{
    PrinterColor& rColor( maVirtualStatus.maColor );

    if( currentState().maColor != rColor )
    {
        currentState().maColor = rColor;

        char pBuffer[128];
        sal_Int32 nChar = 0;

        if( mbColor )
        {
            nChar  = psp::getValueOfDouble (pBuffer,
                                            (double)rColor.GetRed() / 255.0, 5);
            nChar += psp::appendStr (" ", pBuffer + nChar);
            nChar += psp::getValueOfDouble (pBuffer + nChar,
                                            (double)rColor.GetGreen() / 255.0, 5);
            nChar += psp::appendStr (" ", pBuffer + nChar);
            nChar += psp::getValueOfDouble (pBuffer + nChar,
                                            (double)rColor.GetBlue() / 255.0, 5);
            nChar += psp::appendStr (" setrgbcolor\n", pBuffer + nChar );
        }
        else
        {
            Color aColor( rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() );
            sal_uInt8 nCol = aColor.GetLuminance();
            nChar  = psp::getValueOfDouble( pBuffer, (double)nCol / 255.0, 5 );
            nChar += psp::appendStr( " setgray\n", pBuffer + nChar );
        }

        WritePS (mpPageBody, pBuffer, nChar);
    }
}

void
PrinterGfx::PSSetFont ()
{
    GraphicsStatus& rCurrent( currentState() );
    if( maVirtualStatus.maFont          != rCurrent.maFont          ||
        maVirtualStatus.mnTextHeight    != rCurrent.mnTextHeight    ||
        maVirtualStatus.maEncoding      != rCurrent.maEncoding      ||
        maVirtualStatus.mnTextWidth     != rCurrent.mnTextWidth     ||
        maVirtualStatus.mbArtBold       != rCurrent.mbArtBold       ||
        maVirtualStatus.mbArtItalic     != rCurrent.mbArtItalic
        )
    {
        rCurrent.maFont              = maVirtualStatus.maFont;
        rCurrent.maEncoding          = maVirtualStatus.maEncoding;
        rCurrent.mnTextWidth         = maVirtualStatus.mnTextWidth;
        rCurrent.mnTextHeight        = maVirtualStatus.mnTextHeight;
        rCurrent.mbArtItalic         = maVirtualStatus.mbArtItalic;
        rCurrent.mbArtBold           = maVirtualStatus.mbArtBold;

        sal_Int32 nTextHeight = rCurrent.mnTextHeight;
        sal_Int32 nTextWidth  = rCurrent.mnTextWidth ? rCurrent.mnTextWidth
                                                     : rCurrent.mnTextHeight;

        sal_Char  pSetFont [256];
        sal_Int32 nChar = 0;

        // postscript based fonts need reencoding
        if (   (   rCurrent.maEncoding == RTL_TEXTENCODING_MS_1252)
            || (   rCurrent.maEncoding == RTL_TEXTENCODING_ISO_8859_1)
            || (   rCurrent.maEncoding >= RTL_TEXTENCODING_USER_START
                && rCurrent.maEncoding <= RTL_TEXTENCODING_USER_END)
           )
        {
            rtl::OString aReencodedFont =
                        psp::GlyphSet::GetReencodedFontName (rCurrent.maEncoding,
                                                                rCurrent.maFont);

            nChar += psp::appendStr  ("(",          pSetFont + nChar);
            nChar += psp::appendStr  (aReencodedFont.getStr(),
                                                    pSetFont + nChar);
            nChar += psp::appendStr  (") cvn findfont ",
                                                    pSetFont + nChar);
        }
        else
        // tt based fonts mustn't reencode, the encoding is implied by the fontname
        // same for symbol type1 fonts, dont try to touch them
        {
            nChar += psp::appendStr  ("(",          pSetFont + nChar);
            nChar += psp::appendStr  (rCurrent.maFont.getStr(),
                                                    pSetFont + nChar);
            nChar += psp::appendStr  (") cvn findfont ",
                                                    pSetFont + nChar);
        }

        if( ! rCurrent.mbArtItalic )
        {
            nChar += psp::getValueOf (nTextWidth,   pSetFont + nChar);
            nChar += psp::appendStr  (" ",          pSetFont + nChar);
            nChar += psp::getValueOf (-nTextHeight, pSetFont + nChar);
            nChar += psp::appendStr  (" matrix scale makefont setfont\n", pSetFont + nChar);
        }
        else // skew 15 degrees to right
        {
            nChar += psp::appendStr  ( " [",        pSetFont + nChar);
            nChar += psp::getValueOf (nTextWidth,   pSetFont + nChar);
            nChar += psp::appendStr  (" 0 ",        pSetFont + nChar);
            nChar += psp::getValueOfDouble (pSetFont + nChar, 0.27*(double)nTextWidth, 3 );
            nChar += psp::appendStr  ( " ",         pSetFont + nChar);
            nChar += psp::getValueOf (-nTextHeight, pSetFont + nChar);

            nChar += psp::appendStr  (" 0 0] makefont setfont\n", pSetFont + nChar);
        }

        WritePS (mpPageBody, pSetFont);
    }
}

void
PrinterGfx::PSRotate (sal_Int32 nAngle)
{
    sal_Int32 nPostScriptAngle = -nAngle;
    while( nPostScriptAngle < 0 )
        nPostScriptAngle += 3600;

    if (nPostScriptAngle == 0)
        return;

    sal_Int32 nFullAngle  = nPostScriptAngle / 10;
    sal_Int32 nTenthAngle = nPostScriptAngle % 10;

    sal_Char  pRotate [48];
    sal_Int32 nChar = 0;

    nChar  = psp::getValueOf (nFullAngle,  pRotate);
    nChar += psp::appendStr (".",          pRotate + nChar);
    nChar += psp::getValueOf (nTenthAngle, pRotate + nChar);
    nChar += psp::appendStr (" rotate\n",  pRotate + nChar);

    WritePS (mpPageBody, pRotate);
}

void
PrinterGfx::PSPointOp (const Point& rPoint, const sal_Char* pOperator)
{
    sal_Char  pPSCommand [48];
    sal_Int32 nChar = 0;

    nChar  = psp::getValueOf (rPoint.X(), pPSCommand);
    nChar += psp::appendStr  (" ",        pPSCommand + nChar);
    nChar += psp::getValueOf (rPoint.Y(), pPSCommand + nChar);
    nChar += psp::appendStr  (" ",        pPSCommand + nChar);
    nChar += psp::appendStr  (pOperator,  pPSCommand + nChar);
    nChar += psp::appendStr  ("\n",       pPSCommand + nChar);

    DBG_ASSERT (nChar < 48, "Buffer overflow in PSPointOp");

    WritePS (mpPageBody, pPSCommand);
}

void
PrinterGfx::PSTranslate (const Point& rPoint)
{
    PSPointOp (rPoint, "translate");
}

void
PrinterGfx::PSMoveTo (const Point& rPoint)
{
    PSPointOp (rPoint, "moveto");
}

void
PrinterGfx::PSLineTo (const Point& rPoint)
{
    PSPointOp (rPoint, "lineto");
}

void
PrinterGfx::PSRMoveTo (sal_Int32 nDx, sal_Int32 nDy)
{
    Point aPoint(nDx, nDy);
    PSPointOp (aPoint, "rmoveto");
}

/* get a compressed representation of the path information */

#define DEBUG_BINPATH 0

void
PrinterGfx::PSBinLineTo (const Point& rCurrent, Point& rOld, sal_Int32& nColumn)
{
#if (DEBUG_BINPATH == 1)
    PSLineTo (rCurrent);
#else
    PSBinPath (rCurrent, rOld, lineto, nColumn);
#endif
}

void
PrinterGfx::PSBinMoveTo (const Point& rCurrent, Point& rOld, sal_Int32& nColumn)
{
#if (DEBUG_BINPATH == 1)
    PSMoveTo (rCurrent);
#else
    PSBinPath (rCurrent, rOld, moveto, nColumn);
#endif
}

void
PrinterGfx::PSBinStartPath ()
{
#if (DEBUG_BINPATH == 1)
    WritePS (mpPageBody, "% PSBinStartPath\n");
#else
    WritePS (mpPageBody, "readpath\n" );
#endif
}

void
PrinterGfx::PSBinEndPath ()
{
#if (DEBUG_BINPATH == 1)
    WritePS (mpPageBody, "% PSBinEndPath\n");
#else
    WritePS (mpPageBody, "~\n");
#endif
}

void
PrinterGfx::PSBinCurrentPath (sal_uInt32 nPoints, const Point* pPath)
{
    // create the path
    Point     aPoint (0, 0);
    sal_Int32 nColumn = 0;

    PSBinStartPath ();
    PSBinMoveTo (*pPath, aPoint, nColumn);
    for (int i = 1; i < nPoints; i++)
        PSBinLineTo (pPath[i], aPoint, nColumn);
    PSBinEndPath ();
}

void
PrinterGfx::PSBinPath (const Point& rCurrent, Point& rOld,
                       pspath_t eType, sal_Int32& nColumn)
{
    sal_Char  pPath[48];
    sal_Int32 nChar;

    // create the hex representation of the dx and dy path shift, store the field
    // width as it is needed for the building the command
    sal_Int32 nXPrec = getAlignedHexValueOf (rCurrent.X() - rOld.X(), pPath + 1);
    sal_Int32 nYPrec = getAlignedHexValueOf (rCurrent.Y() - rOld.Y(), pPath + 1 + nXPrec);
    pPath [ 1 + nXPrec + nYPrec ] = 0;

    // build the command, it is a char with bit represention 000cxxyy
    // c represents the char, xx and yy repr. the field width of the dx and dy shift,
    // dx and dy represent the number of bytes to read after the opcode
    sal_Char cCmd = (eType == lineto ? (sal_Char)0x00 : (sal_Char)0x10);
    switch (nYPrec)
    {
        case 2: break;
        case 4: cCmd |= 0x01;   break;
        case 6: cCmd |= 0x02;   break;
        case 8: cCmd |= 0x03;   break;
        default:    DBG_ERROR ("invalid x precision in binary path");
    }
    switch (nXPrec)
    {
        case 2: break;
        case 4: cCmd |= 0x04;   break;
        case 6: cCmd |= 0x08;   break;
        case 8: cCmd |= 0x0c;   break;
        default:    DBG_ERROR ("invalid y precision in binary path");
    }
    cCmd += 'A';
    pPath[0] = cCmd;

    // write the command to file,
    // line breaking at column nMaxTextColumn (80)
    nChar = 1 + nXPrec + nYPrec;
    if ((nColumn + nChar) > nMaxTextColumn)
    {
        sal_Int32 nSegment = nMaxTextColumn - nColumn;

        WritePS (mpPageBody, pPath, nSegment);
        WritePS (mpPageBody, "\n", 1);
        WritePS (mpPageBody, pPath + nSegment, nChar - nSegment);

        nColumn  = nChar - nSegment;
    }
    else
    {
        WritePS (mpPageBody, pPath, nChar);

        nColumn += nChar;
    }

    rOld = rCurrent;
}

void
PrinterGfx::PSScale (double fScaleX, double fScaleY)
{
    sal_Char  pScale [48];
    sal_Int32 nChar = 0;

    nChar  = psp::getValueOfDouble (pScale, fScaleX, 5);
    nChar += psp::appendStr        (" ", pScale + nChar);
    nChar += psp::getValueOfDouble (pScale + nChar, fScaleY, 5);
    nChar += psp::appendStr        (" scale\n", pScale + nChar);

    WritePS (mpPageBody, pScale);
}

/* psshowtext helper routines: draw an hex string for show/xshow */
void
PrinterGfx::PSHexString (const sal_uChar* pString, sal_Int16 nLen)
{
    sal_Char pHexString [128];
    sal_Int32 nChar = 0;

    nChar = psp::appendStr ("<", pHexString);
    for (int i = 0; i < nLen; i++)
    {
        if (nChar >= (nMaxTextColumn - 1))
        {
            nChar += psp::appendStr ("\n", pHexString + nChar);
            WritePS (mpPageBody, pHexString, nChar);
            nChar = 0;
        }
        nChar += psp::getHexValueOf ((sal_Int32)pString[i], pHexString + nChar);
    }

    nChar += psp::appendStr (">\n", pHexString + nChar);
    WritePS (mpPageBody, pHexString, nChar);
}

/* psshowtext helper routines: draw an array for xshow ps operator */
void
PrinterGfx::PSDeltaArray (const sal_Int32 *pArray, sal_Int16 nEntries)
{
    sal_Char pPSArray [128];
    sal_Int32 nChar = 0;

    nChar  = psp::appendStr  ("[", pPSArray + nChar);
    nChar += psp::getValueOf (pArray[0], pPSArray + nChar);

    for (int i = 1; i < nEntries; i++)
    {
        if (nChar >= (nMaxTextColumn - 1))
        {
            nChar += psp::appendStr ("\n", pPSArray + nChar);
            WritePS (mpPageBody, pPSArray, nChar);
            nChar = 0;
        }

        nChar += psp::appendStr  (" ", pPSArray + nChar);
        nChar += psp::getValueOf (pArray[i] - pArray[i-1], pPSArray + nChar);
    }

    nChar  += psp::appendStr (" 0]\n", pPSArray + nChar);
    WritePS (mpPageBody, pPSArray);
}

/* the DrawText equivalent, pDeltaArray may be NULL. For Type1 fonts or single byte
 * fonts in general nBytes and nGlyphs is the same. For printer resident Composite
 * fonts it may be different (these fonts may be SJIS encoded for example) */
void
PrinterGfx::PSShowText (const sal_uChar* pStr, sal_Int16 nGlyphs, sal_Int16 nBytes,
                        const sal_Int32* pDeltaArray)
{
    PSSetColor (maTextColor);
    PSSetColor ();
    PSSetFont  ();
    // rotate the user coordinate system
    if (mnTextAngle != 0)
    {
        PSGSave ();
        PSRotate (mnTextAngle);
    }

    sal_Char pBuffer[256];
    if( maVirtualStatus.mbArtBold )
    {
        sal_Int32 nLW = maVirtualStatus.mnTextWidth;
        if( nLW == 0 )
            nLW = maVirtualStatus.mnTextHeight;
        else
            nLW = nLW < maVirtualStatus.mnTextHeight ? nLW : maVirtualStatus.mnTextHeight;
        psp::getValueOfDouble( pBuffer, (double)nLW / 30.0 );
    }
    // dispatch to the drawing method
    if (pDeltaArray == NULL)
    {
        PSHexString (pStr, nBytes);

        if( maVirtualStatus.mbArtBold )
        {
            WritePS( mpPageBody, pBuffer );
            WritePS( mpPageBody, " bshow\n" );
        }
        else
            WritePS (mpPageBody, "show\n");
    }
    else
    {
        PSHexString (pStr, nBytes);
        PSDeltaArray (pDeltaArray, nGlyphs - 1);
        if( maVirtualStatus.mbArtBold )
        {
            WritePS( mpPageBody, pBuffer );
            WritePS( mpPageBody, " bxshow\n" );
        }
        else
            WritePS (mpPageBody, "xshow\n");
    }

    // restore the user coordinate system
    if (mnTextAngle != 0)
        PSGRestore ();
}

void
PrinterGfx::PSComment( const sal_Char* pComment )
{
    const sal_Char* pLast = pComment;
    while( pComment && *pComment )
    {
        while( *pComment && *pComment != '\n' && *pComment != '\r' )
            pComment++;
        if( pComment - pLast > 1 )
        {
            WritePS( mpPageBody, "% ", 2 );
            WritePS( mpPageBody, pLast, pComment - pLast );
            WritePS( mpPageBody, "\n", 1 );
        }
        if( *pComment )
            pLast = ++pComment;
    }
}

sal_Bool
PrinterGfx::DrawEPS( const Rectangle& rBoundingBox, void* pPtr, sal_uInt32 nSize )
{
    if( nSize == 0 )
        return sal_True;
    if( ! mpPageBody )
        return sal_False;

    sal_Bool bSuccess = sal_False;

    // first search the BoundingBox of the EPS data
    SvMemoryStream aStream( pPtr, nSize, STREAM_READ );
    aStream.Seek( STREAM_SEEK_TO_BEGIN );
    ByteString aLine;

    ByteString aDocTitle;
    double fLeft = 0, fRight = 0, fTop = 0, fBottom = 0;
    bool bEndComments = false;
    while( ! aStream.IsEof()
           && ( ( fLeft == 0 && fRight == 0 && fTop == 0 && fBottom == 0 ) ||
                ( aDocTitle.Len() == 0 && bEndComments == false ) )
           )
    {
        aStream.ReadLine( aLine );
        if( aLine.Len() > 1 && aLine.GetChar( 0 ) == '%' )
        {
            char cChar = aLine.GetChar(1);
            if( cChar == '%' )
            {
                if( aLine.CompareIgnoreCaseToAscii( "%%BoundingBox:", 14 ) == COMPARE_EQUAL )
                {
                    aLine = WhitespaceToSpace( aLine.GetToken( 1, ':' ) );
                    if( aLine.Len() && aLine.Search( "atend" ) == STRING_NOTFOUND )
                    {
                        fLeft   = StringToDouble( GetCommandLineToken( 0, aLine ) );
                        fBottom = StringToDouble( GetCommandLineToken( 1, aLine ) );
                        fRight  = StringToDouble( GetCommandLineToken( 2, aLine ) );
                        fTop    = StringToDouble( GetCommandLineToken( 3, aLine ) );
                    }
                }
                else if( aLine.CompareIgnoreCaseToAscii( "%%Title:", 8 ) == COMPARE_EQUAL )
                    aDocTitle = WhitespaceToSpace( aLine.Copy( 8 ) );
                else if( aLine.CompareIgnoreCaseToAscii( "%%EndComments", 13 ) == COMPARE_EQUAL )
                    bEndComments = true;
            }
            else if( cChar == ' ' || cChar == '\t' || cChar == '\r' || cChar == '\n' )
                bEndComments = true;
        }
        else
            bEndComments = true;
    }

    static sal_uInt16 nEps = 0;
    if( ! aDocTitle.Len() )
        aDocTitle = ByteString::CreateFromInt32( (sal_Int32)(nEps++) );

    if( fLeft != fRight && fTop != fBottom )
    {
        double fScaleX = (double)rBoundingBox.GetWidth()/(fRight-fLeft);
        double fScaleY = -(double)rBoundingBox.GetHeight()/(fTop-fBottom);
        Point aTranslatePoint( rBoundingBox.Left()-fLeft*fScaleX,
                               rBoundingBox.Bottom()+1-fBottom*fScaleY );
        char buffer[128];

        // prepare EPS
        WritePS( mpPageBody,
                 "/b4_Inc_state save def\n"
                 "/dict_count countdictstack def\n"
                 "/op_count count 1 sub def\n"
                 "userdict begin\n"
                 "/showpage {} def\n"
                 "0 setgray 0 setlinecap 1 setlinewidth 0 setlinejoin\n"
                 "10 setmiterlimit [] 0 setdash newpath\n"
                 "/languagelevel where\n"
                 "{pop languagelevel\n"
                 "1 ne\n"
                 "  {false setstrokeadjust false setoverprint\n"
                 "  } if\n"
                 "}if\n" );
        // set up clip path and scale
        BeginSetClipRegion( 1 );
        UnionClipRegion( rBoundingBox.Left(), rBoundingBox.Top(), rBoundingBox.GetWidth(), rBoundingBox.GetHeight() );
        EndSetClipRegion();
        PSTranslate( aTranslatePoint );
        PSScale( fScaleX, fScaleY );

        // DSC requires BeginDocument
        WritePS( mpPageBody, "%%BeginDocument: " );
        WritePS( mpPageBody, aDocTitle );
        WritePS( mpPageBody, "\n" );

        // write the EPS data
        sal_uInt64 nOutLength;
        mpPageBody->write( pPtr, nSize, nOutLength );
        bSuccess = nOutLength == nSize;

        // corresponding EndDocument
        if( ((char*)pPtr)[ nSize-1 ] != '\n' )
            WritePS( mpPageBody, "\n" );
        WritePS( mpPageBody, "%%EndDocument\n" );

        // clean up EPS
        WritePS( mpPageBody,
                 "count op_count sub {pop} repeat\n"
                 "countdictstack dict_count sub {end} repeat\n"
                 "b4_Inc_state restore\n" );
    }
    return bSuccess;
}
