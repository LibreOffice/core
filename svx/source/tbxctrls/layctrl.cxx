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

#include <string>
#include <vcl/toolbox.hxx>
#include <vcl/button.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>

#include <svx/dialogs.hrc>
#include "svx/layctrl.hxx"
#include <svx/dialmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <svtools/colorcfg.hxx>
#include <com/sun/star/util/URLTransformer.hpp>

// namespaces
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL(SvxTableToolBoxControl,SfxUInt16Item);
SFX_IMPL_TOOLBOX_CONTROL(SvxColumnsToolBoxControl,SfxUInt16Item);

// class TableWindow -----------------------------------------------------

const long TABLE_CELL_WIDTH  = 15;
const long TABLE_CELL_HEIGHT = 15;

const long TABLE_CELLS_HORIZ = 10;
const long TABLE_CELLS_VERT  = 15;

const long TABLE_POS_X = 2;
const long TABLE_POS_Y = 2;

const long TABLE_WIDTH  = TABLE_POS_X + TABLE_CELLS_HORIZ*TABLE_CELL_WIDTH;
const long TABLE_HEIGHT = TABLE_POS_Y + TABLE_CELLS_VERT*TABLE_CELL_HEIGHT;

class TableWindow : public SfxPopupWindow
{
private:
    PushButton          aTableButton;
    ::Color             aLineColor;
    ::Color             aFillColor;
    ::Color             aHighlightFillColor;
    ::Color             aBackgroundColor;
    long                nCol;
    long                nLine;
    bool                bInitialKeyInput;
    bool                m_bMod1;
    ToolBox&            rTbx;
    Reference< XFrame > mxFrame;
    OUString       maCommand;

    DECL_LINK( SelectHdl, void * );

public:
                            TableWindow( sal_uInt16                 nSlotId,
                                         const OUString&            rCmd,
                                         const OUString&            rText,
                                         ToolBox&                   rParentTbx,
                                         const Reference< XFrame >& rFrame );
                            ~TableWindow();

    void                    KeyInput( const KeyEvent& rKEvt );
    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            Paint( const Rectangle& );
    virtual void            PopupModeEnd();
    virtual SfxPopupWindow* Clone() const;

private:
    void                    Update( long nNewCol, long nNewLine );
    void                    TableDialog( const Sequence< PropertyValue >& rArgs );
    void                    CloseAndShowTableDialog();
};

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(TableWindow, SelectHdl)
{
    CloseAndShowTableDialog();
    return 0;
}

// -----------------------------------------------------------------------

TableWindow::TableWindow( sal_uInt16 nSlotId, const OUString& rCmd, const OUString& rText, ToolBox& rParentTbx, const Reference< XFrame >& rFrame ) :
    SfxPopupWindow( nSlotId, rFrame, WinBits( WB_STDPOPUP ) ),
    aTableButton( this ),
    nCol( 0 ),
    nLine( 0 ),
    rTbx(rParentTbx),
    mxFrame( rFrame ),
    maCommand( rCmd )
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;

    aLineColor = rStyles.GetShadowColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();
    aBackgroundColor = GetSettings().GetStyleSettings().GetFaceColor();

    SetBackground( aBackgroundColor );
    Font aFont = GetFont();
    aFont.SetColor( ::Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor )  );
    aFont.SetFillColor( aBackgroundColor );
    aFont.SetTransparent( sal_False );
    SetFont( aFont );

    SetText( rText );

    aTableButton.SetPosSizePixel( Point( TABLE_POS_X + TABLE_CELL_WIDTH, TABLE_HEIGHT + 5 ),
            Size( TABLE_WIDTH - TABLE_POS_X - 2*TABLE_CELL_WIDTH, 24 ) );
    aTableButton.SetText( SVX_RESSTR( RID_SVXSTR_MORE ) );
    aTableButton.SetClickHdl( LINK( this, TableWindow, SelectHdl ) );
    aTableButton.Show();

    SetOutputSizePixel( Size( TABLE_WIDTH + 3, TABLE_HEIGHT + 33 ) );
}

// -----------------------------------------------------------------------

TableWindow::~TableWindow()
{
}

// -----------------------------------------------------------------------

SfxPopupWindow* TableWindow::Clone() const
{
    return new TableWindow( GetId(), maCommand, GetText(), rTbx, mxFrame );
}

// -----------------------------------------------------------------------

void TableWindow::MouseMove( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseMove( rMEvt );
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos( aPos );

    long nNewCol = ( aMousePos.X() - TABLE_POS_X + TABLE_CELL_WIDTH ) / TABLE_CELL_WIDTH;
    long nNewLine = ( aMousePos.Y() - TABLE_POS_Y + TABLE_CELL_HEIGHT ) / TABLE_CELL_HEIGHT;

    Update( nNewCol, nNewLine );
}

