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



#if ! defined INCLUDED_DP_SCRIPTBACKENDDB_HXX
#define INCLUDED_DP_SCRIPTBACKENDDB_HXX

#include "rtl/ustring.hxx"
#include "dp_backenddb.hxx"
#include "boost/optional.hpp"
namespace css = ::com::sun::star;

namespace com { namespace sun { namespace star {
        namespace uno {
        class XComponentContext;
        }
}}}

namespace dp_registry {
namespace backend {
namespace script {

/* The XML file stores the extensions which are currently registered.
   They will be removed when they are revoked.
 */
class ScriptBackendDb: public dp_registry::backend::RegisteredDb
{
protected:
    virtual ::rtl::OUString getDbNSName();

    virtual ::rtl::OUString getNSPrefix();

    virtual ::rtl::OUString getRootElementName();

    virtual ::rtl::OUString getKeyElementName();


public:

    ScriptBackendDb( css::uno::Reference<css::uno::XComponentContext> const &  xContext,
                        ::rtl::OUString const & url);
};



}
}
}
#endif

