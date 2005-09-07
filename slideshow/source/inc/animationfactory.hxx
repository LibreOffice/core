/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationfactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:05:50 $
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

#ifndef _SLIDESHOW_ANIMATIONFACTORY_HXX
#define _SLIDESHOW_ANIMATIONFACTORY_HXX


#include <numberanimation.hxx>
#include <enumanimation.hxx>
#include <coloranimation.hxx>
#include <stringanimation.hxx>
#include <boolanimation.hxx>
#include <pairanimation.hxx>

#include <shape.hxx>
#include <shapeattributelayer.hxx>
#include <layermanager.hxx>

namespace rtl
{
    class OUString;
}


/* Definition of AnimationFactory class */

namespace presentation
{
    namespace internal
    {

        /** Factory for Animation objects

            Given a SMIL XAnimate node, this factory generates the
            appropriate Animation object from that, which will modify
            the attribute as specified.
         */
        class AnimationFactory
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
                                                                           const LayerManagerSharedPtr&         rLayerManager,
                                                                           int                                  nFlags=0 );

            static EnumAnimationSharedPtr createEnumPropertyAnimation( const ::rtl::OUString&                   rAttrName,
                                                                       const AnimatableShapeSharedPtr&          rShape,
                                                                       const LayerManagerSharedPtr&             rLayerManager,
                                                                       int                                      nFlags=0 );

            static ColorAnimationSharedPtr  createColorPropertyAnimation( const ::rtl::OUString&                rAttrName,
                                                                          const AnimatableShapeSharedPtr&       rShape,
                                                                          const LayerManagerSharedPtr&          rLayerManager,
                                                                          int                                   nFlags=0 );

            /** Create scale or move animation

                @param nTransformType
                Must be one of
                animations::AnimationTransformType::TRANSLATE or
                animations::AnimationTransformType::SCALE.
             */
            static PairAnimationSharedPtr   createPairPropertyAnimation( const AnimatableShapeSharedPtr&        rShape,
                                                                         const LayerManagerSharedPtr&           rLayerManager,
                                                                         sal_Int16                              nTransformType,
                                                                         int                                    nFlags=0 );

            static StringAnimationSharedPtr createStringPropertyAnimation( const ::rtl::OUString&               rAttrName,
                                                                           const AnimatableShapeSharedPtr&      rShape,
                                                                           const LayerManagerSharedPtr&         rLayerManager,
                                                                           int                                  nFlags=0 );

            static BoolAnimationSharedPtr   createBoolPropertyAnimation( const ::rtl::OUString&                 rAttrName,
                                                                         const AnimatableShapeSharedPtr&        rShape,
                                                                         const LayerManagerSharedPtr&           rLayerManager,
                                                                         int                                    nFlags=0 );

            static NumberAnimationSharedPtr createPathMotionAnimation( const ::rtl::OUString&                   rSVGDPath,
                                                                       const AnimatableShapeSharedPtr&          rShape,
                                                                       const LayerManagerSharedPtr&             rLayerManager,
                                                                       int                                      nFlags=0);
        private:
            // default: constructor/destructor disabed
            AnimationFactory();
            ~AnimationFactory();

            // default: disabled copy/assignment
            AnimationFactory(const AnimationFactory&);
            AnimationFactory& operator=( const AnimationFactory& );
        };
    }
}

#endif /* _SLIDESHOW_ANIMATIONFACTORY_HXX */
