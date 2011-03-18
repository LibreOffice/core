/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
