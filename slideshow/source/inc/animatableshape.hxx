/*************************************************************************
 *
 *  $RCSfile: animatableshape.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:11:59 $
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
