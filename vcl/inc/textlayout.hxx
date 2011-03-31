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

#ifndef VCL_TEXTLAYOUT_HXX
#define VCL_TEXTLAYOUT_HXX

#include "vcl/outdev.hxx"

#include <tools/solar.h>
#include <tools/string.hxx>

#include <memory>

class Control;

//........................................................................
namespace vcl
{
//........................................................................

    //====================================================================
    //= ITextLayout
    //====================================================================
    class SAL_NO_VTABLE ITextLayout
    {
    public:
        virtual long        GetTextWidth( const XubString& _rText, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const = 0;
        virtual void        DrawText( const Point& _rStartPoint, const XubString& _rText, xub_StrLen _nStartIndex, xub_StrLen _nLength,
                                MetricVector* _pVector, String* _pDisplayText ) = 0;
        virtual bool        GetCaretPositions( const XubString& _rText, sal_Int32* _pCaretXArray, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const = 0;
        virtual xub_StrLen  GetTextBreak( const XubString& _rText, long _nMaxTextWidth, xub_StrLen _nStartIndex, xub_StrLen _nLength ) const = 0;
        virtual bool        DecomposeTextRectAction() const = 0;
    };

    //====================================================================
    //= DefaultTextLayout
    //====================================================================
    /** is an implementation of the ITextLayout interface which simply delegates its calls to the respective
        methods of an OutputDevice instance, without any inbetween magic.
    */
    class DefaultTextLayout : public ITextLayout
    {
    public:
        DefaultTextLayout( OutputDevice& _rTargetDevice )
            :m_rTargetDevice( _rTargetDevice )
        {
        }
        virtual ~DefaultTextLayout();

        // ITextLayout overridables
        virtual long        GetTextWidth(
                                const XubString& _rText,
                                xub_StrLen _nStartIndex,
                                xub_StrLen _nLength
                            ) const;
        virtual void        DrawText(
                                const Point& _rStartPoint,
                                const XubString& _rText,
                                xub_StrLen _nStartIndex,
                                xub_StrLen _nLength,
                                MetricVector* _pVector,
                                String* _pDisplayText
                            );
        virtual bool        GetCaretPositions(
                                const XubString& _rText,
                                sal_Int32* _pCaretXArray,
                                xub_StrLen _nStartIndex,
                                xub_StrLen _nLength
                            ) const;
        virtual xub_StrLen  GetTextBreak(
                                const XubString& _rText,
                                long _nMaxTextWidth,
                                xub_StrLen _nStartIndex,
                                xub_StrLen _nLength
                            ) const;
        virtual bool        DecomposeTextRectAction() const;

    private:
        OutputDevice&   m_rTargetDevice;
    };

    //====================================================================
    //= ControlTextRenderer
    //====================================================================
    class ReferenceDeviceTextLayout;
    /** a class which allows rendering text of a Control onto a device, by taking into account the metrics of
        a reference device.
    */
    class ControlTextRenderer
    {
    public:
        ControlTextRenderer( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice );
        virtual ~ControlTextRenderer();

        Rectangle   DrawText( const Rectangle& _rRect,
                              const XubString& _rText, sal_uInt16 _nStyle = 0,
                              MetricVector* _pVector = NULL, String* _pDisplayText = NULL );

    private:
        ControlTextRenderer();                                                  // never implemented
        ControlTextRenderer( const ControlTextRenderer& );              // never implemented
        ControlTextRenderer& operator=( const ControlTextRenderer& );   // never implemented

    private:
        ::std::auto_ptr< ReferenceDeviceTextLayout >   m_pImpl;
    };

//........................................................................
} // namespace vcl
//........................................................................

#endif // VCL_TEXTLAYOUT_HXX
