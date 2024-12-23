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

#include <tools/color.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
#include <hyperlabel.hxx>

namespace vcl
{
    HyperLabel::HyperLabel( vcl::Window* _pParent, WinBits _nWinStyle )
        :FixedText( _pParent, _nWinStyle )
        , ID(0)
        , Index(0)
        , bInteractive(false)
        , m_bHyperMode(false)
    {
        ToggleBackgroundColor( COL_TRANSPARENT );

        WinBits nWinStyle = GetStyle();
        nWinStyle |= WB_EXTRAOFFSET;
        SetStyle( nWinStyle );

        Show();
    }

    Size const & HyperLabel::CalcMinimumSize( tools::Long nMaxWidth )
    {
        m_aMinSize = FixedText::CalcMinimumSize( nMaxWidth );
        // the MinimumSize is used to size the FocusRectangle
        // and for the MouseMove method
        m_aMinSize.AdjustHeight(2 );
        m_aMinSize.AdjustWidth(1 );
        return m_aMinSize;
    }

    void HyperLabel::ToggleBackgroundColor( const Color& _rGBColor )
    {
        SetControlBackground( _rGBColor );
    }

    void HyperLabel::MouseMove( const MouseEvent& rMEvt )
    {
        vcl::Font aFont = GetControlFont( );

        bool bHyperMode = false;
        if (!rMEvt.IsLeaveWindow() && IsEnabled() && bInteractive)
        {
            Point aPoint = GetPointerPosPixel();
            if (aPoint.X() < m_aMinSize.Width())
                bHyperMode = true;
        }

        m_bHyperMode = bHyperMode;
        if (bHyperMode)
        {
            aFont.SetUnderline(LINESTYLE_SINGLE);
            SetPointer(PointerStyle::RefHand);
        }
        else
        {
            aFont.SetUnderline(LINESTYLE_NONE);
            SetPointer(PointerStyle::Arrow);
        }
        SetControlFont(aFont);
    }

    void HyperLabel::MouseButtonDown( const MouseEvent& )
    {
        if ( m_bHyperMode && bInteractive )
        {
            maClickHdl.Call( this );
        }
    }

    void HyperLabel::GetFocus()
    {
        if ( IsEnabled() && bInteractive )
        {
            Point aPoint(0,0);
            tools::Rectangle rRect(aPoint, Size( m_aMinSize.Width(), GetSizePixel().Height() ) );
            ShowFocus( rRect );
        }
    }

    void HyperLabel::LoseFocus()
    {
        HideFocus();
    }

    HyperLabel::~HyperLabel( )
    {
        disposeOnce();
    }

    void HyperLabel::SetInteractive( bool _bInteractive )
    {
        bInteractive = ( _bInteractive && IsEnabled() );
    }

    sal_Int16 HyperLabel::GetID() const
    {
        return ID;
    }

    sal_Int32 HyperLabel::GetIndex() const
    {
        return Index;
    }

    void HyperLabel::SetID( sal_Int16 newID )
    {
        this->ID = newID;
    }

    void HyperLabel::SetIndex( sal_Int32 newIndex )
    {
        Index = newIndex;
    }

    void HyperLabel::SetLabel( const OUString& _rText )
    {
        SetText(_rText);
    }

    void HyperLabel::ApplySettings(vcl::RenderContext& rRenderContext)
    {
        FixedText::ApplySettings(rRenderContext);

        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        if (GetControlBackground() == COL_TRANSPARENT)
            rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());
        else
            rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
    }

    void HyperLabel::DataChanged( const DataChangedEvent& rDCEvt )
    {
        FixedText::DataChanged( rDCEvt );

        if ((( rDCEvt.GetType() == DataChangedEventType::SETTINGS )   ||
            ( rDCEvt.GetType() == DataChangedEventType::DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE        ))
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            if (GetControlBackground() != COL_TRANSPARENT)
                SetControlBackground(rStyleSettings.GetHighlightColor());
            Invalidate();
        }
    }

}   // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
