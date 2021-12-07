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

#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        void BackgroundColorPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
        {
            if(!rViewInformation.getViewport().isEmpty())
            {
                const basegfx::B2DPolygon aOutline(basegfx::utils::createPolygonFromRect(rViewInformation.getViewport()));
                rContainer.push_back(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aOutline), getBColor()));
            }
        }

        BackgroundColorPrimitive2D::BackgroundColorPrimitive2D(
            const basegfx::BColor& rBColor,
            double fTransparency)
        :   maBColor(rBColor),
            mfTransparency(fTransparency)
        {
        }

        bool BackgroundColorPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const BackgroundColorPrimitive2D& rCompare = static_cast<const BackgroundColorPrimitive2D&>(rPrimitive);

                return (getBColor() == rCompare.getBColor() && getTransparency() == rCompare.getTransparency());
            }

            return false;
        }

        basegfx::B2DRange BackgroundColorPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // always as big as the view
            return rViewInformation.getViewport();
        }

        void BackgroundColorPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            if(!getBuffered2DDecomposition().empty() && (maLastViewport != rViewInformation.getViewport()))
            {
                // conditions of last local decomposition have changed, delete
                const_cast< BackgroundColorPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
            }

            if(getBuffered2DDecomposition().empty())
            {
                // remember ViewRange
                const_cast< BackgroundColorPrimitive2D* >(this)->maLastViewport = rViewInformation.getViewport();
            }

            // use parent implementation
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        sal_uInt32 BackgroundColorPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
