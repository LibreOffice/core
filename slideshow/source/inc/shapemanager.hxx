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

#ifndef INCLUDED_SLIDESHOW_SHAPEMANAGER_HXX
#define INCLUDED_SLIDESHOW_SHAPEMANAGER_HXX

#include "disposable.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <boost/shared_ptr.hpp>

namespace com {  namespace sun { namespace star { namespace drawing {
    class XShape;
} } } }

/* Definition of ShapeManager interface */

namespace slideshow
{
    namespace internal
    {
        class HyperlinkArea;
        class AnimatableShape;
        class Shape;

        /** ShapeManager interface

            Implementers of this interface manage appearance and
            animation of slideshow shapes.
         */
        class ShapeManager : public Disposable
        {
        public:
            /** Notify the ShapeManager that the given Shape starts an
                animation now.

                This method enters animation mode for the Shape. If
                the shape is already in animation mode, the call is
                counted, and the shape only leaves animation mode
                after a corresponding number of leaveAnimationMode()
                calls.
             */
            virtual void enterAnimationMode( const boost::shared_ptr<AnimatableShape>& rShape ) = 0;

            /** Notify the ShapeManager that the given Shape is no
                longer animated.

                When called a corresponding number of times as
                enterAnimationMode() for a given shape, this methods
                ends animation mode for the given Shape. It is illegal
                to call this method more often than
                enterAnimationMode().
             */
            virtual void leaveAnimationMode( const boost::shared_ptr<AnimatableShape>& rShape ) = 0;

            /** Notify that a shape needs an update

                This method notifies the ShapeManager that a shape
                update is necessary. Use this if e.g. a running
                animation changed the shape appearance.

                @param rShape
                Shape which needs an update
             */
            virtual void notifyShapeUpdate( const boost::shared_ptr<Shape>& rShape ) = 0;

            /** Lookup a Shape from an XShape model object

                This method looks up the internal shape map for one
                representing the given XShape.

                @param xShape
                The XShape object, for which the representing Shape
                should be looked up.
             */
            virtual boost::shared_ptr<Shape> lookupShape(
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape > const & xShape ) const = 0;

            /** Register given shape as a hyperlink target

                @param rArea
                Hyperlink sensitive area. Will participate in
                hyperlink region lookup. Must be in absolute user
                space coordinates.
             */
            virtual void addHyperlinkArea( const boost::shared_ptr<HyperlinkArea>& rArea ) = 0;

            /** Unregister given shape as a hyperlink target

                @param rArea
                Hyperlink sensitive area. Will cease to participate in
                hyperlink region lookup.
             */
            virtual void removeHyperlinkArea( const boost::shared_ptr<HyperlinkArea>& rArea ) = 0;
        };

        typedef ::boost::shared_ptr< ShapeManager > ShapeManagerSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_SHAPEMANAGER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
