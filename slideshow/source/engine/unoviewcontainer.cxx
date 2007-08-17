/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoviewcontainer.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-17 12:43:29 $
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
