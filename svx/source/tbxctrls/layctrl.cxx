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
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>

#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/layctrl.hxx>
#include <svx/dialmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/toolbarmenu.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>

// namespaces
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL(SvxColumnsToolBoxControl,SfxUInt16Item);

namespace {

class TableWindow final : public svtools::ToolbarPopup
{
private:
    VclPtr<PushButton>  aTableButton;
    VclPtr<VclDrawingArea> aDrawingArea;
    ::Color             aLineColor;
    ::Color             aFillColor;
    ::Color             aHighlightFillColor;
    ::Color             aBackgroundColor;
    long                nCol;
    long                nLine;
    OUString const      maCommand;
    rtl::Reference<SvxTableToolBoxControl> mxControl;

    static const long TABLE_CELLS_HORIZ;
    static const long TABLE_CELLS_VERT;

    long mnTableCellWidth;
    long mnTableCellHeight;

    long mnTableWidth;
    long mnTableHeight;

    DECL_LINK( SelectHdl, Button*, void );

public:
    TableWindow( SvxTableToolBoxControl* pControl, vcl::Window* pParent,
                 const OUString&            rCmd,
                 const OUString&            rText );
    virtual ~TableWindow() override;
    virtual void            dispose() override;

    DECL_LINK(KeyInputHdl, const KeyEvent& rKEvt, bool);
    DECL_LINK(MouseMoveHdl, const MouseEvent&, bool);
    DECL_LINK(MouseButtonUpHdl, const MouseEvent&, bool);
    typedef std::pair<vcl::RenderContext&, const tools::Rectangle&> target_and_area;
    DECL_LINK(PaintHdl, target_and_area, void);
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;

private:
    void                    Update( long nNewCol, long nNewLine );
    void                    InsertTable();
    void                    TableDialog( const Sequence< PropertyValue >& rArgs );
    void                    CloseAndShowTableDialog();
};

}

const long TableWindow::TABLE_CELLS_HORIZ = 10;
const long TableWindow::TABLE_CELLS_VERT = 15;


IMPL_LINK_NOARG(TableWindow, SelectHdl, Button*, void)
{
    CloseAndShowTableDialog();
}

TableWindow::TableWindow( SvxTableToolBoxControl* pControl, vcl::Window* pParent, const OUString& rCmd,
                          const OUString& rText )
    : ToolbarPopup(pControl->getFrameInterface(), pParent, "TableWindow", "svx/ui/tablewindow.ui")
    , aTableButton(get<PushButton>("moreoptions"))
    , aDrawingArea(get<VclDrawingArea>("table"))
    , nCol( 0 )
    , nLine( 0 )
    , maCommand( rCmd )
    , mxControl(pControl)
{
    float fScaleFactor = GetDPIScaleFactor();

    mnTableCellWidth  = 15 * fScaleFactor;
    mnTableCellHeight = 15 * fScaleFactor;

    mnTableWidth  = TABLE_CELLS_HORIZ*mnTableCellWidth;
    mnTableHeight = TABLE_CELLS_VERT*mnTableCellHeight;

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;

    aLineColor = rStyles.GetShadowColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();
    aBackgroundColor = aDrawingArea->GetSettings().GetStyleSettings().GetFaceColor();

    aDrawingArea->SetBackground( aBackgroundColor );
    vcl::Font aFont = aDrawingArea->GetFont();
    aFont.SetColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    aFont.SetFillColor( aBackgroundColor );
    aFont.SetTransparent( false );
    aDrawingArea->SetFont( aFont );

    aDrawingArea->SetKeyPressHdl(LINK(this, TableWindow, KeyInputHdl));
    aDrawingArea->SetMouseMoveHdl(LINK(this, TableWindow, MouseMoveHdl));
    aDrawingArea->SetMouseReleaseHdl(LINK(this, TableWindow, MouseButtonUpHdl));
    aDrawingArea->SetPaintHdl(LINK(this, TableWindow, PaintHdl));

    SetText( rText );

    // if parent window is a toolbox only display table button when mouse activated
    ToolBox* pToolBox = nullptr;
    if (pParent->GetType() == WindowType::TOOLBOX)
        pToolBox = dynamic_cast<ToolBox*>( pParent );
    if ( !pToolBox || !pToolBox->IsKeyEvent() )
    {
        aTableButton->SetText( SvxResId( RID_SVXSTR_MORE ) );
        aTableButton->SetClickHdl( LINK( this, TableWindow, SelectHdl ) );
        aTableButton->Show();
    }

    // + 1 to leave space to draw the right/bottom borders
    aDrawingArea->set_width_request(mnTableWidth + 1);
    aDrawingArea->set_height_request(mnTableHeight + 1);
}

