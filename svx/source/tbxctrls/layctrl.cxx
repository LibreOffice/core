/*************************************************************************
 *
 *  $RCSfile: layctrl.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 16:42:55 $
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
#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#include <comphelper/processfactory.hxx>

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

// namespaces
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL(SvxTableToolBoxControl,SfxUInt16Item);
SFX_IMPL_TOOLBOX_CONTROL(SvxColumnsToolBoxControl,SfxUInt16Item);

// class TableWindow -----------------------------------------------------

class TableWindow : public SfxPopupWindow
{
private:
    Color               aLineColor;
    Color               aHighlightLineColor;
    Color               aFillColor;
    Color               aHighlightFillColor;
    long                nCol;
    long                nLine;
    long                nWidth;
    long                nHeight;
    long                nMX;
    long                nMY;
    long                nTextHeight;
    BOOL                bInitialKeyInput;
    BOOL                m_bMod1;
    ToolBox&            rTbx;
    Reference< XFrame > mxFrame;
    rtl::OUString       maCommand;

    void UpdateSize_Impl( long nNewCol, long nNewLine);

public:
                            TableWindow( USHORT                     nSlotId,
                                         const rtl::OUString&       rCmd,
                                         ToolBox&                   rParentTbx,
                                         const Reference< XFrame >& rFrame );
                            ~TableWindow();

    void                    KeyInput( const KeyEvent& rKEvt );
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

TableWindow::TableWindow( USHORT nSlotId, const rtl::OUString& rCmd, ToolBox& rParentTbx, const Reference< XFrame >& rFrame ) :
    SfxPopupWindow( nSlotId, rFrame, WB_SYSTEMWINDOW ),
    bInitialKeyInput(TRUE),
    m_bMod1(FALSE),
    rTbx(rParentTbx),
    mxFrame( rFrame ),
    maCommand( rCmd )
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    aLineColor = Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    aHighlightLineColor = rStyles.GetHighlightTextColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();

    nTextHeight = GetTextHeight()+1;
    SetBackground();
    Font aFont = GetFont();
    aFont.SetColor( aLineColor );
    aFont.SetFillColor( aFillColor );
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
TableWindow::~TableWindow()
{
}
// -----------------------------------------------------------------------

SfxPopupWindow* TableWindow::Clone() const
{
    return new TableWindow( GetId(), maCommand, rTbx, mxFrame );
}

// -----------------------------------------------------------------------

void TableWindow::MouseMove( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseMove( rMEvt );
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos( aPos );

    if ( rMEvt.IsEnterWindow() )
        CaptureMouse();
    else if ( aMousePos.X() < 0 || aMousePos.Y() < 0 )
    {
        nCol = 0;
        nLine = 0;
        ReleaseMouse();
        Invalidate();
        return;
    }

    long    nNewCol = 0;
    long    nNewLine = 0;

    if ( aPos.X() > 0 )
        nNewCol = aPos.X() / nMX + 1;
    if ( aPos.Y() > 0 )
        nNewLine = aPos.Y() / nMY + 1;

    if ( nNewCol > 500 )
        nNewCol = 500;
    if ( nNewLine > 1000 )
        nNewLine = 1000;

    UpdateSize_Impl( nNewCol, nNewLine);

}
/* -----------------------------15.05.2002 17:14------------------------------

 ---------------------------------------------------------------------------*/
