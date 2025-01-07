/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swtiledrenderingtest.hxx>
#include <swtestviewcallback.hxx>

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <swmodule.hxx>
#include <swdll.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/lokcallback.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>

class TestResultListener : public cppu::WeakImplHelper<css::frame::XDispatchResultListener>
{
public:
    sal_uInt32 m_nDocRepair;

    TestResultListener()
        : m_nDocRepair(0)
    {
    }

    virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& rEvent) override
    {
        if (rEvent.State == frame::DispatchResultState::SUCCESS)
        {
            rEvent.Result >>= m_nDocRepair;
        }
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject&) override {}
};

/// Test callback that works with comphelper::LibreOfficeKit::setAnyInputCallback().
class AnyInputCallback final
{
public:
    static bool callback(void* /*pData*/, int /*nPriority*/) { return true; }

    AnyInputCallback()
    {
        comphelper::LibreOfficeKit::setAnyInputCallback(&callback, this,
                                                        []() -> int { return -1; });
    }

    ~AnyInputCallback()
    {
        comphelper::LibreOfficeKit::setAnyInputCallback(nullptr, nullptr,
                                                        []() -> int { return -1; });
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
