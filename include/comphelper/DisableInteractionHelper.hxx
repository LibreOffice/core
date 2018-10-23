/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_DISABLEINTERACTIONHELPER_HXX
#define INCLUDED_COMPHELPER_DISABLEINTERACTIONHELPER_HXX

#include <cppuhelper/implbase.hxx>
#include <uno/current_context.hxx>

namespace comphelper
{
class NoEnableJavaInteractionContext : public cppu::WeakImplHelper<css::uno::XCurrentContext>
{
public:
    explicit NoEnableJavaInteractionContext(
        css::uno::Reference<css::uno::XCurrentContext> const& xContext)
        : mxContext(xContext)
    {
    }
    NoEnableJavaInteractionContext(const NoEnableJavaInteractionContext&) = delete;
    NoEnableJavaInteractionContext& operator=(const NoEnableJavaInteractionContext&) = delete;

private:
    virtual ~NoEnableJavaInteractionContext() override {}

    virtual css::uno::Any SAL_CALL getValueByName(OUString const& Name) override
    {
        if (Name == "DontEnableJava")
            return css::uno::Any(true);
        else if (mxContext.is())
            return mxContext->getValueByName(Name);
        else
            return css::uno::Any();
    }

    css::uno::Reference<css::uno::XCurrentContext> mxContext;
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_DISABLEINTERACTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
