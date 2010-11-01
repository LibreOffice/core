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

#ifndef INCLUDED_SLIDESHOW_VIEW_HXX
#define INCLUDED_SLIDESHOW_VIEW_HXX

#include "viewlayer.hxx"

#include <boost/shared_ptr.hpp>
#include <vector>


namespace basegfx { class B2DRange; class B2DVector; }


/* Definition of View interface */

namespace slideshow
{
    namespace internal
    {
        class View : public ViewLayer
        {
        public:
            /** Create a new view layer for this view

                @param rLayerBounds
                Specifies the bound rect of the layer relative to the
                user view coordinate system.

                This method sets the bounds of the view layer in
                document coordinates (i.e. 'logical' coordinates). The
                resulting transformation is then concatenated with the
                underlying view transformation, returned by the
                getTransformation() method.
             */
            virtual ViewLayerSharedPtr createViewLayer( const basegfx::B2DRange& rLayerBounds ) const = 0;

            /** Update screen representation from backbuffer
             */
            virtual bool updateScreen() const = 0;

            /** Paint screen content unconditionally from backbuffer
             */
            virtual bool paintScreen() const = 0;

            /** Set the size of the user view coordinate system.

                This method sets the width and height of the view in
                document coordinates (i.e. 'logical' coordinates). The
                resulting transformation is then concatenated with the
                underlying view transformation, returned by the
                getTransformation() method.
            */
            virtual void setViewSize( const ::basegfx::B2DVector& ) = 0;

            /** Change the view's mouse cursor.

                @param nPointerShape
                One of the ::com::sun::star::awt::SystemPointer
                constant group members.
             */
            virtual void setCursorShape( sal_Int16 nPointerShape ) = 0;
        };

        typedef ::boost::shared_ptr< View >     ViewSharedPtr;
        typedef ::std::vector< ViewSharedPtr >  ViewVector;
    }
}

#endif /* INCLUDED_SLIDESHOW_VIEW_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
