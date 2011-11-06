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



#ifndef INCLUDED_STOC_SOURCE_SIMPLEREGISTRY_TEXTUALSERVICES_HXX
#define INCLUDED_STOC_SOURCE_SIMPLEREGISTRY_TEXTUALSERVICES_HXX

#include "sal/config.h"

#include <memory>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

namespace com { namespace sun { namespace star { namespace registry {
    class XRegistryKey;
} } } }

namespace stoc { namespace simpleregistry {

class Data;

class TextualServices: private boost::noncopyable {
public:
    explicit TextualServices(rtl::OUString const & uri);
        // throws com::sun::star::registry::InvalidRegistryException

    virtual ~TextualServices();

    inline rtl::OUString getUri() { return uri_; }

    com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey >
    getRootKey();

private:
    rtl::OUString uri_;
    rtl::Reference< Data > data_;
};

} }

#endif
