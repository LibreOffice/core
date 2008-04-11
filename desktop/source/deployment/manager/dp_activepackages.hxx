/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_activepackages.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_ACTIVEPACKAGES_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_ACTIVEPACKAGES_HXX

#include "sal/config.h"

#include <utility>
#include <vector>

#include "dp_persmap.h"

namespace rtl { class OUString; }

namespace dp_manager {

class ActivePackages {
public:
    struct Data {
        ::rtl::OUString temporaryName;
        ::rtl::OUString fileName;
        ::rtl::OUString mediaType;
    };

    typedef ::std::vector< ::std::pair< ::rtl::OUString, Data > > Entries;

    ActivePackages();

    ActivePackages(::rtl::OUString const & url, bool readOnly);

    ~ActivePackages();

    bool has(::rtl::OUString const & id, ::rtl::OUString const & fileName)
        const;

    bool get(
        Data * data, ::rtl::OUString const & id,
        ::rtl::OUString const & fileName) const;

    Entries getEntries() const;

    void put(::rtl::OUString const & id, Data const & value);

    void erase(::rtl::OUString const & id, ::rtl::OUString const & fileName);

private:
    ActivePackages(ActivePackages &); // not defined
    void operator =(ActivePackages &); // not defined

    ::dp_misc::PersistentMap m_map;
};

}

#endif
