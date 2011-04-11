/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_stoc_source_registry_tdprovider_structtypedescription_hxx
#define INCLUDED_stoc_source_registry_tdprovider_structtypedescription_hxx

#include "com/sun/star/reflection/XPublished.hpp"
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppuhelper/implbase2.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace container { class XHierarchicalNameAccess; }
    namespace reflection { class XCompoundTypeDescription; }
} } }
namespace rtl { class OUString; }
namespace stoc_rdbtdp { class CompoundTypeDescriptionImpl; }

namespace stoc { namespace registry_tdprovider {

class StructTypeDescription:
    public cppu::WeakImplHelper2<
        com::sun::star::reflection::XStructTypeDescription,
        com::sun::star::reflection::XPublished >
{
public:
    StructTypeDescription(
        com::sun::star::uno::Reference<
            com::sun::star::container::XHierarchicalNameAccess > const &
            manager,
        rtl::OUString const & name, rtl::OUString const & baseTypeName,
        com::sun::star::uno::Sequence< sal_Int8 > const & data, bool published);

    virtual ~StructTypeDescription();

    virtual com::sun::star::uno::TypeClass SAL_CALL getTypeClass()
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getName()
        throw (com::sun::star::uno::RuntimeException);

    virtual
    com::sun::star::uno::Reference<
        com::sun::star::reflection::XTypeDescription >
    SAL_CALL getBaseType() throw (com::sun::star::uno::RuntimeException);

    virtual
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getMemberNames() throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getTypeParameters() throw (com::sun::star::uno::RuntimeException);

    virtual
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isPublished()
        throw (com::sun::star::uno::RuntimeException);

private:
    StructTypeDescription(StructTypeDescription &); // not implemented
    void operator =(StructTypeDescription); // not implemented

    com::sun::star::uno::Sequence< sal_Int8 > m_data;
    rtl::Reference< stoc_rdbtdp::CompoundTypeDescriptionImpl > m_base;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
