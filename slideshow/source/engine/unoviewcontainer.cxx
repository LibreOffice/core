/*************************************************************************
 *
 *  $RCSfile: unoviewcontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:00:38 $
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

#include <canvas/debug.hxx>
#include <unoviewcontainer.hxx>

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif

#include <algorithm>


using namespace ::drafts::com::sun::star;
using namespace ::com::sun::star;

// -----------------------------------------------------------------------------

namespace presentation
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
                                    ::std::equal_to< uno::Reference< ::com::sun::star::presentation::XSlideShowView > >(),
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

        UnoViewSharedPtr UnoViewContainer::removeView( const uno::Reference< ::com::sun::star::presentation::XSlideShowView >& xView )
        {
            // check whether same view is already added
            const UnoViewVector::iterator aEnd( maViews.end() );
            UnoViewVector::iterator aIter;

            // added in the first place?
            if( (aIter=::std::remove_if( maViews.begin(),
                                         aEnd,
                                         ::boost::bind(
                                             ::std::equal_to< uno::Reference< ::com::sun::star::presentation::XSlideShowView > >(),
                                             ::boost::cref( xView ),
                                             ::boost::bind(
                                                 &UnoView::getUnoView,
                                                 _1 ) ) ) ) == aEnd )
            {
                // nope, nothing to do
                return UnoViewSharedPtr();
            }

            OSL_ENSURE( ::std::distance( aIter, aEnd ) == 1,
                        "UnoViewContainer::removeView(): View was added multiple times" );

            UnoViewSharedPtr pView( *aIter );

            // actually erase from container
            maViews.erase( aIter, aEnd );

            return pView;
        }

        bool UnoViewContainer::removeView( const UnoViewSharedPtr& rView )
        {
            // remove locally
            const UnoViewVector::iterator aEnd( maViews.end() );
            UnoViewVector::iterator aIter;
            if( (aIter=::std::remove( maViews.begin(),
                                      aEnd,
                                      rView )) == aEnd )
            {
                // view seemingly was not added, failed
                return false;
            }

            OSL_ENSURE( ::std::distance( aIter, aEnd ) == 1,
                        "UnoViewContainer::removeView(): View was added multiple times" );

            // actually erase from container
            maViews.erase( aIter, aEnd );

            return true;
        }

        bool UnoViewContainer::empty() const
        {
            return maViews.empty();
        }

        void UnoViewContainer::clear()
        {
            maViews.clear();
        }

        UnoViewVector::iterator UnoViewContainer::begin()
        {
            return maViews.begin();
        }

        UnoViewVector::const_iterator UnoViewContainer::begin() const
        {
            return maViews.begin();
        }

        UnoViewVector::iterator UnoViewContainer::end()
        {
            return maViews.end();
        }

        UnoViewVector::const_iterator UnoViewContainer::end() const
        {
            return maViews.end();
        }

    }
}
