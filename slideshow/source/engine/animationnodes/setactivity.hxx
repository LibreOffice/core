/*************************************************************************
 *
 *  $RCSfile: setactivity.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:08:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SLIDESHOW_SETACTIVITY_HXX
#define _SLIDESHOW_SETACTIVITY_HXX

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <animationactivity.hxx>
#include <animation.hxx>
#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>


namespace presentation
{
    namespace internal
    {
        /** Templated setter for animation values

            This template class implements the AnimationActivity
            interface, but only the perform() and
            setAttributeLayer() methods are functional. To be used
            for set animations, see AnimationSetNode.
        */
        template< class AnimationT > class SetActivity : public AnimationActivity
        {
        public:
            typedef ::boost::shared_ptr< AnimationT >   AnimationSharedPtrT;
            typedef typename AnimationT::ValueType      ValueT;

            SetActivity( const AnimationSharedPtrT& rAnimation,
                         const ValueT&              rToValue ) :
                mpAnimation( rAnimation ),
                mpShape(),
                mpAttributeLayer(),
                maToValue( rToValue )
            {
                ENSURE_AND_THROW( mpAnimation.get(),
                                  "SetActivity::SetActivity(): Invalid animation");
            }

            virtual void dispose()
            {
                mpAnimation.reset();
                mpShape.reset();
                mpAttributeLayer.reset();
            }

            virtual bool perform()
            {
                if( !mpAnimation.get() || !mpAttributeLayer.get() || !mpShape.get() )
                    return false;

                mpAnimation->start( mpShape,
                                    mpAttributeLayer );
                (*mpAnimation)(maToValue);
                mpAnimation->end();

                return false; // we're going inactive immediately
            }

            virtual bool isActive() const
            {
                return false;
            }

            virtual bool needsScreenUpdate() const
            {
                return true;
            }

            virtual void end()
            {
            }

            virtual void setTargets( const AnimatableShapeSharedPtr&        rShape,
                                     const ShapeAttributeLayerSharedPtr&    rAttrLayer )
            {
                ENSURE_AND_THROW( rShape.get(),
                                  "SetActivity::setTargets(): Invalid shape" );
                ENSURE_AND_THROW( rAttrLayer.get(),
                                  "SetActivity::setTargets(): Invalid attribute layer" );

                mpShape = rShape;
                mpAttributeLayer = rAttrLayer;
            }

        private:
            AnimationSharedPtrT             mpAnimation;
            AnimatableShapeSharedPtr        mpShape;
            ShapeAttributeLayerSharedPtr    mpAttributeLayer;
            ValueT                          maToValue;
        };

        template< class AnimationT > AnimationActivitySharedPtr makeSetActivity( const ::boost::shared_ptr< AnimationT >&   rAnimation,
                                                                                 const typename AnimationT::ValueType&      rToValue )
        {
            return AnimationActivitySharedPtr( new SetActivity< AnimationT >(rAnimation,rToValue) );
        }
    }
}

#endif /* _SLIDESHOW_SETACTIVITY_HXX */
