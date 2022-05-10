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

#include <sal/log.hxx>
#include <sal/config.h>
#include <svtools/languagetoolcfg.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/debug.hxx>

using namespace utl;
using namespace com::sun::star::uno;

struct LanguageToolOptions_Impl
{
    OUString sBaseURL;
    OUString sUsername;
    OUString sApiKey;
    bool bEnabled;
};

const Sequence<OUString>& SvxLanguageToolOptions::GetPropertyNames()
{
    static Sequence<OUString> const aNames{
        "LanguageTool/BaseURL",
        "LanguageTool/Username",
        "LanguageTool/ApiKey",
        "LanguageTool/IsEnabled",
    };
    return aNames;
}

const OUString& SvxLanguageToolOptions::getBaseURL() const { return pImpl->sBaseURL; }

void SvxLanguageToolOptions::setBaseURL(const OUString& rVal)
{
    pImpl->sBaseURL = rVal;
    SetModified();
}

const OUString& SvxLanguageToolOptions::getUsername() const { return pImpl->sUsername; }

void SvxLanguageToolOptions::setUsername(const OUString& rVal)
{
    pImpl->sUsername = rVal;
    SetModified();
}

OUString SvxLanguageToolOptions::getLocaleListURL() const { return pImpl->sBaseURL + "/languages"; }

OUString SvxLanguageToolOptions::getCheckerURL() const { return pImpl->sBaseURL + "/check"; }

const OUString& SvxLanguageToolOptions::getApiKey() const { return pImpl->sApiKey; }

void SvxLanguageToolOptions::setApiKey(const OUString& rVal)
{
    pImpl->sApiKey = rVal;
    SetModified();
}

bool SvxLanguageToolOptions::getEnabled() const { return pImpl->bEnabled; }

void SvxLanguageToolOptions::setEnabled(bool bEnabled)
{
    pImpl->bEnabled = bEnabled;
    SetModified();
}

namespace
{
class theSvxLanguageToolOptions
    : public rtl::Static<SvxLanguageToolOptions, theSvxLanguageToolOptions>
{
};
}

SvxLanguageToolOptions& SvxLanguageToolOptions::Get() { return theSvxLanguageToolOptions::get(); }

SvxLanguageToolOptions::SvxLanguageToolOptions()
    : ConfigItem("Office.Linguistic/GrammarChecking")
    , pImpl(new LanguageToolOptions_Impl)
{
    Load(GetPropertyNames());
}

SvxLanguageToolOptions::~SvxLanguageToolOptions() {}
void SvxLanguageToolOptions::Notify(const css::uno::Sequence<OUString>&)
{
    Load(GetPropertyNames());
}

void SvxLanguageToolOptions::Load(const css::uno::Sequence<OUString>& aNames)
{
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;
    for (int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if (!pValues[nProp].hasValue())
            continue;
        switch (nProp)
        {
            case 0:
                pValues[nProp] >>= pImpl->sBaseURL;
                break;
            case 1:
                pValues[nProp] >>= pImpl->sUsername;
                break;
            case 2:
                pValues[nProp] >>= pImpl->sApiKey;
                break;
            case 3:
                pValues[nProp] >>= pImpl->bEnabled;
                break;
            default:
                break;
        }
    }
}

void SvxLanguageToolOptions::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    for (int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch (nProp)
        {
            case 0:
                pValues[nProp] <<= pImpl->sBaseURL;
                break;
            case 1:
                pValues[nProp] <<= pImpl->sUsername;
                break;
            case 2:
                pValues[nProp] <<= pImpl->sApiKey;
                break;
            case 3:
                pValues[nProp] <<= pImpl->bEnabled;
                break;
            default:
                break;
        }
    }
    PutProperties(aNames, aValues);
}