TableWindow::~TableWindow()
{
    disposeOnce();
}

void TableWindow::dispose()
{
    aTableButton.disposeAndClear();
    aDrawingArea.disposeAndClear();
    ToolbarPopup::dispose();
}

IMPL_LINK(TableWindow, MouseMoveHdl, const MouseEvent&, rMEvt, bool)
{
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos( aPos );

    long nNewCol = ( aMousePos.X() + mnTableCellWidth ) / mnTableCellWidth;
    long nNewLine = ( aMousePos.Y() + mnTableCellHeight ) / mnTableCellHeight;

    Update( nNewCol, nNewLine );

    return true;
}

IMPL_LINK(TableWindow, KeyInputHdl, const KeyEvent&, rKEvt, bool)
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
                    EndPopupMode();
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
                    EndPopupMode();
                break;
            case KEY_RIGHT:
                if ( nNewCol < TABLE_CELLS_HORIZ )
                    nNewCol++;
                else
                    CloseAndShowTableDialog();
                break;
            case KEY_ESCAPE:
                EndPopupMode();
                break;
            case KEY_RETURN:
                InsertTable();
                EndPopupMode();
                GrabFocusToDocument();
                return true;
            default:
                bHandled = false;
        }
        if ( bHandled )
        {
            Update( nNewCol, nNewLine );
        }
    }
    else if(KEY_MOD1 == nModifier && KEY_RETURN == nKey)
    {
        InsertTable();
        EndPopupMode();
        GrabFocusToDocument();
        return true;
    }

    return bHandled;
}

IMPL_LINK_NOARG(TableWindow, MouseButtonUpHdl, const MouseEvent&, bool)
{
    InsertTable();
    EndPopupMode();
    GrabFocusToDocument();

    return true;
}

