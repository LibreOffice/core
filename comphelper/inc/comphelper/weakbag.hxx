/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: weakbag.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 13:18:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_COMPHELPER_WEAKBAG_HXX
#define INCLUDED_COMPHELPER_WEAKBAG_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include <list>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include "cppuhelper/weakref.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif

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
