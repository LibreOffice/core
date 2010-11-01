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
        Data(): failedPrerequisites(::rtl::OUString::valueOf((sal_Int32)0))
            {}
        /* name of the temporary file (shared, user extension) or the name of
           the folder of the bundled extension.
           It does not contain the trailing '_' of the folder.
           UTF-8 encoded
        */
        ::rtl::OUString temporaryName;
        /* The file name (shared, user) or the folder name (bundled)
           If the key is the file name, then file name is not encoded.
           If the key is the idendifier then the file name is UTF-8 encoded.
         */
        ::rtl::OUString fileName;
        ::rtl::OUString mediaType;
        ::rtl::OUString version;
        /* If this string contains the value according to
           com::sun::star::deployment::Prerequisites or "0". That is, if
           the value is > 0 then
           the call to XPackage::checkPrerequisites failed.
           In this case the extension must not be registered.
         */
        ::rtl::OUString failedPrerequisites;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
