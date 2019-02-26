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

#ifndef INCLUDED_CPPCANVAS_BITMAPCANVAS_HXX
#define INCLUDED_CPPCANVAS_BITMAPCANVAS_HXX

#include <basegfx/vector/b2isize.hxx>
#include <cppcanvas/canvas.hxx>
#include <memory>


/* Definition of BitmapCanvas */

namespace cppcanvas
{
    class BitmapCanvas;

    // forward declaration, since cloneBitmapCanvas() also references BitmapCanvas
    typedef std::shared_ptr< BitmapCanvas > BitmapCanvasSharedPtr;

    /** BitmapCanvas interface
     */
    class BitmapCanvas : public virtual Canvas
    {
    public:
        virtual ::basegfx::B2ISize      getSize() const = 0;
    };

}

#endif // INCLUDED_CPPCANVAS_BITMAPCANVAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
