/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
