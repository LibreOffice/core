/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "volatile.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <algorithm>

using namespace css;
using namespace cpo;
using namespace co::pythoncompute;

PythonComputeVolatileResult::PythonComputeVolatileResult()
{
    // Interim value shown in the formula cell until HTTP completes.
    m_aCurrent <<= u"#BUSY!"_ustr;
}

PythonComputeVolatileResult::~PythonComputeVolatileResult() = default;

void PythonComputeVolatileResult::finish(const cpo::uno::Any& aValue)
{
    // Listener and result state are SolarMutex-only. Callers already hold it; the guard
    // keeps ScAddInListener::modified -> TrackFormulas safe if one ever does not.
    SolarMutexGuard aGuard;
    if (m_bFinished)
    {
        SAL_INFO("scaddins.pythoncompute", "finish: already finished, ignoring");
        return;
    }
    m_bFinished = true;
    m_aCurrent = aValue;

    SAL_INFO("scaddins.pythoncompute",
             "finish: notify under SolarMutexGuard listeners=" << m_aListeners.size());
    notifyListeners(aValue);
}

void PythonComputeVolatileResult::notifyListeners(const cpo::uno::Any& aValue)
{
    const std::vector<uno::Reference<sheet::XResultListener>> aCopy = m_aListeners;
    sheet::ResultEvent aEvent;
    aEvent.Source = getXWeak();
    aEvent.Value = aValue;
    for (const auto& xLis : aCopy)
    {
        if (!xLis.is())
            continue;
        // One throwing listener must not starve the others.
        try
        {
            xLis->modified(aEvent);
        }
        catch (...)
        {
            SAL_WARN("scaddins.pythoncompute", "result listener: exception");
        }
    }
}

void PythonComputeVolatileResult::addResultListener(
    const uno::Reference<sheet::XResultListener>& xListener)
{
    if (!xListener.is())
        return;
    // Same Solar requirement as finish(): ScAddInListener::modified → TrackFormulas.
    SolarMutexGuard aSolar;
    m_aListeners.push_back(xListener);
    const cpo::uno::Any aCurrent = m_aCurrent;
    sheet::ResultEvent aEvent;
    aEvent.Source = getXWeak();
    aEvent.Value = aCurrent;
    // Always push current value (including "#BUSY!") so the cell paints interim text.
    try
    {
        xListener->modified(aEvent);
    }
    catch (const cpo::uno::RuntimeException&)
    {
        // The caller sees the add fail, so do not keep the listener registered.
        if (!m_aListeners.empty() && m_aListeners.back() == xListener)
            m_aListeners.pop_back();
        throw;
    }
    catch (...)
    {
        SAL_WARN("scaddins.pythoncompute", "addResultListener: listener exception");
    }
}

void PythonComputeVolatileResult::removeResultListener(
    const uno::Reference<sheet::XResultListener>& xListener)
{
    SolarMutexGuard aSolar;
    auto& v = m_aListeners;
    v.erase(std::remove_if(
                v.begin(), v.end(),
                [&](const uno::Reference<sheet::XResultListener>& x) { return x == xListener; }),
            v.end());
}

OUString PythonComputeVolatileResult::getImplementationName()
{
    return u"org.collaboraoffice.sheet.addin.PythonComputeVolatileResult"_ustr;
}

bool PythonComputeVolatileResult::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

cpo::uno::Sequence<OUString> PythonComputeVolatileResult::getSupportedServiceNames()
{
    return { u"com.sun.star.sheet.VolatileResult"_ustr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
