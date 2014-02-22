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

#include <drawinglayer/primitive2d/cropprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        CropPrimitive2D::CropPrimitive2D(
            const Primitive2DSequence& rChildren,
            const basegfx::B2DHomMatrix& rTransformation,
            double fCropLeft,
            double fCropTop,
            double fCropRight,
            double fCropBottom)
        :   GroupPrimitive2D(rChildren),
            maTransformation(rTransformation),
            mfCropLeft(fCropLeft),
            mfCropTop(fCropTop),
            mfCropRight(fCropRight),
            mfCropBottom(fCropBottom)
        {
        }

        bool CropPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const CropPrimitive2D& rCompare = static_cast< const CropPrimitive2D& >(rPrimitive);

                return (getTransformation() == rCompare.getTransformation()
                    && getCropLeft() == rCompare.getCropLeft()
                    && getCropTop() == rCompare.getCropTop()
                    && getCropRight() == rCompare.getCropRight()
                    && getCropBottom() == rCompare.getCropBottom());
            }

            return false;
        }

        Primitive2DSequence CropPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;

            if(getChildren().hasElements())
            {
                
                const basegfx::B2DVector aObjectScale(basegfx::absolute(getTransformation() * basegfx::B2DVector(1.0, 1.0)));

                
                
                if(!aObjectScale.equalZero())
                {
                    
                    
                    const double fBackScaleX(basegfx::fTools::equalZero(aObjectScale.getX()) ? 1.0 : 1.0 / fabs(aObjectScale.getX()));
                    const double fBackScaleY(basegfx::fTools::equalZero(aObjectScale.getY()) ? 1.0 : 1.0 / fabs(aObjectScale.getY()));
                    const double fLeft(getCropLeft() * fBackScaleX);
                    const double fTop(getCropTop() * fBackScaleY);
                    const double fRight(getCropRight() * fBackScaleX);
                    const double fBottom(getCropBottom() * fBackScaleY);

                    
                    const basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);
                    const basegfx::B2DRange aNewRange(
                        -fLeft,
                        -fTop,
                        1.0 + fRight,
                        1.0 + fBottom);

                    
                    
                    if(aNewRange.overlaps(aUnitRange))
                    {
                        
                        
                        
                        basegfx::B2DHomMatrix aNewTransform(getTransformation());

                        aNewTransform.invert();

                        
                        aNewTransform = basegfx::tools::createScaleTranslateB2DHomMatrix(
                            aNewRange.getRange(),
                            aNewRange.getMinimum()) * aNewTransform;

                        
                        
                        
                        
                        aNewTransform = getTransformation() * aNewTransform;

                        
                        const Primitive2DReference xTransformPrimitive(
                            new TransformPrimitive2D(
                                aNewTransform,
                                getChildren()));

                        if(aUnitRange.isInside(aNewRange))
                        {
                            
                            
                            xRetval = Primitive2DSequence(&xTransformPrimitive, 1);
                        }
                        else
                        {
                            
                            basegfx::B2DPolyPolygon aMaskPolyPolygon(basegfx::tools::createUnitPolygon());
                            aMaskPolyPolygon.transform(getTransformation());

                            
                            const Primitive2DReference xMask(
                                new MaskPrimitive2D(
                                    aMaskPolyPolygon,
                                    Primitive2DSequence(&xTransformPrimitive, 1)));

                            xRetval = Primitive2DSequence(&xMask, 1);
                        }
                    }
                }
            }

            return xRetval;
        }

        
        ImplPrimitive2DIDBlock(CropPrimitive2D, PRIMITIVE2D_ID_CROPPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
