/*************************************************************************
 *
 *  $RCSfile: grid.cxx,v $
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
#include <grid.hrc>
#include <cstdio>
#include <cmath>
#include <grid.hxx>

ResId SaneResId( ULONG );

/***********************************************************************
 *
 *  GridWindow
 *
 ***********************************************************************/

// ---------------------------------------------------------------------

GridWindow::GridWindow(
    double* pXValues, double* pYValues, int nValues,
    Window* pParent, BOOL bCutValues ) :
        ModalDialog( pParent, SaneResId( GRID_DIALOG ) ),
        m_aOKButton( this, SaneResId( GRID_DIALOG_OK_BTN ) ),
        m_aCancelButton( this, SaneResId( GRID_DIALOG_CANCEL_BTN ) ),
        m_aResetTypeBox( this, SaneResId( GRID_DIALOG_TYPE_BOX ) ),
        m_aResetButton( this, SaneResId( GRID_DIALOG_RESET_BTN ) ),
        m_aGridArea( 50, 15, 100, 100 ),
        m_pXValues( pXValues ),
        m_pOrigYValues( pYValues ),
        m_nValues( nValues ),
        m_pNewYValues( NULL ),
        m_aIAOManager( this ),
        m_pLeftMarker( NULL ),
        m_pRightMarker( NULL ),
        m_pDragMarker( NULL ),
        m_aMarkerBitmap( Bitmap( SaneResId( GRID_DIALOG_HANDLE_BMP ) ),
                         Color( 255, 255, 255 ) ),
        m_bCutValues( bCutValues )
{
    USHORT nPos = m_aResetTypeBox.InsertEntry( String( SaneResId( RESET_TYPE_LINEAR_ASCENDING ) ) );
    m_aResetTypeBox.SetEntryData( nPos, (void *)RESET_TYPE_LINEAR_ASCENDING );

    nPos = m_aResetTypeBox.InsertEntry( String( SaneResId( RESET_TYPE_LINEAR_DESCENDING ) ) );
    m_aResetTypeBox.SetEntryData( nPos, (void *)RESET_TYPE_LINEAR_DESCENDING );

    nPos = m_aResetTypeBox.InsertEntry( String( SaneResId( RESET_TYPE_RESET ) ) );
    m_aResetTypeBox.SetEntryData( nPos, (void *)RESET_TYPE_RESET );

    nPos = m_aResetTypeBox.InsertEntry( String( SaneResId( RESET_TYPE_EXPONENTIAL ) ) );
    m_aResetTypeBox.SetEntryData( nPos, (void *)RESET_TYPE_EXPONENTIAL );

    m_aResetTypeBox.SelectEntryPos( 0 );

    m_aResetButton.SetClickHdl( LINK( this, GridWindow, ClickButtonHdl ) );

    SetMapMode( MapMode( MAP_PIXEL ) );
    Size aSize = GetOutputSizePixel();
    Size aBtnSize = m_aOKButton.GetOutputSizePixel();
    m_aGridArea.setWidth( aSize.Width() - aBtnSize.Width() - 80 );
    m_aGridArea.setHeight( aSize.Height() - 40 );

    if( m_pOrigYValues && m_nValues )
    {
        m_pNewYValues = new double[ m_nValues ];
        memcpy( m_pNewYValues, m_pOrigYValues, sizeof( double ) * m_nValues );
    }

    setBoundings( 0, 0, 1023, 1023 );
    computeExtremes();

    m_pLeftMarker = new B2dIAOBitmapExReference(
        &m_aIAOManager,
        transform( findMinX(), findMinY() ),
        &m_aMarkerBitmap,
        m_aMarkerBitmap.GetSizePixel().Width()/2,
        m_aMarkerBitmap.GetSizePixel().Height()/2 );
    m_pLeftMarker->SetHittable( TRUE );
    m_pRightMarker = new B2dIAOBitmapExReference(
        &m_aIAOManager,
        transform( findMaxX(), findMaxY() ),
        &m_aMarkerBitmap,
        m_aMarkerBitmap.GetSizePixel().Width()/2,
        m_aMarkerBitmap.GetSizePixel().Height()/2 );
    m_pRightMarker->SetHittable( TRUE );

    FreeResource();
}

// ---------------------------------------------------------------------

GridWindow::~GridWindow()
{
    if( m_pNewYValues )
        delete m_pNewYValues;
}

// ---------------------------------------------------------------------

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

// ---------------------------------------------------------------------

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

// ---------------------------------------------------------------------

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

// ---------------------------------------------------------------------

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

