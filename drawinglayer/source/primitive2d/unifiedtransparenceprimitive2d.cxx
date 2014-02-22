/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        UnifiedTransparencePrimitive2D::UnifiedTransparencePrimitive2D(
            const Primitive2DSequence& rChildren,
            double fTransparence)
        :   GroupPrimitive2D(rChildren),
            mfTransparence(fTransparence)
        {
        }

        bool UnifiedTransparencePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const UnifiedTransparencePrimitive2D& rCompare = (UnifiedTransparencePrimitive2D&)rPrimitive;

                return (getTransparence() == rCompare.getTransparence());
            }

            return false;
        }

        basegfx::B2DRange UnifiedTransparencePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            
            
            return getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation);
        }

        Primitive2DSequence UnifiedTransparencePrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(0.0 == getTransparence())
            {
                
                return getChildren();
            }
            else if(getTransparence() > 0.0 && getTransparence() < 1.0)
            {
                
                
                
                
                //
                
                
                //
                
                
                //
                
                
                //
                
                
                const basegfx::B2DRange aPolygonRange(getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation));
                const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(aPolygonRange));
                const basegfx::BColor aGray(getTransparence(), getTransparence(), getTransparence());
                Primitive2DSequence aTransparenceContent(2);

                aTransparenceContent[0] = Primitive2DReference(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon), aGray));
                aTransparenceContent[1] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, aGray));

                
                const Primitive2DReference xRefB(new TransparencePrimitive2D(getChildren(), aTransparenceContent));
                return Primitive2DSequence(&xRefB, 1L);
            }
            else
            {
                
                return Primitive2DSequence();
            }
        }

        
        ImplPrimitive2DIDBlock(UnifiedTransparencePrimitive2D, PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