// -----------------------------------------------------------------------

void TableWindow::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = false;
    sal_uInt16 nModifier = rKEvt.GetKeyCode().GetModifier();
    sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();
    if ( !nModifier )
    {
        bHandled = true;
        long nNewCol = nCol;
        long nNewLine = nLine;
        switch(nKey)
        {
            case KEY_UP:
                if ( nNewLine > 1 )
                    nNewLine--;
                else
                    EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );
                break;
            case KEY_DOWN:
                if ( nNewLine < TABLE_CELLS_VERT )
                    nNewLine++;
                else
                    CloseAndShowTableDialog();
                break;
            case KEY_LEFT:
                if ( nNewCol > 1 )
                    nNewCol--;
                else
                    EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );
                break;
            case KEY_RIGHT:
                if ( nNewCol < TABLE_CELLS_HORIZ )
                    nNewCol++;
                else
                    CloseAndShowTableDialog();
                break;
            case KEY_ESCAPE:
                EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );
                break;
            case KEY_RETURN:
                EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
                break;
            case KEY_TAB:
                CloseAndShowTableDialog();
                break;
            default:
                bHandled = false;
        }
        if ( bHandled )
        {
            //make sure that a table can initially be created
            if(bInitialKeyInput)
            {
                bInitialKeyInput = false;
                if(!nNewLine)
                    nNewLine = 1;
                if(!nNewCol)
                    nNewCol = 1;
            }
            Update( nNewCol, nNewLine );
        }
    }
    else if(KEY_MOD1 == nModifier && KEY_RETURN == nKey)
    {
        m_bMod1 = true;
        EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
    }

    if(!bHandled)
        SfxPopupWindow::KeyInput(rKEvt);
}

// -----------------------------------------------------------------------

void TableWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseButtonUp( rMEvt );
    EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
}

// -----------------------------------------------------------------------

void TableWindow::Paint( const Rectangle& )
{
    const long nSelectionWidth = TABLE_POS_X + nCol*TABLE_CELL_WIDTH;
    const long nSelectionHeight = TABLE_POS_Y + nLine*TABLE_CELL_HEIGHT;

    // the non-selected parts of the table
    SetLineColor( aLineColor );
    SetFillColor( aFillColor );
    DrawRect( Rectangle( nSelectionWidth, TABLE_POS_Y, TABLE_WIDTH, nSelectionHeight ) );
    DrawRect( Rectangle( TABLE_POS_X, nSelectionHeight, nSelectionWidth, TABLE_HEIGHT ) );
    DrawRect( Rectangle( nSelectionWidth, nSelectionHeight, TABLE_WIDTH, TABLE_HEIGHT ) );

    // the selection
    if ( nCol > 0 && nLine > 0 )
    {
        SetFillColor( aHighlightFillColor );
        DrawRect( Rectangle( TABLE_POS_X, TABLE_POS_Y,
                    nSelectionWidth, nSelectionHeight ) );
    }

    // lines inside of the table
    SetLineColor( aLineColor );
    for ( long i = 1; i < TABLE_CELLS_VERT; ++i )
        DrawLine( Point( TABLE_POS_X, TABLE_POS_Y + i*TABLE_CELL_HEIGHT ),
                  Point( TABLE_WIDTH, TABLE_POS_Y + i*TABLE_CELL_HEIGHT ) );

    for ( long i = 1; i < TABLE_CELLS_HORIZ; ++i )
        DrawLine( Point( TABLE_POS_X + i*TABLE_CELL_WIDTH, TABLE_POS_Y ),
                  Point( TABLE_POS_X + i*TABLE_CELL_WIDTH, TABLE_HEIGHT ) );

    // the text near the mouse cursor telling the table dimensions
    if ( nCol && nLine )
    {
        OUString aText;
        aText += OUString::number( nCol );
        aText += " x ";
        aText += OUString::number( nLine );
        if(GetId() == FN_SHOW_MULTIPLE_PAGES)
        {
            aText += " ";
            aText += SVX_RESSTR(RID_SVXSTR_PAGES);
        }

        Size aTextSize( GetTextWidth( aText ), GetTextHeight() );

        long nTextX = nSelectionWidth + TABLE_CELL_WIDTH;
        long nTextY = nSelectionHeight + TABLE_CELL_HEIGHT;
        const long nTipBorder = 2;

        if ( aTextSize.Width() + TABLE_POS_X + TABLE_CELL_WIDTH + 2*nTipBorder < nSelectionWidth )
            nTextX = nSelectionWidth - TABLE_CELL_WIDTH - aTextSize.Width();

        if ( aTextSize.Height() + TABLE_POS_Y + TABLE_CELL_HEIGHT + 2*nTipBorder < nSelectionHeight )
            nTextY = nSelectionHeight - TABLE_CELL_HEIGHT - aTextSize.Height();

        SetLineColor( aLineColor );
        SetFillColor( aBackgroundColor );
        DrawRect( Rectangle ( nTextX - 2*nTipBorder, nTextY - 2*nTipBorder,
                    nTextX + aTextSize.Width() + nTipBorder, nTextY + aTextSize.Height() + nTipBorder ) );

        // #i95350# force RTL output
        if ( IsRTLEnabled() )
            aText = OUString(0x202D) + aText;

        DrawText( Point( nTextX, nTextY ), aText );
    }
}

