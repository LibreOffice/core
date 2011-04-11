/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
 ***********************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vcl/ctrl.hxx"
#include "vcl/outdev.hxx"
#include "vcl/outfont.hxx"
#include "vcl/textlayout.hxx"

#include <com/sun/star/i18n/ScriptDirection.hpp>

#include <tools/diagnose_ex.h>

#if OSL_DEBUG_LEVEL > 1
#include <rtl/strbuf.hxx>
#endif

//........................................................................
namespace vcl
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Exception;
    namespace ScriptDirection = ::com::sun::star::i18n::ScriptDirection;

    //====================================================================
    //= DefaultTextLayout
    //====================================================================
    //--------------------------------------------------------------------
    DefaultTextLayout::~DefaultTextLayout()
    {
    }

    //--------------------------------------------------------------------
    long DefaultTextLayout::GetTextWidth( const XubString& _rText, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const
    {
        return m_rTargetDevice.GetTextWidth( _rText, _nStartIndex, _nLength );
    }

    //--------------------------------------------------------------------
    void DefaultTextLayout::DrawText( const Point& _rStartPoint, const XubString& _rText, xub_StrLen _nStartIndex,
        xub_StrLen _nLength, MetricVector* _pVector, String* _pDisplayText )
    {
        m_rTargetDevice.DrawText( _rStartPoint, _rText, _nStartIndex, _nLength, _pVector, _pDisplayText );
    }

    //--------------------------------------------------------------------
    bool DefaultTextLayout::GetCaretPositions( const XubString& _rText, sal_Int32* _pCaretXArray,
        xub_StrLen _nStartIndex, xub_StrLen _nLength ) const
    {
        return m_rTargetDevice.GetCaretPositions( _rText, _pCaretXArray, _nStartIndex, _nLength );
    }

    //--------------------------------------------------------------------
    xub_StrLen DefaultTextLayout::GetTextBreak( const XubString& _rText, long _nMaxTextWidth, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const
    {
        return m_rTargetDevice.GetTextBreak( _rText, _nMaxTextWidth, _nStartIndex, _nLength );
    }

    //--------------------------------------------------------------------
    bool DefaultTextLayout::DecomposeTextRectAction() const
    {
        return false;
    }

    //====================================================================
    //= ReferenceDeviceTextLayout
    //====================================================================
    class ReferenceDeviceTextLayout : public ITextLayout
    {
    public:
        ReferenceDeviceTextLayout( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice );
        virtual ~ReferenceDeviceTextLayout();

        // ITextLayout
        virtual long        GetTextWidth( const XubString& rStr, xub_StrLen nIndex, xub_StrLen nLen ) const;
        virtual void        DrawText( const Point& _rStartPoint, const XubString& _rText, xub_StrLen _nStartIndex, xub_StrLen _nLength, MetricVector* _pVector, String* _pDisplayText );
        virtual bool        GetCaretPositions( const XubString& _rText, sal_Int32* _pCaretXArray, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const;
        virtual xub_StrLen  GetTextBreak( const XubString& _rText, long _nMaxTextWidth, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const;
        virtual bool        DecomposeTextRectAction() const;

    public:
        // equivalents to the respective OutputDevice methods, which take the reference device into account
        long        GetTextArray( const XubString& _rText, sal_Int32* _pDXAry, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const;
        Rectangle   DrawText( const Rectangle& _rRect, const XubString& _rText, sal_uInt16 _nStyle, MetricVector* _pVector, String* _pDisplayText );

    protected:
        void onBeginDrawText()
        {
            m_aCompleteTextRect.SetEmpty();
        }
        Rectangle onEndDrawText()
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

    //====================================================================
    //= ControlTextRenderer
    //====================================================================
    ReferenceDeviceTextLayout::ReferenceDeviceTextLayout( const Control& _rControl, OutputDevice& _rTargetDevice,
        OutputDevice& _rReferenceDevice )
        :m_rTargetDevice( _rTargetDevice )
        ,m_rReferenceDevice( _rReferenceDevice )
        ,m_aUnzoomedPointFont( _rControl.GetUnzoomedControlPointFont() )
        ,m_aZoom( _rControl.GetZoom() )
        ,m_bRTLEnabled( _rControl.IsRTLEnabled() )
    {
        m_rTargetDevice.Push( PUSH_MAPMODE | PUSH_FONT | PUSH_TEXTLAYOUTMODE );

        MapMode aTargetMapMode( m_rTargetDevice.GetMapMode() );
        OSL_ENSURE( aTargetMapMode.GetOrigin() == Point(), "ReferenceDeviceTextLayout::ReferenceDeviceTextLayout: uhm, the code below won't work here ..." );

        // normally, controls simulate "zoom" by "zooming" the font. This is responsible for (part of) the discrepancies
        // between text in Writer and text in controls in Writer, though both have the same font.
        // So, if we have a zoom set at the control, then we do not scale the font, but instead modify the map mode
        // to accomodate for the zoom.
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
        aDrawFont.SetSize( m_rTargetDevice.LogicToLogic( aDrawFont.GetSize(), MAP_POINT, eTargetMapUnit ) );
        _rTargetDevice.SetFont( aDrawFont );

        // transfer font to the reference device
        m_rReferenceDevice.Push( PUSH_FONT | PUSH_TEXTLAYOUTMODE );
        Font aRefFont( m_aUnzoomedPointFont );
        aRefFont.SetSize( OutputDevice::LogicToLogic(
            aRefFont.GetSize(), MAP_POINT, m_rReferenceDevice.GetMapMode().GetMapUnit() ) );
        m_rReferenceDevice.SetFont( aRefFont );
    }

    //--------------------------------------------------------------------
    ReferenceDeviceTextLayout::~ReferenceDeviceTextLayout()
    {
        m_rReferenceDevice.Pop();
        m_rTargetDevice.Pop();
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        bool lcl_normalizeLength( const XubString& _rText, const xub_StrLen _nStartIndex, xub_StrLen& _io_nLength )
        {
            xub_StrLen nTextLength = _rText.Len();
            if ( _nStartIndex > nTextLength )
                return false;
            if ( _nStartIndex + _io_nLength > nTextLength )
                _io_nLength = nTextLength - _nStartIndex;
            return true;
        }
    }

    //--------------------------------------------------------------------
    long ReferenceDeviceTextLayout::GetTextArray( const XubString& _rText, sal_Int32* _pDXAry, xub_StrLen _nStartIndex,
        xub_StrLen _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        // retrieve the character widths from the reference device
        long nTextWidth = m_rReferenceDevice.GetTextArray( _rText, _pDXAry, _nStartIndex, _nLength );
#if OSL_DEBUG_LEVEL > 1
        if ( _pDXAry )
        {
            ::rtl::OStringBuffer aTrace;
            aTrace.append( "ReferenceDeviceTextLayout::GetTextArray( " );
            aTrace.append( ::rtl::OUStringToOString( _rText, RTL_TEXTENCODING_UTF8 ) );
            aTrace.append( " ): " );
            aTrace.append( nTextWidth );
            aTrace.append( " = ( " );
            for ( size_t i=0; i<_nLength; )
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

    //--------------------------------------------------------------------
    long ReferenceDeviceTextLayout::GetTextWidth( const XubString& _rText, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const
    {
        return GetTextArray( _rText, NULL, _nStartIndex, _nLength );
    }

    //--------------------------------------------------------------------
    void ReferenceDeviceTextLayout::DrawText( const Point& _rStartPoint, const XubString& _rText, xub_StrLen _nStartIndex, xub_StrLen _nLength, MetricVector* _pVector, String* _pDisplayText )
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
            _pDisplayText->Append( _rText.Copy( _nStartIndex, _nLength ) );
            return;
        }

        sal_Int32* pCharWidths = new sal_Int32[ _nLength ];
        long nTextWidth = GetTextArray( _rText, pCharWidths, _nStartIndex, _nLength );
        m_rTargetDevice.DrawTextArray( _rStartPoint, _rText, pCharWidths, _nStartIndex, _nLength );
        delete[] pCharWidths;

        m_aCompleteTextRect.Union( Rectangle( _rStartPoint, Size( nTextWidth, m_rTargetDevice.GetTextHeight() ) ) );
    }

    //--------------------------------------------------------------------
    bool ReferenceDeviceTextLayout::GetCaretPositions( const XubString& _rText, sal_Int32* _pCaretXArray,
        xub_StrLen _nStartIndex, xub_StrLen _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return false;

        // retrieve the caret positions from the reference device
        if ( !m_rReferenceDevice.GetCaretPositions( _rText, _pCaretXArray, _nStartIndex, _nLength ) )
            return false;

        return true;
    }

    //--------------------------------------------------------------------
    xub_StrLen ReferenceDeviceTextLayout::GetTextBreak( const XubString& _rText, long _nMaxTextWidth, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        return m_rReferenceDevice.GetTextBreak( _rText, _nMaxTextWidth, _nStartIndex, _nLength );
    }

    //--------------------------------------------------------------------
    bool ReferenceDeviceTextLayout::DecomposeTextRectAction() const
    {
        return true;
    }

    //--------------------------------------------------------------------
    namespace
    {
        long zoomBy( long _value, const Fraction& _zoom )
        {
            double n = (double)_value;
            n *= (double)_zoom.GetNumerator();
            n /= (double)_zoom.GetDenominator();
            return (long)::rtl::math::round( n );
        }
        long unzoomBy( long _value, const Fraction& _zoom )
        {
            return zoomBy( _value, Fraction( _zoom.GetDenominator(), _zoom.GetNumerator() ) );
        }
    }

    //--------------------------------------------------------------------
    Rectangle ReferenceDeviceTextLayout::DrawText( const Rectangle& _rRect, const XubString& _rText, sal_uInt16 _nStyle, MetricVector* _pVector, String* _pDisplayText )
    {
        if ( !_rText.Len() )
            return Rectangle();

        // determine text layout mode from the RTL-ness of the control whose text we render
        sal_uLong nTextLayoutMode = m_bRTLEnabled ? TEXT_LAYOUT_BIDI_RTL : TEXT_LAYOUT_BIDI_LTR;
        m_rReferenceDevice.SetLayoutMode( nTextLayoutMode );
        m_rTargetDevice.SetLayoutMode( nTextLayoutMode | TEXT_LAYOUT_TEXTORIGIN_LEFT );
            // TEXT_LAYOUT_TEXTORIGIN_LEFT is because when we do actually draw the text (in DrawText( Point, ... )), then
            // our caller gives us the left border of the draw position, regardless of script type, text layout,
            // and the like

        // in our ctor, we set the map mode of the target device from pixel to twip, but our caller doesn't know this,
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
            // single "DrawText( Point, ... )" calls, since more intermediate arithmetics will translate
            // from ref- to target-units.
            aTextRect = m_rTargetDevice.GetTextRect( aRect, _rText, _nStyle, NULL, this );
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

    //====================================================================
    //= ControlTextRenderer
    //====================================================================
    //--------------------------------------------------------------------
    ControlTextRenderer::ControlTextRenderer( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice )
        :m_pImpl( new ReferenceDeviceTextLayout( _rControl, _rTargetDevice, _rReferenceDevice ) )
    {
    }

    //--------------------------------------------------------------------
    ControlTextRenderer::~ControlTextRenderer()
    {
    }

    //--------------------------------------------------------------------
    Rectangle ControlTextRenderer::DrawText( const Rectangle& _rRect, const XubString& _rText, sal_uInt16 _nStyle,
        MetricVector* _pVector, String* _pDisplayText )
    {
        return m_pImpl->DrawText( _rRect, _rText, _nStyle, _pVector, _pDisplayText );
    }

//........................................................................
} // namespace vcl
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
