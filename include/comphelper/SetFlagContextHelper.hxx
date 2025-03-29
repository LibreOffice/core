/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <cppuhelper/implbase.hxx>
#include <uno/current_context.hxx>

namespace comphelper
{
// Returns a new context layer that assigns the given boolean value to the name
inline css::uno::Reference<css::uno::XCurrentContext> NewFlagContext(const OUString& sName,
                                                                     bool bValue = true)
{
    class SetFlagContext : public cppu::WeakImplHelper<css::uno::XCurrentContext>
    {
    public:
        SetFlagContext(const OUString& sName, bool bValue)
            : msName(sName)
            , mbValue(bValue)
        {
        }

        virtual css::uno::Any SAL_CALL getValueByName(const OUString& Name) override
        {
            if (Name == msName)
                return css::uno::Any(mbValue);
            if (mxNext)
                return mxNext->getValueByName(Name);
            return css::uno::Any();
        }

    private:
        OUString msName;
        bool mbValue;
        css::uno::Reference<css::uno::XCurrentContext> mxNext = css::uno::getCurrentContext();
    };
    return new SetFlagContext(sName, bValue);
}

// A specialization for preventing "Java must be enabled" interaction
inline css::uno::Reference<css::uno::XCurrentContext> NoEnableJavaInteractionContext()
{
    return NewFlagContext(u"DontEnableJava"_ustr);
}

inline bool IsContextFlagActive(const OUString& sName)
{
    bool bFlag = false;
    if (const auto xContext = css::uno::getCurrentContext())
        xContext->getValueByName(sName) >>= bFlag;
    return bFlag;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
