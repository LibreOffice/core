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

#include "sal/config.h"

#include <utility>
#include <vector>

#if !defined(ANDROID) && !defined(IOS)
#include "dp_persmap.h"
#endif

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
#if !defined(ANDROID) && !defined(IOS)
    ::dp_misc::PersistentMap m_map;
#endif
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
