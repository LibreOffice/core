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

#include <drawinglayer/processor2d/textextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

namespace drawinglayer::processor2d
{
void TextExtractor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    switch (rCandidate.getPrimitive2DID())
    {
        case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
        case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
        {
            processTextPrimitive2D(rCandidate);
            break;
        }

        // usage of transformation stack is needed
        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
        {
            // remember current transformation and ViewInformation
            const primitive2d::TransformPrimitive2D& rTransformCandidate(
                static_cast<const primitive2d::TransformPrimitive2D&>(rCandidate));
            const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

            // create new transformations for CurrentTransformation and for local ViewInformation2D
            geometry::ViewInformation2D aViewInformation2D(getViewInformation2D());
            aViewInformation2D.setObjectTransformation(
                getViewInformation2D().getObjectTransformation()
                * rTransformCandidate.getTransformation());
            setViewInformation2D(aViewInformation2D);

            // process content
            process(rTransformCandidate.getChildren());

            // restore transformations
            setViewInformation2D(aLastViewInformation2D);

            break;
        }

        // ignorable primitives
        case PRIMITIVE2D_ID_SCENEPRIMITIVE2D:
        case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D:
        case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D:
        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
        case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D:
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
            break;

        default:
        {
            // process recursively
            process(rCandidate);
            break;
        }
    }
}

TextExtractor2D::TextExtractor2D(const geometry::ViewInformation2D& rViewInformation)
    : BaseProcessor2D(rViewInformation)
{
}

TextExtractor2D::~TextExtractor2D() {}
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
