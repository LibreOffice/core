/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animatedprimitive2d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-11-07 15:49:07 $
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
                const sal_Int32 nLen(getChildren().getLength());
                sal_Int32 nIndex(basegfx::fround(fState * (double)nLen));

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
        ImplPrimitrive2DIDBlock(AnimatedSwitchPrimitive2D, '2','A','S','w')

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
        ImplPrimitrive2DIDBlock(AnimatedBlinkPrimitive2D, '2','A','B','l')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence AnimatedInterpolatePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // ensure matrices are decomposed
            if(!mbDecomposed)
            {
                const_cast< AnimatedInterpolatePrimitive2D* >(this)->implDecompose();
            }

            // create state at time
            double fState(getAnimationEntry().getStateAtTime(rViewInformation.getViewTime()));

            if(fState < 0.0)
            {
                fState = 0.0;
            }

            if(fState > 1.0)
            {
                fState = 1.0;
            }

            // interpolate for state
            basegfx::B2DVector aScale(basegfx::interpolate(maScaleA, maScaleB, fState));
            basegfx::B2DVector aTranslate(basegfx::interpolate(maTranslateA, maTranslateB, fState));
            const double fRotate(((mfRotateB - mfRotateA) * fState) + mfRotateA);
            const double fShearX(((mfShearXB - mfShearXA) * fState) + mfShearXA);

            // build matrix for state
            basegfx::B2DHomMatrix aMergedTransform;
            aMergedTransform.scale(aScale.getX(), aScale.getY());
            aMergedTransform.shearX(fShearX);
            aMergedTransform.rotate(fRotate);
            aMergedTransform.translate(aTranslate.getX(), aTranslate.getY());

            // create new transform primitive reference, return new sequence
            const Primitive2DReference xRef(new TransformPrimitive2D(aMergedTransform, getChildren()));
            return Primitive2DSequence(&xRef, 1L);
        }

        void AnimatedInterpolatePrimitive2D::implDecompose()
        {
            maStart.decompose(maScaleA, maTranslateA, mfRotateA, mfShearXA);
            maStop.decompose(maScaleB, maTranslateB, mfRotateB, mfShearXB);
            mbDecomposed = true;
        }

        AnimatedInterpolatePrimitive2D::AnimatedInterpolatePrimitive2D(
            const animation::AnimationEntry& rAnimationEntry,
            const Primitive2DSequence& rChildren,
            const basegfx::B2DHomMatrix& rStart,
            const basegfx::B2DHomMatrix& rStop,
            bool bIsTextAnimation)
        :   AnimatedSwitchPrimitive2D(rAnimationEntry, rChildren, bIsTextAnimation),
            maStart(rStart),
            maStop(rStop),
            mbDecomposed(false)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(AnimatedInterpolatePrimitive2D, '2','A','I','n')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
