/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertyinfohelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:17:46 $
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

#ifndef CONFIGMGR_API_PROPERTYINFOIMPL_HXX_
#define CONFIGMGR_API_PROPERTYINFOIMPL_HXX_

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

namespace configmgr
{
    namespace uno = ::com::sun::star::uno;
    namespace beans = ::com::sun::star::beans;

    /* implementations of the interfaces supported by a (parent) node
        within the configuration tree.
        (read-only operation)
    */
    namespace node  { struct Attributes; }
    namespace configuration { class Name; }

    namespace configapi
    {
        // translation helper
        beans::Property helperMakeProperty( configuration::Name const& aName, node::Attributes const aAttributes, uno::Type const& aType, bool bDefaultable )
            throw(uno::RuntimeException);

    }

}
#endif // CONFIGMGR_API_PROPERTYINFOIMPL_HXX_


