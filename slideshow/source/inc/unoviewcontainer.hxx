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

#ifndef INCLUDED_SLIDESHOW_UNOVIEWCONTAINER_HXX
#define INCLUDED_SLIDESHOW_UNOVIEWCONTAINER_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <vector>

#include "unoview.hxx"


namespace com { namespace sun { namespace star { namespace presentation
{
    class XSlideShowView;
} } } }

/* Definition of UnoViewContainer class */

namespace slideshow
{
    namespace internal
    {
        /** Contains UnoViews
         */
        class UnoViewContainer : private boost::noncopyable
        {
        public:
            UnoViewContainer();

            /** Add a view to this container

                @return true, if the view was successfully added
                (false is e.g. returned, if the view was already
                added)
             */
            bool addView( const UnoViewSharedPtr& rView );

            /** Remove a previously added a view from this container

                @return true, if this view was successfully removed, false
                otherwise (e.g. if this view wasn't added in the first place)
            */
            bool removeView( const UnoViewSharedPtr& rView );

            /** Remove a previously added a view from this container

                @return the View object, if this view was successfully
                removed, and an empty shared_ptr otherwise (e.g. if
                this view wasn't added in the first place)
            */
            UnoViewSharedPtr removeView( const ::com::sun::star::uno::Reference<
                                                     ::com::sun::star::presentation::XSlideShowView >& xView );

            /// Dispose all stored views. Implies clear().
            void dispose();

            // the following parrots STL container concept methods
            // ===================================================

            std::size_t size() const { return maViews.size(); }
            bool empty() const { return maViews.empty(); }

            void clear() { maViews.clear(); }


            UnoViewVector::iterator         begin() { return maViews.begin(); }
            UnoViewVector::const_iterator   begin() const { return maViews.begin(); }
            UnoViewVector::iterator         end() { return maViews.end(); }
            UnoViewVector::const_iterator   end() const { return maViews.end(); }

        private:
            /// All added views
            UnoViewVector   maViews;
        };

        typedef ::boost::shared_ptr< UnoViewContainer > UnoViewContainerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_UNOVIEWCONTAINER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
