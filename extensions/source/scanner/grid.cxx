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

#include <sal/config.h>
#include <osl/thread.h>
#include <cstdio>
#include <math.h>
#include <boost/math/special_functions/expm1.hpp>

#include <cmath>

#include <grid.hxx>
#include <vcl/builderfactory.hxx>

#include <algorithm>
#include <memory>

class GridWindow : public vcl::Window
{
    // helper class for handles
    struct impHandle
    {
        Point           maPos;
        sal_uInt16      mnOffX;
        sal_uInt16      mnOffY;

        impHandle(const Point& rPos, sal_uInt16 nX, sal_uInt16 nY)
        :   maPos(rPos), mnOffX(nX), mnOffY(nY)
        {
        }

        bool operator<(const impHandle& rComp) const
        {
            return (maPos.X() < rComp.maPos.X());
        }

        void draw(vcl::RenderContext& rRenderContext, const BitmapEx& rBitmapEx)
        {
            const Point aOffset(rRenderContext.PixelToLogic(Point(mnOffX, mnOffY)));
            rRenderContext.DrawBitmapEx(maPos - aOffset, rBitmapEx);
        }

        bool isHit(vcl::Window& rWin, const Point& rPos)
        {
            const Point aOffset(rWin.PixelToLogic(Point(mnOffX, mnOffY)));
            const Rectangle aTarget(maPos - aOffset, maPos + aOffset);
            return aTarget.IsInside(rPos);
        }
    };

    Rectangle       m_aGridArea;

    double          m_fMinX;
    double          m_fMinY;
    double          m_fMaxX;
    double          m_fMaxY;

    double          m_fChunkX;
    double          m_fMinChunkX;
    double          m_fChunkY;
    double          m_fMinChunkY;

    double*         m_pXValues;
    double*         m_pOrigYValues;
    int             m_nValues;
    double*         m_pNewYValues;

    sal_uInt16      m_BmOffX;
    sal_uInt16      m_BmOffY;

    bool            m_bCutValues;

    // stuff for handles
    std::vector< impHandle >    m_aHandles;
    sal_uInt32                  m_nDragIndex;

    BitmapEx        m_aMarkerBitmap;

    Point transform( double x, double y );
    void transform( const Point& rOriginal, double& x, double& y );

    double findMinX();
    double findMinY();
    double findMaxX();
    double findMaxY();

    void drawGrid(vcl::RenderContext& rRenderContext);
    void drawOriginal(vcl::RenderContext& rRenderContext);
    void drawNew(vcl::RenderContext& rRenderContext);
    void drawHandles(vcl::RenderContext& rRenderContext);

    void computeExtremes();
    static void computeChunk( double fMin, double fMax, double& fChunkOut, double& fMinChunkOut );
    void computeNew();
    static double interpolate( double x, double* pNodeX, double* pNodeY, int nNodes );

    virtual void MouseMove( const MouseEvent& ) override;
    virtual void MouseButtonDown( const MouseEvent& ) override;
    virtual void MouseButtonUp( const MouseEvent& ) override;
    void onResize();
    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;
    void drawLine(vcl::RenderContext& rRenderContext, double x1, double y1, double x2, double y2);
public:
    GridWindow(vcl::Window* pParent);
    void Init(double* pXValues, double* pYValues, int nValues, bool bCutValues, const BitmapEx &rMarkerBitmap);
    virtual ~GridWindow();
    virtual void dispose() override;

    void setBoundings( double fMinX, double fMinY, double fMaxX, double fMaxY );

    double* getNewYValues() { return m_pNewYValues; }

    void ChangeMode(int nType);

    virtual void Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect ) override;
};

GridWindow::GridWindow(vcl::Window* pParent)
    : Window(pParent, 0)
    , m_aGridArea(50, 15, 100, 100)
    , m_fMinX(0.0)
    , m_fMinY(0.0)
    , m_fMaxX(0.0)
    , m_fMaxY(0.0)
    , m_fChunkX(0.0)
    , m_fMinChunkX(0.0)
    , m_fChunkY(0.0)
    , m_fMinChunkY(0.0)
    , m_pXValues(NULL)
    , m_pOrigYValues(NULL)
    , m_nValues(0)
    , m_pNewYValues(NULL)
    , m_BmOffX(0)
    , m_BmOffY(0)
    , m_bCutValues(false)
    , m_aHandles()
    , m_nDragIndex(0xffffffff)
{
    SetMapMode(MapMode(MAP_PIXEL));
}

