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

#include <vcl/customweld.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <svx/strings.hrc>
#include <svx/layctrl.hxx>
#include <svx/dialmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/toolbarmenu.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

// namespaces
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;

namespace {

class TableWidget final : public weld::CustomWidgetController
{
private:
    rtl::Reference<SvxTableToolBoxControl> mxControl;
    OUString maCommand;

    tools::Long                nCol;
    tools::Long                nLine;

    static const tools::Long TABLE_CELLS_HORIZ;
    static const tools::Long TABLE_CELLS_VERT;

    tools::Long mnTableCellWidth;
    tools::Long mnTableCellHeight;

    tools::Long mnTableWidth;
    tools::Long mnTableHeight;

    ::Color             aFontColor;
    ::Color             aLineColor;
    ::Color             aFillColor;
    ::Color             aHighlightFillColor;
    ::Color             aBackgroundColor;

    void                Update(tools::Long nNewCol, tools::Long nNewLine);
    void                InsertTable();

public:
    TableWidget(SvxTableToolBoxControl* pControl, const OUString& rCommand);

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual bool KeyInput(const KeyEvent&) override;
    virtual bool MouseButtonDown(const MouseEvent&) override;
    virtual bool MouseMove(const MouseEvent&) override;
    virtual bool MouseButtonUp(const MouseEvent&) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
};

class TableWindow final : public WeldToolbarPopup
{
private:
    std::unique_ptr<weld::Button> mxTableButton;
    std::unique_ptr<TableWidget> mxTableWidget;
    std::unique_ptr<weld::CustomWeld> mxTableWidgetWin;
    rtl::Reference<SvxTableToolBoxControl> mxControl;

    DECL_LINK(SelectHdl, weld::Button&, void);

public:
    TableWindow( SvxTableToolBoxControl* pControl, weld::Widget* pParent,
                 const OUString& rCmd);
    virtual void GrabFocus() override
    {
        mxTableWidget->GrabFocus();
    }
};

}

const tools::Long TableWidget::TABLE_CELLS_HORIZ = 10;
const tools::Long TableWidget::TABLE_CELLS_VERT = 15;

IMPL_LINK_NOARG(TableWindow, SelectHdl, weld::Button&, void)
{
    mxControl->CloseAndShowTableDialog();
}

TableWindow::TableWindow(SvxTableToolBoxControl* pControl, weld::Widget* pParent, const OUString& rCmd)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/tablewindow.ui", "TableWindow")
    , mxTableButton(m_xBuilder->weld_button("moreoptions"))
    , mxTableWidget(new TableWidget(pControl, rCmd))
    , mxTableWidgetWin(new weld::CustomWeld(*m_xBuilder, "table", *mxTableWidget))
    , mxControl(pControl)
{
    mxTableButton->set_label( SvxResId( RID_SVXSTR_MORE ) );
    mxTableButton->connect_clicked( LINK( this, TableWindow, SelectHdl ) );
    mxTableButton->show();
}

TableWidget::TableWidget(SvxTableToolBoxControl* pControl, const OUString& rCommand)
    : mxControl(pControl)
    , maCommand(rCommand)
    , nCol(0)
    , nLine(0)
    , mnTableCellWidth(0)
    , mnTableCellHeight(0)
    , mnTableWidth(0)
    , mnTableHeight(0)
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    aFontColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    aLineColor = rStyles.GetShadowColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();
    aBackgroundColor = rStyles.GetFaceColor();
}

void TableWidget::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    float fScaleFactor = pDrawingArea->get_ref_device().GetDPIScaleFactor();

    mnTableCellWidth  = 15 * fScaleFactor;
    mnTableCellHeight = 15 * fScaleFactor;

    mnTableWidth  = TABLE_CELLS_HORIZ*mnTableCellWidth;
    mnTableHeight = TABLE_CELLS_VERT*mnTableCellHeight;

    // + 1 to leave space to draw the right/bottom borders
    Size aSize(mnTableWidth + 1, mnTableHeight + 1);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    SetOutputSizePixel(aSize);
}

