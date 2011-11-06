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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "rtl/string.h"
#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XDocument.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "dp_misc.h"
#include "dp_scriptbackenddb.hxx"


namespace css = ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/script-registry/2010"
#define NS_PREFIX "script"
#define ROOT_ELEMENT_NAME "script-backend-db"
#define KEY_ELEMENT_NAME "script"

namespace dp_registry {
namespace backend {
namespace script {

ScriptBackendDb::ScriptBackendDb(
    Reference<XComponentContext> const &  xContext,
    ::rtl::OUString const & url):RegisteredDb(xContext, url)
{

}

OUString ScriptBackendDb::getDbNSName()
{
    return OUSTR(EXTENSION_REG_NS);
}

OUString ScriptBackendDb::getNSPrefix()
{
    return OUSTR(NS_PREFIX);
}

OUString ScriptBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

OUString ScriptBackendDb::getKeyElementName()
{
    return OUSTR(KEY_ELEMENT_NAME);
}



} // namespace executable
} // namespace backend
} // namespace dp_registry

