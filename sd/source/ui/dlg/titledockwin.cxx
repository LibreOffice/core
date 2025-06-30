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

#include <framework/FrameworkHelper.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/toolbox.hxx>

#include <ViewShellBase.hxx>
#include <bitmaps.hlst>
#include <strings.hrc>
#include <sdresid.hxx>
#include <titledockwin.hxx>

namespace sd
{
    //= TitledDockingWindow
    TitledDockingWindow::TitledDockingWindow( SfxBindings* i_pBindings, SfxChildWindow* i_pChildWindow, vcl::Window* i_pParent, const OUString& rsTitle )
        :SfxDockingWindow( i_pBindings, i_pChildWindow, i_pParent, WB_MOVEABLE|WB_CLOSEABLE|WB_DOCKABLE|WB_HIDE|WB_3DLOOK )
        ,m_aToolbox( VclPtr<ToolBox>::Create(this) )
        ,m_aContentWindow( VclPtr<vcl::Window>::Create(this, WB_DIALOGCONTROL) )
        ,m_aBorder( 3, 1, 3, 3 )
        ,m_nTitleBarHeight(0)
    {
        SetBackground( Wallpaper() );

        m_aToolbox->SetSelectHdl( LINK( this, TitledDockingWindow, OnToolboxItemSelected ) );
        m_aToolbox->SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetDialogColor() ) );
        m_aToolbox->Show();
        impl_resetToolBox();

        m_aContentWindow->Show();

        SetTitle(rsTitle);
        SetSizePixel(LogicToPixel(Size(80,200), MapMode(MapUnit::MapAppFont)));
    }

    TitledDockingWindow::~TitledDockingWindow()
    {
        disposeOnce();
    }

    void TitledDockingWindow::dispose()
    {
        m_aToolbox.disposeAndClear();
        m_aContentWindow.disposeAndClear();
        SfxDockingWindow::dispose();
    }

    void TitledDockingWindow::SetTitle(const OUString& i_rText)
    {
        m_sTitle = i_rText;
        SetAccessibleName(m_sTitle);
        Invalidate();
    }

    void TitledDockingWindow::SetText( const OUString& i_rText )
    {
        SfxDockingWindow::SetText( i_rText );
        if ( m_sTitle.isEmpty() )
            // our text is used as title, too => repaint
            Invalidate();
    }


    void TitledDockingWindow::Resize()
    {
        SfxDockingWindow::Resize();
        impl_layout();
    }


    void TitledDockingWindow::impl_layout()
    {
        m_aToolbox->ShowItem( ToolBoxItemId(1), !IsFloatingMode() );

        const Size aToolBoxSize( m_aToolbox->CalcWindowSizePixel() );
        Size aWindowSize( GetOutputSizePixel() );

        // position the tool box
        m_nTitleBarHeight = GetSettings().GetStyleSettings().GetTitleHeight();
        if ( aToolBoxSize.Height() > m_nTitleBarHeight )
            m_nTitleBarHeight = aToolBoxSize.Height();
        m_aToolbox->SetPosSizePixel(
            Point(
                aWindowSize.Width() - aToolBoxSize.Width(),
                ( m_nTitleBarHeight - aToolBoxSize.Height() ) / 2
            ),
            aToolBoxSize
        );

        // Place the content window.
        if ( m_nTitleBarHeight < aToolBoxSize.Height() )
            m_nTitleBarHeight = aToolBoxSize.Height();
        aWindowSize.AdjustHeight( -m_nTitleBarHeight );
        m_aContentWindow->SetPosSizePixel(
            Point( m_aBorder.Left(), m_nTitleBarHeight + m_aBorder.Top() ),
            Size(
                aWindowSize.Width() - m_aBorder.Left() - m_aBorder.Right(),
                aWindowSize.Height() - m_aBorder.Top() - m_aBorder.Bottom()
            )
        );
    }

    void TitledDockingWindow::ApplySettings(vcl::RenderContext& rRenderContext)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

        // Font
        ApplyControlFont(rRenderContext, rStyleSettings.GetAppFont());

        // Color
        ApplyControlForeground(rRenderContext, rStyleSettings.GetButtonTextColor());
        rRenderContext.SetTextFillColor();
    }

    void TitledDockingWindow::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& i_rArea)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

        SfxDockingWindow::Paint(rRenderContext, i_rArea);

        rRenderContext.Push(vcl::PushFlags::FONT | vcl::PushFlags::FILLCOLOR | vcl::PushFlags::LINECOLOR);

        rRenderContext.SetFillColor(rStyleSettings.GetDialogColor());
        rRenderContext.SetLineColor();

        // bold font
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        rRenderContext.SetFont(aFont);

        // Set border values.
        Size aWindowSize(GetOutputSizePixel());
        int nOuterLeft = 0;
        int nInnerLeft = nOuterLeft + m_aBorder.Left() - 1;
        int nOuterRight = aWindowSize.Width() - 1;
        int nInnerRight = nOuterRight - m_aBorder.Right() + 1;
        int nInnerTop = m_nTitleBarHeight + m_aBorder.Top() - 1;
        int nOuterBottom = aWindowSize.Height() - 1;
        int nInnerBottom = nOuterBottom - m_aBorder.Bottom() + 1;

        // Paint title bar background.
        ::tools::Rectangle aTitleBarBox(nOuterLeft, 0, nOuterRight, nInnerTop - 1);
        rRenderContext.DrawRect(aTitleBarBox);

        if (nInnerLeft > nOuterLeft)
            rRenderContext.DrawRect(::tools::Rectangle(nOuterLeft, nInnerTop, nInnerLeft, nInnerBottom));
        if (nOuterRight > nInnerRight)
            rRenderContext.DrawRect(::tools::Rectangle(nInnerRight, nInnerTop, nOuterRight, nInnerBottom));
        if (nInnerBottom < nOuterBottom)
            rRenderContext.DrawRect(::tools::Rectangle(nOuterLeft, nInnerBottom, nOuterRight, nOuterBottom));

        // Paint bevel border.
        rRenderContext.SetFillColor();
        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
        if (m_aBorder.Top() > 0)
            rRenderContext.DrawLine(Point(nInnerLeft, nInnerTop), Point(nInnerLeft, nInnerBottom));
        if (m_aBorder.Left() > 0)
            rRenderContext.DrawLine(Point(nInnerLeft, nInnerTop), Point(nInnerRight, nInnerTop));

        rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
        if (m_aBorder.Bottom() > 0)
            rRenderContext.DrawLine(Point(nInnerRight, nInnerBottom), Point(nInnerLeft, nInnerBottom));
        if (m_aBorder.Right() > 0)
            rRenderContext.DrawLine(Point(nInnerRight, nInnerBottom), Point(nInnerRight, nInnerTop));

        // Paint title bar text.
        rRenderContext.SetLineColor(rStyleSettings.GetActiveTextColor());
        aTitleBarBox.AdjustLeft(3 );
        rRenderContext.DrawText(aTitleBarBox,
                                !m_sTitle.isEmpty() ? m_sTitle : GetText(),
                                DrawTextFlags::Left | DrawTextFlags::VCenter | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak);

        // Restore original values of the output device.
        rRenderContext.Pop();
    }


    void TitledDockingWindow::impl_resetToolBox()
    {
        m_aToolbox->Clear();

        // Get the closer bitmap and set it as right most button.
        m_aToolbox->InsertItem(ToolBoxItemId(1), Image(StockImage::Yes, SFX_BMP_CLOSE_DOC));
        m_aToolbox->SetQuickHelpText(ToolBoxItemId(1), SdResId(STR_CLOSE_PANE));
        m_aToolbox->ShowItem( ToolBoxItemId(1) );
    }


    IMPL_LINK( TitledDockingWindow, OnToolboxItemSelected, ToolBox*, pToolBox, void )
    {
        const ToolBoxItemId nId = pToolBox->GetCurItemId();

        if ( nId == ToolBoxItemId(1) )
        {
            // the closer
            EndTracking();
            const sal_uInt16 nChildWindowId( GetChildWindow_Impl()->GetType() );
            const SfxBoolItem aVisibility( nChildWindowId, false );
            GetBindings().GetDispatcher()->ExecuteList(
                nChildWindowId,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                { &aVisibility }
            );
        }
    }


    void TitledDockingWindow::StateChanged( StateChangedType i_nType )
    {
        switch (i_nType)
        {
            case StateChangedType::InitShow:
                Resize();
                GetContentWindow().SetStyle(GetContentWindow().GetStyle() | WB_DIALOGCONTROL);
                impl_layout();
                break;

            case StateChangedType::Visible:
            {
                // The visibility of the docking window has changed.  Tell the
                // ConfigurationController so that it can activate or deactivate
                // a/the view for the pane.
                // Without this the side panes remain empty after closing an
                // in-place slide show.
                ViewShellBase* pBase = ViewShellBase::GetViewShellBase(
                    GetBindings().GetDispatcher()->GetFrame());
                if (pBase != nullptr)
                {
                    framework::FrameworkHelper::Instance(*pBase)->UpdateConfiguration();
                }
            }
            break;

            default:;
        }
        SfxDockingWindow::StateChanged( i_nType );
    }

    void TitledDockingWindow::DataChanged( const DataChangedEvent& i_rDataChangedEvent )
    {
        SfxDockingWindow::DataChanged( i_rDataChangedEvent );

        switch ( i_rDataChangedEvent.GetType() )
        {
            case DataChangedEventType::SETTINGS:
                if ( !( i_rDataChangedEvent.GetFlags() & AllSettingsFlags::STYLE ) )
                    break;
                [[fallthrough]];
            case DataChangedEventType::FONTS:
            case DataChangedEventType::FONTSUBSTITUTION:
            {
                impl_layout();
                Invalidate();
            }
            break;
            default: break;
        }
    }

    void TitledDockingWindow::MouseButtonDown (const MouseEvent& rEvent)
    {
        if (rEvent.GetButtons() == MOUSE_LEFT)
        {
            // For some strange reason we have to set the WB_DIALOGCONTROL at
            // the content window in order to have it pass focus to its content
            // window.  Without setting this flag here that works only on views
            // that have not been taken from the cash and relocated to this pane
            // docking window.
            GetContentWindow().SetStyle(GetContentWindow().GetStyle() | WB_DIALOGCONTROL);
            GetContentWindow().GrabFocus();
        }
        SfxDockingWindow::MouseButtonDown(rEvent);
    }

    void TitledDockingWindow::SetValidSizeRange (const Range& rValidSizeRange)
    {
        SplitWindow* pSplitWindow = dynamic_cast<SplitWindow*>(GetParent());
        if (pSplitWindow == nullptr)
            return;

        const sal_uInt16 nId (pSplitWindow->GetItemId(static_cast< vcl::Window*>(this)));
        const sal_uInt16 nSetId (pSplitWindow->GetSet(nId));
        // Because the TitledDockingWindow paints its own decoration, we have
        // to compensate the valid size range for that.
        const SvBorder aBorder (GetDecorationBorder());
        sal_Int32 nCompensation (pSplitWindow->IsHorizontal()
            ? aBorder.Top() + aBorder.Bottom()
            : aBorder.Left() + aBorder.Right());
        pSplitWindow->SetItemSizeRange(
            nSetId,
            Range(
                rValidSizeRange.Min() + nCompensation,
                rValidSizeRange.Max() + nCompensation));
    }

    TitledDockingWindow::Orientation TitledDockingWindow::GetOrientation() const
    {
        SplitWindow* pSplitWindow = dynamic_cast<SplitWindow*>(GetParent());
        if (pSplitWindow == nullptr)
            return UnknownOrientation;
        else if (pSplitWindow->IsHorizontal())
            return HorizontalOrientation;
        else
            return VerticalOrientation;
    }

} // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
