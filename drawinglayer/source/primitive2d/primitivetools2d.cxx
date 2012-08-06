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

#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence DiscreteMetricDependentPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // get the current DiscreteUnit
            const double fDiscreteUnit((rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0)).getLength());

            if(getBuffered2DDecomposition().hasElements() && !basegfx::fTools::equal(fDiscreteUnit, getDiscreteUnit()))
            {
                // conditions of last local decomposition have changed, delete
                const_cast< DiscreteMetricDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                // remember new valid DiscreteUnit
                const_cast< DiscreteMetricDependentPrimitive2D* >(this)->mfDiscreteUnit = fDiscreteUnit;
            }

            // call base implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence ViewportDependentPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // get the current Viewport
            const basegfx::B2DRange& rViewport = rViewInformation.getViewport();

            if(getBuffered2DDecomposition().hasElements() && !rViewport.equal(getViewport()))
            {
                // conditions of last local decomposition have changed, delete
                const_cast< ViewportDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                // remember new valid DiscreteUnit
                const_cast< ViewportDependentPrimitive2D* >(this)->maViewport = rViewport;
            }

            // call base implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence ViewTransformationDependentPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // get the current ViewTransformation
            const basegfx::B2DHomMatrix& rViewTransformation = rViewInformation.getViewTransformation();

            if(getBuffered2DDecomposition().hasElements() && rViewTransformation != getViewTransformation())
            {
                // conditions of last local decomposition have changed, delete
                const_cast< ViewTransformationDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                // remember new valid ViewTransformation
                const_cast< ViewTransformationDependentPrimitive2D* >(this)->maViewTransformation = rViewTransformation;
            }

            // call base implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence ObjectAndViewTransformationDependentPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // get the current ViewTransfromation
            const basegfx::B2DHomMatrix& rViewTransformation = rViewInformation.getViewTransformation();

            if(getBuffered2DDecomposition().hasElements() && rViewTransformation != getViewTransformation())
            {
                // conditions of last local decomposition have changed, delete
                const_cast< ObjectAndViewTransformationDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            // get the current ObjectTransformation
            const basegfx::B2DHomMatrix& rObjectTransformation = rViewInformation.getObjectTransformation();

            if(getBuffered2DDecomposition().hasElements() && rObjectTransformation != getObjectTransformation())
            {
                // conditions of last local decomposition have changed, delete
                const_cast< ObjectAndViewTransformationDependentPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                // remember new valid ViewTransformation, and ObjectTransformation
                const_cast< ObjectAndViewTransformationDependentPrimitive2D* >(this)->maViewTransformation = rViewTransformation;
                const_cast< ObjectAndViewTransformationDependentPrimitive2D* >(this)->maObjectTransformation = rObjectTransformation;
            }

            // call base implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
