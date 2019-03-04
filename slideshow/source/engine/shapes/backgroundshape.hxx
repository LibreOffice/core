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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_BACKGROUNDSHAPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_BACKGROUNDSHAPE_HXX

#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star { namespace drawing
{
    class XDrawPage;
} } } }

namespace slideshow
{
    namespace internal
    {
        class  Shape;
        struct SlideShowContext;
        typedef ::std::shared_ptr< Shape > ShapeSharedPtr;

        /** Representation of a draw document's background shape.

            This function generates the Shape for the background
            shape. Since the background shape is neither animatable
            nor attributable, those more specialized derivations of
            the Shape interface are not implemented here.
         */
        ShapeSharedPtr createBackgroundShape(
            const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
            const css::uno::Reference< css::drawing::XDrawPage >& xMasterPage,
            const SlideShowContext&                       rContext ); // throw ShapeLoadFailedException;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_BACKGROUNDSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
