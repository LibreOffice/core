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

#ifndef INCLUDED_CANVAS_BASE_INTEGERBITMAPBASE_HXX
#define INCLUDED_CANVAS_BASE_INTEGERBITMAPBASE_HXX

#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <canvas/verifyinput.hxx>
#include <osl/diagnose.h>


namespace canvas
{
    /** Helper template to handle XIntegerBitmap method forwarding to
        BitmapCanvasHelper

        Use this helper to handle the XIntegerBitmap part of your
        implementation.

        @tpl Base
        Either BitmapCanvasBase (just XBitmap) or BitmapCanvasBase2 (XBitmap and
        XBitmapCanvas).
     */
    template< class Base > class IntegerBitmapBase :
        public Base
    {
    public:
        // XIntegerBitmap
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getData( css::rendering::IntegerBitmapLayout&     bitmapLayout,
                                                                              const css::geometry::IntegerRectangle2D& rect ) override
        {
            tools::verifyArgs(rect,
                              OSL_THIS_FUNC,
                              static_cast< typename Base::UnambiguousBaseType* >(this));
            tools::verifyIndexRange(rect, Base::getSize() );

            typename Base::MutexType aGuard( Base::m_aMutex );

            return Base::maCanvasHelper.getData( bitmapLayout,
                                                     rect );
        }

        virtual void SAL_CALL setData( const css::uno::Sequence< sal_Int8 >&,
                                       const css::rendering::IntegerBitmapLayout& bitmapLayout,
                                       const css::geometry::IntegerRectangle2D&   rect ) override
        {
            tools::verifyArgs(bitmapLayout, rect,
                              OSL_THIS_FUNC,
                              static_cast< typename Base::UnambiguousBaseType* >(this));
            tools::verifyIndexRange(rect, Base::getSize() );

            typename Base::MutexType aGuard( Base::m_aMutex );

            Base::mbSurfaceDirty = true;
        }

        virtual void SAL_CALL setPixel( const css::uno::Sequence< sal_Int8 >&,
                                        const css::rendering::IntegerBitmapLayout& bitmapLayout,
                                        const css::geometry::IntegerPoint2D&       pos ) override
        {
            tools::verifyArgs(bitmapLayout, pos,
                              OSL_THIS_FUNC,
                              static_cast< typename Base::UnambiguousBaseType* >(this));
            tools::verifyIndexRange(pos, Base::getSize() );

            typename Base::MutexType aGuard( Base::m_aMutex );

            Base::mbSurfaceDirty = true;
        }

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout,
                                                                  const css::geometry::IntegerPoint2D& pos ) override
        {
            tools::verifyArgs(pos,
                              OSL_THIS_FUNC,
                              static_cast< typename Base::UnambiguousBaseType* >(this));
            tools::verifyIndexRange(pos, Base::getSize() );

            typename Base::MutexType aGuard( Base::m_aMutex );

            return Base::maCanvasHelper.getPixel( bitmapLayout,
                                                      pos );
        }

        virtual css::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout(  ) override
        {
            typename Base::MutexType aGuard( Base::m_aMutex );

            return Base::maCanvasHelper.getMemoryLayout();
        }
    };
}

#endif // INCLUDED_CANVAS_BASE_INTEGERBITMAPBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