bool TableWidget::MouseMove(const MouseEvent& rMEvt)
{
    Point aPos = rMEvt.GetPosPixel();
    Point aMousePos( aPos );

    tools::Long nNewCol = ( aMousePos.X() + mnTableCellWidth ) / mnTableCellWidth;
    tools::Long nNewLine = ( aMousePos.Y() + mnTableCellHeight ) / mnTableCellHeight;

    Update( nNewCol, nNewLine );

    return true;
}

bool TableWidget::KeyInput(const KeyEvent& rKEvt)
{
    bool bHandled = false;
    sal_uInt16 nModifier = rKEvt.GetKeyCode().GetModifier();
    sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();
    if ( !nModifier )
    {
        bHandled = true;
        tools::Long nNewCol = nCol;
        tools::Long nNewLine = nLine;
        switch(nKey)
        {
            case KEY_UP:
                if ( nNewLine > 1 )
                    nNewLine--;
                else
                    mxControl->EndPopupMode();
                break;
            case KEY_DOWN:
                if ( nNewLine < TABLE_CELLS_VERT )
                {
                    nNewLine++;
                    if ( nNewCol == 0 )
                        nNewCol = 1;
                }
                else
                    mxControl->CloseAndShowTableDialog();
                break;
            case KEY_LEFT:
                if ( nNewCol > 1 )
                    nNewCol--;
                else
                    mxControl->EndPopupMode();
                break;
            case KEY_RIGHT:
                if ( nNewCol < TABLE_CELLS_HORIZ )
                {
                    nNewCol++;
                    if ( nNewLine == 0 )
                        nNewLine = 1;
                }
                else
                    mxControl->CloseAndShowTableDialog();
                break;
            case KEY_ESCAPE:
                mxControl->EndPopupMode();
                break;
            case KEY_RETURN:
                InsertTable();
                mxControl->EndPopupMode();
                return true;
            default:
                bHandled = false;
        }
        if ( bHandled )
        {
            Update( nNewCol, nNewLine );
        }
    }
    else if (KEY_MOD1 == nModifier && KEY_RETURN == nKey)
    {
        InsertTable();
        mxControl->EndPopupMode();
        return true;
    }

    return bHandled;
}

bool TableWidget::MouseButtonUp(const MouseEvent&)
{
    InsertTable();
    mxControl->EndPopupMode();
    return true;
}

bool TableWidget::MouseButtonDown(const MouseEvent&)
{
    return true;
}

void TableWidget::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push(PushFlags::FONT);

    rRenderContext.SetBackground( aBackgroundColor );
    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetColor( aFontColor );
    aFont.SetFillColor( aBackgroundColor );
    aFont.SetTransparent( false );
    rRenderContext.SetFont( aFont );

    const tools::Long nSelectionWidth = nCol * mnTableCellWidth;
    const tools::Long nSelectionHeight = nLine * mnTableCellHeight;

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
    for (tools::Long i = 1; i < TABLE_CELLS_VERT; ++i)
    {
        rRenderContext.DrawLine(Point(0, i*mnTableCellHeight),
                                Point(mnTableWidth, i*mnTableCellHeight));
    }

    for (tools::Long i = 1; i < TABLE_CELLS_HORIZ; ++i)
    {
        rRenderContext.DrawLine(Point( i*mnTableCellWidth, 0),
                                Point( i*mnTableCellWidth, mnTableHeight));
    }

    // the text near the mouse cursor telling the table dimensions
    if (!nCol || !nLine)
    {
        rRenderContext.Pop();
        return;
    }

    OUString aText = OUString::number( nCol ) + " x " + OUString::number( nLine );
    if (maCommand == ".uno:ShowMultiplePages")
    {
        aText += " " + SvxResId(RID_SVXSTR_PAGES);
    }

    Size aTextSize(rRenderContext.GetTextWidth(aText), rRenderContext.GetTextHeight());

    tools::Long nTextX = nSelectionWidth + mnTableCellWidth;
    tools::Long nTextY = nSelectionHeight + mnTableCellHeight;
    const tools::Long nTipBorder = 2;

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

    rRenderContext.Pop();
}

