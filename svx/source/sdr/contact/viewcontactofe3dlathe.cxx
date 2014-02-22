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


#include <svx/sdr/contact/viewcontactofe3dlathe.hxx>
#include <svx/lathe3d.hxx>
#include <drawinglayer/primitive3d/sdrlatheprimitive3d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive3d/sdrattributecreator3d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>



namespace sdr
{
    namespace contact
    {
        ViewContactOfE3dLathe::ViewContactOfE3dLathe(E3dLatheObj& rLathe)
        :   ViewContactOfE3d(rLathe)
        {
        }

        ViewContactOfE3dLathe::~ViewContactOfE3dLathe()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3dLathe::createViewIndependentPrimitive3DSequence() const
        {
            drawinglayer::primitive3d::Primitive3DSequence xRetval;
            const SfxItemSet& rItemSet = GetE3dLatheObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowAttribute3D aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowAttribute(rItemSet, false));

            
            const basegfx::B2DPolyPolygon aPolyPolygon(GetE3dLatheObj().GetPolyPoly2D());

            
            drawinglayer::attribute::Sdr3DObjectAttribute* pSdr3DObjectAttribute = drawinglayer::primitive2d::createNewSdr3DObjectAttribute(rItemSet);

            
            
            
            const sal_uInt32 nPolygonCount(aPolyPolygon.count());
            double fPolygonMaxLength(0.0);

            for(sal_uInt32 a(0); a < nPolygonCount; a++)
            {
                const basegfx::B2DPolygon aCandidate(aPolyPolygon.getB2DPolygon(a));
                const double fPolygonLength(basegfx::tools::getLength(aCandidate));
                fPolygonMaxLength = std::max(fPolygonMaxLength, fPolygonLength);
            }

            const basegfx::B2DRange aPolyPolygonRange(basegfx::tools::getRange(aPolyPolygon));
            const basegfx::B2DVector aTextureSize(
                F_PI * fabs(aPolyPolygonRange.getCenter().getX()), 
                fPolygonMaxLength);

            
            const sal_uInt32 nHorizontalSegments(GetE3dLatheObj().GetHorizontalSegments());
            const sal_uInt32 nVerticalSegments(GetE3dLatheObj().GetVerticalSegments());
            const double fDiagonal((double)GetE3dLatheObj().GetPercentDiagonal() / 100.0);
            const double fBackScale((double)GetE3dLatheObj().GetBackScale() / 100.0);
            const double fRotation(((double)GetE3dLatheObj().GetEndAngle() / 1800.0) * F_PI);
            const bool bSmoothNormals(GetE3dLatheObj().GetSmoothNormals()); 
            const bool bSmoothLids(GetE3dLatheObj().GetSmoothLids()); 
            const bool bCharacterMode(GetE3dLatheObj().GetCharacterMode());
            const bool bCloseFront(GetE3dLatheObj().GetCloseFront());
            const bool bCloseBack(GetE3dLatheObj().GetCloseBack());

            
            const basegfx::B3DHomMatrix aWorldTransform;
            const drawinglayer::primitive3d::Primitive3DReference xReference(
                new drawinglayer::primitive3d::SdrLathePrimitive3D(
                    aWorldTransform, aTextureSize, aAttribute, *pSdr3DObjectAttribute,
                    aPolyPolygon, nHorizontalSegments, nVerticalSegments,
                    fDiagonal, fBackScale, fRotation,
                    bSmoothNormals, true, bSmoothLids, bCharacterMode, bCloseFront, bCloseBack));
            xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);

            
            delete pSdr3DObjectAttribute;

            return xRetval;
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
