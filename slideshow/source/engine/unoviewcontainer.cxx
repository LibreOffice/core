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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <unoviewcontainer.hxx>

#include <boost/bind.hpp>

#include <algorithm>


using namespace ::com::sun::star;

// -----------------------------------------------------------------------------

namespace slideshow
{
    namespace internal
    {
        UnoViewContainer::UnoViewContainer() :
            maViews()
        {
        }

        bool UnoViewContainer::addView( const UnoViewSharedPtr& rView )
        {
            // check whether same view is already added
            const UnoViewVector::iterator aEnd( maViews.end() );

            // already added?
            if( ::std::find_if( maViews.begin(),
                                aEnd,
                                ::boost::bind(
                                    ::std::equal_to< uno::Reference< presentation::XSlideShowView > >(),
                                    ::boost::cref( rView->getUnoView() ),
                                    ::boost::bind(
                                        &UnoView::getUnoView,
                                        _1 ) ) ) != aEnd )
            {
                // yes, nothing to do
                return false;
            }

            // add locally
            maViews.push_back( rView );

            return true;
        }

        UnoViewSharedPtr UnoViewContainer::removeView( const uno::Reference< presentation::XSlideShowView >& xView )
        {
            // check whether same view is already added
            const UnoViewVector::iterator aEnd( maViews.end() );
            UnoViewVector::iterator aIter;

            // added in the first place?
            if( (aIter=::std::find_if( maViews.begin(),
                                       aEnd,
                                       ::boost::bind(
                                           ::std::equal_to< uno::Reference< presentation::XSlideShowView > >(),
                                           ::boost::cref( xView ),
                                           ::boost::bind(
                                               &UnoView::getUnoView,
                                               _1 ) ) ) ) == aEnd )
            {
                // nope, nothing to do
                return UnoViewSharedPtr();
            }

            OSL_ENSURE(
                ::std::count_if(
                    maViews.begin(),
                    aEnd,
                    ::boost::bind(
                        ::std::equal_to< uno::Reference< presentation::XSlideShowView > >(),
                        ::boost::cref( xView ),
                        ::boost::bind(
                            &UnoView::getUnoView,
                            _1 ))) == 1,
                "UnoViewContainer::removeView(): View was added multiple times" );

            UnoViewSharedPtr pView( *aIter );

            // actually erase from container
            maViews.erase( aIter );

            return pView;
        }

        bool UnoViewContainer::removeView( const UnoViewSharedPtr& rView )
        {
            // remove locally
            const UnoViewVector::iterator aEnd( maViews.end() );
            UnoViewVector::iterator aIter;
            if( (aIter=::std::find( maViews.begin(),
                                    aEnd,
                                    rView )) == aEnd )
            {
                // view seemingly was not added, failed
                return false;
            }

            OSL_ENSURE( ::std::count( maViews.begin(),
                                      aEnd,
                                      rView ) == 1,
                        "UnoViewContainer::removeView(): View was added multiple times" );

            // actually erase from container
            maViews.erase( aIter );

            return true;
        }

        void UnoViewContainer::dispose()
        {
            ::std::for_each( maViews.begin(),
                             maViews.end(),
                             ::boost::mem_fn(&UnoView::_dispose) );
            maViews.clear();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
