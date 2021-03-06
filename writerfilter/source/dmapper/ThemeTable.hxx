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

#include "LoggedResources.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <i18nlangtag/lang.h>
#include <memory>

namespace writerfilter::dmapper
{
struct ThemeTable_Impl;

class ThemeTable : public LoggedProperties, public LoggedTable
{
    std::unique_ptr<ThemeTable_Impl> m_pImpl;

public:
    ThemeTable();
    virtual ~ThemeTable() override;

    OUString getFontNameForTheme(const Id id) const;
    static OUString getStringForTheme(const Id id);
    void setThemeFontLangProperties(const css::uno::Sequence<css::beans::PropertyValue>& aPropSeq);

private:
    // Properties
    virtual void lcl_attribute(Id Name, Value& val) override;
    virtual void lcl_sprm(Sprm& sprm) override;

    // Table
    virtual void lcl_entry(writerfilter::Reference<Properties>::Pointer_t ref) override;

    // Helper methods
    static OUString fromLocaleToScriptTag(const OUString& sLocale);
    static OUString fromLCIDToScriptTag(LanguageType lang);
};
typedef tools::SvRef<ThemeTable> ThemeTablePtr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
