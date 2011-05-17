/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        TransformPrimitive3D::TransformPrimitive3D(
            const basegfx::B3DHomMatrix& rTransformation,
            const Primitive3DSequence& rChildren)
        :   GroupPrimitive3D(rChildren),
            maTransformation(rTransformation)
        {
        }

        bool TransformPrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(GroupPrimitive3D::operator==(rPrimitive))
            {
                const TransformPrimitive3D& rCompare = static_cast< const TransformPrimitive3D& >(rPrimitive);

                return (getTransformation() == rCompare.getTransformation());
            }

            return false;
        }

        basegfx::B3DRange TransformPrimitive3D::getB3DRange(const geometry::ViewInformation3D& rViewInformation) const
        {
            basegfx::B3DRange aRetval(getB3DRangeFromPrimitive3DSequence(getChildren(), rViewInformation));
            aRetval.transform(getTransformation());
            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(TransformPrimitive3D, PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
