/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grid.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:44:01 $
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
#ifndef _EXTENSIONS_SCANNER_GRID_HXX
#define _EXTENSIONS_SCANNER_GRID_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

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

    BOOL            m_bCutValues;

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
                Window* pParent, BOOL bCutValues = TRUE );
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

    void drawPoint( double x, double y );
    void drawLine( double x1, double y1, double x2, double y2 );

    virtual void Paint( const Rectangle& rRect );
};

#endif // _EXTENSIONS_SCANNER_GRID_HXX
