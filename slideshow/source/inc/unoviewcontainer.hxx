/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoviewcontainer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:06:14 $
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

#ifndef _SLIDESHOW_UNOVIEWCONTAINER_HXX
#define _SLIDESHOW_UNOVIEWCONTAINER_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <vector>

#include <unoview.hxx>


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

#endif /* _SLIDESHOW_UNOVIEWCONTAINER_HXX */