void GridWindow::Init(double* pXValues, double* pYValues, int nValues, bool bCutValues, const BitmapEx &rMarkerBitmap)
{
    m_aMarkerBitmap = rMarkerBitmap;
    m_pXValues = pXValues;
    m_pOrigYValues = pYValues;
    m_nValues = nValues;
    m_bCutValues = bCutValues;

    SetSizePixel(GetOptimalSize());
    onResize();

    if (m_pOrigYValues && m_nValues)
    {
        m_pNewYValues = new double[ m_nValues ];
        memcpy( m_pNewYValues, m_pOrigYValues, sizeof( double ) * m_nValues );
    }

    setBoundings( 0, 0, 1023, 1023 );
    computeExtremes();

    // create left and right marker as first and last entry
    m_BmOffX = sal_uInt16(m_aMarkerBitmap.GetSizePixel().Width() >> 1);
    m_BmOffY = sal_uInt16(m_aMarkerBitmap.GetSizePixel().Height() >> 1);
    m_aHandles.push_back(impHandle(transform(findMinX(), findMinY()), m_BmOffX, m_BmOffY));
    m_aHandles.push_back(impHandle(transform(findMaxX(), findMaxY()), m_BmOffX, m_BmOffY));
}

void GridWindow::Resize()
{
    onResize();
}

void GridWindow::onResize()
{
    Size aSize = GetSizePixel();
    m_aGridArea.setWidth( aSize.Width() - 80 );
    m_aGridArea.setHeight( aSize.Height() - 40 );
}

Size GridWindow::GetOptimalSize() const
{
    return LogicToPixel(Size(240, 200), MAP_APPFONT);
}

GridDialog::GridDialog(double* pXValues, double* pYValues, int nValues, vcl::Window* pParent, bool bCutValues )
    : ModalDialog(pParent, "GridDialog", "modules/scanner/ui/griddialog.ui")
{
    get(m_pOKButton, "ok");
    get(m_pResetTypeBox, "resetTypeCombobox");
    get(m_pResetButton, "resetButton");
    get(m_pGridWindow, "gridwindow");
    m_pGridWindow->Init(pXValues, pYValues, nValues, bCutValues, get<FixedImage>("handle")->GetImage().GetBitmapEx());

    m_pResetTypeBox->SelectEntryPos( 0 );

    m_pResetButton->SetClickHdl( LINK( this, GridDialog, ClickButtonHdl ) );
}

GridDialog::~GridDialog()
{
    disposeOnce();
}

void GridDialog::dispose()
{
    m_pOKButton.clear();
    m_pResetTypeBox.clear();
    m_pResetButton.clear();
    m_pGridWindow.clear();
    ModalDialog::dispose();
}

GridWindow::~GridWindow()
{
    disposeOnce();
}

void GridWindow::dispose()
{
    delete [] m_pNewYValues;
    vcl::Window::dispose();
}

double GridWindow::findMinX()
{
    if( ! m_pXValues )
        return 0.0;
    double fMin = m_pXValues[0];
    for( int i = 1; i < m_nValues; i++ )
        if( m_pXValues[ i ] < fMin )
            fMin = m_pXValues[ i ];
    return fMin;
}

double GridWindow::findMinY()
{
    if( ! m_pNewYValues )
        return 0.0;
    double fMin = m_pNewYValues[0];
    for( int i = 1; i < m_nValues; i++ )
        if( m_pNewYValues[ i ] < fMin )
            fMin = m_pNewYValues[ i ];
    return fMin;
}



double GridWindow::findMaxX()
{
    if( ! m_pXValues )
        return 0.0;
    double fMax = m_pXValues[0];
    for( int i = 1; i < m_nValues; i++ )
        if( m_pXValues[ i ] > fMax )
            fMax = m_pXValues[ i ];
    return fMax;
}



