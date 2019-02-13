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

#ifndef INCLUDED_CANVAS_RENDERING_ISURFACE_HXX
#define INCLUDED_CANVAS_RENDERING_ISURFACE_HXX

namespace basegfx
{
    class B2IRange;
    class B2IPoint;
}

namespace canvas
{
    struct IColorBuffer;

    struct ISurface
    {
        virtual ~ISurface() {}

        /** Select texture behind this interface to be the current one
            for primitive output.
         */
        virtual bool selectTexture() = 0;

        /** Tells whether the surface is valid or not
        */
        virtual bool isValid() = 0;

        /** Update surface content from given IColorBuffer

            This method updates the given subarea of the surface from
            the given color buffer bits.

            @param rDestPos
            Position in the surface, where the subset update should
            have its left, top edge

            @param rSourceRect
            Size and position of the rectangular subset update in the
            source color buffer

            @param rSource
            Source bits to use for the update

            @return true, if the update was successful
         */
        virtual bool update( const ::basegfx::B2IPoint& rDestPos,
                             const ::basegfx::B2IRange& rSourceRect,
                             IColorBuffer&              rSource ) = 0;
    };
}

#endif // INCLUDED_CANVAS_RENDERING_ISURFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
