/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <vcl/bitmap.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/pageitem.hxx>
#include <svx/pagectrl.hxx>
#include <editeng/boxitem.hxx>
#include <algorithm>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

// STATIC DATA -----------------------------------------------------------

#define CELL_WIDTH      1600L
#define CELL_HEIGHT      800L

// class SvxPageWindow ---------------------------------------------------

SvxPageWindow::SvxPageWindow(Window* pParent,const ResId& rId)
: Window(pParent,rId),
    aWinSize(),
    aSize(),

    nTop(0),
    nBottom(0),
    nLeft(0),
    nRight(0),

    //UUUU
    pBorder(0),
    bResetBackground(sal_False),
    bFrameDirection(sal_False),
    nFrameDirection(0),

    nHdLeft(0),
    nHdRight(0),
    nHdDist(0),
    nHdHeight(0),

    pHdBorder(0),
    nFtLeft(0),
    nFtRight(0),
    nFtDist(0),
    nFtHeight(0),

    pFtBorder(0),

    maHeaderFillAttributes(),
    maFooterFillAttributes(),
    maPageFillAttributes(),

    bFooter(sal_False),
    bHeader(sal_False),
    bTable(sal_False),
    bHorz(sal_False),
    bVert(sal_False),
    eUsage(SVX_PAGE_ALL),

    aLeftText(),
    aRightText()

{
    // defaultmaessing in Twips rechnen
    SetMapMode(MapMode(MAP_TWIP));
    aWinSize = GetOutputSizePixel();
    aWinSize.Height() -= 4;
    aWinSize.Width() -= 4;

    aWinSize = PixelToLogic(aWinSize);
    SetBackground();
}

// -----------------------------------------------------------------------

SvxPageWindow::~SvxPageWindow()
{
    delete pHdBorder;
    delete pFtBorder;
}

// -----------------------------------------------------------------------

void __EXPORT SvxPageWindow::Paint(const Rectangle&)
{
    Fraction aXScale(aWinSize.Width(),std::max((long)(aSize.Width() * 2 + aSize.Width() / 8),1L));
    Fraction aYScale(aWinSize.Height(),std::max(aSize.Height(),1L));
    MapMode aMapMode(GetMapMode());

    if(aYScale < aXScale)
    {
        aMapMode.SetScaleX(aYScale);
        aMapMode.SetScaleY(aYScale);
    }
    else
    {
        aMapMode.SetScaleX(aXScale);
        aMapMode.SetScaleY(aXScale);
    }
    SetMapMode(aMapMode);
    Size aSz(PixelToLogic(GetSizePixel()));
    long nYPos = (aSz.Height() - aSize.Height()) / 2;

    if(eUsage == SVX_PAGE_ALL)
    {
        // alle Seiten gleich -> eine Seite malen
        if(aSize.Width() > aSize.Height())
        {
            // Querformat in gleicher Gr"osse zeichnen
            Fraction aX = aMapMode.GetScaleX();
            Fraction aY = aMapMode.GetScaleY();
            Fraction a2(1.5);
            aX *= a2;
            aY *= a2;
            aMapMode.SetScaleX(aX);
            aMapMode.SetScaleY(aY);
            SetMapMode(aMapMode);
            aSz = PixelToLogic(GetSizePixel());
            nYPos = (aSz.Height() - aSize.Height()) / 2;
            long nXPos = (aSz.Width() - aSize.Width()) / 2;
            DrawPage(Point(nXPos,nYPos),sal_True,sal_True);
        }
        else
            // Hochformat
            DrawPage(Point((aSz.Width() - aSize.Width()) / 2,nYPos),sal_True,sal_True);
    }
    else
    {
        // Linke und rechte Seite unterschiedlich -> ggf. zwei Seiten malen
        DrawPage(Point(0,nYPos),sal_False,(sal_Bool)(eUsage & SVX_PAGE_LEFT));
        DrawPage(Point(aSize.Width() + aSize.Width() / 8,nYPos),sal_True,
            (sal_Bool)(eUsage & SVX_PAGE_RIGHT));
    }
}

