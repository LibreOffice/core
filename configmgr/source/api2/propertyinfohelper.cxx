/*************************************************************************
 *
 *  $RCSfile: propertyinfohelper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:29:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


