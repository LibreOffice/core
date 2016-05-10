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


#include <sfx2/titledockwin.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxlocal.hrc"
#include <sfx2/sfxresid.hxx>

#include <svl/eitem.hxx>
#include <vcl/settings.hxx>


namespace sfx2
{
    //= TitledDockingWindow
    TitledDockingWindow::TitledDockingWindow( SfxBindings* i_pBindings, SfxChildWindow* i_pChildWindow, vcl::Window* i_pParent,
            WinBits i_nStyle )
        :SfxDockingWindow( i_pBindings, i_pChildWindow, i_pParent, i_nStyle )
        ,m_sTitle()
        ,m_aToolbox( VclPtr<ToolBox>::Create(this) )
        ,m_aContentWindow( VclPtr<vcl::Window>::Create(this, WB_DIALOGCONTROL) )
        ,m_aBorder( 3, 1, 3, 3 )
        ,m_bLayoutPending( false )
        ,m_nTitleBarHeight(0)
    {
        impl_construct();
    }

    void TitledDockingWindow::impl_construct()
    {
        SetBackground( Wallpaper() );

        m_aToolbox->SetSelectHdl( LINK( this, TitledDockingWindow, OnToolboxItemSelected ) );
        m_aToolbox->SetOutStyle( TOOLBOX_STYLE_FLAT );
        m_aToolbox->SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetDialogColor() ) );
        m_aToolbox->Show();
        impl_resetToolBox();

        m_aContentWindow->Show();
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

    void TitledDockingWindow::SetTitle( const OUString& i_rTitle )
    {
        m_sTitle = i_rTitle;
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
        impl_scheduleLayout();
    }


    void TitledDockingWindow::onLayoutDone()
    {
        // not interested in
    }


    void TitledDockingWindow::impl_scheduleLayout()
    {
        m_bLayoutPending = true;
    }


    void TitledDockingWindow::impl_layout()
    {
        m_bLayoutPending = false;

        m_aToolbox->ShowItem( 1, !IsFloatingMode() );

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
        aWindowSize.Height() -= m_nTitleBarHeight;
        m_aContentWindow->SetPosSizePixel(
            Point( m_aBorder.Left(), m_nTitleBarHeight + m_aBorder.Top() ),
            Size(
                aWindowSize.Width() - m_aBorder.Left() - m_aBorder.Right(),
                aWindowSize.Height() - m_aBorder.Top() - m_aBorder.Bottom()
            )
        );

        onLayoutDone();
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

    void TitledDockingWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& i_rArea)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

        if (m_bLayoutPending)
            impl_layout();

        SfxDockingWindow::Paint(rRenderContext, i_rArea);

        rRenderContext.Push(PushFlags::FONT | PushFlags::FILLCOLOR | PushFlags::LINECOLOR);

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
        Rectangle aTitleBarBox(Rectangle(nOuterLeft, 0, nOuterRight, nInnerTop - 1));
        rRenderContext.DrawRect(aTitleBarBox);

        if (nInnerLeft > nOuterLeft)
            rRenderContext.DrawRect(Rectangle(nOuterLeft, nInnerTop, nInnerLeft, nInnerBottom));
        if (nOuterRight > nInnerRight)
            rRenderContext.DrawRect(Rectangle(nInnerRight, nInnerTop, nOuterRight, nInnerBottom));
        if (nInnerBottom < nOuterBottom)
            rRenderContext.DrawRect(Rectangle(nOuterLeft, nInnerBottom, nOuterRight, nOuterBottom));

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
        aTitleBarBox.Left() += 3;
        rRenderContext.DrawText(aTitleBarBox, impl_getTitle(),
                               DrawTextFlags::Left | DrawTextFlags::VCenter | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak);

        // Restore original values of the output device.
        rRenderContext.Pop();
    }


    OUString TitledDockingWindow::impl_getTitle() const
    {
        return !m_sTitle.isEmpty() ? m_sTitle : GetText();
    }


    void TitledDockingWindow::impl_resetToolBox()
    {
        m_aToolbox->Clear();

        // Get the closer bitmap and set it as right most button.
        Image aImage( SfxResId( SFX_IMG_CLOSE_DOC ) );
        m_aToolbox->InsertItem( 1, aImage );
        m_aToolbox->ShowItem( 1 );
    }


    sal_uInt16 TitledDockingWindow::impl_addDropDownToolBoxItem( const OUString& i_rItemText, const OString& i_nHelpId, const Link<ToolBox *, void>& i_rCallback )
    {
        // Add the menu before the closer button.
        const sal_uInt16 nItemCount( m_aToolbox->GetItemCount() );
        const sal_uInt16 nItemId( nItemCount + 1 );
        m_aToolbox->InsertItem( nItemId, i_rItemText, ToolBoxItemBits::DROPDOWNONLY, nItemCount > 0 ? nItemCount - 1 : TOOLBOX_APPEND );
        m_aToolbox->SetHelpId( nItemId, i_nHelpId );
        m_aToolbox->SetClickHdl( i_rCallback );
        m_aToolbox->SetDropdownClickHdl( i_rCallback );

        // The tool box has likely changed its size. The title bar has to be
        // resized.
        impl_scheduleLayout();
        Invalidate();

        return nItemId;
    }


    IMPL_LINK_TYPED( TitledDockingWindow, OnToolboxItemSelected, ToolBox*, pToolBox, void )
    {
        const sal_uInt16 nId = pToolBox->GetCurItemId();

        if ( nId == 1 )
        {
            // the closer
            EndTracking();
            const sal_uInt16 nChildWindowId( GetChildWindow_Impl()->GetType() );
            const SfxBoolItem aVisibility( nChildWindowId, false );
            GetBindings().GetDispatcher()->Execute(
                nChildWindowId,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                &aVisibility,
                nullptr
            );
        }
    }


    void TitledDockingWindow::StateChanged( StateChangedType i_nType )
    {
        switch ( i_nType )
        {
            case StateChangedType::InitShow:
                impl_scheduleLayout();
                break;
            default:;
        }
        SfxDockingWindow::StateChanged( i_nType );
    }

    void TitledDockingWindow::EndDocking( const Rectangle& i_rRect, bool i_bFloatMode )
    {
        SfxDockingWindow::EndDocking( i_rRect, i_bFloatMode );

        m_aEndDockingHdl.Call( this );
    }


    void TitledDockingWindow::DataChanged( const DataChangedEvent& i_rDataChangedEvent )
    {
        SfxDockingWindow::DataChanged( i_rDataChangedEvent );

        switch ( i_rDataChangedEvent.GetType() )
        {
            case DataChangedEventType::SETTINGS:
                if ( !( i_rDataChangedEvent.GetFlags() & AllSettingsFlags::STYLE ) )
                    break;
                SAL_FALLTHROUGH;
            case DataChangedEventType::FONTS:
            case DataChangedEventType::FONTSUBSTITUTION:
            {
                impl_scheduleLayout();
                Invalidate();
            }
            break;
            default: break;
        }
    }


} // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
