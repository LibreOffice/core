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

#include <svtools/hyperlabel.hxx>
#include <tools/color.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/ptrstyle.hxx>


namespace svt
{
    class HyperLabelImpl
    {
    public:
        sal_Int16           ID;
        sal_Int32           Index;
        bool                bInteractive;
        Size                m_aMinSize;
        bool                m_bHyperMode;

        HyperLabelImpl();
    };


    HyperLabelImpl::HyperLabelImpl()
        : ID(0)
        , Index(0)
        , bInteractive(false)
        , m_bHyperMode(false)
    {
    }

    HyperLabel::HyperLabel( vcl::Window* _pParent, WinBits _nWinStyle )
        :FixedText( _pParent, _nWinStyle )
        ,m_pImpl( new HyperLabelImpl )
    {
        implInit();
    }

    Size const & HyperLabel::CalcMinimumSize( long nMaxWidth ) const
    {
        m_pImpl->m_aMinSize = FixedText::CalcMinimumSize( nMaxWidth );
        // the MinimumSize is used to size the FocusRectangle
        // and for the MouseMove method
        m_pImpl->m_aMinSize.AdjustHeight(2 );
        m_pImpl->m_aMinSize.AdjustWidth(1 );
        return m_pImpl->m_aMinSize;
    }

    void HyperLabel::implInit()
    {
        ToggleBackgroundColor( COL_TRANSPARENT );

        WinBits nWinStyle = GetStyle();
        nWinStyle |= WB_EXTRAOFFSET;
        SetStyle( nWinStyle );

        Show();
    }

    void HyperLabel::ToggleBackgroundColor( const Color& _rGBColor )
    {
        SetControlBackground( _rGBColor );
    }

    void HyperLabel::MouseMove( const MouseEvent& rMEvt )
    {
        vcl::Font aFont = GetControlFont( );

        bool bHyperMode = false;
        if (!rMEvt.IsLeaveWindow() && IsEnabled() && m_pImpl->bInteractive)
        {
            Point aPoint = GetPointerPosPixel();
            if (aPoint.X() < m_pImpl->m_aMinSize.Width())
                bHyperMode = true;
        }

        m_pImpl->m_bHyperMode = bHyperMode;
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
        if ( m_pImpl->m_bHyperMode && m_pImpl->bInteractive )
        {
            maClickHdl.Call( this );
        }
    }

    void HyperLabel::GetFocus()
    {
        if ( IsEnabled() && m_pImpl->bInteractive )
        {
            Point aPoint(0,0);
            tools::Rectangle rRect(aPoint, Size( m_pImpl->m_aMinSize.Width(), GetSizePixel().Height() ) );
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

    void HyperLabel::dispose()
    {
        m_pImpl.reset();
        FixedText::dispose();
    }

    void HyperLabel::SetInteractive( bool _bInteractive )
    {
        m_pImpl->bInteractive = ( _bInteractive && IsEnabled() );
    }

    sal_Int16 HyperLabel::GetID() const
    {
        return m_pImpl->ID;
    }

    sal_Int32 HyperLabel::GetIndex() const
    {
        return m_pImpl->Index;
    }

    void HyperLabel::SetID( sal_Int16 ID )
    {
        m_pImpl->ID = ID;
    }

    void HyperLabel::SetIndex( sal_Int32 Index )
    {
        m_pImpl->Index = Index;
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

}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
