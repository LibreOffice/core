/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vcl/ctrl.hxx"
#include "vcl/outdev.hxx"
#include "vcl/outfont.hxx"
#include "vcl/textlayout.hxx"

//........................................................................
namespace vcl
{
//........................................................................

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

    public:
        // equivalents to the respective OutputDevice methods, which take the reference device into account
        long        GetTextArray( const XubString& _rText, sal_Int32* _pDXAry, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const;
        Rectangle   DrawText( const Rectangle& _rRect, const XubString& _rText, USHORT _nStyle, MetricVector* _pVector, String* _pDisplayText);

        bool        IsZoom() const;

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

        Rectangle       m_aCompleteTextRect;
    };

    //====================================================================
    //= ControlTextRenderer
    //====================================================================
    ReferenceDeviceTextLayout::ReferenceDeviceTextLayout( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice )
        :m_rTargetDevice( _rTargetDevice )
        ,m_rReferenceDevice( _rReferenceDevice )
        ,m_aUnzoomedPointFont( _rControl.GetUnzoomedControlPointFont() )
        ,m_aZoom( _rControl.GetZoom() )
    {
        // normally, controls simulate "zoom" by "zooming" the font. This is responsible for (part of) the discrepancies
        // between text in Writer and text in controls in Writer, though both have the same font.
        // So, if we have a zoom set at the control, then we do not scale the font, but instead modify the map mode
        // to accomodate for the zoom.
        if ( IsZoom() )
        {
            m_rTargetDevice.Push( PUSH_MAPMODE | PUSH_FONT );

            MapMode aDrawMapMode( m_rTargetDevice.GetMapMode() );
            OSL_ENSURE( aDrawMapMode.GetOrigin() == Point(), "ReferenceDeviceTextLayout::ReferenceDeviceTextLayout: uhm, the code below won't work here ..." );

            aDrawMapMode.SetScaleX( m_aZoom );    // TODO: shouldn't this be "current_scale * zoom"?
            aDrawMapMode.SetScaleY( m_aZoom );
            m_rTargetDevice.SetMapMode( aDrawMapMode );

            MapUnit eTargetMapUnit = m_rTargetDevice.GetMapMode().GetMapUnit();
            Font aDrawFont( m_aUnzoomedPointFont );
            if ( eTargetMapUnit == MAP_PIXEL )
                aDrawFont.SetSize( m_rTargetDevice.LogicToPixel( aDrawFont.GetSize(), MAP_POINT ) );
            else
                aDrawFont.SetSize( m_rTargetDevice.LogicToLogic( aDrawFont.GetSize(), MAP_POINT, eTargetMapUnit ) );
            _rTargetDevice.SetFont( aDrawFont );
        }

        // transfer font to the reference device
        m_rReferenceDevice.Push( PUSH_FONT );
        Font aRefFont( m_aUnzoomedPointFont );
        aRefFont.SetSize( OutputDevice::LogicToLogic(
            aRefFont.GetSize(), MAP_POINT, m_rReferenceDevice.GetMapMode().GetMapUnit() ) );
        m_rReferenceDevice.SetFont( aRefFont );

    }

    //--------------------------------------------------------------------
    ReferenceDeviceTextLayout::~ReferenceDeviceTextLayout()
    {
        if ( IsZoom() )
            m_rTargetDevice.Pop();
        m_rReferenceDevice.Pop();
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

        //................................................................
        class DeviceUnitMapping
        {
        public:
            DeviceUnitMapping( const OutputDevice& _rTargetDevice, const OutputDevice& _rReferenceDevice )
                :m_rTargetDevice( _rTargetDevice )
                ,m_rReferenceDevice( _rReferenceDevice )
                ,m_eTargetMapUnit( _rTargetDevice.GetMapMode().GetMapUnit() )
                ,m_bTargetIsPixel( _rTargetDevice.GetMapMode().GetMapUnit() == MAP_PIXEL )
                ,m_eRefMapUnit( _rReferenceDevice.GetMapMode().GetMapUnit() )
            {
                OSL_ENSURE( m_eRefMapUnit != MAP_PIXEL, "a reference device with MAP_PIXEL?" );
            }

            long mapToTarget( long _nWidth )
            {
                return  m_bTargetIsPixel
                    ?   m_rTargetDevice.LogicToPixel( Size( _nWidth, 0 ), m_eRefMapUnit ).Width()
                    :   OutputDevice::LogicToLogic( Size( _nWidth, 0 ), m_eRefMapUnit, m_eTargetMapUnit ).Width();
            }
            long mapToReference( long _nWidth )
            {
                return  m_bTargetIsPixel
                    ?   m_rTargetDevice.PixelToLogic( Size( _nWidth, 0 ), m_eRefMapUnit ).Width()
                    :   OutputDevice::LogicToLogic( Size( _nWidth, 0 ), m_eTargetMapUnit, m_eRefMapUnit ).Width();
            }

        private:
            const OutputDevice& m_rTargetDevice;
            const OutputDevice& m_rReferenceDevice;
            const MapUnit       m_eTargetMapUnit;
            const bool          m_bTargetIsPixel;
            const MapUnit       m_eRefMapUnit;
        };
    }

