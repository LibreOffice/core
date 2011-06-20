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

#ifndef INCLUDED_SLIDESHOW_ANIMATIONFACTORY_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONFACTORY_HXX


#include "numberanimation.hxx"
#include "enumanimation.hxx"
#include "coloranimation.hxx"
#include "stringanimation.hxx"
#include "boolanimation.hxx"
#include "pairanimation.hxx"

#include "shape.hxx"
#include "shapeattributelayer.hxx"
#include "shapemanager.hxx"

#include <boost/noncopyable.hpp>


namespace rtl
{
    class OUString;
}


/* Definition of AnimationFactory class */

namespace slideshow
{
    namespace internal
    {

        /** Factory for Animation objects

            Given a SMIL XAnimate node, this factory generates the
            appropriate Animation object from that, which will modify
            the attribute as specified.
         */
        class AnimationFactory : private boost::noncopyable
        {
        public:
            /** Classifies the attribute name.

                This enum maps names to appropriate factory methods.
             */
            enum AttributeClass
            {
                /// Unknown, prolly invalid name
                CLASS_UNKNOWN_PROPERTY,
                /// Use createNumberPropertyAnimation
                CLASS_NUMBER_PROPERTY,
                /// Use createEnumPropertyAnimation
                CLASS_ENUM_PROPERTY,
                /// Use createColorPropertyAnimation
                CLASS_COLOR_PROPERTY,
                /// Use createStringPropertyAnimation
                CLASS_STRING_PROPERTY,
                /// Use createBoolPropertyAnimation
                CLASS_BOOL_PROPERTY
            };

            static AttributeClass classifyAttributeName( const ::rtl::OUString& rAttrName );

            /// Collection of flags common to all factory methods
            enum FactoryFlags
            {
                /** Don't call enter/leaveAnimation for the Shape.

                    This is useful for set effects
                 */
                FLAG_NO_SPRITE = 1
            };

            static NumberAnimationSharedPtr createNumberPropertyAnimation( const ::rtl::OUString&               rAttrName,
                                                                           const AnimatableShapeSharedPtr&      rShape,
                                                                           const ShapeManagerSharedPtr&         rShapeManager,
                                                                           const ::basegfx::B2DVector&          rSlideSize,
                                                                           int                                  nFlags=0 );

            static EnumAnimationSharedPtr createEnumPropertyAnimation( const ::rtl::OUString&                   rAttrName,
                                                                       const AnimatableShapeSharedPtr&          rShape,
                                                                       const ShapeManagerSharedPtr&             rShapeManager,
                                                                       const ::basegfx::B2DVector&              rSlideSize,
                                                                       int                                      nFlags=0 );

            static ColorAnimationSharedPtr  createColorPropertyAnimation( const ::rtl::OUString&                rAttrName,
                                                                          const AnimatableShapeSharedPtr&       rShape,
                                                                          const ShapeManagerSharedPtr&          rShapeManager,
                                                                          const ::basegfx::B2DVector&           rSlideSize,
                                                                          int                                   nFlags=0 );

            /** Create scale or move animation

                @param nTransformType
                Must be one of
                animations::AnimationTransformType::TRANSLATE or
                animations::AnimationTransformType::SCALE.
             */
            static PairAnimationSharedPtr   createPairPropertyAnimation( const AnimatableShapeSharedPtr&        rShape,
                                                                         const ShapeManagerSharedPtr&           rShapeManager,
                                                                         const ::basegfx::B2DVector&            rSlideSize,
                                                                         sal_Int16                              nTransformType,
                                                                         int                                    nFlags=0 );

            static StringAnimationSharedPtr createStringPropertyAnimation( const ::rtl::OUString&               rAttrName,
                                                                           const AnimatableShapeSharedPtr&      rShape,
                                                                           const ShapeManagerSharedPtr&         rShapeManager,
                                                                           const ::basegfx::B2DVector&          rSlideSize,
                                                                           int                                  nFlags=0 );

            static BoolAnimationSharedPtr   createBoolPropertyAnimation( const ::rtl::OUString&                 rAttrName,
                                                                         const AnimatableShapeSharedPtr&        rShape,
                                                                         const ShapeManagerSharedPtr&           rShapeManager,
                                                                         const ::basegfx::B2DVector&            rSlideSize,
                                                                         int                                    nFlags=0 );

            static NumberAnimationSharedPtr createPathMotionAnimation( const ::rtl::OUString&                   rSVGDPath,
                                                                       sal_Int16                                nAdditive,
                                                                       const AnimatableShapeSharedPtr&          rShape,
                                                                       const ShapeManagerSharedPtr&             rShapeManager,
                                                                       const ::basegfx::B2DVector&              rSlideSize,
                                                                       int                                      nFlags=0);
        private:
            // default: constructor/destructor disabed
            AnimationFactory();
            ~AnimationFactory();
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_ANIMATIONFACTORY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
