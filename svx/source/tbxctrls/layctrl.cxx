/*************************************************************************
 *
 *  $RCSfile: layctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:26 $
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

// include ---------------------------------------------------------------

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#pragma hdrstop

#include "dialogs.hrc"
#include "layctrl.hxx"

SFX_IMPL_TOOLBOX_CONTROL(SvxTableToolBoxControl,SfxUInt16Item);
SFX_IMPL_TOOLBOX_CONTROL(SvxColumnsToolBoxControl,SfxUInt16Item);

// class TableWindow -----------------------------------------------------

class TableWindow : public SfxPopupWindow
{
private:
    Color           aLineColor;
    Color           aHighlightLineColor;
    Color           aFillColor;
    Color           aHighlightFillColor;
    long            nCol;
    long            nLine;
    long            nWidth;
    long            nHeight;
    long            nMX;
    long            nMY;
    long            nTextHeight;

public:
                            TableWindow( USHORT nId, SfxBindings& rBind );

    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            Paint( const Rectangle& );
    virtual void            PopupModeEnd();
    virtual SfxPopupWindow* Clone() const;

    USHORT                  GetColCount() const { return (USHORT)nCol; }
    USHORT                  GetLineCount() const { return (USHORT)nLine; }
};

// -----------------------------------------------------------------------

TableWindow::TableWindow( USHORT nId, SfxBindings& rBind ) :

    SfxPopupWindow( nId, (WinBits)0, rBind )

{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    aLineColor = rStyles.GetWindowTextColor();
    aHighlightLineColor = rStyles.GetHighlightTextColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();

    nTextHeight = GetTextHeight()+1;
    SetBackgroundBrush( Brush( BRUSH_NULL ) );
    Font aFont = GetFont();
    aFont.SetFillColor( Color( COL_WHITE ) );
    aFont.SetTransparent( FALSE );
    SetFont( aFont );

    nCol    = 0;
    nLine   = 0;
    nWidth  = 5;
    nHeight = 5;

    Size aLogicSize = LogicToPixel( Size( 55, 35 ), MapMode( MAP_10TH_MM ) );
    nMX = aLogicSize.Width();
    nMY = aLogicSize.Height();
    SetOutputSizePixel( Size( nMX*nWidth-1, nMY*nHeight-1+nTextHeight ) );
}

// -----------------------------------------------------------------------

SfxPopupWindow* TableWindow::Clone() const
{
    return new TableWindow( GetId(), (SfxBindings&)GetBindings() );
}

// -----------------------------------------------------------------------

void TableWindow::MouseMove( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseMove( rMEvt );
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos = OutputToScreenPixel( aPos );
    Point aWinPos = GetPosPixel();

    if ( rMEvt.IsEnterWindow() )
        CaptureMouse();
    else if ( aMousePos.X() < aWinPos.X() || aMousePos.Y() < aWinPos.Y() )
    {
        nCol = 0;
        nLine = 0;
        ReleaseMouse();
        Invalidate();
        return;
    }

    Size    aWinSize = GetOutputSizePixel();
    long    nNewCol = 0;
    long    nNewLine = 0;
    long    nMinCol = 0;
    long    nMinLine = 0;
    long    nMaxCol = 0;
    long    nMaxLine = 0;

    if ( aPos.X() > 0 )
        nNewCol = aPos.X() / nMX + 1;
    if ( aPos.Y() > 0 )
        nNewLine = aPos.Y() / nMY + 1;

    if ( nNewCol > 500 )
        nNewCol = 500;
    if ( nNewLine > 1000 )
        nNewLine = 1000;

    if ( (nWidth <= nNewCol) || (nHeight < nNewLine) )
    {
        Point aMaxPos = OutputToScreenPixel( GetDesktopRectPixel().BottomRight() );
        long    nOff = 0;

        if ( nWidth <= nNewCol )
        {
            nWidth = nNewCol;
            nWidth++;
        }
        if ( nHeight <= nNewLine )
        {
            nHeight = nNewLine;
            nOff = 1;
        }
        while ( nWidth > 0 &&
                (short)(aWinPos.X()+(nMX*nWidth-1)) >= aMaxPos.X()-3 )
            nWidth--;

        while ( nHeight > 0 &&
                (short)(aWinPos.Y()+(nMY*nHeight-1+nTextHeight)) >=
                aMaxPos.Y()-3 )
            nHeight--;

        if ( nNewCol > nWidth )
            nNewCol = nWidth;

        if ( nNewLine > nHeight )
            nNewLine = nHeight;

        Invalidate( Rectangle( 0, aWinSize.Height()-nTextHeight+2-nOff,
                               aWinSize.Width(), aWinSize.Height() ) );
        SetOutputSizePixel( Size( nMX*nWidth-1, nMY*nHeight-1+nTextHeight ) );
    }

    if ( nNewCol < nCol )
    {
        nMinCol = nNewCol;
        nMaxCol = nCol;
    }
    else
    {
        nMinCol = nCol;
        nMaxCol = nNewCol;
    }
    if ( nNewLine < nLine )
    {
        nMinLine = nNewLine;
        nMaxLine = nLine;
    }
    else
    {
        nMinLine = nLine;
        nMaxLine = nNewLine;
    }

    if ( (nNewCol != nCol) || (nNewLine != nLine) )
    {
        Invalidate( Rectangle( 0, aWinSize.Height()-nTextHeight+2,
                               aWinSize.Width(), aWinSize.Height() ) );

        if ( nNewCol != nCol )
        {
            Invalidate( Rectangle( nMinCol*nMX-1, 0, nMaxCol*nMX+1, nMaxLine*nMY ) );
            nCol  = nNewCol;
        }
        if ( nNewLine != nLine )
        {
            Invalidate( Rectangle( 0, nMinLine*nMY-2, nMaxCol*nMX, nMaxLine*nMY+1 ) );
            nLine = nNewLine;
        }
    }
    Update();
}

// -----------------------------------------------------------------------

void TableWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseButtonDown( rMEvt );
    CaptureMouse();
}

// -----------------------------------------------------------------------

void TableWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseButtonUp( rMEvt );
    ReleaseMouse();

    if ( IsInPopupMode() )
        EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
}

// -----------------------------------------------------------------------

void TableWindow::Paint( const Rectangle& )
{
    long    i;
    long    nStart;
    Size    aSize = GetOutputSizePixel();

    SetLineColor();
    SetFillColor( aHighlightFillColor );
    DrawRect( Rectangle( 0, 0, nCol*nMX-1, nLine*nMY-1 ) );
    SetFillColor( aFillColor );
    DrawRect( Rectangle( nCol*nMX-1, 0,
                         aSize.Width(), aSize.Height()-nTextHeight+1 ) );
    DrawRect( Rectangle( 0, nLine*nMY-1,
                         aSize.Width(), aSize.Height()-nTextHeight+1 ) );

    SetLineColor( aHighlightLineColor );
    for ( i = 1; i < nCol; i++ )
        DrawLine( Point( i*nMX-1, 0 ), Point( i*nMX-1, nLine*nMY-1 ) );
    for ( i = 1; i < nLine; i++ )
        DrawLine( Point( 0, i*nMY-1 ), Point( nCol*nMX-1, i*nMY-1 ) );
    SetLineColor( aLineColor );
    for ( i = 1; i <= nWidth; i++ )
    {
        if ( i < nCol )
            nStart = nLine*nMY-1;
        else
            nStart = 0;
        DrawLine( Point( i*nMX-1, nStart ), Point( i*nMX-1, nHeight*nMY-1 ) );
    }
    for ( i = 1; i <= nHeight; i++ )
    {
        if ( i < nLine )
            nStart = nCol*nMX-1;
        else
            nStart = 0;
        DrawLine( Point( nStart, i*nMY-1 ), Point( nWidth*nMX-1, i*nMY-1 ) );
    }

    SetLineColor();
    String aText;
    if ( nCol && nLine )
    {
        aText += String::CreateFromInt32( nCol );
        aText.AppendAscii( " x " );
        aText += String::CreateFromInt32( nLine );
    }
    else
        aText = Button::GetStandardText( BUTTON_CANCEL );
    Size aTextSize( GetTextWidth( aText ), GetTextHeight() );
    DrawText( Point( (aSize.Width() - aTextSize.Width()) / 2, aSize.Height() - nTextHeight + 2 ), aText );
    DrawRect( Rectangle( 0, aSize.Height()-nTextHeight+2,
                         (aSize.Width()-aTextSize.Width())/2-1, aSize.Height() ) );
    DrawRect( Rectangle( (aSize.Width()-aTextSize.Width())/2+aTextSize.Width(),
                         aSize.Height()-nTextHeight+2,
                         aSize.Width(), aSize.Height() ) );
    SetLineColor( Color( COL_BLACK ) );
    SetFillColor();
    DrawRect( Rectangle( Point(0,0), aSize ) );
}

// -----------------------------------------------------------------------

void TableWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol && nLine )
    {
        SfxUInt16Item aCol( SID_ATTR_TABLE_COLUMN, (UINT16)nCol );
        SfxUInt16Item aRow( SID_ATTR_TABLE_ROW, (UINT16)nLine );
        GetBindings().GetDispatcher()->Execute(
            GetId(), SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aCol, &aRow, 0L );
    }
    else if ( IsPopupModeCanceled() )
        ReleaseMouse();
    SfxPopupWindow::PopupModeEnd();
}

// class ColumnsWindow ---------------------------------------------------

class ColumnsWindow : public SfxPopupWindow
{
private:
    Color           aLineColor;
    Color           aHighlightLineColor;
    Color           aFillColor;
    Color           aHighlightFillColor;
    long            nCol;
    long            nWidth;
    long            nMX;
    long            nTextHeight;

public:
                            ColumnsWindow( USHORT nId, SfxBindings& rBind );

    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            Paint( const Rectangle& );
    virtual void            PopupModeEnd();
    virtual SfxPopupWindow* Clone() const;

    USHORT                  GetColCount() const { return (USHORT)nCol; }
};

// -----------------------------------------------------------------------

ColumnsWindow::ColumnsWindow( USHORT nId, SfxBindings& rBind ) :

    SfxPopupWindow( nId, (WinBits)0, rBind )

{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    aLineColor = rStyles.GetWindowTextColor();
    aHighlightLineColor = rStyles.GetHighlightTextColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();

    nTextHeight = GetTextHeight()+1;
    SetBackgroundBrush( Brush( BRUSH_NULL ) );
    Font aFont = GetFont();
    aFont.SetFillColor( Color( COL_WHITE ) );
    aFont.SetTransparent( FALSE );
    SetFont( aFont );

    nCol        = 0;
    nWidth      = 4;

    Size aLogicSize = LogicToPixel( Size( 95, 155 ), MapMode( MAP_10TH_MM ) );
    nMX = aLogicSize.Width();
    SetOutputSizePixel( Size( nMX*nWidth-1, aLogicSize.Height()+nTextHeight ) );
}

// -----------------------------------------------------------------------

SfxPopupWindow* ColumnsWindow::Clone() const
{
    return new ColumnsWindow( GetId(), (SfxBindings&)GetBindings() );
}

// -----------------------------------------------------------------------

void ColumnsWindow::MouseMove( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseMove( rMEvt );
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos = OutputToScreenPixel( aPos );
    Point aWinPos = GetPosPixel();

    if ( rMEvt.IsEnterWindow() )
        CaptureMouse();
    else if ( aMousePos.X() < aWinPos.X() || aMousePos.Y() < aWinPos.Y() )
    {
        nCol = 0;
        ReleaseMouse();
        Invalidate();
        return;
    }

    Size    aWinSize = GetOutputSizePixel();
    long    nNewCol = 0;
    long    nMinCol = 0;
    long    nMaxCol = 0;

    if ( aPos.X() > 0 )
        nNewCol = aPos.X() / nMX + 1;

    if ( nNewCol > 20 )
        nNewCol = 20;

    if ( nWidth <= nNewCol )
    {
        Point aMaxPos = OutputToScreenPixel( GetDesktopRectPixel().BottomRight() );

        if ( nWidth <= nNewCol )
        {
            nWidth = nNewCol;
            nWidth++;
        }

        while ( nWidth > 0 &&
                (short)(aWinPos.X()+(nMX*nWidth-1)) >= aMaxPos.X()-3 )
            nWidth--;

        if ( nNewCol > nWidth )
            nNewCol = nWidth;

        Invalidate( Rectangle( 0, aWinSize.Height()-nTextHeight+2,
                               aWinSize.Width(), aWinSize.Height() ) );
        SetOutputSizePixel( Size( nMX*nWidth-1, aWinSize.Height() ) );
    }

    if ( aPos.Y() < 0 )
        nNewCol = 0;

    if ( nNewCol != nCol )
    {
        Invalidate( Rectangle( 0, aWinSize.Height()-nTextHeight+2,
                               aWinSize.Width(), aWinSize.Height() ) );

        if ( nNewCol < nCol )
        {
            nMinCol = nNewCol;
            nMaxCol = nCol;
        }
        else
        {
            nMinCol = nCol;
            nMaxCol = nNewCol;
        }

        Invalidate( Rectangle( nMinCol*nMX-1, 0,
                               nMaxCol*nMX+1, aWinSize.Height()-nTextHeight+2 ) );
        nCol  = nNewCol;
    }
    Update();
}

// -----------------------------------------------------------------------

void ColumnsWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseButtonDown( rMEvt );
    CaptureMouse();
}

// -----------------------------------------------------------------------

void ColumnsWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseButtonUp( rMEvt );
    ReleaseMouse();

    if ( IsInPopupMode() )
        EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
}

// -----------------------------------------------------------------------

void ColumnsWindow::Paint( const Rectangle& )
{
    long    i;
    long    j;
    long    nLineWidth;
    Size    aSize = GetOutputSizePixel();

    for ( i = 0; i < nWidth; i++ )
    {
        if ( i < nCol )
        {
            SetLineColor( aHighlightLineColor );
            SetFillColor( aHighlightFillColor );
        }
        else
        {
            SetLineColor( aLineColor );
            SetFillColor( aFillColor );
        }

        DrawRect( Rectangle( i*nMX-1, -1,
                             i*nMX+nMX, aSize.Height()-nTextHeight+1 ) );

        j = 4;
        while ( j < aSize.Height()-nTextHeight-4 )
        {
            if ( !(j % 16) )
                nLineWidth = 10;
            else
                nLineWidth = 4;
            DrawLine( Point( i*nMX+4, j ), Point( i*nMX+nMX-nLineWidth-4, j ) );
            j += 4;
        }
    }

    SetLineColor();
    SetFillColor( aFillColor );
    String aText;
    if ( nCol )
        aText = String( String::CreateFromInt32(nCol) );
    else
        aText = Button::GetStandardText( BUTTON_CANCEL );
    Size aTextSize(GetTextWidth( aText ), GetTextHeight());
    DrawText( Point( (aSize.Width()-aTextSize.Width()) / 2,
                     aSize.Height()-nTextHeight+2 ),
              aText );
    DrawRect( Rectangle( 0, aSize.Height()-nTextHeight+2,
                         (aSize.Width()-aTextSize.Width())/2-1, aSize.Height() ) );
    DrawRect( Rectangle( (aSize.Width()-aTextSize.Width())/2+aTextSize.Width(),
                         aSize.Height()-nTextHeight+2,
                         aSize.Width(), aSize.Height() ) );
    SetLineColor( Color( COL_BLACK ) );
    SetFillColor();
    DrawRect( Rectangle( Point(0,0), aSize ) );
}

// -----------------------------------------------------------------------

void ColumnsWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol )
    {
        SfxUInt16Item aCol( SID_ATTR_COLUMNS, (UINT16)nCol );
        GetBindings().GetDispatcher()->Execute(
            GetId(), SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aCol, 0L );
    }
    else if ( IsPopupModeCanceled() )
        ReleaseMouse();
    SfxPopupWindow::PopupModeEnd();
}

// class SvxTableToolBoxControl ------------------------------------------

SvxTableToolBoxControl::SvxTableToolBoxControl( USHORT nId, ToolBox& rTbx,
                                                SfxBindings& rBind ) :

    SfxToolBoxControl( nId, rTbx, rBind ),

    bEnabled( TRUE )

{
}

// -----------------------------------------------------------------------

SvxTableToolBoxControl::~SvxTableToolBoxControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxTableToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUTANDMOVE;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxTableToolBoxControl::CreatePopupWindow()
{
    if ( bEnabled )
    {
        TableWindow* pWin = new TableWindow( GetId(), GetBindings() );
        pWin->StartPopupMode( &GetToolBox(), FALSE );
        return pWin;
    }
    return 0;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxTableToolBoxControl::CreatePopupWindowCascading()
{
    if ( bEnabled )
        return new TableWindow( GetId(), GetBindings() );
    return 0;
}

// -----------------------------------------------------------------------

void SvxTableToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    if ( pState && pState->ISA(SfxUInt16Item) )
        bEnabled = FALSE;
    else
        bEnabled = SFX_ITEM_DISABLED != eState;

    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId,
        ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

// class SvxColumnsToolBoxControl ------------------------------------------

SvxColumnsToolBoxControl::SvxColumnsToolBoxControl( USHORT nId, ToolBox& rTbx,
                                                    SfxBindings& rBind ) :

    SfxToolBoxControl( nId, rTbx, rBind )
{
}

// -----------------------------------------------------------------------

SvxColumnsToolBoxControl::~SvxColumnsToolBoxControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxColumnsToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUTANDMOVE;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxColumnsToolBoxControl::CreatePopupWindow()
{
    ColumnsWindow* pWin = 0;
    if(bEnabled)
    {
        pWin = new ColumnsWindow( GetId(), GetBindings() );
        pWin->StartPopupMode( &GetToolBox(), FALSE );
    }
    return pWin;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxColumnsToolBoxControl::CreatePopupWindowCascading()
{
    ColumnsWindow* pWin = 0;
    if(bEnabled)
    {
        pWin = new ColumnsWindow( GetId(), GetBindings() );
    }
    return pWin;
}
/* -----------------18.11.99 16:38-------------------

 --------------------------------------------------*/
void SvxColumnsToolBoxControl::StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    bEnabled = SFX_ITEM_DISABLED != eState;
    SfxToolBoxControl::StateChanged(nSID,   eState, pState );
}

