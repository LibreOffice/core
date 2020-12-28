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

#include <sal/config.h>

#include <string_view>
#include <utility>
#include <vector>

#include <rtl/ustring.hxx>

#include <dp_backenddb.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace dp_registry::backend::bundle
{
/* The XML file stores the extensions which are currently registered.
   They will be removed when they are revoked.
 */
class ExtensionBackendDb : public dp_registry::backend::BackendDb
{
protected:
    virtual OUString getDbNSName() override;
    virtual OUString getNSPrefix() override;
    virtual OUString getRootElementName() override;
    virtual OUString getKeyElementName() override;

public:
    struct Data
    {
        /* every element consists of a pair of the url to the item (jar,rdb, etc)
           and the media type
         */
        std::vector<std::pair<OUString, OUString>> items;
    };

public:
    ExtensionBackendDb(css::uno::Reference<css::uno::XComponentContext> const& xContext,
                       OUString const& url);

    void addEntry(OUString const& url, Data const& data);

    Data getEntry(std::u16string_view url);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
