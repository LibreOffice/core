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



#ifndef _XMLHELPER_HXX
#define _XMLHELPER_HXX

#include <sal/types.h>


namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace container { class XNameContainer; }
    namespace xml { namespace dom { class XDocumentBuilder; } }
} } }


bool isValidQName( const rtl::OUString& sName,
                   const com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>& xNamespaces );

bool isValidPrefixName( const rtl::OUString& sName,
                        const com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>& xNamespaces );

com::sun::star::uno::Reference<com::sun::star::xml::dom::XDocumentBuilder> getDocumentBuilder();


#endif
