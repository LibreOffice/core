/*************************************************************************
 *
 *  $RCSfile: structtypedescription.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 15:04:03 $
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

#include "structtypedescription.hxx"

#include "base.hxx"

#include "com/sun/star/reflection/XTypeDescription.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "registry/reader.hxx"
#include "registry/types.h"
#include "registry/version.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include <new>

namespace css = com::sun::star;
using stoc::registry_tdprovider::StructTypeDescription;

StructTypeDescription::StructTypeDescription(
    css::uno::Reference< css::container::XHierarchicalNameAccess > const &
        manager,
    rtl::OUString const & name, rtl::OUString const & baseTypeName,
    css::uno::Sequence< sal_Int8 > const & data, bool published):
    m_data(data),
    m_base(
        new stoc_rdbtdp::CompoundTypeDescriptionImpl(
            manager, css::uno::TypeClass_STRUCT, name, baseTypeName, data,
            published))
{}

StructTypeDescription::~StructTypeDescription()
{}

css::uno::TypeClass StructTypeDescription::getTypeClass()
    throw (css::uno::RuntimeException)
{
    return m_base->getTypeClass();
}

rtl::OUString StructTypeDescription::getName()
    throw (css::uno::RuntimeException)
{
    return m_base->getName();
}

css::uno::Reference< css::reflection::XTypeDescription >
StructTypeDescription::getBaseType() throw (css::uno::RuntimeException)
{
    return m_base->getBaseType();
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
StructTypeDescription::getMemberTypes() throw (css::uno::RuntimeException)
{
    return m_base->getMemberTypes();
}

css::uno::Sequence< rtl::OUString > StructTypeDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    return m_base->getMemberNames();
}

css::uno::Sequence< rtl::OUString > StructTypeDescription::getTypeParameters()
    throw (css::uno::RuntimeException)
{
    try {
        typereg::Reader reader(
            m_data.getConstArray(), m_data.getLength(), false,
            TYPEREG_VERSION_1);
        OSL_ASSERT(reader.isValid());
        sal_uInt16 n = reader.getReferenceCount();
        css::uno::Sequence< rtl::OUString > parameters(n);
        for (sal_uInt16 i = 0; i < n; ++i) {
            if (reader.getReferenceFlags(i) != RT_ACCESS_INVALID
                || reader.getReferenceSort(i) != RT_REF_TYPE_PARAMETER)
            {
                throw css::uno::RuntimeException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "type parameter of polymorphic struct type template"
                            " not RT_ACCESS_INVALID/RT_REF_TYPE_PARAMETER")),
                    static_cast< cppu::OWeakObject * >(this));
            }
            parameters[i] = reader.getReferenceTypeName(i);
        }
        return parameters;
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("std::bad_alloc")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
StructTypeDescription::getTypeArguments() throw (css::uno::RuntimeException)
{
    return css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >();
}

sal_Bool StructTypeDescription::isPublished() throw (css::uno::RuntimeException)
{
    return m_base->isPublished();
}
