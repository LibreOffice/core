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
#include <vcl/bitmap.hxx>
#include <tools/color.hxx>

#include <vcl/tabpage.hxx>


//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= FontChanger
    //=====================================================================
    class FontChanger
    {
    protected:
        OutputDevice*   m_pDev;

    public:
        FontChanger( OutputDevice* _pDev, const Font& _rNewFont )
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_FONT );
            m_pDev->SetFont( _rNewFont );
        }

        ~FontChanger()
        {
            m_pDev->Pop( );
        }
    };

    class HyperLabelImpl
    {
    public:
        sal_Int16           ID;
        sal_Int32           Index;
        sal_Bool            bInteractive;
        Size                m_aMinSize;
        sal_Bool            m_bHyperMode;

        HyperLabelImpl();
    };

    //---------------------------------------------------------------------
    HyperLabelImpl::HyperLabelImpl()
    {
    }

    HyperLabel::HyperLabel( Window* _pParent, WinBits _nWinStyle )
        :FixedText( _pParent, _nWinStyle )
        ,m_pImpl( new HyperLabelImpl )
    {
        implInit();
    }

    Size HyperLabel::CalcMinimumSize( long nMaxWidth ) const
    {
        m_pImpl->m_aMinSize = FixedText::CalcMinimumSize( nMaxWidth );
        // the MinimumSize is used to size the FocusRectangle
        // and for the MouseMove method
        m_pImpl->m_aMinSize.Height() += 2;
        m_pImpl->m_aMinSize.Width() += 1;
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
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetControlBackground( _rGBColor );
        if (_rGBColor == COL_TRANSPARENT)
            SetTextColor( rStyleSettings.GetFieldTextColor( ) );
        else
            SetTextColor( rStyleSettings.GetHighlightTextColor( ) );
    }


    void HyperLabel::MouseMove( const MouseEvent& rMEvt )
    {
           Font aFont = GetControlFont( );
        const Color aColor = GetTextColor();

        if (rMEvt.IsLeaveWindow())
        {
            DeactivateHyperMode(aFont, aColor);
        }
        else
        {
            Point aPoint = GetPointerPosPixel();
            if (aPoint.X() < m_pImpl->m_aMinSize.Width())
            {
                if ( IsEnabled() && (m_pImpl->bInteractive) )
                {
                    ActivateHyperMode( aFont, aColor);
                    return;
                }
            }
            DeactivateHyperMode(aFont, aColor);
        }
    }

    void HyperLabel::ActivateHyperMode(Font aFont, const Color aColor)
    {
        aFont.SetUnderline(UNDERLINE_SINGLE);
        m_pImpl->m_bHyperMode = sal_True;
        SetPointer( POINTER_REFHAND );
        SetControlFont( aFont);
        SetTextColor( aColor);

    }

    void HyperLabel::DeactivateHyperMode(Font aFont, const Color aColor)
    {
        m_pImpl->m_bHyperMode = sal_False;
        aFont.SetUnderline(UNDERLINE_NONE);
        SetPointer( POINTER_ARROW );
        SetControlFont( aFont);
        SetTextColor( aColor);
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
            Rectangle rRect(aPoint, Size( m_pImpl->m_aMinSize.Width(), GetSizePixel().Height() ) );
            ShowFocus( rRect );
        }
    }

    void HyperLabel::LoseFocus()
    {
        HideFocus();
    }

    HyperLabel::~HyperLabel( )
    {
        delete m_pImpl;
    }

    void HyperLabel::SetInteractive( sal_Bool _bInteractive )
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

    void HyperLabel::SetID( sal_Int16 _ID )
    {
        m_pImpl->ID = _ID;
    }

    void HyperLabel::SetIndex( sal_Int32 _Index )
    {
        m_pImpl->Index = _Index;
    }

    void HyperLabel::SetLabel( const OUString& _rText )
    {
        SetText(_rText);
    }


    //------------------------------------------------------------------------------
    void HyperLabel::DataChanged( const DataChangedEvent& rDCEvt )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        FixedText::DataChanged( rDCEvt );
        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            const Color& rGBColor = GetControlBackground();
            if (rGBColor == COL_TRANSPARENT)
                SetTextColor( rStyleSettings.GetFieldTextColor( ) );
            else
            {
                SetControlBackground(rStyleSettings.GetHighlightColor());
                SetTextColor( rStyleSettings.GetHighlightTextColor( ) );
            }
            Invalidate();
        }
    }

//.........................................................................
}   // namespace svt
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
