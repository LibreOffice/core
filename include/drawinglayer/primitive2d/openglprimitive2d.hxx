/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_OPENGL_PRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_OPENGL_PRIMITIVE2D_HXX

#include <tools/gen.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

namespace drawinglayer
{
    namespace primitive2d
    {

        class DRAWINGLAYER_DLLPUBLIC OpenGLPrimitive2D : public BasePrimitive2D
        {
        public:
            explicit OpenGLPrimitive2D(const Point& rPos);

            const Point& getPos() const { return m_aPos; }

            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// provide unique ID
            DeclPrimitive2DIDBlock()

        private:
            Point m_aPos;
        };

    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
