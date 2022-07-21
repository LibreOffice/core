/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <test/testdllapi.hxx>

#include <functional>
#include <string>

#include <cppunit/TestAssert.h>

#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

class OOO_DLLPUBLIC_TEST AccessibilityTools
{
public:
    /** Maximum number of children to work on. This is especially useful for
     * Calc which has a million elements, if not more. */
    static const sal_Int32 MAX_CHILDREN = 500;

    static css::uno::Reference<css::accessibility::XAccessibleContext>
    getAccessibleObjectForRole(const css::uno::Reference<css::accessibility::XAccessible>& xacc,
                               sal_Int16 role);

    static bool equals(const css::uno::Reference<css::accessibility::XAccessible>& xacc1,
                       const css::uno::Reference<css::accessibility::XAccessible>& xacc2);
    static bool equals(const css::uno::Reference<css::accessibility::XAccessibleContext>& xctx1,
                       const css::uno::Reference<css::accessibility::XAccessibleContext>& xctx2);

    static OUString getRoleName(const sal_Int16 role);
    static OUString getStateName(const sal_Int16 state);
    static OUString getEventIdName(const sal_Int16 event_id);
    static OUString getRelationTypeName(const sal_Int16 rel_type);

    template <typename T> static std::string debugString(const css::uno::Reference<T>& x)
    {
        return debugString(x.get());
    }

    template <typename T> static std::string debugString(const T& x) { return debugString(&x); }

    template <typename T> static std::string debugString(const T* p)
    {
        /* only the forwarding to debugName() might actually dereference @c p,
         * and we rely on specializations to be as constant as possible and not
         * violate the cast here.  In practice it'll be the case for all types
         * handle if we carefully write the specializations.  In most case the
         * specialization could take a const itself if the methods were
         * properly marked const, but well. */
        return debugString(const_cast<T*>(p));
    }

    template <typename T> static std::string debugString(T* p)
    {
        CPPUNIT_NS::OStringStream ost;

        ost << "(" << static_cast<const void*>(p) << ")";
        if (p != nullptr)
            ost << " " << debugName(p);

        return ost.str();
    }

    static OUString debugAccessibleStateSet(sal_Int64 p);

    /**
     * @brief Process events until a condition or a timeout
     * @param cUntilCallback Callback condition
     * @param nTimeoutMs Maximum time in ms to wait for condition
     * @returns @c true if the condition was met, or @c false if the timeout
     *          has been reached.
     *
     * Processes events until idle, and either until the given condition
     * becomes @c true or a timeout is reached.
     *
     * This is similar to Scheduler::ProcessEventsToIdle() but awaits a
     * condition up to a timeout.  This is useful if the waited-on condition
     * might happen after the first idle time.  The timeout helps in case the
     * condition is not satisfied in reasonable time.
     *
     * @p cUntilCallback is called each time the scheduler reaches idle to check
     * whether the condition is met.
     *
     * Example:
     * @code
     * ProcessEvents([&]() { return taskHasRun; });
     * @endcode
     *
     * @see Scheduler::ProcessEventsToIdle()
     */
    static bool Await(const std::function<bool()>& cUntilCallback, sal_uInt64 nTimeoutMs = 3000);

    /**
     * @brief Process events for a given time
     * @param nTimeoutMs Time to dispatch events for
     *
     * Process events for a given time.  This can be useful if waiting is in
     * order but there is no actual condition to wait on (e.g. expect
     * something *not* to happen).  This similar in spirit to
     * @c sleep(nTimeoutMs), but dispatches events during the wait.
     *
     * This function should be used sparsely because waiting a given time is
     * rarely a good solution for a problem, but in some specific situations
     * there is no better alternative (like, again, waiting for something not
     * to happen).
     */
    static void Wait(sal_uInt64 nTimeoutMs);

private:
    static OUString debugName(css::accessibility::XAccessibleContext* xctx);
    static OUString debugName(css::accessibility::XAccessible* xacc);
    static OUString debugName(const css::accessibility::AccessibleEventObject* evobj);
};

CPPUNIT_NS_BEGIN
/* How to generate those automatically?  We don't want to match all types
 * not to mess up cppunit for types we don't support */
#define AT_ASSERTION_TRAITS(T)                                                                     \
    template <> struct assertion_traits<css::uno::Reference<T>>                                    \
    {                                                                                              \
        static bool equal(const css::uno::Reference<T>& x, const css::uno::Reference<T>& y)        \
        {                                                                                          \
            return AccessibilityTools::equals(x, y);                                               \
        }                                                                                          \
                                                                                                   \
        static std::string toString(const css::uno::Reference<T>& x)                               \
        {                                                                                          \
            return AccessibilityTools::debugString(x);                                             \
        }                                                                                          \
    }

AT_ASSERTION_TRAITS(css::accessibility::XAccessible);
AT_ASSERTION_TRAITS(css::accessibility::XAccessibleContext);

#undef AT_ASSERTION_TRAITS

CPPUNIT_NS_END

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
