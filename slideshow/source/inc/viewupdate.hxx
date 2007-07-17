/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewupdate.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:20:40 $
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

#ifndef INCLUDED_SLIDESHOW_VIEWUPDATE_HXX
#define INCLUDED_SLIDESHOW_VIEWUPDATE_HXX

#include "disposable.hxx"
#include "view.hxx"
#include <boost/shared_ptr.hpp>

/* Definition of ViewUpdate interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface for something that can perform view updates.

            Use this interface for objects that perform view updates
            from time to time, e.g. slide content. Availability of
            updates can be queried, and subsequently performed.
         */
        class ViewUpdate : public Disposable
        {
        public:
            /** Perform the update action on all views

                @return true, if the update was performed
                successfully, false otherwise.
             */
            virtual bool update() = 0;

            /** Perform the update action on given view only

                @return true, if the update was performed
                successfully, false otherwise.
             */
            virtual bool update( ViewSharedPtr const& rView ) = 0;

            /** Query whether updates are pending

                @return true, if updates are pending. Calling update()
                subsequently will perform the pending update then.
             */
            virtual bool needsUpdate() const = 0;
        };

        typedef ::boost::shared_ptr< ViewUpdate > ViewUpdateSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWUPDATE_HXX */