double GridWindow::findMaxY()
{
    if( ! m_pNewYValues )
        return 0.0;
    double fMax = m_pNewYValues[0];
    for( int i = 1; i < m_nValues; i++ )
        if( m_pNewYValues[ i ] > fMax )
            fMax = m_pNewYValues[ i ];
    return fMax;
}



void GridWindow::computeExtremes()
{
    if( m_nValues && m_pXValues && m_pOrigYValues )
    {
        m_fMaxX = m_fMinX = m_pXValues[0];
        m_fMaxY = m_fMinY = m_pOrigYValues[0];
        for( int i = 1; i < m_nValues; i++ )
        {
            if( m_pXValues[ i ] > m_fMaxX )
                m_fMaxX = m_pXValues[ i ];
            else if( m_pXValues[ i ] < m_fMinX )
                m_fMinX = m_pXValues[ i ];
            if( m_pOrigYValues[ i ] > m_fMaxY )
                m_fMaxY = m_pOrigYValues[ i ];
            else if( m_pOrigYValues[ i ] < m_fMinY )
                m_fMinY = m_pOrigYValues[ i ];
        }
        setBoundings( m_fMinX, m_fMinY, m_fMaxX, m_fMaxY );
    }
}



Point GridWindow::transform( double x, double y )
{
    Point aRet;

    aRet.X() = (long)( ( x - m_fMinX ) *
        (double)m_aGridArea.GetWidth() / ( m_fMaxX - m_fMinX )
        + m_aGridArea.Left() );
    aRet.Y() = (long)(
        m_aGridArea.Bottom() -
        ( y - m_fMinY ) *
        (double)m_aGridArea.GetHeight() / ( m_fMaxY - m_fMinY ) );
    return aRet;
}

void GridWindow::transform( const Point& rOriginal, double& x, double& y )
{
    const long nWidth = m_aGridArea.GetWidth();
    const long nHeight = m_aGridArea.GetHeight();
    if (!nWidth || !nHeight)
        return;
    x = ( rOriginal.X() - m_aGridArea.Left() ) * (m_fMaxX - m_fMinX) / (double)nWidth + m_fMinX;
    y = ( m_aGridArea.Bottom() - rOriginal.Y() ) * (m_fMaxY - m_fMinY) / (double)nHeight + m_fMinY;
}

void GridWindow::drawLine(vcl::RenderContext& rRenderContext, double x1, double y1, double x2, double y2 )
{
    rRenderContext.DrawLine(transform(x1, y1), transform(x2, y2));
}

void GridWindow::computeChunk( double fMin, double fMax, double& fChunkOut, double& fMinChunkOut )
{
    // get a nice chunk size like 10, 100, 25 or such
    fChunkOut = ( fMax - fMin ) / 6.0;
    int logchunk = (int)std::log10( fChunkOut );
    int nChunk = (int)( fChunkOut / std::exp( (double)(logchunk-1) * M_LN10 ) );
    if( nChunk >= 75 )
        nChunk = 100;
    else if( nChunk >= 35 )
        nChunk = 50;
    else if ( nChunk > 20 )
        nChunk = 25;
    else if ( nChunk >= 13 )
        nChunk = 20;
    else if( nChunk > 5 )
        nChunk = 10;
    else
        nChunk = 5;
    fChunkOut = (double) nChunk * exp( (double)(logchunk-1) * M_LN10 );
    // compute whole chunks fitting into fMin
    nChunk = (int)( fMin / fChunkOut );
    fMinChunkOut = (double)nChunk * fChunkOut;
    while( fMinChunkOut < fMin )
        fMinChunkOut += fChunkOut;
}



