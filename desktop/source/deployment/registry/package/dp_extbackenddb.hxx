/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once
#if 1

#include <utility>
#include <vector>

#include "rtl/ustring.hxx"

#include "dp_backenddb.hxx"

namespace com { namespace sun { namespace star {
        namespace uno {
        class XComponentContext;
        }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
