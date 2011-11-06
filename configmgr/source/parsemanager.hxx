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



#ifndef INCLUDED_CONFIGMGR_SOURCE_PARSEMANAGER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_PARSEMANAGER_HXX

#include "sal/config.h"

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "rtl/ref.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

namespace rtl { class OUString; }

namespace configmgr {

class Parser;

class ParseManager: public salhelper::SimpleReferenceObject {
public:
    ParseManager(
        rtl::OUString const & url, rtl::Reference< Parser > const & parser)
        SAL_THROW((
            com::sun::star::container::NoSuchElementException,
            com::sun::star::uno::RuntimeException));

    bool parse();

    enum { NAMESPACE_OOR = 1, NAMESPACE_XS = 2, NAMESPACE_XSI = 3 };

private:
    virtual ~ParseManager();

    xmlreader::XmlReader reader_;
    rtl::Reference< Parser > parser_;
    xmlreader::Span itemData_;
    int itemNamespaceId_;
};

}

#endif
