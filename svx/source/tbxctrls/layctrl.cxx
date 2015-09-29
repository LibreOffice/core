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
#include <vcl/settings.hxx>
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

class TableWindow : public SfxPopupWindow
{
private:
    VclPtr<PushButton>  aTableButton;
    ::Color             aLineColor;
    ::Color             aFillColor;
    ::Color             aHighlightFillColor;
    ::Color             aBackgroundColor;
    long                nCol;
    long                nLine;
    bool                bInitialKeyInput;
    bool                m_bMod1;
    Reference< XFrame > mxFrame;
    OUString       maCommand;

    static const long TABLE_CELLS_HORIZ;
    static const long TABLE_CELLS_VERT;

    long mnTableCellWidth;
    long mnTableCellHeight;

    long mnTablePosX;
    long mnTablePosY;

    long mnTableWidth;
    long mnTableHeight;

    DECL_LINK_TYPED( SelectHdl, Button*, void );

public:
                            TableWindow( sal_uInt16                 nSlotId,
                                         const OUString&            rCmd,
                                         const OUString&            rText,
                                         const Reference< XFrame >& rFrame );
                            virtual ~TableWindow();
    virtual void            dispose() SAL_OVERRIDE;

    void                    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void            MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void            MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void            Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& ) SAL_OVERRIDE;
    virtual void            PopupModeEnd() SAL_OVERRIDE;

private:
    void                    Update( long nNewCol, long nNewLine );
    void                    TableDialog( const Sequence< PropertyValue >& rArgs );
    void                    CloseAndShowTableDialog();
};

const long TableWindow::TABLE_CELLS_HORIZ = 10;
const long TableWindow::TABLE_CELLS_VERT = 15;



IMPL_LINK_NOARG_TYPED(TableWindow, SelectHdl, Button*, void)
{
    CloseAndShowTableDialog();
}



TableWindow::TableWindow( sal_uInt16 nSlotId, const OUString& rCmd, const OUString& rText,
                          const Reference< XFrame >& rFrame )
    : SfxPopupWindow( nSlotId, rFrame, WinBits( WB_STDPOPUP ) )
    , aTableButton( VclPtr<PushButton>::Create(this) )
    , nCol( 0 )
    , nLine( 0 )
    , bInitialKeyInput(false)
    , m_bMod1(false)
    , mxFrame( rFrame )
    , maCommand( rCmd )
    , mnTablePosX(2)
    , mnTablePosY(2)
{
    mnTableCellWidth  = 15 * GetDPIScaleFactor();
    mnTableCellHeight = 15 * GetDPIScaleFactor();

    mnTableWidth  = mnTablePosX + TABLE_CELLS_HORIZ*mnTableCellWidth;
    mnTableHeight = mnTablePosY + TABLE_CELLS_VERT*mnTableCellHeight;

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;

    aLineColor = rStyles.GetShadowColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();
    aBackgroundColor = GetSettings().GetStyleSettings().GetFaceColor();

    SetBackground( aBackgroundColor );
    vcl::Font aFont = GetFont();
    aFont.SetColor( ::Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor )  );
    aFont.SetFillColor( aBackgroundColor );
    aFont.SetTransparent( false );
    SetFont( aFont );

    SetText( rText );

    aTableButton->SetPosSizePixel( Point( mnTablePosX, mnTableHeight + 5 ),
            Size( mnTableWidth - mnTablePosX, 24 ) );
    aTableButton->SetText( SVX_RESSTR( RID_SVXSTR_MORE ) );
    aTableButton->SetClickHdl( LINK( this, TableWindow, SelectHdl ) );
    aTableButton->Show();

    SetOutputSizePixel( Size( mnTableWidth + 3, mnTableHeight + 33 ) );
}



TableWindow::~TableWindow()
{
    disposeOnce();
}

void TableWindow::dispose()
{
    aTableButton.disposeAndClear();
    SfxPopupWindow::dispose();
}