    //--------------------------------------------------------------------
    bool ReferenceDeviceTextLayout::IsZoom() const
    {
        return m_aZoom.GetNumerator() != m_aZoom.GetDenominator();
    }

    //--------------------------------------------------------------------
    long ReferenceDeviceTextLayout::GetTextArray( const XubString& _rText, sal_Int32* _pDXAry, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        // retrieve the character widths from the reference device
        long nTextWidth = m_rReferenceDevice.GetTextArray( _rText, _pDXAry, _nStartIndex, _nLength );

        // adjust the widths, which are in ref-device units, to the target device
        DeviceUnitMapping aMapping( m_rTargetDevice, m_rReferenceDevice );
        if ( _pDXAry )
        {
            for ( size_t i=0; i<_nLength; ++i )
                _pDXAry[i] = aMapping.mapToTarget( _pDXAry[i] );
        }
        nTextWidth = aMapping.mapToTarget( nTextWidth );

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

        sal_Int32* pCharWidths = new sal_Int32[ _nLength ];
        long nTextWidth = GetTextArray( _rText, pCharWidths, _nStartIndex, _nLength );
        m_rTargetDevice.DrawTextArray( _rStartPoint, _rText, pCharWidths, _nStartIndex, _nLength );
        delete[] pCharWidths;

        m_aCompleteTextRect.Union( Rectangle( _rStartPoint, Size( nTextWidth, m_rTargetDevice.GetTextHeight() ) ) );

        // TODO: use/fill those:
        (void)_pVector;
        (void)_pDisplayText;
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

        // adjust the positions, which are in ref-device units, to the target device
        DeviceUnitMapping aMapping( m_rTargetDevice, m_rReferenceDevice );
        for ( size_t i=0; i<2*size_t(_nLength); ++i )
            _pCaretXArray[i] = aMapping.mapToTarget( _pCaretXArray[i] );

        return true;
    }

    //--------------------------------------------------------------------
    xub_StrLen ReferenceDeviceTextLayout::GetTextBreak( const XubString& _rText, long _nMaxTextWidth, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        DeviceUnitMapping aMapping( m_rTargetDevice, m_rReferenceDevice );
        return m_rReferenceDevice.GetTextBreak( _rText, aMapping.mapToReference( _nMaxTextWidth ), _nStartIndex, _nLength );
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
    Rectangle ReferenceDeviceTextLayout::DrawText( const Rectangle& _rRect, const XubString& _rText, USHORT _nStyle, MetricVector* _pVector, String* _pDisplayText)
    {
        Rectangle aRect( _rRect );
        if ( IsZoom() )
        {
            // if there's a zoom factor involved, then we tampered with the target device's map mode in the ctor.
            // Need to adjust the rectangle to this
            aRect.Left()    = unzoomBy( aRect.Left(),   m_aZoom );
            aRect.Right()   = unzoomBy( aRect.Right(),  m_aZoom );
            aRect.Top()     = unzoomBy( aRect.Top(),    m_aZoom );
            aRect.Bottom()  = unzoomBy( aRect.Bottom(), m_aZoom );
        }

#ifdef FS_DEBUG
        m_rTargetDevice.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
        m_rTargetDevice.SetLineColor( COL_LIGHTBLUE );
        m_rTargetDevice.SetFillColor();
        m_rTargetDevice.DrawRect( aRect );
        m_rTargetDevice.Pop();
#endif
        onBeginDrawText();
        m_rTargetDevice.DrawText( aRect, _rText, _nStyle, _pVector, _pDisplayText, this );
        Rectangle aTextRect = onEndDrawText();

        if ( IsZoom() )
        {
            // similar to above, transform the to-be-returned rectanle to coordinates which are meaningful
            // with the original map mode of the target device
            aTextRect.Left()    = zoomBy( aTextRect.Left(),   m_aZoom );
            aTextRect.Right()   = zoomBy( aTextRect.Right(),  m_aZoom );
            aTextRect.Top()     = zoomBy( aTextRect.Top(),    m_aZoom );
            aTextRect.Bottom()  = zoomBy( aTextRect.Bottom(), m_aZoom );
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
    Rectangle ControlTextRenderer::DrawText( const Rectangle& _rRect, const XubString& _rText, USHORT _nStyle,
        MetricVector* _pVector, String* _pDisplayText )
    {
        return m_pImpl->DrawText( _rRect, _rText, _nStyle, _pVector, _pDisplayText );
    }

//........................................................................
} // namespace vcl
//........................................................................
