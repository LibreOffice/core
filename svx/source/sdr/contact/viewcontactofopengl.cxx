/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/XShape.hpp>
#include <svx/sdr/contact/viewcontactofopengl.hxx>
#include <drawinglayer/primitive2d/openglprimitive2d.hxx>
#include <svx/svdoopengl.hxx>
#include <tools/gen.hxx>


namespace sdr
{
    namespace contact
    {

        ViewContactOfOpenGL::ViewContactOfOpenGL(SdrOpenGLObj& rOpenGLObj)
            : ViewContactOfSdrObj(rOpenGLObj)
        {
        }

        ViewContactOfOpenGL::~ViewContactOfOpenGL()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfOpenGL::createViewIndependentPrimitive2DSequence() const
        {
            com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape(GetSdrObject().getUnoShape(), com::sun::star::uno::UNO_QUERY);
            const Point aPos(xShape->getPosition().X,xShape->getPosition().Y);

            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::OpenGLPrimitive2D(aPos));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
