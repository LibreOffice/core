/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: setactivity.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:47:13 $
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

#ifndef _SLIDESHOW_SETACTIVITY_HXX
#define _SLIDESHOW_SETACTIVITY_HXX

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <animationactivity.hxx>
#include <animation.hxx>
#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>
#include <activitiesfactory.hxx>


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

            SetActivity( const ActivitiesFactory::CommonParameters& rParms,
                         const AnimationSharedPtrT&                 rAnimation,
                         const ValueT&                              rToValue ) :
                mpAnimation( rAnimation ),
                mpShape(),
                mpAttributeLayer(),
                mpEndEvent( rParms.mpEndEvent ),
                mrEventQueue( rParms.mrEventQueue ),
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

            virtual double calcTimeLag() const
            {
                return 0.0;
            }

            virtual bool perform()
            {
                if( !mpAnimation.get() || !mpAttributeLayer.get() || !mpShape.get() )
                    return false;

                mpAnimation->start( mpShape,
                                    mpAttributeLayer );
                (*mpAnimation)(maToValue);
                mpAnimation->end();

                // fire end event, if any
                if( mpEndEvent.get() )
                    mrEventQueue.addEvent( mpEndEvent );

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

            virtual void dequeued()
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
            EventSharedPtr                  mpEndEvent;
            EventQueue&                     mrEventQueue;
            ValueT                          maToValue;
        };

        template< class AnimationT > AnimationActivitySharedPtr makeSetActivity( const ActivitiesFactory::CommonParameters& rParms,
                                                                                 const ::boost::shared_ptr< AnimationT >&   rAnimation,
                                                                                 const typename AnimationT::ValueType&      rToValue )
        {
            return AnimationActivitySharedPtr( new SetActivity< AnimationT >(rParms,rAnimation,rToValue) );
        }
    }
}

#endif /* _SLIDESHOW_SETACTIVITY_HXX */
