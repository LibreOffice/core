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


#include <svx/sdr/contact/viewcontactofe3dpolygon.hxx>
#include <svx/polygn3d.hxx>
#include <drawinglayer/primitive3d/sdrpolypolygonprimitive3d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive3d/sdrattributecreator3d.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>



namespace sdr
{
    namespace contact
    {
        ViewContactOfE3dPolygon::ViewContactOfE3dPolygon(E3dPolygonObj& rPolygon)
        :   ViewContactOfE3d(rPolygon)
        {
        }

        ViewContactOfE3dPolygon::~ViewContactOfE3dPolygon()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3dPolygon::createViewIndependentPrimitive3DSequence() const
        {
            drawinglayer::primitive3d::Primitive3DSequence xRetval;
            const SfxItemSet& rItemSet = GetE3dPolygonObj().GetMergedItemSet();
            const bool bSuppressFill(GetE3dPolygonObj().GetLineOnly());
            const drawinglayer::attribute::SdrLineFillShadowAttribute3D aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowAttribute(rItemSet, bSuppressFill));

            
            basegfx::B3DPolyPolygon aPolyPolygon3D(GetE3dPolygonObj().GetPolyPolygon3D());
            const basegfx::B3DPolyPolygon aPolyNormals3D(GetE3dPolygonObj().GetPolyNormals3D());
            const basegfx::B2DPolyPolygon aPolyTexture2D(GetE3dPolygonObj().GetPolyTexture2D());
            const bool bNormals(aPolyNormals3D.count() && aPolyNormals3D.count() == aPolyPolygon3D.count());
            const bool bTexture(aPolyTexture2D.count() && aPolyTexture2D.count() == aPolyPolygon3D.count());

            if(bNormals || bTexture)
            {
                for(sal_uInt32 a(0L); a < aPolyPolygon3D.count(); a++)
                {
                    basegfx::B3DPolygon aCandidate3D(aPolyPolygon3D.getB3DPolygon(a));
                    basegfx::B3DPolygon aNormals3D;
                    basegfx::B2DPolygon aTexture2D;

                    if(bNormals)
                    {
                        aNormals3D = aPolyNormals3D.getB3DPolygon(a);
                    }

                    if(bTexture)
                    {
                        aTexture2D = aPolyTexture2D.getB2DPolygon(a);
                    }

                    for(sal_uInt32 b(0L); b < aCandidate3D.count(); b++)
                    {
                        if(bNormals)
                        {
                            sal_uInt32 nNormalCount = aNormals3D.count();
                            if( b < nNormalCount )
                                aCandidate3D.setNormal(b, aNormals3D.getB3DPoint(b));
                            else if( nNormalCount > 0 )
                                aCandidate3D.setNormal(b, aNormals3D.getB3DPoint(0));
                        }
                        if(bTexture)
                        {
                            sal_uInt32 nTextureCount = aTexture2D.count();
                            if( b < nTextureCount )
                                aCandidate3D.setTextureCoordinate(b, aTexture2D.getB2DPoint(b));
                            else if( nTextureCount > 0 )
                                aCandidate3D.setTextureCoordinate(b, aTexture2D.getB2DPoint(0));
                        }
                    }

                    aPolyPolygon3D.setB3DPolygon(a, aCandidate3D);
                }
            }

            
            drawinglayer::attribute::Sdr3DObjectAttribute* pSdr3DObjectAttribute = drawinglayer::primitive2d::createNewSdr3DObjectAttribute(rItemSet);

            
            basegfx::B2DVector aTextureSize(1.0, 1.0);

            if(bTexture)
            {
                
                
                const basegfx::B3DRange aObjectRange(basegfx::tools::getRange(aPolyPolygon3D));

                double fWidth(0.0);
                double fHeight(0.0);

                
                
                if(basegfx::fTools::equalZero(aObjectRange.getWidth()))
                {
                    
                    fWidth = aObjectRange.getHeight();
                    fHeight = aObjectRange.getDepth();
                }
                else if(basegfx::fTools::equalZero(aObjectRange.getHeight()))
                {
                    
                    fWidth = aObjectRange.getWidth();
                    fHeight = aObjectRange.getDepth();
                }
                else
                {
                    
                    fWidth = aObjectRange.getWidth();
                    fHeight = aObjectRange.getHeight();
                }

                if(basegfx::fTools::lessOrEqual(fWidth, 0.0) ||basegfx::fTools::lessOrEqual(fHeight, 0.0))
                {
                    
                    aTextureSize.setX(0.01);
                    aTextureSize.setY(0.01);
                }
                else
                {
                    aTextureSize.setX(fWidth);
                    aTextureSize.setY(fHeight);
                }
            }

            
            
            
            
            
            
            
            
            aPolyPolygon3D.flip();

            
            const basegfx::B3DHomMatrix aWorldTransform;
            const drawinglayer::primitive3d::Primitive3DReference xReference(
                new drawinglayer::primitive3d::SdrPolyPolygonPrimitive3D(
                    aPolyPolygon3D, aWorldTransform, aTextureSize, aAttribute, *pSdr3DObjectAttribute));
            xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);

            
            delete pSdr3DObjectAttribute;

            return xRetval;
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
