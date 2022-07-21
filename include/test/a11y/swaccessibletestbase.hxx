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

#include <deque>

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

#include "accessibletestbase.hxx"

namespace test
{
class OOO_DLLPUBLIC_TEST SwAccessibleTestBase : public AccessibleTestBase
{
private:
    void collectText(const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext,
                     rtl::OUStringBuffer& buffer, bool onlyChildren = false);

protected:
    static css::uno::Reference<css::accessibility::XAccessibleContext> getPreviousFlowingSibling(
        const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext);
    static css::uno::Reference<css::accessibility::XAccessibleContext> getNextFlowingSibling(
        const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext);

    /**
     * This fetches regular children plus siblings linked with FLOWS_TO/FLOWS_FROM which are not
     * already in the regular children set.  This is required because most offscreen children of the
     * document contents are not listed as part of their parent children, but as FLOWS_* reference
     * from one to the next.
     * There is currently no guarantee all children will be listed, and it is fairly likely
     * offscreen frames and tables might be missing for example.
     */
    virtual std::deque<css::uno::Reference<css::accessibility::XAccessibleContext>>
    getAllChildren(const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext);

    /** Collects contents of @p xContext in a dummy markup form */
    OUString
    collectText(const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext);

    /** Collects contents of the current document */
    OUString collectText() { return collectText(getDocumentAccessibleContext()); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
