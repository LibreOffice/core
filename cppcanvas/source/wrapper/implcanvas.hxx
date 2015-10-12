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

#ifndef INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLCANVAS_HXX
#define INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLCANVAS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/ViewState.hpp>
#include <cppcanvas/canvas.hxx>

#include <boost/optional.hpp>


namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvas;
} } } }


/* Definition of ImplCanvas */

namespace cppcanvas
{

    namespace internal
    {

        class ImplCanvas : public virtual Canvas
        {
        public:
            explicit ImplCanvas( const css::uno::Reference< css::rendering::XCanvas >& rCanvas );
            virtual ~ImplCanvas();

            virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) SAL_OVERRIDE;
            virtual ::basegfx::B2DHomMatrix          getTransformation() const SAL_OVERRIDE;

            virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) SAL_OVERRIDE;
            virtual void                             setClip() SAL_OVERRIDE;
            virtual ::basegfx::B2DPolyPolygon const* getClip() const SAL_OVERRIDE;

            virtual ColorSharedPtr                   createColor() const SAL_OVERRIDE;

            virtual CanvasSharedPtr                  clone() const SAL_OVERRIDE;

            virtual void                             clear() const SAL_OVERRIDE;

            virtual css::uno::Reference<
                css::rendering::XCanvas >            getUNOCanvas() const SAL_OVERRIDE;

            virtual css::rendering::ViewState        getViewState() const SAL_OVERRIDE;

            // take compiler-provided default copy constructor
            //ImplCanvas(const ImplCanvas&);

        private:
            ImplCanvas& operator=( const ImplCanvas& ) = delete;

            mutable css::rendering::ViewState                    maViewState;
            boost::optional<basegfx::B2DPolyPolygon>             maClipPolyPolygon;
            const css::uno::Reference< css::rendering::XCanvas > mxCanvas;
        };

    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLCANVAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
