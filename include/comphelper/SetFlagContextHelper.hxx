/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_SETFLAGCONTEXTHELPER_HXX
#define INCLUDED_COMPHELPER_SETFLAGCONTEXTHELPER_HXX

#include <com/sun/star/uno/XCurrentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <uno/current_context.hxx>
#include <utility>

namespace comphelper
{
// Used to flag some named value to be true for all code running in this context
class SetFlagContext final : public cppu::WeakImplHelper<css::uno::XCurrentContext>
{
public:
    explicit SetFlagContext(OUString sName, css::uno::Reference<css::uno::XCurrentContext> xContext)
        : m_sName(std::move(sName))
        , mxNextContext(std::move(xContext))
    {
    }
    SetFlagContext(const SetFlagContext&) = delete;
    SetFlagContext& operator=(const SetFlagContext&) = delete;

    virtual css::uno::Any SAL_CALL getValueByName(OUString const& Name) override
    {
        if (Name == m_sName)
            return css::uno::Any(true);
        else if (mxNextContext.is())
            return mxNextContext->getValueByName(Name);
        else
            return css::uno::Any();
    }

private:
    OUString m_sName;
    css::uno::Reference<css::uno::XCurrentContext> mxNextContext;
};

// Returns a new context that reports the named value to be true
inline css::uno::Reference<css::uno::XCurrentContext> NewFlagContext(const OUString& sName)
{
    return new SetFlagContext(sName, css::uno::getCurrentContext());
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

#endif // INCLUDED_COMPHELPER_SETFLAGCONTEXTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