// -----------------------------------------------------------------------

void TableWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol && nLine )
    {
        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = OUString( "Columns" );
        aArgs[0].Value = makeAny( sal_Int16( nCol ));
        aArgs[1].Name = OUString( "Rows" );
        aArgs[1].Value = makeAny( sal_Int16( nLine ));

        TableDialog( aArgs );
    }

    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void TableWindow::Update( long nNewCol, long nNewLine )
{
    if ( nNewCol < 0 || nNewCol > TABLE_CELLS_HORIZ )
        nNewCol = 0;

    if ( nNewLine < 0 || nNewLine > TABLE_CELLS_VERT )
        nNewLine = 0;

    if ( nNewCol != nCol || nNewLine != nLine )
    {
        nCol = nNewCol;
        nLine = nNewLine;
        Invalidate( Rectangle( TABLE_POS_X, TABLE_POS_Y, TABLE_WIDTH, TABLE_HEIGHT ) );
    }
}

// -----------------------------------------------------------------------

void TableWindow::TableDialog( const Sequence< PropertyValue >& rArgs )
{
    Window* pParent = rTbx.GetParent();
    sal_uInt16 nId = GetId();
    pParent->UserEvent(SVX_EVENT_COLUM_WINDOW_EXECUTE, reinterpret_cast<void*>(nId));

    Reference< XDispatchProvider > xDispatchProvider( mxFrame, UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        com::sun::star::util::URL aTargetURL;
        Reference < XURLTransformer > xTrans( URLTransformer::create(::comphelper::getProcessComponentContext()) );
        aTargetURL.Complete = maCommand;
        xTrans->parseStrict( aTargetURL );

        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, rArgs );
    }
}

// -----------------------------------------------------------------------

void TableWindow::CloseAndShowTableDialog()
{
    // close the toolbar tool
    EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );

    // and open the table dialog instead
    TableDialog( Sequence< PropertyValue >() );
}

// class ColumnsWindow ---------------------------------------------------

class ColumnsWindow : public SfxPopupWindow
{
private:
    ::Color             aLineColor;
    ::Color             aHighlightLineColor;
    ::Color             aFillColor;
    ::Color             aHighlightFillColor;
    ::Color             aFaceColor;
    long                nCol;
    long                nWidth;
    long                nMX;
    long                nTextHeight;
    bool                bInitialKeyInput;
    bool                m_bMod1;
    ToolBox&            rTbx;
    Reference< XFrame > mxFrame;
    OUString            maCommand;

    void UpdateSize_Impl( long nNewCol );
public:
                            ColumnsWindow( sal_uInt16 nId, const OUString& rCmd, const OUString &rText, ToolBox& rParentTbx, const Reference< XFrame >& rFrame );

    void                    KeyInput( const KeyEvent& rKEvt );
    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            Paint( const Rectangle& );
    virtual void            PopupModeEnd();
    virtual SfxPopupWindow* Clone() const;

    sal_uInt16                  GetColCount() const { return (sal_uInt16)nCol; }
};

// -----------------------------------------------------------------------

ColumnsWindow::ColumnsWindow( sal_uInt16 nId, const OUString& rCmd, const OUString& rText, ToolBox& rParentTbx, const Reference< XFrame >& rFrame ) :
    SfxPopupWindow( nId, rFrame, WB_STDPOPUP ),
    bInitialKeyInput(true),
    m_bMod1(false),
    rTbx(rParentTbx),
    mxFrame(rFrame),
    maCommand( rCmd )
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    aLineColor = ::Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    aHighlightLineColor = rStyles.GetHighlightTextColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();
    aFaceColor = rStyles.GetFaceColor();

    nTextHeight = GetTextHeight()+1;
    SetBackground();
    Font aFont( GetFont() );
    aFont.SetColor( aLineColor );
    aFont.SetFillColor( aFaceColor );
    aFont.SetTransparent( sal_False );
    SetFont( aFont );

    nCol        = 0;
    nWidth      = 4;

    SetText( rText );

    Size aLogicSize = LogicToPixel( Size( 95, 155 ), MapMode( MAP_10TH_MM ) );
    nMX = aLogicSize.Width();
    SetOutputSizePixel( Size( nMX*nWidth-1, aLogicSize.Height()+nTextHeight ) );
    StartCascading();
}