IMPL_LINK(TableWindow, PaintHdl, target_and_area, aPayload, void)
{
    vcl::RenderContext& rRenderContext = aPayload.first;

    const long nSelectionWidth = nCol * mnTableCellWidth;
    const long nSelectionHeight = nLine * mnTableCellHeight;

    // the non-selected parts of the table
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(aFillColor);
    rRenderContext.DrawRect(tools::Rectangle(nSelectionWidth, 0, mnTableWidth, nSelectionHeight));
    rRenderContext.DrawRect(tools::Rectangle(0, nSelectionHeight, nSelectionWidth, mnTableHeight));
    rRenderContext.DrawRect(tools::Rectangle(nSelectionWidth, nSelectionHeight, mnTableWidth, mnTableHeight));

    // the selection
    if (nCol > 0 && nLine > 0)
    {
        rRenderContext.SetFillColor(aHighlightFillColor);
        rRenderContext.DrawRect(tools::Rectangle(0, 0, nSelectionWidth, nSelectionHeight));
    }

    // lines inside of the table
    rRenderContext.SetLineColor(aLineColor);
    for (long i = 1; i < TABLE_CELLS_VERT; ++i)
    {
        rRenderContext.DrawLine(Point(0, i*mnTableCellHeight),
                                Point(mnTableWidth, i*mnTableCellHeight));
    }

    for (long i = 1; i < TABLE_CELLS_HORIZ; ++i)
    {
        rRenderContext.DrawLine(Point( i*mnTableCellWidth, 0),
                                Point( i*mnTableCellWidth, mnTableHeight));
    }

    // the text near the mouse cursor telling the table dimensions
    if (!nCol || !nLine)
        return;

    OUString aText = OUString::number( nCol ) + " x " + OUString::number( nLine );
    if (maCommand == ".uno:ShowMultiplePages")
    {
        aText += " " + SvxResId(RID_SVXSTR_PAGES);
    }

    Size aTextSize(rRenderContext.GetTextWidth(aText), rRenderContext.GetTextHeight());

    long nTextX = nSelectionWidth + mnTableCellWidth;
    long nTextY = nSelectionHeight + mnTableCellHeight;
    const long nTipBorder = 2;

    if (aTextSize.Width() + mnTableCellWidth + 2 * nTipBorder < nSelectionWidth)
        nTextX = nSelectionWidth - mnTableCellWidth - aTextSize.Width();

    if (aTextSize.Height() + mnTableCellHeight + 2 * nTipBorder < nSelectionHeight)
        nTextY = nSelectionHeight - mnTableCellHeight - aTextSize.Height();

    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(aBackgroundColor);
    rRenderContext.DrawRect(tools::Rectangle(nTextX - 2 * nTipBorder,
                                      nTextY - 2 * nTipBorder,
                                      nTextX + aTextSize.Width() + nTipBorder,
                                      nTextY + aTextSize.Height() + nTipBorder));

    // #i95350# force RTL output
    if (IsRTLEnabled())
        aText = u"\u202D" + aText;

    rRenderContext.DrawText(Point(nTextX, nTextY), aText);
}

void TableWindow::InsertTable()
{
    if (nCol && nLine)
    {
        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = "Columns";
        aArgs[0].Value <<= sal_Int16( nCol );
        aArgs[1].Name = "Rows";
        aArgs[1].Value <<= sal_Int16( nLine );

        TableDialog( aArgs );
    }
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
        aDrawingArea->Invalidate(tools::Rectangle(0, 0, mnTableWidth, mnTableHeight));
    }
}

void TableWindow::TableDialog( const Sequence< PropertyValue >& rArgs )
{
    Reference< XDispatchProvider > xDispatchProvider( mxFrame, UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        css::util::URL aTargetURL;
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
    EndPopupMode();

    // and open the table dialog instead
    TableDialog( Sequence< PropertyValue >() );
}

bool TableWindow::EventNotify( NotifyEvent& rNEvt )
{
    // handle table button key input
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const vcl::KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
        const sal_uInt16 nCode = rKey.GetCode();
        if ( nCode != KEY_RETURN && nCode != KEY_SPACE && nCode != KEY_ESCAPE )
        {
            return true;
        }
    }
    return ToolbarPopup::EventNotify( rNEvt );
}

namespace {

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
    OUString const            maCommand;

    void UpdateSize_Impl( long nNewCol );
public:
                            ColumnsWindow( sal_uInt16 nId, vcl::Window* pParent, const OUString& rCmd,
                                           const OUString &rText, const Reference< XFrame >& rFrame );

    void                    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void            MouseMove( const MouseEvent& rMEvt ) override;
    virtual void            MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void            MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void            Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& ) override;
    virtual void            PopupModeEnd() override;
};

}