void TableWindow::UpdateSize_Impl( long nNewCol, long nNewLine)
{
    Size  aWinSize = GetOutputSizePixel();
    Point aWinPos = GetPosPixel();
    Point aMaxPos = OutputToScreenPixel( GetDesktopRectPixel().BottomRight() );
    if ( (nWidth <= nNewCol) || (nHeight < nNewLine) )
    {
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

        Size    aWinSize = GetOutputSizePixel();
        Invalidate( Rectangle( 0, aWinSize.Height()-nTextHeight+2-nOff,
                               aWinSize.Width(), aWinSize.Height() ) );
        SetOutputSizePixel( Size( nMX*nWidth-1, nMY*nHeight-1+nTextHeight ) );
    }
    long    nMinCol = 0;
    long    nMaxCol = 0;
    long    nMinLine = 0;
    long    nMaxLine = 0;
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
/* -----------------------------15.05.2002 14:22------------------------------

 ---------------------------------------------------------------------------*/
void TableWindow::KeyInput( const KeyEvent& rKEvt )
{
    BOOL bHandled = FALSE;
    USHORT nModifier = rKEvt.GetKeyCode().GetModifier();
    USHORT nKey = rKEvt.GetKeyCode().GetCode();
    if(!nModifier)
    {
        if( KEY_UP == nKey || KEY_DOWN == nKey ||
            KEY_LEFT == nKey || KEY_RIGHT == nKey ||
            KEY_ESCAPE == nKey ||KEY_RETURN == nKey )
        {
            bHandled = TRUE;
            long nNewCol = nCol;
            long nNewLine = nLine;
            switch(nKey)
            {
                case KEY_UP :
                    if(nNewLine > 1)
                    {
                        nNewLine--;
                        break;
                    }
                //no break;
                case KEY_ESCAPE:
                    EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL);
                break;
                case KEY_DOWN :
                    nNewLine++;
                break;
                case KEY_LEFT :

                    if(nNewCol)
                        nNewCol--;
                break;
                case KEY_RIGHT :
                    nNewCol++;
                break;
                case KEY_RETURN :
                    if(IsMouseCaptured())
                        ReleaseMouse();
                    EndPopupMode(FLOATWIN_POPUPMODEEND_CLOSEALL );
                break;
            }
            //make sure that a table can initially be created
            if(bInitialKeyInput)
            {
                bInitialKeyInput = FALSE;
                if(!nNewLine)
                    nNewLine = 1;
                if(!nNewCol)
                    nNewCol = 1;
            }
            UpdateSize_Impl( nNewCol, nNewLine);
        }
    }
    else if(KEY_MOD1 == nModifier && KEY_RETURN == nKey)
    {
        m_bMod1 = TRUE;
        if(IsMouseCaptured())
            ReleaseMouse();
        EndPopupMode(FLOATWIN_POPUPMODEEND_CLOSEALL );
    }

    if(!bHandled)
        SfxPopupWindow::KeyInput(rKEvt);

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
        if(GetId() == FN_SHOW_MULTIPLE_PAGES)
        {
            aText += ' ';
            aText += String(SVX_RESSTR(RID_SVXSTR_PAGES));
        }

    }
    else
        aText = Button::GetStandardText( BUTTON_CANCEL );
    Size aTextSize( GetTextWidth( aText ), GetTextHeight() );

    DrawText( Point( (aSize.Width() - aTextSize.Width()) / 2, aSize.Height() - nTextHeight + 2 ), aText );
    DrawRect( Rectangle( 0, aSize.Height()-nTextHeight+2, (aSize.Width()-aTextSize.Width())/2-1, aSize.Height() ) );
    DrawRect( Rectangle( (aSize.Width()-aTextSize.Width())/2+aTextSize.Width(), aSize.Height()-nTextHeight+2, aSize.Width(), aSize.Height() ) );

    SetLineColor( aLineColor );
    SetFillColor();
    DrawRect( Rectangle( Point(0,0), aSize ) );
}

// -----------------------------------------------------------------------

void TableWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol && nLine )
    {
        Window* pParent = rTbx.GetParent();
        USHORT nId = GetId();
        pParent->UserEvent(SVX_EVENT_COLUM_WINDOW_EXECUTE, (void*)nId);

        Reference< XDispatchProvider > xDispatchProvider( mxFrame, UNO_QUERY );
        if ( xDispatchProvider.is() )
        {
            com::sun::star::util::URL aTargetURL;
            Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                                    rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )),
                                                  UNO_QUERY );
            aTargetURL.Complete = maCommand;
            xTrans->parseStrict( aTargetURL );
            Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, rtl::OUString(), 0 );
            if ( xDispatch.is() )
            {
                Sequence< PropertyValue > aArgs( 2 );
                aArgs[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Columns" ));
                aArgs[0].Value = makeAny( sal_Int16( nCol ));
                aArgs[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Rows" ));
                aArgs[1].Value = makeAny( sal_Int16( nLine ));

                xDispatch->dispatch( aTargetURL, aArgs );
            }
        }
    }
    else if ( IsPopupModeCanceled() )
        ReleaseMouse();
    SfxPopupWindow::PopupModeEnd();
}

