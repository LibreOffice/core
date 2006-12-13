/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: disposable.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:54:30 $
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

#ifndef _SLIDESHOW_DISPOSABLE_HXX
#define _SLIDESHOW_DISPOSABLE_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>


/* Definition of Disposable interface */

namespace slideshow
{
    namespace internal
    {
        /** Disposable interface

            With ref-counted objects, deleting object networks
            containing cycles requires a dispose() call, to enforce
            every object to call dispose on and release local
            references.
         */
        class Disposable
        {
        public:
            virtual ~Disposable() {}

            /** Dispose all object references.

                An implementor of this method must first call
                dispose() on any of its external references, and
                release them after that.
             */
            virtual void dispose() = 0;
        };

    }
}

#endif /* _SLIDESHOW_DISPOSABLE_HXX */
