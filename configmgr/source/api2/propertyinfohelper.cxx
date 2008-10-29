/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertyinfohelper.cxx,v $
 * $Revision: 1.12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "propertyinfohelper.hxx"
#include "configpath.hxx"
#include "attributes.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HDL_
#include <com/sun/star/beans/PropertyAttribute.hdl>
#endif

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;
    namespace beans = ::com::sun::star::beans;

    namespace configapi
    {
//-----------------------------------------------------------------------------
beans::Property helperMakeProperty(rtl::OUString const& aName,
                                   node::Attributes const aAttributes,
                                   uno::Type const& aType,
                                   bool bDefaultable )
    throw(uno::RuntimeException)
{
    namespace PropertyAttribute = com::sun::star::beans::PropertyAttribute;

    sal_Int16 nPropAttributes = 0;
    if (aAttributes.isReadonly())       nPropAttributes |= PropertyAttribute::READONLY;
    if (aAttributes.isNullable())       nPropAttributes |= PropertyAttribute::MAYBEVOID;
    /*if ( aAttributes.bNotified)*/ nPropAttributes |= PropertyAttribute::BOUND;
    /*if ( aAttributes.bConstrained)nPropAttributes |= PropertyAttribute::CONSTRAINED;*/

    if ( aAttributes.isRemovable())     nPropAttributes |= PropertyAttribute::REMOVABLE;
    if ( bDefaultable)  nPropAttributes |= PropertyAttribute::MAYBEDEFAULT;

    return beans::Property(aName, -1, aType, nPropAttributes);
}
//-----------------------------------------------------------------------------
    }

}


