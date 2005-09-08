/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertyinfohelper.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:17:01 $
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

#include "propertyinfohelper.hxx"

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif

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
beans::Property helperMakeProperty(configuration::Name const& aName,
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

    return beans::Property(aName.toString(), -1, aType, nPropAttributes);
}
//-----------------------------------------------------------------------------
    }

}


