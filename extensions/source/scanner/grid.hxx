/*************************************************************************
 *
 *  $RCSfile: grid.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
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
#ifndef _B2D_MBASE_HXX
#include <goodies/b2dmbase.hxx>
#endif
#ifndef _B2D_MPNT_HXX
#include <goodies/b2dmbmp.hxx>
#endif

class GridWindow : public ModalDialog
{
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

    BOOL            m_bCutValues;

    // stuff for handles
    B2dIAOManager               m_aIAOManager;
    B2dIAOBitmapExReference*    m_pLeftMarker;
    B2dIAOBitmapExReference*    m_pRightMarker;
    B2dIAOBitmapExReference*    m_pDragMarker;

    BitmapEx                    m_aMarkerBitmap;

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