void TableWidget::InsertTable()
{
    if (nCol && nLine)
    {
        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = "Columns";
        aArgs[0].Value <<= sal_Int16( nCol );
        aArgs[1].Name = "Rows";
        aArgs[1].Value <<= sal_Int16( nLine );

        mxControl->TableDialog( aArgs );
    }
}

void TableWidget::Update( tools::Long nNewCol, tools::Long nNewLine )
{
    if ( nNewCol < 0 || nNewCol > TABLE_CELLS_HORIZ )
        nNewCol = 0;

    if ( nNewLine < 0 || nNewLine > TABLE_CELLS_VERT )
        nNewLine = 0;

    if ( nNewCol != nCol || nNewLine != nLine )
    {
        nCol = nNewCol;
        nLine = nNewLine;
        Invalidate(tools::Rectangle(0, 0, mnTableWidth, mnTableHeight));
    }
}

void SvxTableToolBoxControl::TableDialog( const Sequence< PropertyValue >& rArgs )
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        css::util::URL aTargetURL;
        Reference < XURLTransformer > xTrans( URLTransformer::create(::comphelper::getProcessComponentContext()) );
        aTargetURL.Complete = m_aCommandURL;
        xTrans->parseStrict( aTargetURL );

        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, rArgs );
    }
}

void SvxTableToolBoxControl::CloseAndShowTableDialog()
{
    // close the toolbar tool
    EndPopupMode();

    // and open the table dialog instead
    TableDialog( Sequence< PropertyValue >() );
}

namespace {

class ColumnsWidget final : public weld::CustomWidgetController
{
private:
    static constexpr tools::Long WIDTH = 5;

    rtl::Reference<SvxColumnsToolBoxControl> mxControl;
    weld::SpinButton& mrSpinButton;

    ::Color             aLineColor;
    ::Color             aHighlightLineColor;
    ::Color             aFillColor;
    ::Color             aHighlightFillColor;
    ::Color             aFaceColor;
    tools::Long                nCol;
    tools::Long                nMX;
    bool                m_bMod1;

    DECL_LINK(ValueChangedHdl, weld::SpinButton&, void);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);

    void InsertColumns();
    void UpdateSize_Impl( tools::Long nNewCol );
public:
    ColumnsWidget(SvxColumnsToolBoxControl* pControl, weld::SpinButton& rSpinButton);

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual bool KeyInput(const KeyEvent&) override;
    virtual bool MouseButtonDown(const MouseEvent&) override;
    virtual bool MouseMove(const MouseEvent&) override;
    virtual bool MouseButtonUp(const MouseEvent&) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
};


class ColumnsWindow final : public WeldToolbarPopup
{
private:
    std::unique_ptr<weld::SpinButton> mxSpinButton;
    std::unique_ptr<ColumnsWidget> mxColumnsWidget;
    std::unique_ptr<weld::CustomWeld> mxColumnsWidgetWin;
    rtl::Reference<SvxColumnsToolBoxControl> mxControl;

public:
    ColumnsWindow(SvxColumnsToolBoxControl* pControl, weld::Widget* pParent);

    virtual void GrabFocus() override
    {
        mxColumnsWidget->GrabFocus();
    }
};

}

