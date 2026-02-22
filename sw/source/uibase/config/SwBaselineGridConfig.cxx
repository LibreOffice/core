/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <ranges>

#include <SwBaselineGridConfig.hxx>
#include <usrpref.hxx>

#include <o3tl/any.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/uno/Sequence.hxx>

SwBaselineGridConfig::SwBaselineGridConfig(SwMasterUsrPref& rParent)
    : ConfigItem(u"Office.Writer/BaselineGrid"_ustr)
    , m_rParent(rParent)
{
    Load();
    EnableNotification(GetPropertyNames());
}

css::uno::Sequence<OUString> SwBaselineGridConfig::GetPropertyNames()
{
    static css::uno::Sequence<OUString> aNames{
        u"VisibleGrid"_ustr, // 0
    };
    return aNames;
}

void SwBaselineGridConfig::ImplCommit()
{
    const css::uno::Sequence<OUString> aNames = GetPropertyNames();
    css::uno::Sequence<css::uno::Any> aValues(aNames.getLength());
    css::uno::Any* pValues = aValues.getArray();

    for (sal_Int32 nProp : std::views::iota(0, aNames.getLength()))
    {
        switch (nProp)
        {
            case 0: // "VisibleGrid"
                pValues[nProp] <<= m_rParent.IsBaselineGridVisible();
                break;
        }
    }
    PutProperties(aNames, aValues);
}

void SwBaselineGridConfig::Load()
{
    const css::uno::Sequence<OUString> aNames = GetPropertyNames();
    const css::uno::Sequence<css::uno::Any> aValues = GetProperties(aNames);

    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;

    const css::uno::Any* pValues = aValues.getConstArray();
    for (sal_Int32 nProp : std::views::iota(0, aNames.getLength()))
    {
        if (pValues[nProp].hasValue())
        {
            switch (nProp)
            {
                case 0: // "VisibleGrid"
                {
                    const bool bVisible = *o3tl::doAccess<bool>(pValues[nProp]);
                    m_rParent.SetBaselineGridVisible(bVisible);
                    break;
                }
            }
        }
    }
}

void SwBaselineGridConfig::Notify(const css::uno::Sequence<OUString>&) { Load(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
