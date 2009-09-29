/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animatedprimitive2d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX

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
        class AnimatedSwitchPrimitive2D : public GroupPrimitive2D
        {
        private:
            // the animation definition which allows translation of a point in time
            // to an animation state [0.0 .. 1.0]. This member contains a cloned
            // definition and is owned by this implementation
            animation::AnimationEntry*                      mpAnimationEntry;

            // bitfield
            // flag if this is a text or graphic animation. Necessary since SdrViews need to differentiate
            // between both types if they are on/off
            unsigned                                        mbIsTextAnimation : 1;

        public:
            AnimatedSwitchPrimitive2D(
                const animation::AnimationEntry& rAnimationEntry,
                const Primitive2DSequence& rChildren,
                bool bIsTextAnimation);
            virtual ~AnimatedSwitchPrimitive2D();

            // get data
            const animation::AnimationEntry& getAnimationEntry() const { return *mpAnimationEntry; }
            bool isTextAnimation() const { return mbIsTextAnimation; }
            bool isGraphicAnimation() const { return !isTextAnimation(); }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()

            // The getDecomposition is overloaded here since the decompose is dependent of the point in time,
            // so the default implementation is nut useful here, it needs to be handled locally
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class AnimatedBlinkPrimitive2D : public AnimatedSwitchPrimitive2D
        {
        protected:
        public:
            AnimatedBlinkPrimitive2D(
                const animation::AnimationEntry& rAnimationEntry,
                const Primitive2DSequence& rChildren,
                bool bIsTextAnimation);

            // create local decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class AnimatedInterpolatePrimitive2D : public AnimatedSwitchPrimitive2D
        {
        private:
            // the transformations
            std::vector< basegfx::tools::B2DHomMatrixBufferedDecompose >        maMatrixStack;

        protected:
        public:
            AnimatedInterpolatePrimitive2D(
                const std::vector< basegfx::B2DHomMatrix >& rmMatrixStack,
                const animation::AnimationEntry& rAnimationEntry,
                const Primitive2DSequence& rChildren,
                bool bIsTextAnimation);

            // create local decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
