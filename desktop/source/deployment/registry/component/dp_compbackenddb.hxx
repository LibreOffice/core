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



#if ! defined INCLUDED_DP_COMPBACKENDDB_HXX
#define INCLUDED_DP_COMPBACKENDDB_HXX

#include "rtl/ustring.hxx"
#include "rtl/string.hxx"
#include <vector>
#include <list>
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
namespace component {

/* The XML file stores the extensions which are currently registered.
   They will be removed when they are revoked.
   The format looks like this:

<?xml version="1.0"?>
<component-backend-db xmlns="http://openoffice.org/extensionmanager/component-registry/2010">
  <component url="vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE/uno_packages/5CD5.tmp_/leaves1.oxt/extensionoptions.jar">
    <name>FileName</name>
    <java-type-library>true</java-type-library>
    <implementation-names>
      <name>com.sun.star.comp.extensionoptions.OptionsEventHandler$_OptionsEventHandler</name>
      ...
    </implementation-names>
    <singletons>
      <item>
        <key>com.sun.star.java.theJavaVirtualMachine</key>
        <value>com.sun.star.java.JavaVirtualMachine</value>
      </item>
      ...
    </singletons>
  </component>

  <component ...>
  ...
</component-backend-db>
 */
class ComponentBackendDb: public dp_registry::backend::BackendDb
{
protected:
    virtual ::rtl::OUString getDbNSName();
    virtual ::rtl::OUString getNSPrefix();
    virtual ::rtl::OUString getRootElementName();
    virtual ::rtl::OUString getKeyElementName();

public:
    struct Data
    {
        Data(): javaTypeLibrary(false) {};

        ::std::list< ::rtl::OUString> implementationNames;
        /* every singleton has a key and a value
         */
        ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString> >singletons;
        bool javaTypeLibrary;
    };

public:

    ComponentBackendDb( css::uno::Reference<css::uno::XComponentContext> const &  xContext,
                        ::rtl::OUString const & url);

    void addEntry(::rtl::OUString const & url, Data const & data);

    Data getEntry(::rtl::OUString const & url);


};



}
}
}
#endif