void GridWindow::computeNew()
{
    if(2L == m_aHandles.size())
    {
        // special case: only left and right markers
        double xleft, yleft;
        double xright, yright;
        transform(m_aHandles[0L].maPos, xleft, yleft);
        transform(m_aHandles[1L].maPos, xright, yright );
        double factor = (yright-yleft)/(xright-xleft);
        for( int i = 0; i < m_nValues; i++ )
        {
            m_pNewYValues[ i ] = yleft + ( m_pXValues[ i ] - xleft )*factor;
        }
    }
    else
    {
        // sort markers
        std::sort(m_aHandles.begin(), m_aHandles.end());
        const int nSorted = m_aHandles.size();
        int i;

        // get node arrays
        std::unique_ptr<double[]> nodex(new double[ nSorted ]);
        std::unique_ptr<double[]> nodey(new double[ nSorted ]);

        for( i = 0L; i < nSorted; i++ )
            transform( m_aHandles[i].maPos, nodex[ i ], nodey[ i ] );

        for( i = 0; i < m_nValues; i++ )
        {
            double x = m_pXValues[ i ];
            m_pNewYValues[ i ] = interpolate( x, nodex.get(), nodey.get(), nSorted );
            if( m_bCutValues )
            {
                if( m_pNewYValues[ i ] > m_fMaxY )
                    m_pNewYValues[ i ] = m_fMaxY;
                else if( m_pNewYValues[ i ] < m_fMinY )
                    m_pNewYValues[ i ] = m_fMinY;
            }
        }
    }
}



double GridWindow::interpolate(
    double x,
    double* pNodeX,
    double* pNodeY,
    int nNodes )
{
    // compute Lagrange interpolation
    double ret = 0;
    for( int i = 0; i < nNodes; i++ )
    {
        double sum = pNodeY[ i ];
        for( int n = 0; n < nNodes; n++ )
        {
            if( n != i )
            {
                sum *= x - pNodeX[ n ];
                sum /= pNodeX[ i ] - pNodeX[ n ];
            }
        }
        ret += sum;
    }
    return ret;
}

void GridDialog::setBoundings(double fMinX, double fMinY, double fMaxX, double fMaxY)
{
    m_pGridWindow->setBoundings(fMinX, fMinY, fMaxX, fMaxY);
}

void GridWindow::setBoundings(double fMinX, double fMinY, double fMaxX, double fMaxY)
{
    m_fMinX = fMinX;
    m_fMinY = fMinY;
    m_fMaxX = fMaxX;
    m_fMaxY = fMaxY;

    computeChunk( m_fMinX, m_fMaxX, m_fChunkX, m_fMinChunkX );
    computeChunk( m_fMinY, m_fMaxY, m_fChunkY, m_fMinChunkY );
}

void GridWindow::drawGrid(vcl::RenderContext& rRenderContext)
{
    char pBuf[256];
    rRenderContext.SetLineColor(Color(COL_BLACK));
    // draw vertical lines
    for (double fX = m_fMinChunkX; fX < m_fMaxX; fX += m_fChunkX)
    {
        drawLine(rRenderContext, fX, m_fMinY, fX, m_fMaxY);
        // draw tickmarks
        Point aPt = transform(fX, m_fMinY);
        std::sprintf(pBuf, "%g", fX);
        OUString aMark(pBuf, strlen(pBuf), osl_getThreadTextEncoding());
        Size aTextSize(rRenderContext.GetTextWidth(aMark), rRenderContext.GetTextHeight());
        aPt.X() -= aTextSize.Width() / 2;
        aPt.Y() += aTextSize.Height() / 2;
        rRenderContext.DrawText(aPt, aMark);
    }
    // draw horizontal lines
    for (double fY = m_fMinChunkY; fY < m_fMaxY; fY += m_fChunkY)
    {
        drawLine(rRenderContext, m_fMinX, fY, m_fMaxX, fY);
        // draw tickmarks
        Point aPt = transform(m_fMinX, fY);
        std::sprintf(pBuf, "%g", fY);
        OUString aMark(pBuf, strlen(pBuf), osl_getThreadTextEncoding());
        Size aTextSize(rRenderContext.GetTextWidth(aMark), rRenderContext.GetTextHeight());
        aPt.X() -= aTextSize.Width() + 2;
        aPt.Y() -= aTextSize.Height() / 2;
        rRenderContext.DrawText(aPt, aMark);
    }

    // draw boundings
    drawLine(rRenderContext, m_fMinX, m_fMinY, m_fMaxX, m_fMinY);
    drawLine(rRenderContext, m_fMinX, m_fMaxY, m_fMaxX, m_fMaxY);
    drawLine(rRenderContext, m_fMinX, m_fMinY, m_fMinX, m_fMaxY);
    drawLine(rRenderContext, m_fMaxX, m_fMinY, m_fMaxX, m_fMaxY);
}

