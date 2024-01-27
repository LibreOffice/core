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

#include <unotools/options.hxx>
#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <memory>

namespace comphelper
{
class ConfigurationChanges;
};
namespace com::sun::star::beans
{
class XPropertySet;
}
namespace com::sun::star::container
{
class XNameAccess;
}

class UNOTOOLS_DLLPUBLIC SvtCompatibility
{
public:
    // Read access
    explicit SvtCompatibility(const OUString& itemName);
    // Write access
    SvtCompatibility(const std::shared_ptr<comphelper::ConfigurationChanges>& batch,
                     const OUString& itemName);
    void set(const OUString& option, bool value);
    bool get(const OUString& option) const;

    bool getPropertyReadOnly(const OUString& option) const;

private:
    css::uno::Reference<css::container::XNameAccess> root;
    css::uno::Reference<css::beans::XPropertySet> item;
};

class SvtCompatibilityDefault : public SvtCompatibility
{
public:
    SvtCompatibilityDefault()
        : SvtCompatibility(u"_default"_ustr)
    {
    }
    SvtCompatibilityDefault(const std::shared_ptr<comphelper::ConfigurationChanges>& batch)
        : SvtCompatibility(batch, u"_default"_ustr)
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
