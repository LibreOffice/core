/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: null_usagecounter.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _NULLCANVAS_USAGECOUNTER_HXX
#define _NULLCANVAS_USAGECOUNTER_HXX

#include <osl/interlck.h>
#include <boost/current_function.hpp>

namespace nullcanvas
{
    /** Little resource tracking counter.

        When using this object, a global use counter, specific to the
        given type is incremented on object construction, and
        decremented on object destruction.
     */
    template< class Type > class UsageCounter
    {
    public:
        UsageCounter()
        {
            OSL_TRACE( "%s, %d objects currently in use.\n",
                       BOOST_CURRENT_FUNCTION,
                       osl_incrementInterlockedCount( &s_nCount ) );
        }

        ~UsageCounter()
        {
            const sal_Int32 nCount( osl_decrementInterlockedCount( &s_nCount ) );

            if( !nCount )
            {
                OSL_TRACE( "%s, last instance deleted.\n",
                           BOOST_CURRENT_FUNCTION );
            }
            else
            {
                OSL_TRACE( "%s, %d instances left.\n",
                           BOOST_CURRENT_FUNCTION,
                           nCount );
            }
        }

    private:
        static oslInterlockedCount s_nCount;
    };

    template< class Type > oslInterlockedCount UsageCounter<Type>::s_nCount = 0;
}

#endif /* _NULLCANVAS_USAGECOUNTER_HXX */
