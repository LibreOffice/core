/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vclpluginapi.h>

#include <QtGui/QAccessibleValueInterface>

#include <QtCore/QVariant>

#include <com/sun/star/accessibility/XAccessible.hpp>

class Qt5Frame;
class Qt5Widget;

class VCLPLUG_QT5_PUBLIC Qt5AccessibleValue : public QAccessibleValueInterface
{
public:
    Qt5AccessibleValue(const css::uno::Reference<css::accessibility::XAccessible> xAccessible);

    QVariant currentValue() const override;
    QVariant maximumValue() const override;
    QVariant minimumStepSize() const override;
    QVariant minimumValue() const override;
    void setCurrentValue(const QVariant& value) override;

private:
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
