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



#ifndef INCLUDED_stoc_source_registry_tdprovider_functiondescription_hxx
#define INCLUDED_stoc_source_registry_tdprovider_functiondescription_hxx

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "osl/mutex.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace container {
        class XHierarchicalNameAccess;
    }
    namespace reflection {
        class XCompoundTypeDescription;
    }
} } }
namespace typereg { class Reader; }

namespace stoc { namespace registry_tdprovider {

class FunctionDescription {
public:
    FunctionDescription(
        com::sun::star::uno::Reference<
            com::sun::star::container::XHierarchicalNameAccess > const &
            manager,
        com::sun::star::uno::Sequence< sal_Int8 > const & bytes,
        sal_uInt16 index);

    ~FunctionDescription();

    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::reflection::XCompoundTypeDescription > >
    getExceptions() const;

protected:
    typereg::Reader getReader() const;

    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess > m_manager;
    com::sun::star::uno::Sequence< sal_Int8 > m_bytes;
    sal_uInt16 m_index;

    mutable osl::Mutex m_mutex;
    mutable com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::reflection::XCompoundTypeDescription > >
                m_exceptions;
    mutable bool m_exceptionsInit;

private:
    FunctionDescription(FunctionDescription &); // not implemented
    void operator =(FunctionDescription); // not implemented
};

} }

#endif
