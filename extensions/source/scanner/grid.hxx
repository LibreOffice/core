/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _EXTENSIONS_SCANNER_GRID_HXX
#define _EXTENSIONS_SCANNER_GRID_HXX

#include <vcl/window.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>

class GridWindow : public ModalDialog
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

        void draw(Window& rWin, const BitmapEx& rBitmapEx)
        {
            const Point aOffset(rWin.PixelToLogic(Point(mnOffX, mnOffY)));
            rWin.DrawBitmapEx(maPos - aOffset, rBitmapEx);
        }

        bool isHit(Window& rWin, const Point& rPos)
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

    sal_Bool            m_bCutValues;

    // stuff for handles
    std::vector< impHandle >    m_aHandles;
    sal_uInt32                  m_nDragIndex;

    BitmapEx        m_aMarkerBitmap;

    OKButton        m_aOKButton;
    CancelButton    m_aCancelButton;

    ListBox         m_aResetTypeBox;
    PushButton      m_aResetButton;


    Point transform( double x, double y );
    void transform( const Point& rOriginal, double& x, double& y );

    double findMinX();
    double findMinY();
    double findMaxX();
    double findMaxY();

    void drawGrid();
    void drawOriginal();
    void drawNew();
    void drawHandles();

    void computeExtremes();
    void computeChunk( double fMin, double fMax, double& fChunkOut, double& fMinChunkOut );
    void computeNew();
    double interpolate( double x, double* pNodeX, double* pNodeY, int nNodes );

    DECL_LINK( ClickButtonHdl, Button* );

    virtual void MouseMove( const MouseEvent& );
    virtual void MouseButtonDown( const MouseEvent& );
    virtual void MouseButtonUp( const MouseEvent& );
public:
    GridWindow( double* pXValues, double* pYValues, int nValues,
                Window* pParent, sal_Bool bCutValues = sal_True );
    ~GridWindow();

    void setBoundings( double fMinX, double fMinY, double fMaxX, double fMaxY );
    double getMinX() { return m_fMinX; }
    double getMinY() { return m_fMinY; }
    double getMaxX() { return m_fMaxX; }
    double getMaxY() { return m_fMaxY; }

    int countValues() { return m_nValues; }
    double* getXValues() { return m_pXValues; }
    double* getOrigYValues() { return m_pOrigYValues; }
    double* getNewYValues() { return m_pNewYValues; }

    void drawLine( double x1, double y1, double x2, double y2 );

    virtual void Paint( const Rectangle& rRect );
};

#endif // _EXTENSIONS_SCANNER_GRID_HXX
