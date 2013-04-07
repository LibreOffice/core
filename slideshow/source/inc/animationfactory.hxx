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

            static AttributeClass classifyAttributeName( const OUString& rAttrName );

            /// Collection of flags common to all factory methods
            enum FactoryFlags
            {
                /** Don't call enter/leaveAnimation for the Shape.

                    This is useful for set effects
                 */
                FLAG_NO_SPRITE = 1
            };

            static NumberAnimationSharedPtr createNumberPropertyAnimation( const OUString&               rAttrName,
                                                                           const AnimatableShapeSharedPtr&      rShape,
                                                                           const ShapeManagerSharedPtr&         rShapeManager,
                                                                           const ::basegfx::B2DVector&          rSlideSize,
                                                                           int                                  nFlags=0 );

            static EnumAnimationSharedPtr createEnumPropertyAnimation( const OUString&                   rAttrName,
                                                                       const AnimatableShapeSharedPtr&          rShape,
                                                                       const ShapeManagerSharedPtr&             rShapeManager,
                                                                       const ::basegfx::B2DVector&              rSlideSize,
                                                                       int                                      nFlags=0 );

            static ColorAnimationSharedPtr  createColorPropertyAnimation( const OUString&                rAttrName,
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

            static StringAnimationSharedPtr createStringPropertyAnimation( const OUString&               rAttrName,
                                                                           const AnimatableShapeSharedPtr&      rShape,
                                                                           const ShapeManagerSharedPtr&         rShapeManager,
                                                                           const ::basegfx::B2DVector&          rSlideSize,
                                                                           int                                  nFlags=0 );

            static BoolAnimationSharedPtr   createBoolPropertyAnimation( const OUString&                 rAttrName,
                                                                         const AnimatableShapeSharedPtr&        rShape,
                                                                         const ShapeManagerSharedPtr&           rShapeManager,
                                                                         const ::basegfx::B2DVector&            rSlideSize,
                                                                         int                                    nFlags=0 );

            static NumberAnimationSharedPtr createPathMotionAnimation( const OUString&                   rSVGDPath,
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