ColumnsWindow::ColumnsWindow( sal_uInt16 nId, vcl::Window* pParent, const OUString& rCmd,
                              const OUString& rText, const Reference< XFrame >& rFrame ) :
    SfxPopupWindow( nId, pParent, rFrame, WB_STDPOPUP ),
    bInitialKeyInput(true),
    m_bMod1(false),
    mxFrame(rFrame),
    maCommand( rCmd )
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    aLineColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
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

    Size aLogicSize = LogicToPixel( Size( 95, 155 ), MapMode( MapUnit::Map10thMM ) );
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
                static_cast<short>(aWinPos.X()+(nMX*nWidth-1)) >= aMaxPos.X()-3 )
            nWidth--;

        if ( nNewCol > nWidth )
            nNewCol = nWidth;

        Invalidate( tools::Rectangle( 0, aWinSize.Height()-nTextHeight+2,
                               aWinSize.Width(), aWinSize.Height() ) );
        SetOutputSizePixel( Size( nMX*nWidth-1, aWinSize.Height() ) );
    }


    if ( nNewCol != nCol )
    {
        Invalidate( tools::Rectangle( 0, aWinSize.Height()-nTextHeight+2,
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

        Invalidate( tools::Rectangle( nMinCol*nMX-1, 0,
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


void ColumnsWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
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

        rRenderContext.DrawRect(tools::Rectangle(i * nMX - 1, -1, i * nMX + nMX, aSize.Height() - nTextHeight + 1));

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
        aText = GetStandardText(StandardButtonType::Cancel).replaceAll("~", "");

    Size aTextSize(rRenderContext.GetTextWidth(aText), rRenderContext.GetTextHeight());
    rRenderContext.DrawText(Point((aSize.Width() - aTextSize.Width()) / 2, aSize.Height() - nTextHeight + 2), aText);

    rRenderContext.DrawRect(tools::Rectangle(0,
                                      aSize.Height() - nTextHeight + 2,
                                      (aSize.Width() - aTextSize.Width()) / 2 - 1,
                                      aSize.Height()));

    rRenderContext.DrawRect(tools::Rectangle((aSize.Width() - aTextSize.Width()) / 2 + aTextSize.Width(),
                                      aSize.Height() - nTextHeight + 2,
                                      aSize.Width(),
                                      aSize.Height()));

    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor();
    rRenderContext.DrawRect(tools::Rectangle( 0, 0, aSize.Width() - 1, aSize.Height() - nTextHeight + 1));
}


void ColumnsWindow::PopupModeEnd()
{
    if ( !IsPopupModeCanceled() && nCol )
    {
        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = "Columns";
        aArgs[0].Value <<= sal_Int16( nCol );
        aArgs[1].Name = "Modifier";
        aArgs[1].Value <<= sal_Int16( m_bMod1 ? KEY_MOD1 : 0 );

        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                        maCommand,
                                        aArgs );
    }
    else if ( IsPopupModeCanceled() )
        ReleaseMouse();
    SfxPopupWindow::PopupModeEnd();
}

SvxTableToolBoxControl::SvxTableToolBoxControl(const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
{
}

void SvxTableToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    PopupWindowController::initialize(rArguments);

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if (getToolboxId(nId, &pToolBox) && pToolBox->GetItemCommand(nId) == m_aCommandURL)
        pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWNONLY | pToolBox->GetItemBits(nId));
}

SvxTableToolBoxControl::~SvxTableToolBoxControl()
{
}

VclPtr<vcl::Window> SvxTableToolBoxControl::createPopupWindow(vcl::Window* pParent)
{
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    bool bToolBox = getToolboxId(nId, &pToolBox);
    return VclPtr<TableWindow>::Create(this, pParent, m_aCommandURL, bToolBox ? pToolBox->GetItemText(nId) : OUString());
}

OUString SvxTableToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.TableToolBoxControl";
}

css::uno::Sequence<OUString> SvxTableToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_TableToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire(new SvxTableToolBoxControl(rContext));
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
    VclPtr<ColumnsWindow> pWin;
    if(bEnabled)
    {
            pWin = VclPtr<ColumnsWindow>::Create( GetSlotId(), &GetToolBox(), m_aCommandURL, GetToolBox().GetItemText( GetId() ), m_xFrame );
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
