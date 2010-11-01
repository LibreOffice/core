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

#ifndef INCLUDED_SLIDESHOW_ANIMATABLESHAPE_HXX
#define INCLUDED_SLIDESHOW_ANIMATABLESHAPE_HXX

#include <boost/shared_ptr.hpp>

#include "shape.hxx"


namespace slideshow
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

#endif /* INCLUDED_SLIDESHOW_ANIMATABLESHAPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
