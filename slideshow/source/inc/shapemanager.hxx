/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapemanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:14:29 $
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