// ---------------------------------------------------------------------

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

// ---------------------------------------------------------------------

Point GridWindow::transform( double x, double y )
{
    Point aRet;

    aRet.X() = ( x - m_fMinX ) *
        (double)m_aGridArea.GetWidth() / ( m_fMaxX - m_fMinX )
        + m_aGridArea.Left();
    aRet.Y() =
        m_aGridArea.Bottom() -
        ( y - m_fMinY ) *
        (double)m_aGridArea.GetHeight() / ( m_fMaxY - m_fMinY );
    return aRet;
}

// ---------------------------------------------------------------------

void GridWindow::transform( const Point& rOriginal, double& x, double& y )
{
    x = ( rOriginal.X() - m_aGridArea.Left() ) * (m_fMaxX - m_fMinX) / (double)m_aGridArea.GetWidth() + m_fMinX;
    y = ( m_aGridArea.Bottom() - rOriginal.Y() ) * (m_fMaxY - m_fMinY) / (double)m_aGridArea.GetHeight() + m_fMinY;
}

// ---------------------------------------------------------------------

void GridWindow::drawPoint( double x, double y )
{
    DrawPixel( transform( x, y ) );
}

// ---------------------------------------------------------------------

void GridWindow::drawLine( double x1, double y1, double x2, double y2 )
{
    DrawLine( transform( x1, y1 ), transform( x2, y2 ) );
}

// ---------------------------------------------------------------------

