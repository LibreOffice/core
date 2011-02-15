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

#if ! defined INCLUDED_DP_HELPBACKENDDB_HXX
#define INCLUDED_DP_HELPBACKENDDB_HXX

#include "rtl/ustring.hxx"
#include <list>
#include "boost/optional.hpp"
#include "dp_backenddb.hxx"

namespace css = ::com::sun::star;

namespace com { namespace sun { namespace star {
        namespace uno {
        class XComponentContext;
        }
}}}

namespace dp_registry {
namespace backend {
namespace help {

/* The XML file stores the extensions which are currently registered.
   They will be removed when they are revoked.
 */
class HelpBackendDb: public dp_registry::backend::BackendDb
{
protected:
    virtual ::rtl::OUString getDbNSName();

    virtual ::rtl::OUString getNSPrefix();

    virtual ::rtl::OUString getRootElementName();

    virtual ::rtl::OUString getKeyElementName();

public:
    struct Data
    {
        /* the URL to the folder containing the compiled help files, etc.
         */
        ::rtl::OUString dataUrl;

    };

public:

    HelpBackendDb( css::uno::Reference<css::uno::XComponentContext> const &  xContext,
                        ::rtl::OUString const & url);

    void addEntry(::rtl::OUString const & url, Data const & data);

    ::boost::optional<Data> getEntry(::rtl::OUString const & url);
    //must also return the data urls for entries with @activ="false". That is,
    //those are currently revoked.
    ::std::list< ::rtl::OUString> getAllDataUrls();

};



}
}
}
#endif

