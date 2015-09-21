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


#include "toolpaneldrawer.hxx"
#include "toolpaneldrawerpeer.hxx"
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <vcl/lineinfo.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/settings.hxx>


namespace svt
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XWindowPeer;
    namespace AccessibleRole = ::com::sun::star::accessibility::AccessibleRole;

    static const int s_nIndentationWidth = 16;


    //= DrawerVisualization


    DrawerVisualization::DrawerVisualization( ToolPanelDrawer& i_rParent )
        :Window( &i_rParent )
        ,m_rDrawer( i_rParent )
    {
        SetMouseTransparent( true );
        Show();
        SetAccessibleRole( AccessibleRole::LABEL );
    }


    void DrawerVisualization::Paint(vcl::RenderContext& rRenderContext, const Rectangle& i_rBoundingBox)
    {
        Window::Paint(rRenderContext, i_rBoundingBox);
        m_rDrawer.Paint(rRenderContext);
    }


    //= ToolPanelDrawer


    ToolPanelDrawer::ToolPanelDrawer( vcl::Window& i_rParent, const OUString& i_rTitle )
        :Window( &i_rParent, WB_TABSTOP )
        ,m_pPaintDevice( VclPtr<VirtualDevice>::Create( *this ) )
        ,m_aVisualization( VclPtr<DrawerVisualization>::Create(*this) )
        ,m_bFocused( false )
        ,m_bExpanded( false )
    {
        EnableMapMode( false );
        SetBackground( Wallpaper() );
        SetPointer( PointerStyle::RefHand );

        SetAccessibleRole( AccessibleRole::LIST_ITEM );

        SetText( i_rTitle );
        SetAccessibleName( i_rTitle );
        SetAccessibleDescription( i_rTitle );

        m_aVisualization->SetAccessibleName( i_rTitle );
        m_aVisualization->SetAccessibleDescription( i_rTitle );
    }

    ToolPanelDrawer::~ToolPanelDrawer()
    {
        disposeOnce();
    }

    void ToolPanelDrawer::dispose()
    {
        m_aVisualization.disposeAndClear();
        vcl::Window::dispose();
    }

    long ToolPanelDrawer::GetPreferredHeightPixel() const
    {
        Rectangle aTitleBarBox( impl_calcTitleBarBox( impl_calcTextBoundingBox() ) );
        return aTitleBarBox.GetHeight();
    }


    void ToolPanelDrawer::Paint(vcl::RenderContext& rRenderContext)
    {
        m_pPaintDevice->SetMapMode(rRenderContext.GetMapMode());
        m_pPaintDevice->SetOutputSize(GetOutputSizePixel());
        m_pPaintDevice->SetSettings(rRenderContext.GetSettings());
        m_pPaintDevice->SetDrawMode(rRenderContext.GetDrawMode());

        const Rectangle aTextBox(impl_calcTextBoundingBox());
        impl_paintBackground(impl_calcTitleBarBox(aTextBox));

        Rectangle aFocusBox(impl_paintExpansionIndicator(aTextBox));

        m_pPaintDevice->DrawText(aTextBox, GetText(), impl_getTextStyle());

        aFocusBox.Union(aTextBox);
        aFocusBox.Left() += 2;
        impl_paintFocusIndicator(aFocusBox);

        rRenderContext.DrawOutDev(Point(), GetOutputSizePixel(), Point(), GetOutputSizePixel(), *m_pPaintDevice);
    }


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


    DrawTextFlags ToolPanelDrawer::impl_getTextStyle() const
    {
        const DrawTextFlags nBasicStyle =  DrawTextFlags::Left
                                |   DrawTextFlags::Top
                                |   DrawTextFlags::WordBreak;

        if ( IsEnabled() )
            return nBasicStyle;

        return nBasicStyle | DrawTextFlags::Disable;
    }


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


    void ToolPanelDrawer::impl_paintFocusIndicator( const Rectangle& i_rTextBox )
    {
        if ( m_bFocused )
        {
            const Rectangle aTextPixelBox( m_pPaintDevice->LogicToPixel( i_rTextBox ) );

            m_pPaintDevice->EnableMapMode( false );
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
            m_pPaintDevice->DrawPolyLine( tools::Polygon( aTextPixelBox ), aDottedStyle );
            m_pPaintDevice->EnableMapMode( false );
        }
        else
            HideFocus();
    }


    void ToolPanelDrawer::GetFocus()
    {
        m_bFocused = true;
        Invalidate();
    }


    void ToolPanelDrawer::LoseFocus()
    {
        m_bFocused = false;
        Invalidate();
    }


    void ToolPanelDrawer::Resize()
    {
        Window::Resize();
        m_aVisualization->SetPosSizePixel( Point(), GetOutputSizePixel() );
    }


    void ToolPanelDrawer::MouseButtonDown( const MouseEvent& i_rMouseEvent )
    {
        // consume this event, and do not forward to the base class - it would sent a NotifyEvent, which in turn, when
        // we live in a DockingWindow, would start undocking
        (void)i_rMouseEvent;
    }

    void ToolPanelDrawer::ApplySettings(vcl::RenderContext& rRenderContext)
    {
        const StyleSettings& rStyleSettings(rRenderContext.GetSettings().GetStyleSettings());
        ApplyControlFont(rRenderContext, rStyleSettings.GetAppFont());
        ApplyControlForeground(rRenderContext, rStyleSettings.GetButtonTextColor());
        rRenderContext.SetTextFillColor();
    }

    void ToolPanelDrawer::DataChanged( const DataChangedEvent& i_rEvent )
    {
        Window::DataChanged( i_rEvent );

        switch ( i_rEvent.GetType() )
        {
            case DataChangedEventType::SETTINGS:
                if ( !( i_rEvent.GetFlags() & AllSettingsFlags::STYLE ) )
                    break;
                SetSettings( Application::GetSettings() );
                m_pPaintDevice.disposeAndReset( VclPtr<VirtualDevice>::Create( *this ) );

                // fall through.

            case DataChangedEventType::FONTS:
            case DataChangedEventType::FONTSUBSTITUTION:
            {
                const StyleSettings& rStyleSettings( GetSettings().GetStyleSettings() );
                ApplyControlFont(*this, rStyleSettings.GetAppFont());
                ApplyControlForeground(*this, rStyleSettings.GetButtonTextColor());
                SetTextFillColor();
                Invalidate();
            }
            break;
            default: break;
        }
    }


    Reference< XWindowPeer > ToolPanelDrawer::GetComponentInterface( bool i_bCreate )
    {
        Reference< XWindowPeer > xWindowPeer( Window::GetComponentInterface( false ) );
        if ( !xWindowPeer.is() && i_bCreate )
        {
            xWindowPeer.set( new ToolPanelDrawerPeer() );
            SetComponentInterface( xWindowPeer );
        }
        return xWindowPeer;
    }


    Rectangle ToolPanelDrawer::impl_calcTextBoundingBox() const
    {
        vcl::Font aFont( GetFont() );
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


    void ToolPanelDrawer::SetExpanded( const bool i_bExpanded )
    {
        if ( m_bExpanded != i_bExpanded )
        {
            m_bExpanded = i_bExpanded;
            CallEventListeners( m_bExpanded ? VCLEVENT_ITEM_EXPANDED : VCLEVENT_ITEM_COLLAPSED );
            Invalidate();
        }
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