// class ColumnsWindow ---------------------------------------------------

class ColumnsWindow : public SfxPopupWindow
{
private:
    Color               aLineColor;
    Color               aHighlightLineColor;
    Color               aFillColor;
    Color               aHighlightFillColor;
    long                nCol;
    long                nWidth;
    long                nMX;
    long                nTextHeight;
    BOOL                bInitialKeyInput;
    BOOL                m_bMod1;
    ToolBox&            rTbx;
    Reference< XFrame > mxFrame;
    OUString            maCommand;

    void UpdateSize_Impl( long nNewCol );
public:
                            ColumnsWindow( USHORT nId, const OUString& rCmd, ToolBox& rParentTbx, const Reference< XFrame >& rFrame );

    void                    KeyInput( const KeyEvent& rKEvt );
    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            Paint( const Rectangle& );
    virtual void            PopupModeEnd();
    virtual SfxPopupWindow* Clone() const;

    USHORT                  GetColCount() const { return (USHORT)nCol; }
};

// -----------------------------------------------------------------------

ColumnsWindow::ColumnsWindow( USHORT nId, const OUString& rCmd, ToolBox& rParentTbx, const Reference< XFrame >& rFrame ) :
    SfxPopupWindow( nId, rFrame, WB_SYSTEMWINDOW ),
    bInitialKeyInput(TRUE),
    m_bMod1(FALSE),
    rTbx(rParentTbx),
    mxFrame(rFrame),
    maCommand( rCmd )
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    aLineColor = Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    aHighlightLineColor = rStyles.GetHighlightTextColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();

    nTextHeight = GetTextHeight()+1;
    SetBackground();
    Font aFont( GetFont() );
    aFont.SetColor( aLineColor );
    aFont.SetFillColor( aFillColor );
    aFont.SetTransparent( FALSE );
    SetFont( aFont );

    nCol        = 0;
    nWidth      = 4;

    Size aLogicSize = LogicToPixel( Size( 95, 155 ), MapMode( MAP_10TH_MM ) );
    nMX = aLogicSize.Width();
    SetOutputSizePixel( Size( nMX*nWidth-1, aLogicSize.Height()+nTextHeight ) );
    StartCascading();
}

// -----------------------------------------------------------------------

SfxPopupWindow* ColumnsWindow::Clone() const
{
    return new ColumnsWindow( GetId(), maCommand, rTbx, mxFrame );
}

// -----------------------------------------------------------------------

void ColumnsWindow::MouseMove( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseMove( rMEvt );
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos = aPos;
    Point aWinPos = GetPosPixel();

    if ( rMEvt.IsEnterWindow() )
        CaptureMouse();
    else if ( aMousePos.X() < 0 || aMousePos.Y() < 0 )
    {
        nCol = 0;
        ReleaseMouse();
        Invalidate();
        return;
    }

    long    nNewCol = 0;
    if ( aPos.X() > 0 )
        nNewCol = aPos.X() / nMX + 1;
    if ( aPos.Y() < 0 )
        nNewCol = 0;
    if ( nNewCol > 20 )
        nNewCol = 20;
    UpdateSize_Impl( nNewCol );
}
/* -----------------------------21.05.2002 16:16------------------------------

 ---------------------------------------------------------------------------*/
