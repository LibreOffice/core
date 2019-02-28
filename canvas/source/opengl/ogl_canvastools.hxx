/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASTOOLS_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASTOOLS_HXX

#include <sal/config.h>
#include <vector>

namespace com { namespace sun { namespace star { namespace rendering {
    struct ARGBColor;
}}}}
namespace basegfx {
    class B2DPolyPolygon;
    class B2DHomMatrix;
}

namespace oglcanvas
{
    void renderComplexPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly );
    void renderPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly );
    void setupState( const ::basegfx::B2DHomMatrix&                rTransform,
                     GLenum                                        eSrcBlend,
                     GLenum                                        eDstBlend,
                     const com::sun::star::rendering::ARGBColor&              rColor );

    void renderOSD( const std::vector<double>& rNumbers, double scale );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
