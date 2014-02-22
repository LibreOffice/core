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

#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence DiscreteMetricDependentPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            
            const basegfx::B2DVector aDiscreteVector(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));
            const double fDiscreteUnit(std::min(fabs(aDiscreteVector.getX()), fabs(aDiscreteVector.getY())));

            if(getBuffered2DDecomposition().hasElements() && !basegfx::fTools::equal(fDiscreteUnit, getDiscreteUnit()))
            {
                
                const_cast< DiscreteMetricDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                
                const_cast< DiscreteMetricDependentPrimitive2D* >(this)->updateDiscreteUnit(fDiscreteUnit);
            }

            
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence ViewportDependentPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            
            const basegfx::B2DRange& rViewport = rViewInformation.getViewport();

            if(getBuffered2DDecomposition().hasElements() && !rViewport.equal(getViewport()))
            {
                
                const_cast< ViewportDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                
                const_cast< ViewportDependentPrimitive2D* >(this)->maViewport = rViewport;
            }

            
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence ViewTransformationDependentPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            
            const basegfx::B2DHomMatrix& rViewTransformation = rViewInformation.getViewTransformation();

            if(getBuffered2DDecomposition().hasElements() && rViewTransformation != getViewTransformation())
            {
                
                const_cast< ViewTransformationDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                
                const_cast< ViewTransformationDependentPrimitive2D* >(this)->maViewTransformation = rViewTransformation;
            }

            
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence ObjectAndViewTransformationDependentPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            
            const basegfx::B2DHomMatrix& rViewTransformation = rViewInformation.getViewTransformation();

            if(getBuffered2DDecomposition().hasElements() && rViewTransformation != getViewTransformation())
            {
                
                const_cast< ObjectAndViewTransformationDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            
            const basegfx::B2DHomMatrix& rObjectTransformation = rViewInformation.getObjectTransformation();

            if(getBuffered2DDecomposition().hasElements() && rObjectTransformation != getObjectTransformation())
            {
                
                const_cast< ObjectAndViewTransformationDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                
                const_cast< ObjectAndViewTransformationDependentPrimitive2D* >(this)->maViewTransformation = rViewTransformation;
                const_cast< ObjectAndViewTransformationDependentPrimitive2D* >(this)->maObjectTransformation = rObjectTransformation;
            }

            
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