void ColumnsWindow::UpdateSize_Impl( long nNewCol )
{
    Size    aWinSize = GetOutputSizePixel();
    long    nMinCol = 0;
    long    nMaxCol = 0;
    Point   aWinPos;// = GetPosPixel();

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
/* -----------------------------21.05.2002 16:11------------------------------

 ---------------------------------------------------------------------------*/
void ColumnsWindow::KeyInput( const KeyEvent& rKEvt )
{
    BOOL bHandled = FALSE;
    USHORT nModifier = rKEvt.GetKeyCode().GetModifier();
    USHORT nKey = rKEvt.GetKeyCode().GetCode();
    if(!nModifier)
    {
        if( KEY_LEFT == nKey || KEY_RIGHT == nKey ||
            KEY_RETURN == nKey ||KEY_ESCAPE == nKey ||
            KEY_UP == nKey)
        {
            bHandled = TRUE;
            long nNewCol = nCol;
            switch(nKey)
            {
                case KEY_LEFT :
                    if(nNewCol)
                        nNewCol--;
                break;
                case KEY_RIGHT :
                    nNewCol++;
                break;
                case KEY_RETURN :
                    if(IsMouseCaptured())
                        ReleaseMouse();
                    EndPopupMode(FLOATWIN_POPUPMODEEND_CLOSEALL );
                break;
                case KEY_ESCAPE :
                case KEY_UP :
                    EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL);
                break;
            }
            //make sure that a table can initially be created
            if(bInitialKeyInput)
            {
                bInitialKeyInput = FALSE;
                if(!nNewCol)
                    nNewCol = 1;
            }
            UpdateSize_Impl( nNewCol );
        }
    }
    else if(KEY_MOD1 == nModifier && KEY_RETURN == nKey)
    {
        m_bMod1 = TRUE;
        if(IsMouseCaptured())
            ReleaseMouse();
        EndPopupMode(FLOATWIN_POPUPMODEEND_CLOSEALL );
    }
    if(!bHandled)
        SfxPopupWindow::KeyInput(rKEvt);
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
    DrawText( Point( ( aSize.Width() - aTextSize.Width() ) / 2, aSize.Height() - nTextHeight + 2 ), aText );

    DrawRect( Rectangle( 0, aSize.Height()-nTextHeight+2, (aSize.Width()-aTextSize.Width())/2-1, aSize.Height() ) );
    DrawRect( Rectangle( (aSize.Width()-aTextSize.Width())/2+aTextSize.Width(), aSize.Height()-nTextHeight+2, aSize.Width(), aSize.Height() ) );

    SetLineColor( aLineColor );
    SetFillColor();
    DrawRect( Rectangle( Point(0,0), aSize ) );
}

// -----------------------------------------------------------------------

void ColumnsWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol )
    {
        USHORT nId = GetId();
        Window* pParent = rTbx.GetParent();
        pParent->UserEvent(SVX_EVENT_COLUM_WINDOW_EXECUTE, (void*)nId);

        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Columns" ));
        aArgs[0].Value = makeAny( sal_Int16( nCol ));
        aArgs[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Modifier" ));
        aArgs[1].Value = makeAny( sal_Int16( m_bMod1 ? KEY_MOD1 : 0 ));

        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                        maCommand,
                                        aArgs );
    }
    else if ( IsPopupModeCanceled() )
        ReleaseMouse();
    SfxPopupWindow::PopupModeEnd();
}

// class SvxTableToolBoxControl ------------------------------------------

SvxTableToolBoxControl::SvxTableToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    bEnabled( TRUE )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
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
        ToolBox& rTbx = GetToolBox();
        TableWindow* pWin = new TableWindow( GetSlotId(), m_aCommandURL, rTbx, m_xFrame );
        pWin->StartPopupMode( &rTbx, FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_NOKEYCLOSE );
        SetPopupWindow( pWin );
        return pWin;
    }
    return 0;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxTableToolBoxControl::CreatePopupWindowCascading()
{
    if ( bEnabled )
        return new TableWindow( GetSlotId(), m_aCommandURL, GetToolBox(), m_xFrame );
    return 0;
}

// -----------------------------------------------------------------------

void SvxTableToolBoxControl::StateChanged(
    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( pState && pState->ISA(SfxUInt16Item) )
    {
        INT16 nValue = static_cast< const SfxUInt16Item* >( pState )->GetValue();
        bEnabled = ( nValue != 0 );
    }
    else
        bEnabled = SFX_ITEM_DISABLED != eState;

    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId,
        ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

// class SvxColumnsToolBoxControl ------------------------------------------

SvxColumnsToolBoxControl::SvxColumnsToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
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
            pWin = new ColumnsWindow( GetSlotId(), m_aCommandURL, GetToolBox(), m_xFrame );
            pWin->StartPopupMode( &GetToolBox(),
                                  FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_NOKEYCLOSE );
            SetPopupWindow( pWin );
    }
    return pWin;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxColumnsToolBoxControl::CreatePopupWindowCascading()
{
    ColumnsWindow* pWin = 0;
    if(bEnabled)
    {
        pWin = new ColumnsWindow( GetSlotId(), m_aCommandURL, GetToolBox(), m_xFrame );
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

