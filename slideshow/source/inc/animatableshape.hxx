/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animatableshape.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:04:13 $
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

#ifndef _SLIDESHOW_ANIMATABLESHAPE_HXX
#define _SLIDESHOW_ANIMATABLESHAPE_HXX

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <shape.hxx>


namespace presentation
{
    namespace internal
    {
        /** Represents an animatable shape.

            This interface adds animation handling methods to a
            shape. It allows transparent switching between
            sprite-based viewing and static painting, depending on
            whether animations are currently running.
         */
        class AnimatableShape : public Shape
        {
        public:
            // Animation methods
            //------------------------------------------------------------------

            /** Notify the Shape that an animation starts now

                This method enters animation mode on all registered
                views.

                @attention This method is supposed to be called only
                from the LayerManager, since it might involve shifting
                shapes between different layers (and removing this
                shape from the background layer in the first place)
             */
            virtual void enterAnimationMode() = 0;

            /** Notify the Shape that it is no longer animated

                This methods requests the Shape to end animation mode
                on all registered views, if called more or equal the
                times enterAnimationMode() was called. That is, the
                Shape only leaves animation mode, if all requested
                enterAnimationMode() call sites have issued their
                matching leaveAnimationMode().

                @attention This method is supposed to be called only
                from the LayerManager, since it might involve shifting
                shapes between different layers (and adding this
                shape to the background layer again)
             */
            virtual void leaveAnimationMode() = 0;

        };

        typedef ::boost::shared_ptr< AnimatableShape > AnimatableShapeSharedPtr;

    }
}

#endif /* _SLIDESHOW_ANIMATABLESHAPE_HXX */
