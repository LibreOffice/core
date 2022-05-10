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
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <svtools/svtdllapi.h>

using namespace utl;
using namespace com::sun::star::uno;

struct LanguageToolOptions_Impl;

class SVT_DLLPUBLIC SvxLanguageToolOptions final : public utl::ConfigItem
{
public:
    SvxLanguageToolOptions();
    virtual ~SvxLanguageToolOptions() override;

    virtual void Notify(const css::uno::Sequence<OUString>& _rPropertyNames) override;
    static SvxLanguageToolOptions& Get();

    const OUString& getBaseURL() const;
    void setBaseURL(const OUString& rVal);

    const OUString& getUsername() const;
    void setUsername(const OUString& rVal);

    OUString getLocaleListURL() const;
    OUString getCheckerURL() const;

    const OUString& getApiKey() const;
    void setApiKey(const OUString& rVal);

    bool getEnabled() const;
    void setEnabled(bool enabled);

private:
    std::unique_ptr<LanguageToolOptions_Impl> pImpl;
    void Load(const css::uno::Sequence<OUString>& rPropertyNames);
    virtual void ImplCommit() override;
    static const Sequence<OUString>& GetPropertyNames();
};