// -----------------------------------------------------------------------
void SvxPageWindow::DrawPage(const Point& rOrg,const sal_Bool bSecond,const sal_Bool bEnabled)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color& rFieldColor = rStyleSettings.GetFieldColor();
    const Color& rFieldTextColor = rStyleSettings.GetFieldTextColor();
    const Color& rDisableColor = rStyleSettings.GetDisableColor();
    const Color& rDlgColor = rStyleSettings.GetDialogColor();

    // background
    if(!bSecond || bResetBackground)
    {
        SetLineColor(Color(COL_TRANSPARENT));
        SetFillColor(rDlgColor);
        Size winSize(GetOutputSize());
        DrawRect(Rectangle(Point(0,0),winSize));

        if(bResetBackground)
            bResetBackground = sal_False;
    }
    SetLineColor(rFieldTextColor);
    // Schatten
    Size aTempSize = aSize;
    // Seite
    if(!bEnabled)
    {
        SetFillColor(rDisableColor);
        DrawRect(Rectangle(rOrg,aTempSize));
        return;
    }
    SetFillColor(rFieldColor);
    DrawRect(Rectangle(rOrg,aTempSize));

    // Border Top Bottom Left Right
    Point aBegin(rOrg);
    Point aEnd(rOrg);

    long nL = nLeft;
    long nR = nRight;

    if(eUsage == SVX_PAGE_MIRROR && !bSecond)
    {
        // f"ur gespiegelt drehen
        nL = nRight;
        nR = nLeft;
    }

    Rectangle aRect;

    aRect.Left() = rOrg.X() + nL;
    aRect.Right() = rOrg.X() + aTempSize.Width() - nR;
    aRect.Top() = rOrg.Y() + nTop;
    aRect.Bottom() = rOrg.Y() + aTempSize.Height() - nBottom;

    Rectangle aHdRect(aRect);
    Rectangle aFtRect(aRect);

    if(bHeader || bFooter)
    {
        //UUUU Header and/or footer used
        const Color aLineColor(GetLineColor());

        //UUUU draw PageFill first and on the whole page, no outline
        SetLineColor();
        drawFillAttributes(maPageFillAttributes, aRect, aRect);
        SetLineColor(aLineColor);

        if(bHeader)
        {
            // ggf. Header anzeigen
            aHdRect.Left() += nHdLeft;
            aHdRect.Right() -= nHdRight;
            aHdRect.Bottom() = aRect.Top() + nHdHeight;
            aRect.Top() += nHdHeight + nHdDist;

            // draw header over PageFill, plus outline
            drawFillAttributes(maHeaderFillAttributes, aHdRect, aHdRect);
        }

        if(bFooter)
        {
            // ggf. Footer anzeigen
            aFtRect.Left() += nFtLeft;
            aFtRect.Right() -= nFtRight;
            aFtRect.Top() = aRect.Bottom() - nFtHeight;
            aRect.Bottom() -= nFtHeight + nFtDist;

            // draw footer over PageFill, plus outline
            drawFillAttributes(maFooterFillAttributes, aFtRect, aFtRect);
        }

        // draw page's reduced outline, only outline
        drawFillAttributes(drawinglayer::attribute::SdrAllFillAttributesHelperPtr(), aRect, aRect);
    }
    else
    {
        //UUUU draw PageFill and outline
        drawFillAttributes(maPageFillAttributes, aRect, aRect);
    }

    if(bFrameDirection && !bTable)
    {
        //pImpl->nFrameDirection
        Point aPos;
        Font aFont(GetFont());
        const Size aSaveSize = aFont.GetSize();
        Size aDrawSize(0,aRect.GetHeight() / 6);
        aFont.SetSize(aDrawSize);
        SetFont(aFont);
        String sText(String::CreateFromAscii("ABC"));
        Point aMove(1,GetTextHeight());
        sal_Unicode cArrow = 0x2193;
        long nAWidth = GetTextWidth(String(sText.GetChar(0)));
        switch(nFrameDirection)
        {
        case FRMDIR_HORI_LEFT_TOP:
            aPos = aRect.TopLeft();
            aPos.X() += PixelToLogic(Point(1,1)).X();
            aMove.Y() = 0;
            cArrow = 0x2192;
            break;
        case FRMDIR_HORI_RIGHT_TOP:
            aPos = aRect.TopRight();
            aPos.X() -= nAWidth;
            aMove.Y() = 0;
            aMove.X() *= -1;
            cArrow = 0x2190;
            break;
        case FRMDIR_VERT_TOP_LEFT:
            aPos = aRect.TopLeft();
            aPos.X() += PixelToLogic(Point(1,1)).X();
            aMove.X() = 0;
            break;
        case FRMDIR_VERT_TOP_RIGHT:
            aPos = aRect.TopRight();
            aPos.X() -= nAWidth;
            aMove.X() = 0;
            break;
        }
        sText.Append(cArrow);
        for(sal_uInt16 i = 0; i < sText.Len(); i++)
        {
            String sDraw(sText.GetChar(i));
            long nHDiff = 0;
            long nCharWidth = GetTextWidth(sDraw);
            sal_Bool bHorizontal = 0 == aMove.Y();
            if(!bHorizontal)
            {
                nHDiff = (nAWidth - nCharWidth) / 2;
                aPos.X() += nHDiff;
            }
            DrawText(aPos,sDraw);
            if(bHorizontal)
            {
                aPos.X() += aMove.X() < 0 ? -nCharWidth : nCharWidth;
            }
            else
            {
                aPos.X() -= nHDiff;
                aPos.Y() += aMove.Y();
            }
        }
        aFont.SetSize(aSaveSize);
        SetFont(aFont);

    }
    if(bTable)
    {
        // Tabelle malen, ggf. zentrieren
        SetLineColor(Color(COL_LIGHTGRAY));

        long nW = aRect.GetWidth(),nH = aRect.GetHeight();
        long nTW = CELL_WIDTH * 3,nTH = CELL_HEIGHT * 3;
        long _nLeft = bHorz ? aRect.Left() + ((nW - nTW) / 2) : aRect.Left();
        long _nTop = bVert ? aRect.Top() + ((nH - nTH) / 2) : aRect.Top();
        Rectangle aCellRect(Point(_nLeft,_nTop),Size(CELL_WIDTH,CELL_HEIGHT));

        for(sal_uInt16 i = 0; i < 3; ++i)
        {
            aCellRect.Left() = _nLeft;
            aCellRect.Right() = _nLeft + CELL_WIDTH;
            if(i > 0)
                aCellRect.Move(0,CELL_HEIGHT);

            for(sal_uInt16 j = 0; j < 3; ++j)
            {
                if(j > 0)
                    aCellRect.Move(CELL_WIDTH,0);
                DrawRect(aCellRect);
            }
        }
    }
}

