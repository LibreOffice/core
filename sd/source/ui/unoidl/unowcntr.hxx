/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unowcntr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:56:55 $
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

#ifndef _SV_UNOWCNTR_HXX_
#define _SV_UNOWCNTR_HXX_

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

typedef sal_Bool (*weakref_searchfunc)( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > xRef, void* pSearchData );

class WeakRefList;

class SvUnoWeakContainer
{
private:
    WeakRefList*    mpList;

public:
    SvUnoWeakContainer() throw();
    ~SvUnoWeakContainer() throw();

    /** inserts the given ref into this container */
    void    insert( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > xRef ) throw();

    /** removes the given ref from this container */
    void    remove( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > xRef ) throw();

    /** searches the container for a ref that returns true on the given
        search function
    */
    sal_Bool findRef( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >& rRef, void* pSearchData, weakref_searchfunc pSearchFunc );

    void dispose();
};

#endif // _SV_UNOWCNTR_HXX_

