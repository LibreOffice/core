/*************************************************************************
 *
 *  $RCSfile: fieldwnd.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mh $ $Date: 2001-10-23 09:02:53 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif


#ifndef PCH
#include <vcl/virdev.hxx>
#endif

#include "fieldwnd.hxx"
#include "pvlaydlg.hxx"
#include "pvglob.hxx"


//========================================================================

FieldWindow::FieldWindow( ScPivotLayoutDlg* pDialog, const ResId& rResId,
                          FieldType eType )
    :   Window      ( pDialog, rResId ),
        pDlg        ( pDialog ),
        eFieldType  ( eType ),
        nFieldSize  ( (eType != TYPE_SELECT) ? MAX_FIELDS : PAGE_SIZE ),
        nFieldCount ( 0 ),
        aCenterPos  ( 0,0 )
{
    aWndRect = Rectangle( GetPosPixel(), GetSizePixel() );
    aFieldArr = new String*[nFieldSize];

    for ( USHORT i=0; i<nFieldSize; i++ )
        aFieldArr[i] = NULL;

    if ( eFieldType != TYPE_SELECT )
    {
        Size aWinSize( GetSizePixel() );
        Size aTextSize( GetTextWidth( GetText() ), GetTextHeight() );

        aCenterPos = Point( (aWinSize.Width() - aTextSize.Width())/2,
                            (aWinSize.Height() - aTextSize.Height())/2 );
    }
}

//------------------------------------------------------------------------

__EXPORT FieldWindow::~FieldWindow()
{
    for ( USHORT i=0; i<nFieldCount; i++ )
        delete aFieldArr[i];
    delete [] aFieldArr;
}

//------------------------------------------------------------------------

void __EXPORT FieldWindow::Paint( const Rectangle& rRect )
{
    DoPaint( rRect );
}

//------------------------------------------------------------------------

void __EXPORT FieldWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        USHORT  nIndex = 0;

        if ( GetFieldIndex( rMEvt.GetPosPixel(), nIndex ) )
        {
            if ( rMEvt.GetClicks() == 1 )
            {
                const Pointer* pPtr =
                    pDlg->NotifyMouseButtonDown( eFieldType, nIndex );

                CaptureMouse();

                if ( *pPtr != GetPointer() )
                    SetPointer( *pPtr );
            }
            else
                pDlg->NotifyDoubleClick( eFieldType, nIndex );
        }
    }
}

//------------------------------------------------------------------------

void __EXPORT FieldWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() == 1 )
        {
            pDlg->NotifyMouseButtonUp(
                    OutputToScreenPixel( rMEvt.GetPosPixel() ) );
            SetPointer( Pointer( POINTER_ARROW ) );
        }

        if ( IsMouseCaptured() )
            ReleaseMouse();
    }
}

//------------------------------------------------------------------------

void __EXPORT FieldWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( IsMouseCaptured() )
    {
        const Pointer* pPtr =
            pDlg->NotifyMouseMove( OutputToScreenPixel( rMEvt.GetPosPixel() ) );
        if ( *pPtr != GetPointer() )
            SetPointer( *pPtr );
    }
}

//------------------------------------------------------------------------

void FieldWindow::Redraw()
{
    DoPaint( Rectangle( Point(), GetSizePixel() ) );
}

//------------------------------------------------------------------------

void FieldWindow::DoPaint( const Rectangle& rRect )
{
    Point           aPos0;
    VirtualDevice   aVirDev;
    Size            aSize( GetSizePixel() );
    Font            aFont( GetFont() );         // Font vom Window

    aFont.SetTransparent( TRUE );
    aVirDev.SetFont( aFont );
    aVirDev.SetOutputSizePixel  ( aSize );

    if ( eFieldType != TYPE_SELECT )
    {
        aVirDev.SetLineColor( Color( COL_BLACK ) );
        aVirDev.SetFillColor( Color( COL_WHITE ) );
        aVirDev.DrawRect( Rectangle( aPos0, aSize ) );

        aVirDev.DrawText( aCenterPos, GetText() );
    }
    else
    {
        aVirDev.SetLineColor();
        aVirDev.SetFillColor( GetBackground().GetColor() );
        aVirDev.DrawRect( Rectangle( aPos0, aSize ) );
    }

    switch ( eFieldType )
    {
        case TYPE_ROW:
        {
            Rectangle   aRect( aPos0, Size( OWIDTH, OHEIGHT ) );

            for ( USHORT i=0; i<nFieldCount; i++ )
            {
                if ( aFieldArr[i] )
                {
                    aRect.SetPos( Point( 0,OHEIGHT*i ) );
                    DrawField( aVirDev, aRect, *(aFieldArr[i]) );
                }
            }
        }
        break;

        case TYPE_COL:
        {
            Rectangle   aRect( aPos0, Size( OWIDTH, OHEIGHT ) );
            USHORT      nX = 0;
            USHORT      nY = 0;

            for ( USHORT i=0; i<nFieldCount; i++ )
            {
                if ( aFieldArr[i] )
                {
                    if ( i==4 ) { nY = 1; nX = 0; }
                    aRect.SetPos( Point( OWIDTH*nX, OHEIGHT*nY ) );
                    DrawField( aVirDev, aRect, *(aFieldArr[i]) );
                    nX++;
                }
            }
        }
        break;

        case TYPE_DATA:
        {
            Rectangle aRect( aPos0, Size( GetSizePixel().Width(), OHEIGHT ) );

            for ( USHORT i=0; i<nFieldCount; i++ )
            {
                if ( aFieldArr[i] )
                {
                    aRect.SetPos( Point( 0, OHEIGHT*i ) );
                    DrawField( aVirDev, aRect, *(aFieldArr[i]) );
                }
            }
        }
        break;

        case TYPE_SELECT:
        {
            Rectangle   aRect( aPos0, Size( OWIDTH, OHEIGHT ) );
            USHORT      nXOff = OWIDTH + SSPACE;

            for ( USHORT i=0; i<nFieldCount; i++ )
            {
                if ( aFieldArr[i] )
                {
                    aRect.SetPos(
                        Point( (i>7 ? nXOff : 0),
                               ((i%MAX_FIELDS)
                                *(OHEIGHT
                                  + ( (i!=0)&&(i!=MAX_FIELDS)
                                        ? SSPACE : 0 )))
                             ));

                    DrawField( aVirDev, aRect, *(aFieldArr[i]) );
                }
            }
        }
        break;

        default:
        break;
    }
    DrawBitmap( aPos0, aVirDev.GetBitmap( aPos0, aSize ) );
}

//------------------------------------------------------------------------

void FieldWindow::DrawField( OutputDevice& rDev,
                             const Rectangle& rRect,
                             const String& rStr )
{
    VirtualDevice aVirDev( rDev );
    Size    aDevSize( rRect.GetSize() );
    USHORT  nWidth       = (USHORT)aDevSize.Width();
    USHORT  nHeight      = (USHORT)aDevSize.Height();
    USHORT  nLabelWidth  = (USHORT)rDev.GetTextWidth(rStr);
    USHORT  nLabelHeight = (USHORT)rDev.GetTextHeight();
    Point   topLeft  ( 1,        1 );
    Point   topRight ( nWidth-2, 1 );
    Point   botLeft  ( 1,        nHeight-2 );
    Point   botRight ( nWidth-2, nHeight-2 );
    Point   aLabelPos( ((nWidth>nLabelWidth+2)   ?(nWidth-nLabelWidth)/2   :2),
                       ((nHeight>nLabelHeight+2) ?(nHeight-nLabelHeight)/2 :2) );

    aVirDev.SetOutputSizePixel  ( aDevSize );
    aVirDev.SetFont             ( rDev.GetFont() );
    aVirDev.SetFillColor( GetBackground().GetColor() );
    aVirDev.SetLineColor( Color( COL_BLACK ) );

    aVirDev.DrawRect( Rectangle( Point(), aDevSize ) );     // 1 Pixel Umrandung
    aVirDev.DrawText( aLabelPos, rStr );                    // der Text
    aVirDev.SetLineColor( Color( COL_WHITE ) );             // 3D-Rahmen
    aVirDev.DrawLine( topLeft, topRight );
    aVirDev.DrawLine( topLeft, botLeft  );
    aVirDev.SetLineColor( Color( COL_GRAY ) );
    aVirDev.DrawLine( botLeft, botRight );
    aVirDev.DrawLine( topRight, botRight );
    //--------------------------------------------------------------------
    rDev.DrawBitmap( rRect.TopLeft(), aVirDev.GetBitmap( Point(), aDevSize ) );
}

//------------------------------------------------------------------------

BOOL FieldWindow::GetInsertIndex( const Point& rInsertPos, USHORT& rIndex )
{
    BOOL bIndexFound = FALSE;

    if ( eFieldType != TYPE_SELECT )
    {
        bIndexFound = GetFieldIndex( rInsertPos, rIndex );
        if ( rIndex > nFieldCount-1 )
            rIndex = nFieldCount-1;
    }

    return bIndexFound;
}

//------------------------------------------------------------------------

BOOL FieldWindow::GetFirstEmptySlot( USHORT& rIndex )
{
    if ( nFieldCount == nFieldSize )
        return FALSE;

    USHORT i=0;
    while ( (i<nFieldCount) && (aFieldArr[i]!=NULL) )
        i++;

    if ( aFieldArr[i] == NULL )
    {
        rIndex = i;
        return TRUE;
    }
    else
        return FALSE;
}

//------------------------------------------------------------------------

BOOL FieldWindow::GetFieldIndex( const Point& rPos, USHORT& rIndex )
{
    BOOL    bOk = FALSE;
    USHORT  nX  = (USHORT)rPos.X();
    USHORT  nY  = (USHORT)rPos.Y();

    switch ( eFieldType )
    {
        case TYPE_ROW:
        case TYPE_DATA:
        {
            rIndex = nY / OHEIGHT;
            bOk = ( rIndex < nFieldCount );
        }
        break;

        case TYPE_COL:
        {
            USHORT nRow = nY / OHEIGHT;
            USHORT nCol = nX / OWIDTH;
            rIndex = nCol+(nRow*4);
            bOk = ( rIndex < nFieldCount );
        }
        break;

        case TYPE_SELECT:
        {
            USHORT nCol   = nX / (OWIDTH+SSPACE);
            USHORT nXDiff = nX - (nCol*(OWIDTH+SSPACE));

            if ( nXDiff < OWIDTH )
            {
                USHORT nRow   = nY / (OHEIGHT+SSPACE);
                USHORT nYDiff = nY - (nRow*(OHEIGHT+SSPACE));

                if ( nYDiff < OHEIGHT )
                {
                    rIndex = (nCol*MAX_FIELDS)+nRow;
                    bOk = ( rIndex < nFieldCount );
                }
            }
        }
        break;

        default:
        break;
    }

    return bOk;
}

//------------------------------------------------------------------------

void FieldWindow::AddField( const String& rStr, USHORT nIndex )
{
    if ( nIndex < nFieldSize )
    {
        if ( aFieldArr[nIndex] == NULL )
        {
            aFieldArr[nIndex] = new String( rStr );
            nFieldCount++;
        }
    }
}

//------------------------------------------------------------------------

BOOL FieldWindow::AddField( const String& rStr,
                            const Point& rAt,
                            USHORT& rAddedAt )
{
    if ( (eFieldType == TYPE_SELECT) || (nFieldCount == MAX_FIELDS) )
        return FALSE;

    USHORT nFirstEmpty = 0;

    if ( GetFirstEmptySlot( nFirstEmpty ) )
    {
        USHORT nIndex = 0;
        GetFieldIndex( rAt, nIndex);

        if ( nFirstEmpty < nIndex )
            nIndex = nFirstEmpty;

        if ( aFieldArr[nIndex] == NULL )
        {
            nFieldCount++;
            aFieldArr[nIndex] = new String( rStr );
        }
        else
        {
            nFieldCount++;
            for ( USHORT i=nFieldCount-1; i>nIndex; i-- )
                aFieldArr[i] = aFieldArr[i-1];
            aFieldArr[nIndex] = new String( rStr );
        }
        Redraw();
        rAddedAt = nIndex;
        return TRUE;
    }
    else
        return FALSE;
}

//------------------------------------------------------------------------

void FieldWindow::DelField( USHORT nFieldIndex )
{
    if ( nFieldIndex < nFieldCount )
    {
        if ( nFieldIndex == nFieldCount-1 )
        {
            delete aFieldArr[nFieldIndex];
            aFieldArr[nFieldIndex] = NULL;
            nFieldCount--;
        }
        else
        {
            delete aFieldArr[nFieldIndex];
            nFieldCount--;

            for ( USHORT i=nFieldIndex; i<nFieldCount; i++ )
                aFieldArr[i] = aFieldArr[i+1];

            aFieldArr[nFieldCount] = NULL;
        }
        Redraw();
    }
}

//------------------------------------------------------------------------

void FieldWindow::ClearFields()
{
    if ( eFieldType == TYPE_SELECT )
    {
        for ( USHORT i=0; i<nFieldCount; i++ )
        {
            delete aFieldArr[i];
            aFieldArr[i] = NULL;
        }
        nFieldCount = 0;
    }
}

//------------------------------------------------------------------------

void FieldWindow::SetText( const String& rStr, USHORT nIndex )
{
    if ( nIndex < nFieldCount )
    {
        *(aFieldArr[nIndex]) = rStr;
        Redraw();
    }
}
