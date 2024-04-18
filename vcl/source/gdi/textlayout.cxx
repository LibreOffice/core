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
#include <vcl/kernarray.hxx>
#include <vcl/outdev.hxx>

#include <textlayout.hxx>

#include <osl/diagnose.h>
#include <tools/fract.hxx>
#include <sal/log.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <rtl/strbuf.hxx>
#endif

#include <memory>
#include <iterator>

namespace vcl
{

    DefaultTextLayout::~DefaultTextLayout()
    {
    }

    tools::Long DefaultTextLayout::GetTextWidth( const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        return m_rTargetDevice.GetTextWidth( _rText, _nStartIndex, _nLength );
    }

    void DefaultTextLayout::DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex,
        sal_Int32 _nLength, std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText )
    {
        m_rTargetDevice.DrawText( _rStartPoint, _rText, _nStartIndex, _nLength, _pVector, _pDisplayText );
    }

    void DefaultTextLayout::GetCaretPositions( const OUString& _rText, sal_Int32* _pCaretXArray,
        sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        m_rTargetDevice.GetCaretPositions( _rText, _pCaretXArray, _nStartIndex, _nLength );
    }

    sal_Int32 DefaultTextLayout::GetTextBreak( const OUString& _rText, tools::Long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
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
        virtual tools::Long        GetTextWidth( const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen ) const override;
        virtual void        DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength, std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText ) override;
        virtual void        GetCaretPositions( const OUString& _rText, sal_Int32* _pCaretXArray, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const override;
        virtual sal_Int32   GetTextBreak(const OUString& _rText, tools::Long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength) const override;
        virtual bool        DecomposeTextRectAction() const override;

    public:
        // equivalents to the respective OutputDevice methods, which take the reference device into account
        tools::Rectangle   DrawText( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText, const Size* i_pDeviceSize );
        tools::Rectangle   GetTextRect( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, Size* o_pDeviceSize );

    private:
        tools::Long        GetTextArray( const OUString& _rText, KernArray* _pDXAry, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const;

        OutputDevice&   m_rTargetDevice;
        OutputDevice&   m_rReferenceDevice;
        const bool      m_bRTLEnabled;

        tools::Rectangle       m_aCompleteTextRect;
    };

    ReferenceDeviceTextLayout::ReferenceDeviceTextLayout( const Control& _rControl, OutputDevice& _rTargetDevice,
        OutputDevice& _rReferenceDevice )
        :m_rTargetDevice( _rTargetDevice )
        ,m_rReferenceDevice( _rReferenceDevice )
        ,m_bRTLEnabled( _rControl.IsRTLEnabled() )
    {
        Font const aUnzoomedPointFont( _rControl.GetUnzoomedControlPointFont() );
        const Fraction& aZoom( _rControl.GetZoom() );
        m_rTargetDevice.Push( PushFlags::MAPMODE | PushFlags::FONT | PushFlags::TEXTLAYOUTMODE );

        MapMode aTargetMapMode( m_rTargetDevice.GetMapMode() );
        OSL_ENSURE( aTargetMapMode.GetOrigin() == Point(), "ReferenceDeviceTextLayout::ReferenceDeviceTextLayout: uhm, the code below won't work here ..." );

        // normally, controls simulate "zoom" by "zooming" the font. This is responsible for (part of) the discrepancies
        // between text in Writer and text in controls in Writer, though both have the same font.
        // So, if we have a zoom set at the control, then we do not scale the font, but instead modify the map mode
        // to accommodate for the zoom.
        aTargetMapMode.SetScaleX( aZoom );    // TODO: shouldn't this be "current_scale * zoom"?
        aTargetMapMode.SetScaleY( aZoom );

        // also, use a higher-resolution map unit than "pixels", which should save us some rounding errors when
        // translating coordinates between the reference device and the target device.
        OSL_ENSURE( aTargetMapMode.GetMapUnit() == MapUnit::MapPixel,
            "ReferenceDeviceTextLayout::ReferenceDeviceTextLayout: this class is not expected to work with such target devices!" );
            // we *could* adjust all the code in this class to handle this case, but at the moment, it's not necessary
        const MapUnit eTargetMapUnit = m_rReferenceDevice.GetMapMode().GetMapUnit();
        aTargetMapMode.SetMapUnit( eTargetMapUnit );
        OSL_ENSURE( aTargetMapMode.GetMapUnit() != MapUnit::MapPixel,
            "ReferenceDeviceTextLayout::ReferenceDeviceTextLayout: a reference device which has map mode PIXEL?!" );

        m_rTargetDevice.SetMapMode( aTargetMapMode );

        // now that the Zoom is part of the map mode, reset the target device's font to the "unzoomed" version
        Font aDrawFont( aUnzoomedPointFont );
        aDrawFont.SetFontSize( OutputDevice::LogicToLogic(aDrawFont.GetFontSize(), MapMode(MapUnit::MapPoint), MapMode(eTargetMapUnit)) );
        _rTargetDevice.SetFont( aDrawFont );

        // transfer font to the reference device
        m_rReferenceDevice.Push( PushFlags::FONT | PushFlags::TEXTLAYOUTMODE );
        Font aRefFont( aUnzoomedPointFont );
        aRefFont.SetFontSize( OutputDevice::LogicToLogic(
            aRefFont.GetFontSize(), MapMode(MapUnit::MapPoint), m_rReferenceDevice.GetMapMode()) );
        m_rReferenceDevice.SetFont( aRefFont );
    }

    ReferenceDeviceTextLayout::~ReferenceDeviceTextLayout()
    {
        m_rReferenceDevice.Pop();
        m_rTargetDevice.Pop();
    }

    namespace
    {
        bool lcl_normalizeLength( std::u16string_view _rText, const sal_Int32 _nStartIndex, sal_Int32& _io_nLength )
        {
            sal_Int32 nTextLength = _rText.size();
            if ( _nStartIndex > nTextLength )
                return false;
            if ( _nStartIndex + _io_nLength > nTextLength )
                _io_nLength = nTextLength - _nStartIndex;
            return true;
        }
    }

    tools::Long ReferenceDeviceTextLayout::GetTextArray( const OUString& _rText, KernArray* _pDXAry, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        // retrieve the character widths from the reference device
        tools::Long nTextWidth = basegfx::fround<tools::Long>(m_rReferenceDevice.GetTextArray( _rText, _pDXAry, _nStartIndex, _nLength ));
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
                aTrace.append( _pDXAry->at(i) );
                if ( ++i < _nLength )
                    aTrace.append( ", " );
            }
            aTrace.append( ")" );
            SAL_INFO( "vcl", aTrace.makeStringAndClear() );
        }
