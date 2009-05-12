/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: weakbag.hxx,v $
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

#ifndef INCLUDED_COMPHELPER_WEAKBAG_HXX
#define INCLUDED_COMPHELPER_WEAKBAG_HXX

#include "sal/config.h"

#include <list>
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/weakref.hxx"
#include "osl/diagnose.h"

namespace comphelper {

/**
   A bag of UNO weak references.
*/
template< typename T > class WeakBag {
public:
    /**
       Add a new weak reference.

       The implementation keeps the amount of memory consumed linear in the
       number of living references added, not linear in the number of total
       references added.

       @param e
       a non-null reference.
    */
    void add(com::sun::star::uno::Reference< T > const & e) {
        OSL_ASSERT(e.is());
        for (typename List::iterator i(m_list.begin()); i != m_list.end();) {
            if (com::sun::star::uno::Reference< T >(*i).is()) {
                ++i;
            } else {
                i = m_list.erase(i);
            }
        }
        m_list.push_back(com::sun::star::uno::WeakReference< T >(e));
    }

    /**
       Remove a living reference.

       @return
       a living reference, or null if there are none.
    */
    com::sun::star::uno::Reference< T > remove() {
        while (!m_list.empty()) {
            com::sun::star::uno::Reference< T > r(m_list.front());
            m_list.pop_front();
            if (r.is()) {
                return r;
            }
        }
        return com::sun::star::uno::Reference< T >();
    }

private:
    typedef std::list< com::sun::star::uno::WeakReference< T > > List;

    List m_list;
};

}

#endif
