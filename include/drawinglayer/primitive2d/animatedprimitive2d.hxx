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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <memory>

// predefines
namespace drawinglayer { namespace animation {
    class AnimationEntry;
}}


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
            std::unique_ptr<animation::AnimationEntry>      mpAnimationEntry;

            /** flag if this is a text or graphic animation. Necessary since SdrViews need to differentiate
                between both types if they are on/off
             */
            bool                                            mbIsTextAnimation : 1;

        protected:
            /** write access right for classes deriving from this who want to do special
                things (e.g. optimization, buffering).
                Caution: This is an exception from the read-only, non-modifiable paradigm
                for primitives, so special preparations may be needed. Usually should
                only be used for initialization (e.g. in a derived constructor)
            */
            void setAnimationEntry(const animation::AnimationEntry& rNew);

        public:
            /// constructor
            AnimatedSwitchPrimitive2D(
                const animation::AnimationEntry& rAnimationEntry,
                const Primitive2DContainer& rChildren,
                bool bIsTextAnimation);

            /// destructor - needed due to mpAnimationEntry
            virtual ~AnimatedSwitchPrimitive2D() override;

            /// data read access
            const animation::AnimationEntry& getAnimationEntry() const { return *mpAnimationEntry; }
            bool isTextAnimation() const { return mbIsTextAnimation; }
            bool isGraphicAnimation() const { return !isTextAnimation(); }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()

            /** Override getDecomposition() here since the decompose
                depends on the point in time, so the default implementation is
                not useful here, it needs to be handled locally
             */
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


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
                const Primitive2DContainer& rChildren);

            /// create local decomposition
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace primitive2d
    {
        /** AnimatedInterpolatePrimitive2D class

            Specialized on multi-step animations based on matrix transformations. The
            Child sequence will be embedded in a matrix transformation. That transformation
            will be linearly combined from the decomposed values and the animation value
            to allow a smooth animation.
         */
        class DRAWINGLAYER_DLLPUBLIC AnimatedInterpolatePrimitive2D : public AnimatedSwitchPrimitive2D
        {
        private:
            /// the transformations
            std::vector< basegfx::utils::B2DHomMatrixBufferedDecompose >        maMatrixStack;

        protected:
        public:
            /// constructor
            AnimatedInterpolatePrimitive2D(
                const std::vector< basegfx::B2DHomMatrix >& rmMatrixStack,
                const animation::AnimationEntry& rAnimationEntry,
                const Primitive2DContainer& rChildren);

            /// create local decomposition
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
