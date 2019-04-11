/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <test/testdllapi.hxx>

#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST TextContent
{
public:
    TextContent(const css::text::TextContentAnchorType aExpectedTCAT,
                const css::text::TextContentAnchorType aNewTCAT,
                const css::text::WrapTextMode aExpectedWTM, const css::text::WrapTextMode aNewWTM)
        : m_aExpectedTCAT(aExpectedTCAT)
        , m_aNewTCAT(aNewTCAT)
        , m_aExpectedWTM(aExpectedWTM)
        , m_aNewWTM(aNewWTM)
    {
    }

    virtual css::uno::Reference<css::uno::XInterface> init() = 0;

    void testTextContentProperties();

protected:
    ~TextContent() {}

private:
    css::text::TextContentAnchorType const m_aExpectedTCAT;
    css::text::TextContentAnchorType const m_aNewTCAT;
    css::text::WrapTextMode const m_aExpectedWTM;
    css::text::WrapTextMode const m_aNewWTM;
};

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