void GridWindow::computeChunk( double fMin, double fMax, double& fChunkOut, double& fMinChunkOut )
{
    // get a nice chunk size like 10, 100, 25 or such
    fChunkOut = ( fMax - fMin ) / 6.0;
    int logchunk = (int)log10( fChunkOut );
    int nChunk = (int)( fChunkOut / exp( (double)(logchunk-1) * M_LN10 ) );
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

// ---------------------------------------------------------------------

void GridWindow::computeNew()
{
    // special case: only left and right markers
    if( m_aIAOManager.GetIAOCount() == 2 )
    {
        double xleft, yleft;
        double xright, yright;
        transform( m_pLeftMarker->GetBasePosition(), xleft, yleft );
        transform( m_pRightMarker->GetBasePosition(), xright, yright );
        double factor = (yright-yleft)/(xright-xleft);
        for( int i = 0; i < m_nValues; i++ )
        {
            m_pNewYValues[ i ] = yleft + ( m_pXValues[ i ] - xleft )*factor;
        }
        return;
    }

    // X sort markers
    int nMarkers = m_aIAOManager.GetIAOCount();
    int nSorted = 0;
    B2dIAObject** pList = new B2dIAObject*[ nMarkers ];
    for( B2dIAObject* pMarker = m_aIAOManager.GetIAObjectList();
         pMarker; pMarker = pMarker->GetNext() )
    {
        int n = nSorted;
        for( int i = 0; i < nSorted; i++ )
        {
            if( pList[ i ]->GetBasePosition().X() > pMarker->GetBasePosition().X() )
            {
                for( n=nSorted; n > i; n-- )
                    pList[ n ] = pList[ n-1 ];
                pList[ i ] = pMarker;
                nSorted++;
                break;
            }
        }
        if( n >= nSorted )
            pList[ nSorted++ ] = pMarker;
    }

    // get node arrays
    double* nodex = new double[ nSorted ];
    double* nodey = new double[ nSorted ];
    int i;

    for( i = 0; i < nSorted; i++ )
        transform( pList[ i ]->GetBasePosition(), nodex[ i ], nodey[ i ] );

    for( i = 0; i < m_nValues; i++ )
    {
        double x = m_pXValues[ i ];
        m_pNewYValues[ i ] = interpolate( x, nodex, nodey, nSorted );
        if( m_bCutValues )
        {
            if( m_pNewYValues[ i ] > m_fMaxY )
                m_pNewYValues[ i ] = m_fMaxY;
            else if( m_pNewYValues[ i ] < m_fMinY )
                m_pNewYValues[ i ] = m_fMinY;
        }
    }

    delete nodex;
    delete nodey;
    delete pList;
}

// ---------------------------------------------------------------------

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

// ---------------------------------------------------------------------

void GridWindow::setBoundings( double fMinX, double fMinY, double fMaxX, double fMaxY )
{
    m_fMinX = fMinX;
    m_fMinY = fMinY;
    m_fMaxX = fMaxX;
    m_fMaxY = fMaxY;

    computeChunk( m_fMinX, m_fMaxX, m_fChunkX, m_fMinChunkX );
    computeChunk( m_fMinY, m_fMaxY, m_fChunkY, m_fMinChunkY );
}

// ---------------------------------------------------------------------

void GridWindow::drawGrid()
{
    char pBuf[256];
    SetLineColor( Color( COL_BLACK ) );
    TextAlign aAlign = GetTextAlign();
    // draw vertical lines
    for( double fX = m_fMinChunkX; fX < m_fMaxX; fX += m_fChunkX )
    {
        drawLine( fX, m_fMinY, fX, m_fMaxY );
        // draw tickmarks
        Point aPt = transform( fX, m_fMinY );
        sprintf( pBuf, "%g", fX );
        String aMark( pBuf, gsl_getSystemTextEncoding() );
        Size aTextSize( GetTextWidth( aMark ), GetTextHeight() );
        aPt.X() -= aTextSize.Width()/2;
        aPt.Y() += aTextSize.Height()/2;
        DrawText( aPt, aMark );
    }
    // draw horizontal lines
    for( double fY = m_fMinChunkY; fY < m_fMaxY; fY += m_fChunkY )
    {
        drawLine( m_fMinX, fY, m_fMaxX, fY );
        // draw tickmarks
        Point aPt = transform( m_fMinX, fY );
        sprintf( pBuf, "%g", fY );
        String aMark( pBuf, gsl_getSystemTextEncoding() );
        Size aTextSize( GetTextWidth( aMark ), GetTextHeight() );
        aPt.X() -= aTextSize.Width() + 2;
        aPt.Y() -= aTextSize.Height()/2;
        DrawText( aPt, aMark );
    }

    // draw boundings
    drawLine( m_fMinX, m_fMinY, m_fMaxX, m_fMinY );
    drawLine( m_fMinX, m_fMaxY, m_fMaxX, m_fMaxY );
    drawLine( m_fMinX, m_fMinY, m_fMinX, m_fMaxY );
    drawLine( m_fMaxX, m_fMinY, m_fMaxX, m_fMaxY );
}

// ---------------------------------------------------------------------

void GridWindow::drawOriginal()
{
    if( m_nValues && m_pXValues && m_pOrigYValues )
    {
        SetLineColor( Color( COL_RED ) );
        for( int i = 0; i < m_nValues-1; i++ )
        {
            drawLine( m_pXValues[ i   ], m_pOrigYValues[ i   ],
                      m_pXValues[ i+1 ], m_pOrigYValues[ i+1 ] );
        }
    }
}

// ---------------------------------------------------------------------

void GridWindow::drawNew()
{
    if( m_nValues && m_pXValues && m_pNewYValues )
    {
        SetClipRegion( m_aGridArea );
        SetLineColor( Color( COL_YELLOW ) );
        for( int i = 0; i < m_nValues-1; i++ )
        {
            drawLine( m_pXValues[ i   ], m_pNewYValues[ i   ],
                      m_pXValues[ i+1 ], m_pNewYValues[ i+1 ] );
        }
        SetClipRegion();
    }
}

// ---------------------------------------------------------------------

void GridWindow::Paint( const Rectangle& rRect )
{
    ModalDialog::Paint( rRect );
    drawGrid();
    drawOriginal();
    drawNew();
    m_aIAOManager.UpdateDisplay();
}

// ---------------------------------------------------------------------

void GridWindow::MouseMove( const MouseEvent& rEvt )
{
    if( rEvt.GetButtons() == MOUSE_LEFT && m_pDragMarker )
    {
        Point aPoint( rEvt.GetPosPixel() );
        if( m_pDragMarker == m_pLeftMarker || m_pDragMarker == m_pRightMarker )
        {
            aPoint.X() = m_pDragMarker->GetBasePosition().X();
        }
        if( aPoint.Y() < m_aGridArea.Top() )
            aPoint.Y() = m_aGridArea.Top();
        else if( aPoint.Y() > m_aGridArea.Bottom() )
            aPoint.Y() = m_aGridArea.Bottom();
        // avoid flicker
        if( aPoint != m_pDragMarker->GetBasePosition() )
        {
            m_pDragMarker->SetBasePosition( aPoint );
            m_aIAOManager.UpdateDisplay();
        }
    }
    ModalDialog::MouseMove( rEvt );
}

// ---------------------------------------------------------------------

void GridWindow::MouseButtonUp( const MouseEvent& rEvt )
{
    if( rEvt.GetButtons() == MOUSE_LEFT )
    {
        if( m_pDragMarker )
        {
            m_pDragMarker = NULL;
            computeNew();
            Invalidate( m_aGridArea );
            Paint( m_aGridArea );
        }
    }

    ModalDialog::MouseButtonUp( rEvt );
}

// ---------------------------------------------------------------------

void GridWindow::MouseButtonDown( const MouseEvent& rEvt )
{
    Point aPoint( rEvt.GetPosPixel() );
    B2dIAObject* pMarker = m_aIAOManager.GetIAObjectList();
    while( pMarker && ! pMarker->IsHit( aPoint ) )
        pMarker = pMarker->GetNext();

    if( rEvt.GetButtons() == MOUSE_LEFT )
    {
        // user wants to drag a button
        if( pMarker )
            m_pDragMarker = (B2dIAOBitmapExReference*)pMarker;
    }
    else if( rEvt.GetButtons() == MOUSE_RIGHT )
    {
        // user wants to add/delete a button
        if( pMarker && pMarker != m_pLeftMarker && pMarker != m_pRightMarker )
        {
            // delete marker under mouse
            if( m_pDragMarker == pMarker )
                m_pDragMarker = NULL;
            delete pMarker;
        }
        else if( ! pMarker )
        {
            pMarker = new B2dIAOBitmapExReference(
                &m_aIAOManager,
                aPoint,
                &m_aMarkerBitmap,
                m_aMarkerBitmap.GetSizePixel().Width()/2,
                m_aMarkerBitmap.GetSizePixel().Height()/2 );
            pMarker->SetHittable( TRUE );
        }
        computeNew();
        Invalidate( m_aGridArea );
        Paint( m_aGridArea );
    }
    ModalDialog::MouseButtonDown( rEvt );
}

// ---------------------------------------------------------------------

IMPL_LINK( GridWindow, ClickButtonHdl, Button*, pButton )
{
    if( pButton == &m_aResetButton )
    {
        int nType = (int)m_aResetTypeBox.GetEntryData( m_aResetTypeBox.GetSelectEntryPos() );
        switch( nType )
        {
            case RESET_TYPE_LINEAR_ASCENDING:
            {
                for( int i = 0; i < m_nValues; i++ )
                {
                    m_pNewYValues[ i ] = m_fMinY + (m_fMaxY-m_fMinY)/(m_fMaxX-m_fMinX)*(m_pXValues[i]-m_fMinX);
                }
            }
            break;
            case RESET_TYPE_LINEAR_DESCENDING:
            {
                for( int i = 0; i < m_nValues; i++ )
                {
                    m_pNewYValues[ i ] = m_fMaxY - (m_fMaxY-m_fMinY)/(m_fMaxX-m_fMinX)*(m_pXValues[i]-m_fMinX);
                }
            }
            break;
            case RESET_TYPE_RESET:
            {
                if( m_pOrigYValues && m_pNewYValues && m_nValues )
                    memcpy( m_pNewYValues, m_pOrigYValues, m_nValues*sizeof(double) );
            }
            break;
            case RESET_TYPE_EXPONENTIAL:
            {
                for( int i = 0; i < m_nValues; i++ )
                {
                    m_pNewYValues[ i ] = m_fMinY + (m_fMaxY-m_fMinY)*(exp((m_pXValues[i]-m_fMinX)/(m_fMaxX-m_fMinX))-1.0)/(M_E-1.0);
                }
            }
            break;

            default:
                break;
        }
        int nMarkers = m_aIAOManager.GetIAOCount();
        B2dIAObject* pMarker = m_aIAOManager.GetIAObjectList();
        for( int i = 0 ; i < nMarkers; i++, pMarker = pMarker->GetNext() )
        {
            // find nearest xvalue
            double x, y;
            transform( pMarker->GetBasePosition(), x, y );
            int nIndex = 0;
            double delta = fabs( x-m_pXValues[0] );
            for( int n = 1; n < m_nValues; n++ )
            {
                if( delta > fabs( x - m_pXValues[ n ] ) )
                {
                    delta = fabs( x - m_pXValues[ n ] );
                    nIndex = n;
                }
            }
            if( pMarker == m_pLeftMarker )
                pMarker->SetBasePosition( transform( m_fMinX, m_pNewYValues[ nIndex ] ) );
            else if( pMarker == m_pRightMarker )
                pMarker->SetBasePosition( transform( m_fMaxX, m_pNewYValues[ nIndex ] ) );
            else
                pMarker->SetBasePosition( transform( m_pXValues[ nIndex ], m_pNewYValues[ nIndex ] ) );
        }
        Invalidate( m_aGridArea );
        Paint(Rectangle());
    }
    return 0;
}
