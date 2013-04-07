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

#ifndef _CPPCANVAS_IMPLCANVAS_HXX
#define _CPPCANVAS_IMPLCANVAS_HXX

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
            ImplCanvas( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::rendering::XCanvas >& rCanvas );
            virtual ~ImplCanvas();

            virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual ::basegfx::B2DHomMatrix          getTransformation() const;

            virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void                             setClip();
            virtual ::basegfx::B2DPolyPolygon const* getClip() const;

            virtual FontSharedPtr                    createFont( const OUString& rFontName, const double& rCellSize ) const;

            virtual ColorSharedPtr                   createColor() const;

            virtual CanvasSharedPtr                  clone() const;

            virtual void                             clear() const;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvas >      getUNOCanvas() const;

            virtual ::com::sun::star::rendering::ViewState  getViewState() const;

            // take compiler-provided default copy constructor
            //ImplCanvas(const ImplCanvas&);

        private:
            // default: disabled assignment
            ImplCanvas& operator=( const ImplCanvas& );

            mutable ::com::sun::star::rendering::ViewState                                  maViewState;
            boost::optional<basegfx::B2DPolyPolygon>                                        maClipPolyPolygon;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >  mxCanvas;
        };

    }
}

#endif /* _CPPCANVAS_IMPLCANVAS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
