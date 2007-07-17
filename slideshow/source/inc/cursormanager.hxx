/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cursormanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:04:22 $
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

#ifndef INCLUDED_SLIDESHOW_CURSORMANAGER_HXX
#define INCLUDED_SLIDESHOW_CURSORMANAGER_HXX

#include <sal/types.h>
#include <boost/shared_ptr.hpp>


/* Definition of CursorManager interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling the view cursor.

            Classes implementing this interface interact with the
            View, arbitrating access to the mouse cursor shape.
         */
        class CursorManager
        {
        public:
            virtual ~CursorManager() {}

            /** Request different cursor shape.

                @param nCursorShape
                Shape ID of the new mouse cursor
             */
            virtual bool requestCursor( sal_Int16 nCursorShape ) = 0;

            /** Reset cursor to default one.

                This method resets the cursor to whatever default to
                manager deems appropriate.
             */
            virtual void resetCursor() = 0;
        };

        typedef ::boost::shared_ptr< CursorManager > CursorManagerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_CURSORMANAGER_HXX */
