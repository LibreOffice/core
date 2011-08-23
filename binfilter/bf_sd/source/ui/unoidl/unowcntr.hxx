/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SV_UNOWCNTR_HXX_
#define _SV_UNOWCNTR_HXX_

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
namespace binfilter {

typedef sal_Bool (*weakref_searchfunc)( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > xRef, void* pSearchData );

class WeakRefList;

class SvUnoWeakContainer
{
private:
    WeakRefList*	mpList;

public:
    SvUnoWeakContainer() throw();
    ~SvUnoWeakContainer() throw();

    /** inserts the given ref into this container */
    void	insert( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > xRef ) throw();

    /** searches the container for a ref that returns true on the given 
        search function
    */
    sal_Bool findRef( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >& rRef, void* pSearchData, weakref_searchfunc pSearchFunc );	
};

} //namespace binfilter
#endif // _SV_UNOWCNTR_HXX_

