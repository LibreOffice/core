/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachemapobjectcontainer2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:09:36 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#ifndef INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX
#include "cachemapobjectcontainer2.hxx"
#endif

#ifndef INCLUDED_UCB_CACHEMAPOBJECT2_HXX
#include "cachemapobject2.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_XWEAK_HPP_
#include "com/sun/star/uno/XWeak.hpp"
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include "cppuhelper/weakref.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include "osl/mutex.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

using namespace com::sun;
using ucb::cachemap::Object2;
using ucb::cachemap::ObjectContainer2;

ObjectContainer2::ObjectContainer2()
{}

ObjectContainer2::~ObjectContainer2() SAL_THROW(())
{}

rtl::Reference< Object2 > ObjectContainer2::get(rtl::OUString const & rKey)
{
    rtl::Reference< Object2 > xElement;
    {
        osl::MutexGuard aGuard(m_aMutex);
        Map::iterator aIt(m_aMap.find(rKey));
        if (aIt != m_aMap.end())
            xElement = static_cast< Object2 * >(
                           star::uno::Reference< star::uno::XWeak >(
                                   aIt->second.get(), star::uno::UNO_QUERY).
                               get());
        if (!xElement.is())
        {
            xElement = new Object2;
            m_aMap[rKey]
                = star::uno::WeakReference< Object2 >(xElement.get());
        }
    }
    return xElement;
}
