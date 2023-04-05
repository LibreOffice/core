/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/uno/Reference.h>

#include "atspi2testbase.hxx"
#include "atspiwrapper.hxx"

class Atspi2TestTree : public Atspi2TestBase
{
protected:
    enum RecurseFlags : sal_uInt16
    {
        NONE = 0,
        PARENT = 1 << 0,
        RELATIONS_TARGETS = 1 << 1,
        ALL = 0xffff
    };

    static sal_Int64
    swChildIndex(css::uno::Reference<css::accessibility::XAccessibleContext> xContext);
    static void
    testSwScroll(const css::uno::Reference<css::accessibility::XAccessibleContext>& xLOContext,
                 const Atspi::Accessible& pAtspiAccessible);
    static void
    compareObjects(const css::uno::Reference<css::accessibility::XAccessible>& xLOAccessible,
                   const Atspi::Accessible& pAtspiAccessible, const sal_uInt16 recurseFlags);
    static void
    compareTrees(const css::uno::Reference<css::accessibility::XAccessible>& xLOAccessible,
                 const Atspi::Accessible& xAtspiAccessible, const int depth = 0);
    static void
    compareTextObjects(const css::uno::Reference<css::accessibility::XAccessibleText>& xLOText,
                       const Atspi::Text& pAtspiText);
};