void GridWindow::drawOriginal(vcl::RenderContext& rRenderContext)
{
    if (m_nValues && m_pXValues && m_pOrigYValues)
    {
        rRenderContext.SetLineColor(Color(COL_RED));
        for (int i = 0; i < m_nValues - 1; i++)
        {
            drawLine(rRenderContext,
                      m_pXValues[i],     m_pOrigYValues[i],
                      m_pXValues[i + 1], m_pOrigYValues[i + 1]);
        }
    }
}

void GridWindow::drawNew(vcl::RenderContext& rRenderContext)
{
    if (m_nValues && m_pXValues && m_pNewYValues)
    {
        rRenderContext.SetClipRegion(vcl::Region(m_aGridArea));
        rRenderContext.SetLineColor(Color(COL_YELLOW));
        for (int i = 0; i < m_nValues - 1; i++)
        {
            drawLine(rRenderContext,
                     m_pXValues[i],     m_pNewYValues[i],
                     m_pXValues[i + 1], m_pNewYValues[i + 1]);
        }
        rRenderContext.SetClipRegion();
    }
}

void GridWindow::drawHandles(vcl::RenderContext& rRenderContext)
{
    for(size_t i(0L); i < m_aHandles.size(); i++)
    {
        m_aHandles[i].draw(rRenderContext, m_aMarkerBitmap);
    }
}

void GridWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    Window::Paint(rRenderContext, rRect);
    drawGrid(rRenderContext);
    drawOriginal(rRenderContext);
    drawNew(rRenderContext);
    drawHandles(rRenderContext);
}

void GridWindow::MouseMove( const MouseEvent& rEvt )
{
    if( rEvt.GetButtons() == MOUSE_LEFT && m_nDragIndex != 0xffffffff )
    {
        Point aPoint( rEvt.GetPosPixel() );

        if( m_nDragIndex == 0L || m_nDragIndex == m_aHandles.size() - 1L)
        {
            aPoint.X() = m_aHandles[m_nDragIndex].maPos.X();
        }
        else
        {
            if(aPoint.X() < m_aGridArea.Left())
                aPoint.X() = m_aGridArea.Left();
            else if(aPoint.X() > m_aGridArea.Right())
                aPoint.X() = m_aGridArea.Right();
        }

        if( aPoint.Y() < m_aGridArea.Top() )
            aPoint.Y() = m_aGridArea.Top();
        else if( aPoint.Y() > m_aGridArea.Bottom() )
            aPoint.Y() = m_aGridArea.Bottom();

        if( aPoint != m_aHandles[m_nDragIndex].maPos )
        {
            m_aHandles[m_nDragIndex].maPos = aPoint;
            Invalidate( m_aGridArea );
        }
    }

    Window::MouseMove( rEvt );
}

void GridWindow::MouseButtonUp( const MouseEvent& rEvt )
{
    if( rEvt.GetButtons() == MOUSE_LEFT )
    {
        if( m_nDragIndex != 0xffffffff )
        {
            m_nDragIndex = 0xffffffff;
            computeNew();
            Invalidate(m_aGridArea);
        }
    }

    Window::MouseButtonUp( rEvt );
}

