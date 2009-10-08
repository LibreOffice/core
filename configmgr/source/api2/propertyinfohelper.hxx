/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertyinfohelper.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_API_PROPERTYINFOIMPL_HXX_
#define CONFIGMGR_API_PROPERTYINFOIMPL_HXX_

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace configmgr
{
    namespace uno = ::com::sun::star::uno;
    namespace beans = ::com::sun::star::beans;

    /* implementations of the interfaces supported by a (parent) node
        within the configuration tree.
        (read-only operation)
    */
    namespace node  { struct Attributes; }

    namespace configapi
    {
        // translation helper
        beans::Property helperMakeProperty( rtl::OUString const& aName, node::Attributes const aAttributes, uno::Type const& aType, bool bDefaultable )
            throw(uno::RuntimeException);

    }

}
#endif // CONFIGMGR_API_PROPERTYINFOIMPL_HXX_


