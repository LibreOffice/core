/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoviewcontainer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:23:45 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <vector>

#include <unoview.hxx>


namespace com { namespace sun { namespace star { namespace presentation
{
    class XSlideShowView;
} } } }

/* Definition of UnoViewContainer class */

namespace presentation
{
    namespace internal
    {
        /** Contains UnoViews
         */
        class UnoViewContainer
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

            // the following parrots STL container concept methods
            // ===================================================

            bool empty() const;
            void clear();

            UnoViewVector::iterator         begin();
            UnoViewVector::const_iterator   begin() const;
            UnoViewVector::iterator         end();
            UnoViewVector::const_iterator   end() const;

        private:
            // default: disabled copy/assignment
            UnoViewContainer(const UnoViewContainer&);
            UnoViewContainer& operator=( const UnoViewContainer& );

            /// All added views
            UnoViewVector   maViews;
        };

        typedef ::boost::shared_ptr< UnoViewContainer > UnoViewContainerSharedPtr;

    }
}

#endif /* _SLIDESHOW_UNOVIEWCONTAINER_HXX */