// -----------------------------------------------------------------------

SfxPopupWindow* ColumnsWindow::Clone() const
{
    return new ColumnsWindow( GetId(), maCommand, GetText(), rTbx, mxFrame );
}

// -----------------------------------------------------------------------

void ColumnsWindow::MouseMove( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseMove( rMEvt );
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos = aPos;

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

void ColumnsWindow::UpdateSize_Impl( long nNewCol )
{
    Size    aWinSize = GetOutputSizePixel();
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

        long nMinCol = 0, nMaxCol = 0;

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

void ColumnsWindow::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = false;
    sal_uInt16 nModifier = rKEvt.GetKeyCode().GetModifier();
    sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();
    if(!nModifier)
    {
        if( KEY_LEFT == nKey || KEY_RIGHT == nKey ||
            KEY_RETURN == nKey ||KEY_ESCAPE == nKey ||
            KEY_UP == nKey)
        {
            bHandled = true;
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
                bInitialKeyInput = false;
                if(!nNewCol)
                    nNewCol = 1;
            }
            UpdateSize_Impl( nNewCol );
        }
    }
    else if(KEY_MOD1 == nModifier && KEY_RETURN == nKey)
    {
        m_bMod1 = true;
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
    SetFillColor( aFaceColor );
    OUString aText;
    if ( nCol )
        aText = OUString::number(nCol);
    else
        aText = comphelper::string::remove(Button::GetStandardText(BUTTON_CANCEL), '~');

    Size aTextSize(GetTextWidth( aText ), GetTextHeight());
    DrawText( Point( ( aSize.Width() - aTextSize.Width() ) / 2, aSize.Height() - nTextHeight + 2 ), aText );

    DrawRect( Rectangle( 0, aSize.Height()-nTextHeight+2, (aSize.Width()-aTextSize.Width())/2-1, aSize.Height() ) );
    DrawRect( Rectangle( (aSize.Width()-aTextSize.Width())/2+aTextSize.Width(), aSize.Height()-nTextHeight+2, aSize.Width(), aSize.Height() ) );

    SetLineColor( aLineColor );
    SetFillColor();
    DrawRect( Rectangle( 0, 0, aSize.Width() - 1, aSize.Height() - nTextHeight + 1 ) );
}

// -----------------------------------------------------------------------

void ColumnsWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol )
    {
        sal_uInt16 nId = GetId();
        Window* pParent = rTbx.GetParent();
        pParent->UserEvent(SVX_EVENT_COLUM_WINDOW_EXECUTE, reinterpret_cast<void*>(nId));

        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = OUString( "Columns" );
        aArgs[0].Value = makeAny( sal_Int16( nCol ));
        aArgs[1].Name = OUString( "Modifier" );
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

SvxTableToolBoxControl::SvxTableToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    bEnabled( sal_True )
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
        TableWindow* pWin = new TableWindow( GetSlotId(), m_aCommandURL, GetToolBox().GetItemText( GetId() ), rTbx, m_xFrame );
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
        return new TableWindow( GetSlotId(), m_aCommandURL, GetToolBox().GetItemText( GetId() ), GetToolBox(), m_xFrame );
    return 0;
}

// -----------------------------------------------------------------------

void SvxTableToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( pState && pState->ISA(SfxUInt16Item) )
    {
        sal_Int16 nValue = static_cast< const SfxUInt16Item* >( pState )->GetValue();
        bEnabled = ( nValue != 0 );
    }
    else
        bEnabled = SFX_ITEM_DISABLED != eState;

    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId,
        ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

// class SvxColumnsToolBoxControl ------------------------------------------

SvxColumnsToolBoxControl::SvxColumnsToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
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
            pWin = new ColumnsWindow( GetSlotId(), m_aCommandURL, GetToolBox().GetItemText( GetId() ), GetToolBox(), m_xFrame );
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
        pWin = new ColumnsWindow( GetSlotId(), m_aCommandURL, GetToolBox().GetItemText( GetId() ), GetToolBox(), m_xFrame );
    }
    return pWin;
}

void SvxColumnsToolBoxControl::StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    bEnabled = SFX_ITEM_DISABLED != eState;
    SfxToolBoxControl::StateChanged(nSID,   eState, pState );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
