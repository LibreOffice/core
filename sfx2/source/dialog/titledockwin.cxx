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


#include "sfx2/titledockwin.hxx"
#include "sfx2/bindings.hxx"
#include "sfx2/dispatch.hxx"
#include "sfxlocal.hrc"
#include <sfx2/sfxresid.hxx>

#include <svl/eitem.hxx>

//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

    //==================================================================================================================
    //= TitledDockingWindow
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TitledDockingWindow::TitledDockingWindow( SfxBindings* i_pBindings, SfxChildWindow* i_pChildWindow, Window* i_pParent,
            WinBits i_nStyle )
        :SfxDockingWindow( i_pBindings, i_pChildWindow, i_pParent, i_nStyle )
        ,m_sTitle()
        ,m_aToolbox( this )
        ,m_aContentWindow( this, WB_DIALOGCONTROL )
        ,m_aBorder( 3, 1, 3, 3 )
        ,m_bLayoutPending( false )
        ,m_nTitleBarHeight(0)
    {
        impl_construct();
    }

    //------------------------------------------------------------------------------------------------------------------
    TitledDockingWindow::TitledDockingWindow( SfxBindings* i_pBindings, SfxChildWindow* i_pChildWindow, Window* i_pParent,
            const ResId& i_rResId )
        :SfxDockingWindow( i_pBindings, i_pChildWindow, i_pParent, i_rResId )
        ,m_sTitle()
        ,m_aToolbox( this )
        ,m_aContentWindow( this )
        ,m_aBorder( 3, 1, 3, 3 )
        ,m_bLayoutPending( false )
    {
        impl_construct();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::impl_construct()
    {
        SetBackground( Wallpaper() );

        m_aToolbox.SetSelectHdl( LINK( this, TitledDockingWindow, OnToolboxItemSelected ) );
        m_aToolbox.SetOutStyle( TOOLBOX_STYLE_FLAT );
        m_aToolbox.SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetDialogColor() ) );
        m_aToolbox.Show();
        impl_resetToolBox();

        m_aContentWindow.Show();
    }

    //------------------------------------------------------------------------------------------------------------------
    TitledDockingWindow::~TitledDockingWindow()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::SetTitle( const OUString& i_rTitle )
    {
        m_sTitle = i_rTitle;
        Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::SetText( const OUString& i_rText )
    {
        SfxDockingWindow::SetText( i_rText );
        if ( m_sTitle.isEmpty() )
            // our text is used as title, too => repaint
            Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::Resize()
    {
        SfxDockingWindow::Resize();
        impl_scheduleLayout();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::onLayoutDone()
    {
        // not interested in
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::impl_scheduleLayout()
    {
        m_bLayoutPending = true;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::impl_layout()
    {
        m_bLayoutPending = false;

        m_aToolbox.ShowItem( 1, !IsFloatingMode() );

        const Size aToolBoxSize( m_aToolbox.CalcWindowSizePixel() );
        Size aWindowSize( GetOutputSizePixel() );

        // position the tool box
        m_nTitleBarHeight = GetSettings().GetStyleSettings().GetTitleHeight();
        if ( aToolBoxSize.Height() > m_nTitleBarHeight )
            m_nTitleBarHeight = aToolBoxSize.Height();
        m_aToolbox.SetPosSizePixel(
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
        m_aContentWindow.SetPosSizePixel(
            Point( m_aBorder.Left(), m_nTitleBarHeight + m_aBorder.Top() ),
            Size(
                aWindowSize.Width() - m_aBorder.Left() - m_aBorder.Right(),
                aWindowSize.Height() - m_aBorder.Top() - m_aBorder.Bottom()
            )
        );

        onLayoutDone();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::Paint( const Rectangle& i_rArea )
    {
        if ( m_bLayoutPending )
            impl_layout();

        SfxDockingWindow::Paint( i_rArea );

        Push( PUSH_FONT | PUSH_FILLCOLOR | PUSH_LINECOLOR );

        SetFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
        SetLineColor();

        // bold font
        Font aFont( GetFont() );
        aFont.SetWeight( WEIGHT_BOLD );
        SetFont( aFont );

        // Set border values.
        Size aWindowSize( GetOutputSizePixel() );
        int nOuterLeft = 0;
        int nInnerLeft = nOuterLeft + m_aBorder.Left() - 1;
        int nOuterRight = aWindowSize.Width() - 1;
        int nInnerRight = nOuterRight - m_aBorder.Right() + 1;
        int nInnerTop = m_nTitleBarHeight + m_aBorder.Top() - 1;
        int nOuterBottom = aWindowSize.Height() - 1;
        int nInnerBottom = nOuterBottom - m_aBorder.Bottom() + 1;

        // Paint title bar background.
        Rectangle aTitleBarBox( Rectangle(
            nOuterLeft,
            0,
            nOuterRight,
            nInnerTop-1
        ) );
        DrawRect( aTitleBarBox );

        if ( nInnerLeft > nOuterLeft )
            DrawRect( Rectangle( nOuterLeft, nInnerTop, nInnerLeft, nInnerBottom ) );
        if ( nOuterRight > nInnerRight )
            DrawRect( Rectangle( nInnerRight, nInnerTop, nOuterRight, nInnerBottom ) );
        if ( nInnerBottom < nOuterBottom )
            DrawRect( Rectangle( nOuterLeft, nInnerBottom, nOuterRight, nOuterBottom ) );

        // Paint bevel border.
        SetFillColor();
        SetLineColor( GetSettings().GetStyleSettings().GetShadowColor() );
        if ( m_aBorder.Top() > 0 )
            DrawLine( Point( nInnerLeft, nInnerTop ), Point( nInnerLeft, nInnerBottom ) );
        if ( m_aBorder.Left() > 0 )
            DrawLine( Point( nInnerLeft, nInnerTop ), Point( nInnerRight, nInnerTop ) );

        SetLineColor( GetSettings().GetStyleSettings().GetLightColor() );
        if ( m_aBorder.Bottom() > 0 )
            DrawLine( Point( nInnerRight, nInnerBottom ), Point( nInnerLeft, nInnerBottom ) );
        if ( m_aBorder.Right() > 0 )
            DrawLine( Point( nInnerRight, nInnerBottom ), Point( nInnerRight, nInnerTop ) );

        // Paint title bar text.
        SetLineColor( GetSettings().GetStyleSettings().GetActiveTextColor() );
        aTitleBarBox.Left() += 3;
        DrawText( aTitleBarBox, impl_getTitle(), TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

        // Restore original values of the output device.
        Pop();
    }

    //------------------------------------------------------------------------------------------------------------------
    OUString TitledDockingWindow::impl_getTitle() const
    {
        return !m_sTitle.isEmpty() ? m_sTitle : GetText();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::impl_resetToolBox()
    {
        m_aToolbox.Clear();

        // Get the closer bitmap and set it as right most button.
        Image aImage( SfxResId( SFX_IMG_CLOSE_DOC ) );
        m_aToolbox.InsertItem( 1, aImage );
        m_aToolbox.ShowItem( 1 );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_uInt16 TitledDockingWindow::impl_addDropDownToolBoxItem( const OUString& i_rItemText, const OString& i_nHelpId, const Link& i_rCallback )
    {
        // Add the menu before the closer button.
        const sal_uInt16 nItemCount( m_aToolbox.GetItemCount() );
        const sal_uInt16 nItemId( nItemCount + 1 );
        m_aToolbox.InsertItem( nItemId, i_rItemText, TIB_DROPDOWNONLY, nItemCount > 0 ? nItemCount - 1 : TOOLBOX_APPEND );
        m_aToolbox.SetHelpId( nItemId, i_nHelpId );
        m_aToolbox.SetClickHdl( i_rCallback );
        m_aToolbox.SetDropdownClickHdl( i_rCallback );

        // The tool box has likely changed its size. The title bar has to be
        // resized.
        impl_scheduleLayout();
        Invalidate();

        return nItemId;
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( TitledDockingWindow, OnToolboxItemSelected, ToolBox*, pToolBox )
    {
        const sal_uInt16 nId = pToolBox->GetCurItemId();

        if ( nId == 1 )
        {
            // the closer
            EndTracking();
            const sal_uInt16 nChildWindowId( GetChildWindow_Impl()->GetType() );
            const SfxBoolItem aVisibility( nChildWindowId, sal_False );
            GetBindings().GetDispatcher()->Execute(
                nChildWindowId,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                &aVisibility,
                NULL
            );
        }

        return 0;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::StateChanged( StateChangedType i_nType )
    {
        switch ( i_nType )
        {
            case STATE_CHANGE_INITSHOW:
                impl_scheduleLayout();
                break;
        }
        SfxDockingWindow::StateChanged( i_nType );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::EndDocking( const Rectangle& i_rRect, sal_Bool i_bFloatMode )
    {
        SfxDockingWindow::EndDocking( i_rRect, i_bFloatMode );

        if ( m_aEndDockingHdl.IsSet() )
            m_aEndDockingHdl.Call( this );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TitledDockingWindow::DataChanged( const DataChangedEvent& i_rDataChangedEvent )
    {
        SfxDockingWindow::DataChanged( i_rDataChangedEvent );

        switch ( i_rDataChangedEvent.GetType() )
        {
            case DATACHANGED_SETTINGS:
                if ( ( i_rDataChangedEvent.GetFlags() & SETTINGS_STYLE ) == 0)
                    break;
                // else fall through.
            case DATACHANGED_FONTS:
            case DATACHANGED_FONTSUBSTITUTION:
            {
                const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

                // Font.
                Font aFont = rStyleSettings.GetAppFont();
                if ( IsControlFont() )
                    aFont.Merge( GetControlFont() );
                SetZoomedPointFont( aFont );

                // Color.
                Color aColor;
                if ( IsControlForeground() )
                    aColor = GetControlForeground();
                else
                    aColor = rStyleSettings.GetButtonTextColor();
                SetTextColor( aColor );
                SetTextFillColor();

                impl_scheduleLayout();
                Invalidate();
            }
            break;
        }
    }

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
