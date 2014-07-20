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

#ifndef INCLUDED_VCL_INC_TEXTLAYOUT_HXX
#define INCLUDED_VCL_INC_TEXTLAYOUT_HXX

#include <memory>
#include <rtl/ustring.hxx>
#include <vcl/outdev.hxx>

class Control;

namespace vcl
{
    class SAL_NO_VTABLE ITextLayout
    {
    public:
        virtual long        GetTextWidth( const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const = 0;
        virtual void        DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength,
                                MetricVector* _pVector, OUString* _pDisplayText ) = 0;
        virtual bool        GetCaretPositions( const OUString& _rText, long* _pCaretXArray, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const = 0;
        virtual sal_Int32   GetTextBreak( const OUString& _rText, long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const = 0;
        virtual bool        DecomposeTextRectAction() const = 0;

    protected:
        ~ITextLayout() {}
    };

    /** is an implementation of the ITextLayout interface which simply delegates its calls to the respective
        methods of an OutputDevice instance, without any inbetween magic.
    */
    class DefaultTextLayout : public ITextLayout
    {
    public:
        DefaultTextLayout( OutputDevice& _rTargetDevice )
            : m_rTargetDevice( _rTargetDevice )
        {
        }
        virtual ~DefaultTextLayout();

        // ITextLayout overridables
        virtual long        GetTextWidth( const OUString& _rText,
                                          sal_Int32 _nStartIndex,
                                          sal_Int32 _nLength ) const SAL_OVERRIDE;

        virtual void        DrawText( const Point& _rStartPoint,
                                      const OUString& _rText,
                                      sal_Int32 _nStartIndex,
                                      sal_Int32 _nLength,
                                      MetricVector* _pVector,
                                      OUString* _pDisplayText ) SAL_OVERRIDE;

        virtual bool        GetCaretPositions( const OUString& _rText,
                                               long* _pCaretXArray,
                                               sal_Int32 _nStartIndex,
                                               sal_Int32 _nLength ) const SAL_OVERRIDE;

        virtual sal_Int32   GetTextBreak( const OUString& _rText,
                                          long _nMaxTextWidth,
                                          sal_Int32 _nStartIndex,
                                          sal_Int32 _nLength ) const SAL_OVERRIDE;

        virtual bool        DecomposeTextRectAction() const SAL_OVERRIDE;

    private:
        OutputDevice&   m_rTargetDevice;
    };

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
                              const OUString& _rText, sal_uInt16 _nStyle = 0,
                              MetricVector* _pVector = NULL, OUString* _pDisplayText = NULL );

    private:
        ControlTextRenderer();                                                  // never implemented
        ControlTextRenderer( const ControlTextRenderer& );              // never implemented
        ControlTextRenderer& operator=( const ControlTextRenderer& );   // never implemented

    private:
        ::std::auto_ptr< ReferenceDeviceTextLayout >   m_pImpl;
    };

} // namespace vcl

#endif // INCLUDED_VCL_INC_TEXTLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
