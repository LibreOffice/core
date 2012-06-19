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

#ifndef _DXCANVAS_DXIBITMAP_HXX
#define _DXCANVAS_DXIBITMAP_HXX

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <boost/shared_ptr.hpp>
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include "dx_graphicsprovider.hxx"

namespace dxcanvas
{
    /// Interface for internal canvas bitmap objects
    struct IBitmap : public GraphicsProvider
    {
        virtual BitmapSharedPtr           getBitmap() const = 0;
        virtual ::basegfx::B2IVector      getSize() const = 0;
        virtual bool                      hasAlpha() const = 0;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > getData(
            ::com::sun::star::rendering::IntegerBitmapLayout&       bitmapLayout,
            const ::com::sun::star::geometry::IntegerRectangle2D&   rect ) = 0;

        virtual void setData(
            const ::com::sun::star::uno::Sequence< sal_Int8 >&      data,
            const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
            const ::com::sun::star::geometry::IntegerRectangle2D&   rect ) = 0;

        virtual void setPixel(
            const ::com::sun::star::uno::Sequence< sal_Int8 >&      color,
            const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
            const ::com::sun::star::geometry::IntegerPoint2D&       pos ) = 0;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > getPixel(
            ::com::sun::star::rendering::IntegerBitmapLayout&       bitmapLayout,
            const ::com::sun::star::geometry::IntegerPoint2D&       pos ) = 0;
    };

    typedef boost::shared_ptr<IBitmap> IBitmapSharedPtr;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