void TableWindow::MouseMove( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseMove( rMEvt );
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos( aPos );

    long nNewCol = ( aMousePos.X() - mnTablePosX + mnTableCellWidth ) / mnTableCellWidth;
    long nNewLine = ( aMousePos.Y() - mnTablePosY + mnTableCellHeight ) / mnTableCellHeight;

    Update( nNewCol, nNewLine );
}



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
                    EndPopupMode( FloatWinPopupEndFlags::Cancel );
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
                    EndPopupMode( FloatWinPopupEndFlags::Cancel );
                break;
            case KEY_RIGHT:
                if ( nNewCol < TABLE_CELLS_HORIZ )
                    nNewCol++;
                else
                    CloseAndShowTableDialog();
                break;
            case KEY_ESCAPE:
                EndPopupMode( FloatWinPopupEndFlags::Cancel );
                break;
            case KEY_RETURN:
                EndPopupMode( FloatWinPopupEndFlags::CloseAll );
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
        EndPopupMode( FloatWinPopupEndFlags::CloseAll );
    }

    if(!bHandled)
        SfxPopupWindow::KeyInput(rKEvt);
}



void TableWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseButtonUp( rMEvt );
    EndPopupMode( FloatWinPopupEndFlags::CloseAll );
}



void TableWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    const long nSelectionWidth = mnTablePosX + nCol * mnTableCellWidth;
    const long nSelectionHeight = mnTablePosY + nLine * mnTableCellHeight;

    // the non-selected parts of the table
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(aFillColor);
    rRenderContext.DrawRect(Rectangle(nSelectionWidth, mnTablePosY, mnTableWidth, nSelectionHeight));
    rRenderContext.DrawRect(Rectangle(mnTablePosX, nSelectionHeight, nSelectionWidth, mnTableHeight));
    rRenderContext.DrawRect(Rectangle(nSelectionWidth, nSelectionHeight, mnTableWidth, mnTableHeight));

    // the selection
    if (nCol > 0 && nLine > 0)
    {
        rRenderContext.SetFillColor(aHighlightFillColor);
        rRenderContext.DrawRect(Rectangle(mnTablePosX, mnTablePosY, nSelectionWidth, nSelectionHeight));
    }

    // lines inside of the table
    rRenderContext.SetLineColor(aLineColor);
    for (long i = 1; i < TABLE_CELLS_VERT; ++i)
    {
        rRenderContext.DrawLine(Point(mnTablePosX, mnTablePosY + i*mnTableCellHeight),
                                Point(mnTableWidth, mnTablePosY + i*mnTableCellHeight));
    }

    for (long i = 1; i < TABLE_CELLS_HORIZ; ++i)
    {
        rRenderContext.DrawLine(Point( mnTablePosX + i*mnTableCellWidth, mnTablePosY),
                                Point( mnTablePosX + i*mnTableCellWidth, mnTableHeight));
    }

    // the text near the mouse cursor telling the table dimensions
    if (nCol && nLine)
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

        Size aTextSize(rRenderContext.GetTextWidth(aText), rRenderContext.GetTextHeight());

        long nTextX = nSelectionWidth + mnTableCellWidth;
        long nTextY = nSelectionHeight + mnTableCellHeight;
        const long nTipBorder = 2;

        if (aTextSize.Width() + mnTablePosX + mnTableCellWidth + 2 * nTipBorder < nSelectionWidth)
            nTextX = nSelectionWidth - mnTableCellWidth - aTextSize.Width();

        if (aTextSize.Height() + mnTablePosY + mnTableCellHeight + 2 * nTipBorder < nSelectionHeight)
            nTextY = nSelectionHeight - mnTableCellHeight - aTextSize.Height();

        rRenderContext.SetLineColor(aLineColor);
        rRenderContext.SetFillColor(aBackgroundColor);
        rRenderContext.DrawRect(Rectangle(nTextX - 2 * nTipBorder,
                                          nTextY - 2 * nTipBorder,
                                          nTextX + aTextSize.Width() + nTipBorder,
                                          nTextY + aTextSize.Height() + nTipBorder));

        // #i95350# force RTL output
        if (IsRTLEnabled())
            aText = OUString(sal_Unicode(0x202D)) + aText;

        rRenderContext.DrawText(Point(nTextX, nTextY), aText);
    }
}



void TableWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol && nLine )
    {
        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = "Columns";
        aArgs[0].Value = makeAny( sal_Int16( nCol ));
        aArgs[1].Name = "Rows";
        aArgs[1].Value = makeAny( sal_Int16( nLine ));

        TableDialog( aArgs );
    }

    SfxPopupWindow::PopupModeEnd();
}



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
        Invalidate(Rectangle(mnTablePosX, mnTablePosY, mnTableWidth, mnTableHeight));
    }
}



void TableWindow::TableDialog( const Sequence< PropertyValue >& rArgs )
{
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



void TableWindow::CloseAndShowTableDialog()
{
    // close the toolbar tool
    EndPopupMode( FloatWinPopupEndFlags::Cancel );

    // and open the table dialog instead
    TableDialog( Sequence< PropertyValue >() );
}

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
    Reference< XFrame > mxFrame;
    OUString            maCommand;

    void UpdateSize_Impl( long nNewCol );
public:
                            ColumnsWindow( sal_uInt16 nId, const OUString& rCmd, const OUString &rText, const Reference< XFrame >& rFrame );

    void                    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void            MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void            MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void            MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void            Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& ) SAL_OVERRIDE;
    virtual void            PopupModeEnd() SAL_OVERRIDE;
};



ColumnsWindow::ColumnsWindow( sal_uInt16 nId, const OUString& rCmd, const OUString& rText, const Reference< XFrame >& rFrame ) :
    SfxPopupWindow( nId, rFrame, WB_STDPOPUP ),
    bInitialKeyInput(true),
    m_bMod1(false),
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
    vcl::Font aFont( GetFont() );
    aFont.SetColor( aLineColor );
    aFont.SetFillColor( aFaceColor );
    aFont.SetTransparent( false );
    SetFont( aFont );

    nCol        = 0;
    nWidth      = 4;

    SetText( rText );

    Size aLogicSize = LogicToPixel( Size( 95, 155 ), MapMode( MAP_10TH_MM ) );
    nMX = aLogicSize.Width();
    SetOutputSizePixel( Size( nMX*nWidth-1, aLogicSize.Height()+nTextHeight ) );
    StartCascading();
}



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
                    EndPopupMode(FloatWinPopupEndFlags::CloseAll );
                break;
                case KEY_ESCAPE :
                case KEY_UP :
                    EndPopupMode( FloatWinPopupEndFlags::Cancel);
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
        EndPopupMode(FloatWinPopupEndFlags::CloseAll );
    }
    if(!bHandled)
        SfxPopupWindow::KeyInput(rKEvt);
}



void ColumnsWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    SfxPopupWindow::MouseButtonUp( rMEvt );
    ReleaseMouse();

    if ( IsInPopupMode() )
        EndPopupMode( FloatWinPopupEndFlags::CloseAll );
}



void ColumnsWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    long i;
    long nLineWidth;
    Size aSize(GetOutputSizePixel());

    for (i = 0; i < nWidth; i++)
    {
        if (i < nCol)
        {
            rRenderContext.SetLineColor(aHighlightLineColor);
            rRenderContext.SetFillColor(aHighlightFillColor);
        }
        else
        {
            rRenderContext.SetLineColor(aLineColor);
            rRenderContext.SetFillColor(aFillColor);
        }

        rRenderContext.DrawRect(Rectangle(i * nMX - 1, -1, i * nMX + nMX, aSize.Height() - nTextHeight + 1));

        long j = 4;
        while (j < aSize.Height() - nTextHeight - 4)
        {
            if (!(j % 16))
                nLineWidth = 10;
            else
                nLineWidth = 4;
            rRenderContext.DrawLine(Point(i * nMX + 4, j), Point(i * nMX + nMX - nLineWidth - 4, j));
            j += 4;
        }
    }

    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(aFaceColor);
    OUString aText;
    if (nCol)
        aText = OUString::number(nCol);
    else
        aText = comphelper::string::remove(Button::GetStandardText(StandardButtonType::Cancel), '~');

    Size aTextSize(rRenderContext.GetTextWidth(aText), rRenderContext.GetTextHeight());
    rRenderContext.DrawText(Point((aSize.Width() - aTextSize.Width()) / 2, aSize.Height() - nTextHeight + 2), aText);

    rRenderContext.DrawRect(Rectangle(0,
                                      aSize.Height() - nTextHeight + 2,
                                      (aSize.Width() - aTextSize.Width()) / 2 - 1,
                                      aSize.Height()));

    rRenderContext.DrawRect(Rectangle((aSize.Width() - aTextSize.Width()) / 2 + aTextSize.Width(),
                                      aSize.Height() - nTextHeight + 2,
                                      aSize.Width(),
                                      aSize.Height()));

    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor();
    rRenderContext.DrawRect(Rectangle( 0, 0, aSize.Width() - 1, aSize.Height() - nTextHeight + 1));
}



void ColumnsWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol )
    {
        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = "Columns";
        aArgs[0].Value = makeAny( sal_Int16( nCol ));
        aArgs[1].Name = "Modifier";
        aArgs[1].Value = makeAny( sal_Int16( m_bMod1 ? KEY_MOD1 : 0 ));

        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                        maCommand,
                                        aArgs );
    }
    else if ( IsPopupModeCanceled() )
        ReleaseMouse();
    SfxPopupWindow::PopupModeEnd();
}

SvxTableToolBoxControl::SvxTableToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    bEnabled( true )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}



SvxTableToolBoxControl::~SvxTableToolBoxControl()
{
}



VclPtr<SfxPopupWindow> SvxTableToolBoxControl::CreatePopupWindow()
{
    if ( bEnabled )
    {
        ToolBox& rTbx = GetToolBox();
        VclPtr<TableWindow> pWin = VclPtr<TableWindow>::Create( GetSlotId(), m_aCommandURL, GetToolBox().GetItemText( GetId() ), m_xFrame );
        pWin->StartPopupMode( &rTbx, FloatWinPopupFlags::GrabFocus|FloatWinPopupFlags::NoKeyClose );
        SetPopupWindow( pWin );
        return pWin;
    }
    return 0;
}

void SvxTableToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( pState && dynamic_cast<const SfxUInt16Item* >(pState) !=  nullptr )
    {
        sal_Int16 nValue = static_cast< const SfxUInt16Item* >( pState )->GetValue();
        bEnabled = ( nValue != 0 );
    }
    else
        bEnabled = SfxItemState::DISABLED != eState;

    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SfxItemState::DISABLED != eState );
    rTbx.SetItemState( nId,
        ( SfxItemState::DONTCARE == eState ) ? TRISTATE_INDET : TRISTATE_FALSE );
}

SvxColumnsToolBoxControl::SvxColumnsToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    : SfxToolBoxControl(nSlotId, nId, rTbx)
    , bEnabled(false)
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}



SvxColumnsToolBoxControl::~SvxColumnsToolBoxControl()
{
}



VclPtr<SfxPopupWindow> SvxColumnsToolBoxControl::CreatePopupWindow()
{
    ColumnsWindow* pWin = 0;
    if(bEnabled)
    {
            pWin = VclPtr<ColumnsWindow>::Create( GetSlotId(), m_aCommandURL, GetToolBox().GetItemText( GetId() ), m_xFrame );
            pWin->StartPopupMode( &GetToolBox(),
                                  FloatWinPopupFlags::GrabFocus|FloatWinPopupFlags::NoKeyClose );
            SetPopupWindow( pWin );
    }
    return pWin;
}

void SvxColumnsToolBoxControl::StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    bEnabled = SfxItemState::DISABLED != eState;
    SfxToolBoxControl::StateChanged(nSID,   eState, pState );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