void GridWindow::MouseButtonDown( const MouseEvent& rEvt )
{
    Point aPoint( rEvt.GetPosPixel() );
    sal_uInt32 nMarkerIndex = 0xffffffff;

    for(sal_uInt32 a(0L); nMarkerIndex == 0xffffffff && a < m_aHandles.size(); a++)
    {
        if(m_aHandles[a].isHit(*this, aPoint))
        {
            nMarkerIndex = a;
        }
    }

    if( rEvt.GetButtons() == MOUSE_LEFT )
    {
        // user wants to drag a button
        if( nMarkerIndex != 0xffffffff )
        {
            m_nDragIndex = nMarkerIndex;
        }
    }
    else if( rEvt.GetButtons() == MOUSE_RIGHT )
    {
        // user wants to add/delete a button
        if( nMarkerIndex != 0xffffffff )
        {
            if( nMarkerIndex != 0L && nMarkerIndex != m_aHandles.size() - 1L)
            {
                // delete marker under mouse
                if( m_nDragIndex == nMarkerIndex )
                    m_nDragIndex = 0xffffffff;

                m_aHandles.erase(m_aHandles.begin() + nMarkerIndex);
            }
        }
        else
        {
            m_BmOffX = sal_uInt16(m_aMarkerBitmap.GetSizePixel().Width() >> 1);
            m_BmOffY = sal_uInt16(m_aMarkerBitmap.GetSizePixel().Height() >> 1);
            m_aHandles.push_back(impHandle(aPoint, m_BmOffX, m_BmOffY));
        }

        computeNew();
        Invalidate(m_aGridArea);
    }

    Window::MouseButtonDown( rEvt );
}

void GridWindow::ChangeMode(int nType)
{
    switch( nType )
    {
        case LINEAR_ASCENDING:
        {
            for( int i = 0; i < m_nValues; i++ )
            {
                m_pNewYValues[ i ] = m_fMinY + (m_fMaxY-m_fMinY)/(m_fMaxX-m_fMinX)*(m_pXValues[i]-m_fMinX);
            }
        }
        break;
        case LINEAR_DESCENDING:
        {
            for( int i = 0; i < m_nValues; i++ )
            {
                m_pNewYValues[ i ] = m_fMaxY - (m_fMaxY-m_fMinY)/(m_fMaxX-m_fMinX)*(m_pXValues[i]-m_fMinX);
            }
        }
        break;
        case RESET:
        {
            if( m_pOrigYValues && m_pNewYValues && m_nValues )
                memcpy( m_pNewYValues, m_pOrigYValues, m_nValues*sizeof(double) );
        }
        break;
        case EXPONENTIAL:
        {
            for( int i = 0; i < m_nValues; i++ )
            {
                m_pNewYValues[ i ] = m_fMinY + (m_fMaxY-m_fMinY)*(boost::math::expm1((m_pXValues[i]-m_fMinX)/(m_fMaxX-m_fMinX)))/(M_E-1.0);
            }
        }
        break;

        default:
            break;
    }

    if (m_pNewYValues)
    {
        for(size_t i(0); i < m_aHandles.size(); i++)
        {
            // find nearest xvalue
            double x, y;
            transform( m_aHandles[i].maPos, x, y );
            int nIndex = 0;
            double delta = std::fabs( x-m_pXValues[0] );
            for( int n = 1; n < m_nValues; n++ )
            {
                if( delta > std::fabs( x - m_pXValues[ n ] ) )
                {
                    delta = std::fabs( x - m_pXValues[ n ] );
                    nIndex = n;
                }
            }
            if( 0 == i )
                m_aHandles[i].maPos = transform( m_fMinX, m_pNewYValues[ nIndex ] );
            else if( m_aHandles.size() - 1L == i )
                m_aHandles[i].maPos = transform( m_fMaxX, m_pNewYValues[ nIndex ] );
            else
                m_aHandles[i].maPos = transform( m_pXValues[ nIndex ], m_pNewYValues[ nIndex ] );
        }
    }

    Invalidate();
}

IMPL_LINK_TYPED( GridDialog, ClickButtonHdl, Button*, pButton, void )
{
    if (pButton == m_pResetButton)
    {
        int nType = m_pResetTypeBox->GetSelectEntryPos();
        m_pGridWindow->ChangeMode(nType);
    }
}

double* GridDialog::getNewYValues()
{
    return m_pGridWindow->getNewYValues();
}

VCL_BUILDER_FACTORY(GridWindow)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
