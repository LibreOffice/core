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



#ifndef \
    INCLUDED_stoc_source_uriproc_UriSchemeParser_vndDOTsunDOTstarDOTscript_hxx
#define \
    INCLUDED_stoc_source_uriproc_UriSchemeParser_vndDOTsunDOTstarDOTscript_hxx

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
    class XInterface;
} } } }
namespace rtl { class OUString; }

namespace stoc { namespace uriproc {

namespace UriSchemeParser_vndDOTsunDOTstarDOTscript {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
    create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const &)
        SAL_THROW((com::sun::star::uno::Exception));

    rtl::OUString SAL_CALL getImplementationName();

    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames();
}

} }

#endif
