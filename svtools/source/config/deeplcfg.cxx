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
#include <svtools/deeplcfg.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/debug.hxx>

using namespace utl;
using namespace com::sun::star::uno;

struct DeeplOptions_Impl
{
    OUString sAPIUrl;
    OUString sAuthKey;
};

const Sequence<OUString>& SvxDeeplOptions::GetPropertyNames()
{
    static Sequence<OUString> const aNames{
        "Deepl/ApiURL",
        "Deepl/AuthKey",
    };
    return aNames;
}

const OUString SvxDeeplOptions::getAPIUrl() const { return pImpl->sAPIUrl; }

void SvxDeeplOptions::setAPIUrl(const OUString& rVal)
{
    pImpl->sAPIUrl = rVal;
    SetModified();
}

const OUString& SvxDeeplOptions::getAuthKey() const { return pImpl->sAuthKey; }

void SvxDeeplOptions::setAuthKey(const OUString& rVal)
{
    pImpl->sAuthKey = rVal;
    SetModified();
}

namespace
{
class theSvxDeeplOptions
    : public rtl::Static<SvxDeeplOptions, theSvxDeeplOptions>
{
};
}

SvxDeeplOptions& SvxDeeplOptions::Get() { return theSvxDeeplOptions::get(); }

SvxDeeplOptions::SvxDeeplOptions()
    : ConfigItem("Office.Linguistic/Translation")
    , pImpl(new DeeplOptions_Impl)
{
    Load(GetPropertyNames());
}

SvxDeeplOptions::~SvxDeeplOptions() {}
void SvxDeeplOptions::Notify(const css::uno::Sequence<OUString>&)
{
    Load(GetPropertyNames());
}

void SvxDeeplOptions::Load(const css::uno::Sequence<OUString>& aNames)
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
                pValues[nProp] >>= pImpl->sAPIUrl;
                break;
            case 1:
                pValues[nProp] >>= pImpl->sAuthKey;
                break;
            default:
                break;
        }
    }
}

void SvxDeeplOptions::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    for (int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch (nProp)
        {
            case 0:
                pValues[nProp] <<= pImpl->sAPIUrl;
                break;
            case 1:
                pValues[nProp] <<= pImpl->sAuthKey;
                break;
            default:
                break;
        }
    }
    PutProperties(aNames, aValues);
}