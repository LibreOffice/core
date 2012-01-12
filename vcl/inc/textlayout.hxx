/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
