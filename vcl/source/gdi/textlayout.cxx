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

#include <vcl/ctrl.hxx>
#include <vcl/outdev.hxx>

#include "fontinstance.hxx"
#include "textlayout.hxx"

#include <com/sun/star/i18n/ScriptDirection.hpp>

#include <tools/diagnose_ex.h>
#include <tools/fract.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <rtl/strbuf.hxx>
#endif

#include <memory>
#include <iterator>

namespace vcl
{

    namespace ScriptDirection = ::com::sun::star::i18n::ScriptDirection;

    DefaultTextLayout::~DefaultTextLayout()
    {
    }

    long DefaultTextLayout::GetTextWidth( const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        return m_rTargetDevice.GetTextWidth( _rText, _nStartIndex, _nLength );
    }

    void DefaultTextLayout::DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex,
        sal_Int32 _nLength, MetricVector* _pVector, OUString* _pDisplayText )
    {
        m_rTargetDevice.DrawText( _rStartPoint, _rText, _nStartIndex, _nLength, _pVector, _pDisplayText );
    }

    void DefaultTextLayout::GetCaretPositions( const OUString& _rText, long* _pCaretXArray,
        sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        m_rTargetDevice.GetCaretPositions( _rText, _pCaretXArray, _nStartIndex, _nLength );
    }

    sal_Int32 DefaultTextLayout::GetTextBreak( const OUString& _rText, long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        return m_rTargetDevice.GetTextBreak( _rText, _nMaxTextWidth, _nStartIndex, _nLength );
    }

    bool DefaultTextLayout::DecomposeTextRectAction() const
    {
        return false;
    }

    class ReferenceDeviceTextLayout : public ITextLayout
    {
    public:
        ReferenceDeviceTextLayout( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice );
        virtual ~ReferenceDeviceTextLayout();

        // ITextLayout
        virtual long        GetTextWidth( const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen ) const override;
        virtual void        DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength, MetricVector* _pVector, OUString* _pDisplayText ) override;
        virtual void        GetCaretPositions( const OUString& _rText, long* _pCaretXArray, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const override;
        virtual sal_Int32   GetTextBreak(const OUString& _rText, long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength) const override;
        virtual bool        DecomposeTextRectAction() const override;

    public:
        // equivalents to the respective OutputDevice methods, which take the reference device into account
        long        GetTextArray( const OUString& _rText, long* _pDXAry, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const;
        Rectangle   DrawText( const Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, MetricVector* _pVector, OUString* _pDisplayText );

    protected:
        void onBeginDrawText()
        {
            m_aCompleteTextRect.SetEmpty();
        }
        const Rectangle& onEndDrawText()
        {
            return m_aCompleteTextRect;
        }

    private:
        OutputDevice&   m_rTargetDevice;
        OutputDevice&   m_rReferenceDevice;
        Font            m_aUnzoomedPointFont;
        const Fraction  m_aZoom;
        const bool      m_bRTLEnabled;

        Rectangle       m_aCompleteTextRect;
    };

    ReferenceDeviceTextLayout::ReferenceDeviceTextLayout( const Control& _rControl, OutputDevice& _rTargetDevice,
        OutputDevice& _rReferenceDevice )
        :m_rTargetDevice( _rTargetDevice )
        ,m_rReferenceDevice( _rReferenceDevice )
        ,m_aUnzoomedPointFont( _rControl.GetUnzoomedControlPointFont() )
        ,m_aZoom( _rControl.GetZoom() )
        ,m_bRTLEnabled( _rControl.IsRTLEnabled() )
    {
        m_rTargetDevice.Push( PushFlags::MAPMODE | PushFlags::FONT | PushFlags::TEXTLAYOUTMODE );

        MapMode aTargetMapMode( m_rTargetDevice.GetMapMode() );
        OSL_ENSURE( aTargetMapMode.GetOrigin() == Point(), "ReferenceDeviceTextLayout::ReferenceDeviceTextLayout: uhm, the code below won't work here ..." );

        // normally, controls simulate "zoom" by "zooming" the font. This is responsible for (part of) the discrepancies
        // between text in Writer and text in controls in Writer, though both have the same font.
        // So, if we have a zoom set at the control, then we do not scale the font, but instead modify the map mode
        // to accommodate for the zoom.
        aTargetMapMode.SetScaleX( m_aZoom );    // TODO: shouldn't this be "current_scale * zoom"?
        aTargetMapMode.SetScaleY( m_aZoom );

        // also, use a higher-resolution map unit than "pixels", which should save us some rounding errors when
        // translating coordinates between the reference device and the target device.
        OSL_ENSURE( aTargetMapMode.GetMapUnit() == MAP_PIXEL,
            "ReferenceDeviceTextLayout::ReferenceDeviceTextLayout: this class is not expected to work with such target devices!" );
            // we *could* adjust all the code in this class to handle this case, but at the moment, it's not necessary
        const MapUnit eTargetMapUnit = m_rReferenceDevice.GetMapMode().GetMapUnit();
        aTargetMapMode.SetMapUnit( eTargetMapUnit );
        OSL_ENSURE( aTargetMapMode.GetMapUnit() != MAP_PIXEL,
            "ReferenceDeviceTextLayout::ReferenceDeviceTextLayout: a reference device which has map mode PIXEL?!" );

        m_rTargetDevice.SetMapMode( aTargetMapMode );

        // now that the Zoom is part of the map mode, reset the target device's font to the "unzoomed" version
        Font aDrawFont( m_aUnzoomedPointFont );
        aDrawFont.SetFontSize( OutputDevice::LogicToLogic( aDrawFont.GetFontSize(), MAP_POINT, eTargetMapUnit ) );
        _rTargetDevice.SetFont( aDrawFont );

        // transfer font to the reference device
        m_rReferenceDevice.Push( PushFlags::FONT | PushFlags::TEXTLAYOUTMODE );
        Font aRefFont( m_aUnzoomedPointFont );
        aRefFont.SetFontSize( OutputDevice::LogicToLogic(
            aRefFont.GetFontSize(), MAP_POINT, m_rReferenceDevice.GetMapMode().GetMapUnit() ) );
        m_rReferenceDevice.SetFont( aRefFont );
    }

    ReferenceDeviceTextLayout::~ReferenceDeviceTextLayout()
    {
        m_rReferenceDevice.Pop();
        m_rTargetDevice.Pop();
    }

    namespace
    {
        bool lcl_normalizeLength( const OUString& _rText, const sal_Int32 _nStartIndex, sal_Int32& _io_nLength )
        {
            sal_Int32 nTextLength = _rText.getLength();
            if ( _nStartIndex > nTextLength )
                return false;
            if ( _nStartIndex + _io_nLength > nTextLength )
                _io_nLength = nTextLength - _nStartIndex;
            return true;
        }
    }

    long ReferenceDeviceTextLayout::GetTextArray( const OUString& _rText, long* _pDXAry, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        // retrieve the character widths from the reference device
        long nTextWidth = m_rReferenceDevice.GetTextArray( _rText, _pDXAry, _nStartIndex, _nLength );
#if OSL_DEBUG_LEVEL > 1
        if ( _pDXAry )
        {
            OStringBuffer aTrace;
            aTrace.append( "ReferenceDeviceTextLayout::GetTextArray( " );
            aTrace.append( OUStringToOString( _rText, RTL_TEXTENCODING_UTF8 ) );
            aTrace.append( " ): " );
            aTrace.append( nTextWidth );
            aTrace.append( " = ( " );
            for ( sal_Int32 i=0; i<_nLength; )
            {
                aTrace.append( _pDXAry[i] );
                if ( ++i < _nLength )
                    aTrace.append( ", " );
            }
            aTrace.append( ")" );
            OSL_TRACE( "%s", aTrace.makeStringAndClear().getStr() );
        }
#endif
        return nTextWidth;
    }

    long ReferenceDeviceTextLayout::GetTextWidth( const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        return GetTextArray( _rText, nullptr, _nStartIndex, _nLength );
    }

    void ReferenceDeviceTextLayout::DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength, MetricVector* _pVector, OUString* _pDisplayText )
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return;

        if ( _pVector && _pDisplayText )
        {
            MetricVector aGlyphBounds;
            m_rReferenceDevice.GetGlyphBoundRects( _rStartPoint, _rText, _nStartIndex, _nLength, _nStartIndex, aGlyphBounds );
            ::std::copy(
                aGlyphBounds.begin(), aGlyphBounds.end(),
                ::std::insert_iterator< MetricVector > ( *_pVector, _pVector->end() ) );
            *_pDisplayText += _rText.copy( _nStartIndex, _nLength );
            return;
        }

        std::unique_ptr<long[]> pCharWidths(new long[ _nLength ]);
        long nTextWidth = GetTextArray( _rText, pCharWidths.get(), _nStartIndex, _nLength );
        m_rTargetDevice.DrawTextArray( _rStartPoint, _rText, pCharWidths.get(), _nStartIndex, _nLength );
        pCharWidths.reset();

        m_aCompleteTextRect.Union( Rectangle( _rStartPoint, Size( nTextWidth, m_rTargetDevice.GetTextHeight() ) ) );
    }

    void ReferenceDeviceTextLayout::GetCaretPositions( const OUString& _rText, long* _pCaretXArray,
                                                       sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return;

        // retrieve the caret positions from the reference device
        m_rReferenceDevice.GetCaretPositions( _rText, _pCaretXArray, _nStartIndex, _nLength );
    }

    sal_Int32 ReferenceDeviceTextLayout::GetTextBreak( const OUString& _rText, long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        return m_rReferenceDevice.GetTextBreak( _rText, _nMaxTextWidth, _nStartIndex, _nLength );
    }

    bool ReferenceDeviceTextLayout::DecomposeTextRectAction() const
    {
        return true;
    }

    Rectangle ReferenceDeviceTextLayout::DrawText( const Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, MetricVector* _pVector, OUString* _pDisplayText )
    {
        if ( _rText.isEmpty() )
            return Rectangle();

        // determine text layout mode from the RTL-ness of the control whose text we render
        ComplexTextLayoutMode nTextLayoutMode = m_bRTLEnabled ? TEXT_LAYOUT_BIDI_RTL : TEXT_LAYOUT_DEFAULT;
        m_rReferenceDevice.SetLayoutMode( nTextLayoutMode );
        m_rTargetDevice.SetLayoutMode( nTextLayoutMode | TEXT_LAYOUT_TEXTORIGIN_LEFT );

        // TEXT_LAYOUT_TEXTORIGIN_LEFT is because when we do actually draw the text (in DrawText( Point, ... )), then
        // our caller gives us the left border of the draw position, regardless of script type, text layout,
        // and the like in our ctor, we set the map mode of the target device from pixel to twip, but our caller doesn't know this,
        // but passed pixel coordinates. So, adjust the rect.
        Rectangle aRect( m_rTargetDevice.PixelToLogic( _rRect ) );

        onBeginDrawText();
        m_rTargetDevice.DrawText( aRect, _rText, _nStyle, _pVector, _pDisplayText, this );
        Rectangle aTextRect = onEndDrawText();

        if ( aTextRect.IsEmpty() && !aRect.IsEmpty() )
        {
            // this happens for instance if we're in a PaintToDevice call, where only a MetaFile is recorded,
            // but no actual painting happens, so our "DrawText( Point, ... )" is never called
            // In this case, calculate the rect from what OutputDevice::GetTextRect would give us. This has
            // the disadvantage of less accuracy, compared with the approach to calculate the rect from the
            // single "DrawText( Point, ... )" calls, since more intermediate arithmetic will translate
            // from ref- to target-units.
            aTextRect = m_rTargetDevice.GetTextRect( aRect, _rText, _nStyle, nullptr, this );
        }

        // similar to above, the text rect now contains TWIPs (or whatever unit the ref device has), but the caller
        // expects pixel coordinates
        aTextRect = m_rTargetDevice.LogicToPixel( aTextRect );

        // convert the metric vector
        if ( _pVector )
        {
            for (   MetricVector::iterator charRect = _pVector->begin();
                    charRect != _pVector->end();
                    ++charRect
                )
            {
                *charRect = m_rTargetDevice.LogicToPixel( *charRect );
            }
        }

        return aTextRect;
    }

    ControlTextRenderer::ControlTextRenderer( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice )
        :m_pImpl( new ReferenceDeviceTextLayout( _rControl, _rTargetDevice, _rReferenceDevice ) )
    {
    }

    ControlTextRenderer::~ControlTextRenderer()
    {
    }

    Rectangle ControlTextRenderer::DrawText( const Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle,
        MetricVector* _pVector, OUString* _pDisplayText )
    {
        return m_pImpl->DrawText( _rRect, _rText, _nStyle, _pVector, _pDisplayText );
    }

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
