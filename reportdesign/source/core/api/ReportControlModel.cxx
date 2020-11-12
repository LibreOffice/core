/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <ReportControlModel.hxx>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
namespace reportdesign
{
using namespace com::sun::star;
using namespace comphelper;

// XContainer
void OReportControlModel::addContainerListener(
    const uno::Reference<container::XContainerListener>& xListener)
{
    aContainerListeners.addInterface(xListener);
}

void OReportControlModel::removeContainerListener(
    const uno::Reference<container::XContainerListener>& xListener)
{
    aContainerListeners.removeInterface(xListener);
}

bool OReportControlModel::hasElements()
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return !m_aFormatConditions.empty();
}

// XIndexContainer
void OReportControlModel::insertByIndex(::sal_Int32 Index, const uno::Any& Element)
{
    uno::Reference<report::XFormatCondition> xElement(Element, uno::UNO_QUERY);
    if (!xElement.is())
        throw lang::IllegalArgumentException();

    uno::Reference<container::XContainer> xBroadcaster;
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        xBroadcaster = m_pOwner;
        if (Index > static_cast<sal_Int32>(m_aFormatConditions.size()))
            throw lang::IndexOutOfBoundsException();

        m_aFormatConditions.insert(m_aFormatConditions.begin() + Index, xElement);
    }

    // notify our container listeners
    container::ContainerEvent aEvent(xBroadcaster, uno::makeAny(Index), Element, uno::Any());
    aContainerListeners.notifyEach(&container::XContainerListener::elementInserted, aEvent);
}

void OReportControlModel::removeByIndex(::sal_Int32 Index)
{
    uno::Any Element;
    uno::Reference<container::XContainer> xBroadcaster;
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        xBroadcaster = m_pOwner;
        checkIndex(Index);
        Element <<= m_aFormatConditions[Index];
        m_aFormatConditions.erase(m_aFormatConditions.begin() + Index);
    }
    container::ContainerEvent aEvent(xBroadcaster, uno::makeAny(Index), Element, uno::Any());
    aContainerListeners.notifyEach(&container::XContainerListener::elementRemoved, aEvent);
}

// XIndexReplace
void OReportControlModel::replaceByIndex(::sal_Int32 Index, const uno::Any& Element)
{
    uno::Reference<report::XFormatCondition> xElement(Element, uno::UNO_QUERY);
    if (!xElement.is())
        throw lang::IllegalArgumentException();
    uno::Reference<container::XContainer> xBroadcaster;
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        xBroadcaster = m_pOwner;
        checkIndex(Index);
        m_aFormatConditions[Index] = xElement;
    }
    container::ContainerEvent aEvent(xBroadcaster, uno::makeAny(Index), Element, uno::Any());
    aContainerListeners.notifyEach(&container::XContainerListener::elementReplaced, aEvent);
}

// XIndexAccess
::sal_Int32 OReportControlModel::getCount()
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_aFormatConditions.size();
}

uno::Any OReportControlModel::getByIndex(::sal_Int32 Index)
{
    uno::Any aElement;
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        checkIndex(Index);
        aElement <<= m_aFormatConditions[Index];
    }
    return aElement;
}

void OReportControlModel::checkIndex(sal_Int32 _nIndex)
{
    if (_nIndex < 0 || static_cast<sal_Int32>(m_aFormatConditions.size()) <= _nIndex)
        throw lang::IndexOutOfBoundsException();
}

bool OReportControlModel::isInterfaceForbidden(const uno::Type& _rType)
{
    return (_rType == cppu::UnoType<beans::XPropertyState>::get()
            || _rType == cppu::UnoType<beans::XMultiPropertySet>::get());
}

} // reportdesign

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