//UUUU
void SvxPageWindow::drawFillAttributes(
    const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
    const Rectangle& rPaintRange,
    const Rectangle& rDefineRange)
{
    const basegfx::B2DRange aPaintRange(
        rPaintRange.Left(),
        rPaintRange.Top(),
        rPaintRange.Right(),
        rPaintRange.Bottom());

    if(!aPaintRange.isEmpty() &&
        !basegfx::fTools::equalZero(aPaintRange.getWidth()) &&
        !basegfx::fTools::equalZero(aPaintRange.getHeight()))
    {
        const basegfx::B2DRange aDefineRange(
            rDefineRange.Left(),
            rDefineRange.Top(),
            rDefineRange.Right(),
            rDefineRange.Bottom());

        // prepare primitive sequence
        drawinglayer::primitive2d::Primitive2DSequence aSequence;

        // create fill geometry if there is something to fill
        if(rFillAttributes.get() && rFillAttributes->isUsed())
        {
            aSequence = rFillAttributes->getPrimitive2DSequence(
                aPaintRange,
                aDefineRange);
        }

        // create line geometry if a LineColor is set at the target device
        if(IsLineColor())
        {
            const drawinglayer::primitive2d::Primitive2DReference xOutline(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    basegfx::tools::createPolygonFromRect(aPaintRange),
                    GetLineColor().getBColor()));

            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(
                aSequence,
                xOutline);
        }

        // draw that if we have something to draw
        if(aSequence.getLength())
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                basegfx::B2DHomMatrix(),
                GetViewTransformation(),
                aPaintRange,
                0,
                0.0,
                com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >());
            drawinglayer::processor2d::BaseProcessor2D* pProcessor = drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                *this,
                aViewInformation2D);

            if(pProcessor)
            {
                pProcessor->process(aSequence);

                delete pProcessor;
            }
        }
    }
}

// -----------------------------------------------------------------------

void SvxPageWindow::SetBorder(const SvxBoxItem& rNew)
{
    delete pBorder;
    pBorder = new SvxBoxItem(rNew);
}

// -----------------------------------------------------------------------

void SvxPageWindow::SetHdBorder(const SvxBoxItem& rNew)
{
    delete pHdBorder;
    pHdBorder = new SvxBoxItem(rNew);
}
// -----------------------------------------------------------------------

void SvxPageWindow::SetFtBorder(const SvxBoxItem& rNew)
{
    delete pFtBorder;
    pFtBorder = new SvxBoxItem(rNew);
}

void SvxPageWindow::EnableFrameDirection(sal_Bool bEnable)
{
    bFrameDirection = bEnable;
}

void SvxPageWindow::SetFrameDirection(sal_Int32 nDirection)
{
    nFrameDirection = nDirection;
}

void SvxPageWindow::ResetBackground()
{
    bResetBackground = sal_True;
}

//eof
