/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: null_usagecounter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:51:13 $
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