ColumnsWindow::ColumnsWindow(SvxColumnsToolBoxControl* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/columnswindow.ui", "ColumnsWindow")
    , mxSpinButton(m_xBuilder->weld_spin_button("spinbutton"))
    , mxColumnsWidget(new ColumnsWidget(pControl, *mxSpinButton))
    , mxColumnsWidgetWin(new weld::CustomWeld(*m_xBuilder, "columns", *mxColumnsWidget))
    , mxControl(pControl)
{
}

ColumnsWidget::ColumnsWidget(SvxColumnsToolBoxControl* pControl, weld::SpinButton& rSpinButton)
    : mxControl(pControl)
    , mrSpinButton(rSpinButton)
    , nCol(1)
    , nMX(0)
    , m_bMod1(false)
{
    mrSpinButton.connect_value_changed(LINK(this, ColumnsWidget, ValueChangedHdl));
    mrSpinButton.connect_activate(LINK(this, ColumnsWidget, ActivateHdl));

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    aLineColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    aHighlightLineColor = rStyles.GetHighlightTextColor();
    aFillColor = rStyles.GetWindowColor();
    aHighlightFillColor = rStyles.GetHighlightColor();
    aFaceColor = rStyles.GetFaceColor();
}

IMPL_LINK_NOARG(ColumnsWidget, ValueChangedHdl, weld::SpinButton&, void)
{
    UpdateSize_Impl(mrSpinButton.get_value());
}

IMPL_LINK_NOARG(ColumnsWidget, ActivateHdl, weld::Entry&, bool)
{
    InsertColumns();
    mxControl->EndPopupMode();
    return true;
}

void ColumnsWidget::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    OutputDevice& rDevice = pDrawingArea->get_ref_device();
    Size aLogicSize = rDevice.LogicToPixel( Size( 95, 155 ), MapMode( MapUnit::Map10thMM ) );
    nMX = aLogicSize.Width();
    Size aSize(nMX*WIDTH-1, aLogicSize.Height());
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    SetOutputSizePixel(aSize);
}

bool ColumnsWidget::MouseMove(const MouseEvent& rMEvt)
{
    Point aPos = rMEvt.GetPosPixel();

    tools::Long nNewCol = 1;
    if ( aPos.X() > 0 )
        nNewCol = aPos.X() / nMX + 1;
    if ( nNewCol > 20 )
        nNewCol = 20;
    UpdateSize_Impl( nNewCol );

    return true;
}

void ColumnsWidget::UpdateSize_Impl( tools::Long nNewCol )
{
    if ( nNewCol == nCol )
        return;

    Size aWinSize = GetOutputSizePixel();

    Invalidate( tools::Rectangle( 0, aWinSize.Height() - 2,
                           aWinSize.Width(), aWinSize.Height() ) );

    tools::Long nMinCol = 0, nMaxCol = 0;

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
                           nMaxCol*nMX+1, aWinSize.Height() - 2 ) );
    nCol = nNewCol;
    mrSpinButton.set_value(nCol);
}

bool ColumnsWidget::MouseButtonDown(const MouseEvent&)
{
    return true;
}

bool ColumnsWidget::KeyInput(const KeyEvent& rKEvt)
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
            tools::Long nNewCol = nCol;
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
                    InsertColumns();
                    mxControl->EndPopupMode();
                break;
                case KEY_ESCAPE :
                case KEY_UP :
                    mxControl->EndPopupMode();
                break;
            }
            UpdateSize_Impl( nNewCol );
        }
    }
    else if(KEY_MOD1 == nModifier && KEY_RETURN == nKey)
    {
        m_bMod1 = true;
        InsertColumns();
        mxControl->EndPopupMode();
    }
    return bHandled;
}

bool ColumnsWidget::MouseButtonUp(const MouseEvent&)
{
    InsertColumns();
    mxControl->EndPopupMode();
    return true;
}

