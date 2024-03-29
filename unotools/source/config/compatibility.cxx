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

#include <unotools/compatibility.hxx>
#include <tools/debug.hxx>
#include <sal/log.hxx>

#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <officecfg/Office/Compatibility.hxx>

#include <unordered_map>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

SvtCompatibility::SvtCompatibility(const OUString& itemName)
    : root(officecfg::Office::Compatibility::AllFileFormats::get())
    , item(root->getByName(itemName), css::uno::UNO_QUERY_THROW)
{
}

SvtCompatibility::SvtCompatibility(const std::shared_ptr<comphelper::ConfigurationChanges>& batch,
               const OUString& itemName)
    : root(officecfg::Office::Compatibility::AllFileFormats::get(batch))
    , item(root->getByName(itemName), css::uno::UNO_QUERY_THROW)
{
}

void SvtCompatibility::set(const OUString& option, bool value)
{
    item->setPropertyValue(option, css::uno::Any(value));
}

bool SvtCompatibility::get(const OUString& option) const
{
    return item->getPropertyValue(option).get<bool>();
}

bool SvtCompatibility::getPropertyReadOnly(const OUString& option) const
{
    auto info = item->getPropertySetInfo();
    return info->getPropertyByName(option).Attributes & css::beans::PropertyAttribute::READONLY;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
