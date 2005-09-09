/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachemapobjectcontainer2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:21:08 $
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

#ifndef INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX
#define INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include "cppuhelper/weakref.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include "osl/mutex.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef INCLUDED_HASH_MAP
#include <hash_map>
#define INCLUDED_HASH_MAP
#endif

namespace rtl {
    class OUString;
    struct OUStringHash;
}
namespace ucb { namespace cachemap { class Object2; } }

namespace ucb { namespace cachemap {

class ObjectContainer2
{
public:
    ObjectContainer2();

    ~ObjectContainer2() SAL_THROW(());

    rtl::Reference< Object2 > get(rtl::OUString const & rKey);

private:
    typedef std::hash_map< rtl::OUString,
                           com::sun::star::uno::WeakReference< Object2 >,
                           rtl::OUStringHash >
    Map;

    ObjectContainer2(ObjectContainer2 &); // not implemented
    void operator =(ObjectContainer2); // not implemented

    Map m_aMap;
    osl::Mutex m_aMutex;
};

} }

#endif // INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX
