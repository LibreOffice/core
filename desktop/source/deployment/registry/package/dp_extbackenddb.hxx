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

#if ! defined INCLUDED_DP_EXTBACKENDDB_HXX
#define INCLUDED_DP_EXTBACKENDDB_HXX

#include <utility>
#include <vector>

#include "rtl/ustring.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
