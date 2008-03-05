/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animatedprimitive2d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:42 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ANIMATEDPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX
#include <drawinglayer/animation/animationtiming.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence AnimatedSwitchPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(getChildren().hasElements())
            {
                const double fState(getAnimationEntry().getStateAtTime(rViewInformation.getViewTime()));
                const sal_uInt32 nLen(getChildren().getLength());
                sal_uInt32 nIndex(basegfx::fround(fState * (double)nLen));

                if(nIndex >= nLen)
                {
                    nIndex = nLen - 1L;
                }

                const Primitive2DReference xRef(getChildren()[nIndex], uno::UNO_QUERY_THROW);
                return Primitive2DSequence(&xRef, 1L);
            }

            return Primitive2DSequence();
        }

        AnimatedSwitchPrimitive2D::AnimatedSwitchPrimitive2D(
            const animation::AnimationEntry& rAnimationEntry,
            const Primitive2DSequence& rChildren,
            bool bIsTextAnimation)
        :   GroupPrimitive2D(rChildren),
            mpAnimationEntry(0),
            mfDecomposeViewTime(0.0),
            mbIsTextAnimation(bIsTextAnimation)
        {
            // clone given animation description
            mpAnimationEntry = rAnimationEntry.clone();
        }

        AnimatedSwitchPrimitive2D::~AnimatedSwitchPrimitive2D()
        {
            // delete cloned animation description
            delete mpAnimationEntry;
        }

        bool AnimatedSwitchPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const AnimatedSwitchPrimitive2D& rCompare = static_cast< const AnimatedSwitchPrimitive2D& >(rPrimitive);

                return (getAnimationEntry() == rCompare.getAnimationEntry());
            }

            return false;
        }

        Primitive2DSequence AnimatedSwitchPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(getLocalDecomposition().hasElements() && mfDecomposeViewTime != rViewInformation.getViewTime())
            {
                // conditions of last local decomposition have changed, delete
                const_cast< AnimatedSwitchPrimitive2D* >(this)->setLocalDecomposition(Primitive2DSequence());
            }

            if(!getLocalDecomposition().hasElements())
            {
                // remember time
                const_cast< AnimatedSwitchPrimitive2D* >(this)->mfDecomposeViewTime = rViewInformation.getViewTime();
            }

            // use parent implementation
            return GroupPrimitive2D::get2DDecomposition(rViewInformation);
        }

        basegfx::B2DRange AnimatedSwitchPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // to get range from decomposition and not from group content, call implementation from
            // BasePrimitive2D here
            return BasePrimitive2D::getB2DRange(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(AnimatedSwitchPrimitive2D, PRIMITIVE2D_ID_ANIMATEDSWITCHPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence AnimatedBlinkPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(getChildren().hasElements())
            {
                const double fState(getAnimationEntry().getStateAtTime(rViewInformation.getViewTime()));

                if(fState < 0.5)
                {
                    return getChildren();
                }
            }

            return Primitive2DSequence();
        }

        AnimatedBlinkPrimitive2D::AnimatedBlinkPrimitive2D(
            const animation::AnimationEntry& rAnimationEntry,
            const Primitive2DSequence& rChildren,
            bool bIsTextAnimation)
        :   AnimatedSwitchPrimitive2D(rAnimationEntry, rChildren, bIsTextAnimation)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(AnimatedBlinkPrimitive2D, PRIMITIVE2D_ID_ANIMATEDBLINKPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// helper class for AnimatedInterpolatePrimitive2D

namespace drawinglayer
{
    namespace
    {
        BufferedMatrixDecompose::BufferedMatrixDecompose(const basegfx::B2DHomMatrix& rMatrix)
        :   maB2DHomMatrix(rMatrix),
            maScale(0.0, 0.0),
            maTranslate(0.0, 0.0),
            mfRotate(0.0),
            mfShearX(0.0),
            mbDecomposed(false)
        {
        }

        void BufferedMatrixDecompose::ensureDecompose() const
        {
            if(!mbDecomposed)
            {
                BufferedMatrixDecompose* pThis = const_cast< BufferedMatrixDecompose* >(this);
                maB2DHomMatrix.decompose(pThis->maScale, pThis->maTranslate, pThis->mfRotate, pThis->mfShearX);
                pThis->mbDecomposed = true;
            }
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence AnimatedInterpolatePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            const sal_uInt32 nSize(maMatrixStack.size());

            if(nSize)
            {
                double fState(getAnimationEntry().getStateAtTime(rViewInformation.getViewTime()));

                if(fState < 0.0)
                {
                    fState = 0.0;
                }
                else if(fState > 1.0)
                {
                    fState = 1.0;
                }

                const double fIndex(fState * (double)(nSize - 1L));
                const sal_uInt32 nIndA(sal_uInt32(floor(fIndex)));
                const double fOffset(fIndex - (double)nIndA);
                basegfx::B2DHomMatrix aTargetTransform;

                if(basegfx::fTools::equalZero(fOffset))
                {
                    // use matrix from nIndA directly
                    aTargetTransform = maMatrixStack[nIndA].getB2DHomMatrix();
                }
                else
                {
                    // interpolate. Get involved matrices and ensure they are decomposed
                    const sal_uInt32 nIndB((nIndA + 1L) % nSize);
                    std::vector< BufferedMatrixDecompose >::const_iterator aMatA(maMatrixStack.begin() + nIndA);
                    std::vector< BufferedMatrixDecompose >::const_iterator aMatB(maMatrixStack.begin() + nIndB);

                    aMatA->ensureDecompose();
                    aMatB->ensureDecompose();

                    // interpolate for fOffset [0.0 .. 1.0[
                    const basegfx::B2DVector aScale(basegfx::interpolate(aMatA->getScale(), aMatB->getScale(), fOffset));
                    const basegfx::B2DVector aTranslate(basegfx::interpolate(aMatA->getTranslate(), aMatB->getTranslate(), fOffset));
                    const double fRotate(((aMatB->getRotate() - aMatA->getRotate()) * fOffset) + aMatA->getRotate());
                    const double fShearX(((aMatB->getShearX() - aMatA->getShearX()) * fOffset) + aMatA->getShearX());

                    // build matrix for state
                    aTargetTransform.scale(aScale.getX(), aScale.getY());
                    aTargetTransform.shearX(fShearX);
                    aTargetTransform.rotate(fRotate);
                    aTargetTransform.translate(aTranslate.getX(), aTranslate.getY());
                }

                // create new transform primitive reference, return new sequence
                const Primitive2DReference xRef(new TransformPrimitive2D(aTargetTransform, getChildren()));
                return Primitive2DSequence(&xRef, 1L);
            }
            else
            {
                return getChildren();
            }
        }

        AnimatedInterpolatePrimitive2D::AnimatedInterpolatePrimitive2D(
            const std::vector< basegfx::B2DHomMatrix >& rmMatrixStack,
            const animation::AnimationEntry& rAnimationEntry,
            const Primitive2DSequence& rChildren,
            bool bIsTextAnimation)
        :   AnimatedSwitchPrimitive2D(rAnimationEntry, rChildren, bIsTextAnimation),
            maMatrixStack()
        {
            // copy matrices
            const sal_uInt32 nCount(rmMatrixStack.size());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                maMatrixStack.push_back(BufferedMatrixDecompose(rmMatrixStack[a]));
            }
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(AnimatedInterpolatePrimitive2D, PRIMITIVE2D_ID_ANIMATEDINTERPOLATEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
