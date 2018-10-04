/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Qt5AccessibleValue.hxx>

#include <QtGui/QAccessibleInterface>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

using namespace css;
using namespace css::accessibility;
using namespace css::uno;

Qt5AccessibleValue::Qt5AccessibleValue(const Reference<XAccessible> xAccessible)
    : m_xAccessible(xAccessible)
{
}

QVariant Qt5AccessibleValue::currentValue() const
{
    Reference<XAccessibleValue> xValue(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getCurrentValue() >>= aDouble;
    return QVariant(aDouble);
}
QVariant Qt5AccessibleValue::maximumValue() const
{
    Reference<XAccessibleValue> xValue(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getMaximumValue() >>= aDouble;
    return QVariant(aDouble);
}
QVariant Qt5AccessibleValue::minimumStepSize() const { return QVariant(); }
QVariant Qt5AccessibleValue::minimumValue() const
{
    Reference<XAccessibleValue> xValue(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getMinimumValue() >>= aDouble;
    return QVariant(aDouble);
}
void Qt5AccessibleValue::setCurrentValue(const QVariant& value)
{
    Reference<XAccessibleValue> xValue(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xValue.is())
        return;
    xValue->setCurrentValue(Any(value.toDouble()));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
