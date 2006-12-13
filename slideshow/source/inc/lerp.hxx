/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lerp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:59:32 $
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

#ifndef _SLIDESHOW_LERP_HXX
#define _SLIDESHOW_LERP_HXX

#include <sal/types.h>

namespace slideshow
{
    namespace internal
    {

        /** Generic linear interpolator

            @tpl ValueType
            Must have operator+ and operator* defined, and should
            have value semantics.

            @param t
            As usual, t must be in the [0,1] range
        */
        template< typename ValueType > ValueType lerp( const ValueType&     rFrom,
                                                       const ValueType&     rTo,
                                                       double               t )
        {
            // This is only to suppress a double->int warning. All other
            // types should be okay here.
            return static_cast<ValueType>( (1.0-t)*rFrom + t*rTo );
        }

    }
}

#endif /* _SLIDESHOW_LERP_HXX */
