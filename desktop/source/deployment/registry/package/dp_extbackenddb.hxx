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




#if ! defined INCLUDED_DP_EXTBACKENDDB_HXX
#define INCLUDED_DP_EXTBACKENDDB_HXX

#include "rtl/ustring.hxx"
#include "rtl/string.hxx"
#include <vector>
#include "dp_backenddb.hxx"

namespace css = ::com::sun::star;

namespace com { namespace sun { namespace star {
        namespace uno {
        class XComponentContext;
        }
        namespace xml { namespace dom {
            class XDocument;
            class XNode;
        }}
        namespace xml { namespace xpath {
            class XXPathAPI;
        }}
}}}

namespace dp_registry {
namespace backend {
namespace bundle {

/* The XML file stores the extensions which are currently registered.
   They will be removed when they are revoked.
 */
class ExtensionBackendDb: public dp_registry::backend::BackendDb
{
protected:
    virtual ::rtl::OUString getDbNSName();
    virtual ::rtl::OUString getNSPrefix();
    virtual ::rtl::OUString getRootElementName();
    virtual ::rtl::OUString getKeyElementName();

public:
    struct Data
    {
         /* every element consists of a pair of the url to the item (jar,rdb, etc)
           and the media type
         */
        ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString> > items;
        typedef ::std::vector<
            ::std::pair< ::rtl::OUString, ::rtl::OUString> >::const_iterator ITC_ITEMS;

    };

public:

    ExtensionBackendDb( css::uno::Reference<css::uno::XComponentContext> const &  xContext,
                        ::rtl::OUString const & url);

    void addEntry(::rtl::OUString const & url, Data const & data);

    Data getEntry(::rtl::OUString const & url);

};



}
}
}
#endif