void ColumnsWidget::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push(PushFlags::FONT);

    rRenderContext.SetBackground();
    vcl::Font aFont( rRenderContext.GetFont() );
    aFont.SetColor( aLineColor );
    aFont.SetFillColor( aFaceColor );
    aFont.SetTransparent( false );
    rRenderContext.SetFont( aFont );

    tools::Long i;
    tools::Long nLineWidth;
    Size aSize(GetOutputSizePixel());

    for (i = 0; i < WIDTH; i++)
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

        rRenderContext.DrawRect(tools::Rectangle(i * nMX - 1, -1, i * nMX + nMX, aSize.Height() - 1));

        tools::Long j = 4;
        while (j < aSize.Height() - 4)
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

    rRenderContext.DrawRect(tools::Rectangle(0,
                                      aSize.Height() - 2,
                                      aSize.Width() / 2 - 1,
                                      aSize.Height()));

    rRenderContext.DrawRect(tools::Rectangle(aSize.Width() / 2,
                                      aSize.Height() - 2,
                                      aSize.Width(),
                                      aSize.Height()));

    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor();
    rRenderContext.DrawRect(tools::Rectangle( 0, 0, aSize.Width() - 1, aSize.Height() - 1));

    rRenderContext.Pop();
}

void SvxColumnsToolBoxControl::InsertColumns(const Sequence< PropertyValue >& rArgs)
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        css::util::URL aTargetURL;
        Reference < XURLTransformer > xTrans( URLTransformer::create(::comphelper::getProcessComponentContext()) );
        aTargetURL.Complete = m_aCommandURL;
        xTrans->parseStrict( aTargetURL );

        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, rArgs );
    }
}

void ColumnsWidget::InsertColumns()
{
    if (nCol)
    {
        Sequence< PropertyValue > aArgs( 2 );
        aArgs[0].Name = "Columns";
        aArgs[0].Value <<= sal_Int16( nCol );
        aArgs[1].Name = "Modifier";
        aArgs[1].Value <<= sal_Int16( m_bMod1 ? KEY_MOD1 : 0 );
        mxControl->InsertColumns(aArgs);
    }
}

SvxTableToolBoxControl::SvxTableToolBoxControl(const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
{
}

void SvxTableToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    PopupWindowController::initialize(rArguments);

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
        pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWNONLY | pToolBox->GetItemBits(nId));
}

SvxTableToolBoxControl::~SvxTableToolBoxControl()
{
}

std::unique_ptr<WeldToolbarPopup> SvxTableToolBoxControl::weldPopupWindow()
{
    return std::make_unique<TableWindow>(this, m_pToolbar, m_aCommandURL);
}

VclPtr<vcl::Window> SvxTableToolBoxControl::createVclPopupWindow( vcl::Window* pParent )
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    bool bToolBox = getToolboxId(nId, &pToolBox);

    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<TableWindow>(this, pParent->GetFrameWeld(), m_aCommandURL));

    mxInterimPopover->SetText(bToolBox ? pToolBox->GetItemText(nId) : OUString());

    mxInterimPopover->Show();

    return mxInterimPopover;
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

SvxColumnsToolBoxControl::SvxColumnsToolBoxControl(const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
{
}

void SvxColumnsToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    PopupWindowController::initialize(rArguments);

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
        pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWNONLY | pToolBox->GetItemBits(nId));
}

SvxColumnsToolBoxControl::~SvxColumnsToolBoxControl()
{
}

std::unique_ptr<WeldToolbarPopup> SvxColumnsToolBoxControl::weldPopupWindow()
{
    return std::make_unique<ColumnsWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> SvxColumnsToolBoxControl::createVclPopupWindow(vcl::Window* pParent)
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    bool bToolBox = getToolboxId(nId, &pToolBox);

    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<ColumnsWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->SetText(bToolBox ? pToolBox->GetItemText(nId) : OUString());

    mxInterimPopover->Show();

    return mxInterimPopover;
}

OUString SvxColumnsToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.ColumnsToolBoxControl";
}

css::uno::Sequence<OUString> SvxColumnsToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_ColumnsToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire(new SvxColumnsToolBoxControl(rContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
