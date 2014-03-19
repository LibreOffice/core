/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/primitive2d/openglprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>

using namespace drawinglayer::primitive2d;

OpenGLPrimitive2D::OpenGLPrimitive2D(const Point& rPos)
    : m_aPos(rPos)
{
}

bool OpenGLPrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
{
    if(BasePrimitive2D::operator==(rPrimitive))
    {
        const OpenGLPrimitive2D& rCompare = static_cast< const OpenGLPrimitive2D& >(rPrimitive);
        return m_aPos == rCompare.getPos();
    }
    return false;
}

ImplPrimitive2DIDBlock(OpenGLPrimitive2D, PRIMITIVE2D_ID_OPENGLPRIMITIVE2D)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