#endif
        return nTextWidth;
    }

    tools::Long ReferenceDeviceTextLayout::GetTextWidth( const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        return GetTextArray( _rText, nullptr, _nStartIndex, _nLength );
    }

    void ReferenceDeviceTextLayout::DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength, std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText )
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return;

        if ( _pVector && _pDisplayText )
        {
            std::vector< tools::Rectangle > aGlyphBounds;
            m_rReferenceDevice.GetGlyphBoundRects( _rStartPoint, _rText, _nStartIndex, _nLength, aGlyphBounds );
            _pVector->insert( _pVector->end(), aGlyphBounds.begin(), aGlyphBounds.end() );
            *_pDisplayText += _rText.subView( _nStartIndex, _nLength );
            return;
        }

        KernArray aCharWidths;
        tools::Long nTextWidth = GetTextArray( _rText, &aCharWidths, _nStartIndex, _nLength );
        m_rTargetDevice.DrawTextArray( _rStartPoint, _rText, aCharWidths, {}, _nStartIndex, _nLength );

        m_aCompleteTextRect.Union( tools::Rectangle( _rStartPoint, Size( nTextWidth, m_rTargetDevice.GetTextHeight() ) ) );
    }

    void ReferenceDeviceTextLayout::GetCaretPositions( const OUString& _rText, sal_Int32* _pCaretXArray,
                                                       sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return;

        // retrieve the caret positions from the reference device
        m_rReferenceDevice.GetCaretPositions( _rText, _pCaretXArray, _nStartIndex, _nLength );
    }

    sal_Int32 ReferenceDeviceTextLayout::GetTextBreak( const OUString& _rText, tools::Long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        return m_rReferenceDevice.GetTextBreak( _rText, _nMaxTextWidth, _nStartIndex, _nLength );
    }

    bool ReferenceDeviceTextLayout::DecomposeTextRectAction() const
    {
        return true;
    }

    tools::Rectangle ReferenceDeviceTextLayout::DrawText( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle,
                                                   std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText, const Size* i_pDeviceSize )
    {
        if ( _rText.isEmpty() )
            return tools::Rectangle();

        // determine text layout mode from the RTL-ness of the control whose text we render
        text::ComplexTextLayoutFlags nTextLayoutMode = m_bRTLEnabled ? text::ComplexTextLayoutFlags::BiDiRtl : text::ComplexTextLayoutFlags::Default;
        m_rReferenceDevice.SetLayoutMode( nTextLayoutMode );
        m_rTargetDevice.SetLayoutMode( nTextLayoutMode | text::ComplexTextLayoutFlags::TextOriginLeft );

        // ComplexTextLayoutFlags::TextOriginLeft is because when we do actually draw the text (in DrawText( Point, ... )), then
        // our caller gives us the left border of the draw position, regardless of script type, text layout,
        // and the like in our ctor, we set the map mode of the target device from pixel to twip, but our caller doesn't know this,
        // but passed pixel coordinates. So, adjust the rect.
        tools::Rectangle aRect( m_rTargetDevice.PixelToLogic( _rRect ) );
        if (i_pDeviceSize)
        {
            //if i_pDeviceSize is passed in here, it was the original pre logic-to-pixel size of _rRect
            SAL_WARN_IF(std::abs(_rRect.GetSize().Width() - m_rTargetDevice.LogicToPixel(*i_pDeviceSize).Width()) > 1, "vcl", "DeviceSize width was expected to match Pixel width");
            SAL_WARN_IF(std::abs(_rRect.GetSize().Height() - m_rTargetDevice.LogicToPixel(*i_pDeviceSize).Height()) > 1, "vcl", "DeviceSize height was expected to match Pixel height");
            aRect.SetSize(*i_pDeviceSize);
        }

        m_aCompleteTextRect.SetEmpty();
        m_rTargetDevice.DrawText( aRect, _rText, _nStyle, _pVector, _pDisplayText, this );
        tools::Rectangle aTextRect = m_aCompleteTextRect;

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
            for ( auto& rCharRect : *_pVector )
            {
                rCharRect = m_rTargetDevice.LogicToPixel( rCharRect );
            }
        }

        return aTextRect;
    }

    tools::Rectangle ReferenceDeviceTextLayout::GetTextRect( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, Size* o_pDeviceSize )
    {
        if ( _rText.isEmpty() )
            return tools::Rectangle();

        // determine text layout mode from the RTL-ness of the control whose text we render
        text::ComplexTextLayoutFlags nTextLayoutMode = m_bRTLEnabled ? text::ComplexTextLayoutFlags::BiDiRtl : text::ComplexTextLayoutFlags::Default;
        m_rReferenceDevice.SetLayoutMode( nTextLayoutMode );
        m_rTargetDevice.SetLayoutMode( nTextLayoutMode | text::ComplexTextLayoutFlags::TextOriginLeft );

        // ComplexTextLayoutFlags::TextOriginLeft is because when we do actually draw the text (in DrawText( Point, ... )), then
        // our caller gives us the left border of the draw position, regardless of script type, text layout,
        // and the like in our ctor, we set the map mode of the target device from pixel to twip, but our caller doesn't know this,
        // but passed pixel coordinates. So, adjust the rect.
        tools::Rectangle aRect( m_rTargetDevice.PixelToLogic( _rRect ) );

        tools::Rectangle aTextRect = m_rTargetDevice.GetTextRect( aRect, _rText, _nStyle, nullptr, this );

        //if o_pDeviceSize is available, stash the pre logic-to-pixel size in it
        if (o_pDeviceSize)
        {
            *o_pDeviceSize = aTextRect.GetSize();
        }

        // similar to above, the text rect now contains TWIPs (or whatever unit the ref device has), but the caller
        // expects pixel coordinates
        aTextRect = m_rTargetDevice.LogicToPixel( aTextRect );

        return aTextRect;
    }

    ControlTextRenderer::ControlTextRenderer( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice )
        :m_pImpl( new ReferenceDeviceTextLayout( _rControl, _rTargetDevice, _rReferenceDevice ) )
    {
    }

    ControlTextRenderer::~ControlTextRenderer()
    {
    }

    tools::Rectangle ControlTextRenderer::DrawText( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle,
        std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText, const Size* i_pDeviceSize )
    {
        return m_pImpl->DrawText( _rRect, _rText, _nStyle, _pVector, _pDisplayText, i_pDeviceSize );
    }

    tools::Rectangle ControlTextRenderer::GetTextRect( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, Size* o_pDeviceSize = nullptr )
    {
        return m_pImpl->GetTextRect( _rRect, _rText, _nStyle, o_pDeviceSize );
    }

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
