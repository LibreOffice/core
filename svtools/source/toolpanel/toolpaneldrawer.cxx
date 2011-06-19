/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_svtools.hxx"

#include "toolpaneldrawer.hxx"
#include "toolpaneldrawerpeer.hxx"
#include "svtools/svtdata.hxx"
#include "svtools/svtools.hrc"

#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <vcl/lineinfo.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclevent.hxx>

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XWindowPeer;
    namespace AccessibleRole = ::com::sun::star::accessibility::AccessibleRole;

    static const int s_nIndentationWidth = 16;

    //==================================================================================================================
    //= DrawerVisualization
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DrawerVisualization::DrawerVisualization( ToolPanelDrawer& i_rParent )
        :Window( &i_rParent )
        ,m_rDrawer( i_rParent )
    {
        SetMouseTransparent( sal_True );
        Show();
        SetAccessibleRole( AccessibleRole::LABEL );
    }

    //------------------------------------------------------------------------------------------------------------------
    DrawerVisualization::~DrawerVisualization()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void DrawerVisualization::Paint( const Rectangle& i_rBoundingBox )
    {
        Window::Paint( i_rBoundingBox );
        m_rDrawer.Paint();
    }

    //==================================================================================================================
    //= ToolPanelDrawer
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDrawer::ToolPanelDrawer( Window& i_rParent, const ::rtl::OUString& i_rTitle )
        :Window( &i_rParent, WB_TABSTOP )
        ,m_pPaintDevice( new VirtualDevice( *this ) )
        ,m_aVisualization( *this )
        ,m_bFocused( false )
        ,m_bExpanded( false )
    {
        EnableMapMode( sal_False );
        SetBackground( Wallpaper() );
        SetPointer( POINTER_REFHAND );

        SetAccessibleRole( AccessibleRole::LIST_ITEM );

        SetText( i_rTitle );
        SetAccessibleName( i_rTitle );
        SetAccessibleDescription( i_rTitle );

        m_aVisualization.SetAccessibleName( i_rTitle );
        m_aVisualization.SetAccessibleDescription( i_rTitle );
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDrawer::~ToolPanelDrawer()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    long ToolPanelDrawer::GetPreferredHeightPixel() const
    {
        Rectangle aTitleBarBox( impl_calcTitleBarBox( impl_calcTextBoundingBox() ) );
        return aTitleBarBox.GetHeight();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::Paint()
    {
        m_pPaintDevice->SetMapMode( GetMapMode() );
        m_pPaintDevice->SetOutputSize( GetOutputSizePixel() );
        m_pPaintDevice->SetSettings( GetSettings() );
        m_pPaintDevice->SetDrawMode( GetDrawMode() );

        const Rectangle aTextBox( impl_calcTextBoundingBox() );
        impl_paintBackground( impl_calcTitleBarBox( aTextBox ) );

        Rectangle aFocusBox( impl_paintExpansionIndicator( aTextBox ) );

        m_pPaintDevice->DrawText( aTextBox, GetText(), impl_getTextStyle() );

        aFocusBox.Union( aTextBox );
        aFocusBox.Left() += 2;
        impl_paintFocusIndicator( aFocusBox );

        m_aVisualization.DrawOutDev(
            Point(), GetOutputSizePixel(),
            Point(), GetOutputSizePixel(),
            *m_pPaintDevice
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle ToolPanelDrawer::impl_paintExpansionIndicator( const Rectangle& i_rTextBox )
    {
        Rectangle aExpansionIndicatorArea;

        Image aImage( impl_getExpansionIndicator() );
        const int nHeight( aImage.GetSizePixel().Height() );
        if ( nHeight > 0 )
        {
            Point aPosition(
                0,
                i_rTextBox.Top() + ( GetTextHeight() - nHeight ) / 2
            );
            m_pPaintDevice->DrawImage( aPosition, aImage );

            aExpansionIndicatorArea = Rectangle( aPosition, aImage.GetSizePixel() );
        }

        return aExpansionIndicatorArea;
    }


    Image ToolPanelDrawer::impl_getExpansionIndicator() const
    {
        sal_uInt16 nResourceId = 0;
        if ( m_bExpanded )
            nResourceId = IMG_TRIANGLE_DOWN;
        else
            nResourceId = IMG_TRIANGLE_RIGHT;
        return Image( SvtResId( nResourceId ) );
    }


    sal_uInt16 ToolPanelDrawer::impl_getTextStyle() const
    {
        const sal_uInt16 nBasicStyle =  TEXT_DRAW_LEFT
                                |   TEXT_DRAW_TOP
                                |   TEXT_DRAW_WORDBREAK;

        if ( IsEnabled() )
            return nBasicStyle;

        return nBasicStyle | TEXT_DRAW_DISABLE;
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::impl_paintBackground( const Rectangle& i_rTitleBarBox )
    {
        m_pPaintDevice->SetFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
        m_pPaintDevice->DrawRect( i_rTitleBarBox );

        m_pPaintDevice->SetFillColor();
        m_pPaintDevice->SetLineColor( GetSettings().GetStyleSettings().GetLightColor() );
        m_pPaintDevice->DrawLine( i_rTitleBarBox.TopLeft(), i_rTitleBarBox.TopRight() );
        m_pPaintDevice->DrawLine( i_rTitleBarBox.TopLeft(), i_rTitleBarBox.BottomLeft() );

        m_pPaintDevice->SetLineColor( GetSettings().GetStyleSettings().GetShadowColor() );
        m_pPaintDevice->DrawLine( i_rTitleBarBox.BottomLeft(), i_rTitleBarBox.BottomRight() );
        m_pPaintDevice->DrawLine( i_rTitleBarBox.TopRight(), i_rTitleBarBox.BottomRight() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::impl_paintFocusIndicator( const Rectangle& i_rTextBox )
    {
        if ( m_bFocused )
        {
            const Rectangle aTextPixelBox( m_pPaintDevice->LogicToPixel( i_rTextBox ) );

            m_pPaintDevice->EnableMapMode( sal_False );
            m_pPaintDevice->SetFillColor();

            Rectangle aBox( i_rTextBox );
            aBox.Top() -= 1;
            aBox.Bottom() += 1;

            m_pPaintDevice->DrawRect( aTextPixelBox );

            LineInfo aDottedStyle( LINE_DASH );
            aDottedStyle.SetDashCount( 0 );
            aDottedStyle.SetDotCount( 1 );
            aDottedStyle.SetDotLen( 1 );
            aDottedStyle.SetDistance( 1 );

            m_pPaintDevice->SetLineColor( COL_BLACK );
            m_pPaintDevice->DrawPolyLine( Polygon( aTextPixelBox ), aDottedStyle );
            m_pPaintDevice->EnableMapMode( sal_False );
        }
        else
            HideFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::GetFocus()
    {
        m_bFocused = true;
        Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::LoseFocus()
    {
        m_bFocused = false;
        Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::Resize()
    {
        Window::Resize();
        m_aVisualization.SetPosSizePixel( Point(), GetOutputSizePixel() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::MouseButtonDown( const MouseEvent& i_rMouseEvent )
    {
        // consume this event, and do not forward to the base class - it would sent a NotifyEvent, which in turn, when
        // we live in a DockingWindow, would start undocking
        (void)i_rMouseEvent;
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::DataChanged( const DataChangedEvent& i_rEvent )
    {
        Window::DataChanged( i_rEvent );

        switch ( i_rEvent.GetType() )
        {
            case DATACHANGED_SETTINGS:
                if ( ( i_rEvent.GetFlags() & SETTINGS_STYLE ) == 0 )
                    break;
                SetSettings( Application::GetSettings() );
                m_pPaintDevice.reset( new VirtualDevice( *this ) );

                // fall through.

            case DATACHANGED_FONTS:
            case DATACHANGED_FONTSUBSTITUTION:
            {
                const StyleSettings& rStyleSettings( GetSettings().GetStyleSettings() );

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

                Invalidate();
            }
            break;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XWindowPeer > ToolPanelDrawer::GetComponentInterface( sal_Bool i_bCreate )
    {
        Reference< XWindowPeer > xWindowPeer( Window::GetComponentInterface( sal_False ) );
        if ( !xWindowPeer.is() && i_bCreate )
        {
            xWindowPeer.set( new ToolPanelDrawerPeer() );
            SetComponentInterface( xWindowPeer );
        }
        return xWindowPeer;
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle ToolPanelDrawer::impl_calcTextBoundingBox() const
    {
        Font aFont( GetFont() );
        if ( m_bExpanded )
            aFont.SetWeight( m_bExpanded ? WEIGHT_BOLD : WEIGHT_NORMAL );
        m_pPaintDevice->SetFont( aFont );

        int nAvailableWidth = m_pPaintDevice->GetTextWidth( GetText() );

        Rectangle aTextBox(
            Point(),
            Size(
                nAvailableWidth,
                GetSettings().GetStyleSettings().GetTitleHeight()
            )
        );
        aTextBox.Top() += ( aTextBox.GetHeight() - GetTextHeight() ) / 2;
        aTextBox.Left() += s_nIndentationWidth;
        aTextBox.Right() -= 1;

        aTextBox = m_pPaintDevice->GetTextRect( aTextBox, GetText(), impl_getTextStyle() );
        return aTextBox;
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle ToolPanelDrawer::impl_calcTitleBarBox( const Rectangle& i_rTextBox ) const
    {
        Rectangle aTitleBarBox( i_rTextBox );
        aTitleBarBox.Bottom() += aTitleBarBox.Top();
        aTitleBarBox.Top() = 0;
        aTitleBarBox.Left() = 0;

        const long nWidth = GetOutputSizePixel().Width();
        if ( aTitleBarBox.GetWidth() < nWidth )
            aTitleBarBox.Right() = nWidth - 1;

        return aTitleBarBox;
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::SetExpanded( const bool i_bExpanded )
    {
        if ( m_bExpanded != i_bExpanded )
        {
            m_bExpanded = i_bExpanded;
            CallEventListeners( m_bExpanded ? VCLEVENT_ITEM_EXPANDED : VCLEVENT_ITEM_COLLAPSED );
            Invalidate();
        }
    }

//......................................................................................................................
} // namespace svt
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
