/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEMANAGER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEMANAGER_HXX

#include "disposable.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <memory>

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
        typedef ::std::shared_ptr< AnimatableShape > AnimatableShapeSharedPtr;
        typedef ::std::shared_ptr< Shape > ShapeSharedPtr;
        typedef std::shared_ptr< HyperlinkArea > HyperlinkAreaSharedPtr;

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
            virtual void enterAnimationMode( const AnimatableShapeSharedPtr& rShape ) = 0;

            /** Notify the ShapeManager that the given Shape is no
                longer animated.

                When called a corresponding number of times as
                enterAnimationMode() for a given shape, this methods
                ends animation mode for the given Shape. It is illegal
                to call this method more often than
                enterAnimationMode().
             */
            virtual void leaveAnimationMode( const AnimatableShapeSharedPtr& rShape ) = 0;

            /** Notify that a shape needs an update

                This method notifies the ShapeManager that a shape
                update is necessary. Use this if e.g. a running
                animation changed the shape appearance.

                @param rShape
                Shape which needs an update
             */
            virtual void notifyShapeUpdate( const ShapeSharedPtr& rShape ) = 0;

            /** Lookup a Shape from an XShape model object

                This method looks up the internal shape map for one
                representing the given XShape.

                @param xShape
                The XShape object, for which the representing Shape
                should be looked up.
             */
            virtual ShapeSharedPtr lookupShape(
                css::uno::Reference< css::drawing::XShape > const & xShape ) const = 0;

            /** Register given shape as a hyperlink target

                @param rArea
                Hyperlink sensitive area. Will participate in
                hyperlink region lookup. Must be in absolute user
                space coordinates.
             */
            virtual void addHyperlinkArea( const HyperlinkAreaSharedPtr& rArea ) = 0;
        };

        typedef ::std::shared_ptr< ShapeManager > ShapeManagerSharedPtr;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
