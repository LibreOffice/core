/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewrepainthandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:20:15 $
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

#ifndef INCLUDED_SLIDESHOW_VIEWREPAINTHANDLER_HXX
#define INCLUDED_SLIDESHOW_VIEWREPAINTHANDLER_HXX

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "unoview.hxx"


/* Definition of ViewRepaintHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling view repaint events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle view repaint events.
         */
        class ViewRepaintHandler
        {
        public:
            virtual ~ViewRepaintHandler() {}

            /** Notify clobbered view.

                Reasons for a viewChanged notification can be
                different view size, transformation, or other device
                properties (color resolution or profile, etc.)

                @param rView
                The changed view
             */
            virtual void viewClobbered( const UnoViewSharedPtr& rView ) = 0;
        };

        typedef ::boost::shared_ptr< ViewRepaintHandler > ViewRepaintHandlerSharedPtr;
        typedef ::boost::weak_ptr< ViewRepaintHandler >   ViewRepaintHandlerWeakPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWREPAINTHANDLER_HXX */
