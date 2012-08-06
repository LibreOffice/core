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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
namespace drawinglayer { namespace animation {
    class AnimationEntry;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** AnimatedSwitchPrimitive2D class

            This is the basic class for simple, animated primitives. The basic idea
            is to have an animation definition (AnimationEntry) who's basic
            functionality is to return a state value for any given animation time in
            the range of [0.0 .. 1.0]. Depending on the state, the decomposition
            calculates an index, which of the members of the child vector is to
            be visualized.

            An example: For blinking, the Child vector should exist of two entries;
            for values of [0.0 .. 0.5] the first, else the last entry will be used.
            This mechanism is not limited to two entries, though.
         */
        class DRAWINGLAYER_DLLPUBLIC AnimatedSwitchPrimitive2D : public GroupPrimitive2D
        {
        private:
            /**
                The animation definition which allows translation of a point in time
                to an animation state [0.0 .. 1.0]. This member contains a cloned
                definition and is owned by this implementation.
             */
            animation::AnimationEntry*                      mpAnimationEntry;

            /// bitfield
            /** flag if this is a text or graphic animation. Necessary since SdrViews need to differentiate
                between both types if they are on/off
             */
            unsigned                                        mbIsTextAnimation : 1;

        public:
            /// constructor
            AnimatedSwitchPrimitive2D(
                const animation::AnimationEntry& rAnimationEntry,
                const Primitive2DSequence& rChildren,
                bool bIsTextAnimation);

            /// destructor - needed due to mpAnimationEntry
            virtual ~AnimatedSwitchPrimitive2D();

            /// data read access
            const animation::AnimationEntry& getAnimationEntry() const { return *mpAnimationEntry; }
            bool isTextAnimation() const { return mbIsTextAnimation; }
            bool isGraphicAnimation() const { return !isTextAnimation(); }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()

            /** The getDecomposition is overloaded here since the decompose is dependent of the point in time,
                so the default implementation is nut useful here, it needs to be handled locally
             */
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** AnimatedBlinkPrimitive2D class

            Basically the same mechanism as in AnimatedSwitchPrimitive2D, but the
            decomposition is specialized in delivering the children in the
            range [0.0.. 0.5] and an empty sequence else
         */
        class DRAWINGLAYER_DLLPUBLIC AnimatedBlinkPrimitive2D : public AnimatedSwitchPrimitive2D
        {
        protected:
        public:
            /// constructor
            AnimatedBlinkPrimitive2D(
                const animation::AnimationEntry& rAnimationEntry,
                const Primitive2DSequence& rChildren,
                bool bIsTextAnimation);

            /// create local decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** AnimatedInterpolatePrimitive2D class

            Specialized on multi-step animations based on matrix transformations. The
            Child sequelce will be embedded in a matrix transformation. That transformation
            will be linearly combined from the decomposed values and the animation value
            to allow a smooth animation.
         */
        class DRAWINGLAYER_DLLPUBLIC AnimatedInterpolatePrimitive2D : public AnimatedSwitchPrimitive2D
        {
        private:
            /// the transformations
            std::vector< basegfx::tools::B2DHomMatrixBufferedDecompose >        maMatrixStack;

        protected:
        public:
            /// constructor
            AnimatedInterpolatePrimitive2D(
                const std::vector< basegfx::B2DHomMatrix >& rmMatrixStack,
                const animation::AnimationEntry& rAnimationEntry,
                const Primitive2DSequence& rChildren,
                bool bIsTextAnimation);

            /// create local decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
