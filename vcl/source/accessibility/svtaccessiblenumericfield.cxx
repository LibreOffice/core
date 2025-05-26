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

#include <accessibility/svtaccessiblenumericfield.hxx>
#include <comphelper/accessiblecontexthelper.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

SVTXAccessibleNumericField::SVTXAccessibleNumericField(FormattedField* pFormattedField)
    : ImplInheritanceHelper(pFormattedField)
{
}

void SVTXAccessibleNumericField::ProcessWindowEvent(const VclWindowEvent& rVclWindowEvent)
{
    VCLXAccessibleEdit::ProcessWindowEvent(rVclWindowEvent);

    if (rVclWindowEvent.GetId() == VclEventId::EditModify)
    {
        css::uno::Any aNewValue = getCurrentValue();
        NotifyAccessibleEvent(AccessibleEventId::VALUE_CHANGED, css::uno::Any(), aNewValue);
    }
}

sal_Int16 SVTXAccessibleNumericField::getAccessibleRole() { return AccessibleRole::SPIN_BOX; }

css::uno::Any SAL_CALL SVTXAccessibleNumericField::getCurrentValue()
{
    OExternalLockGuard aGuard(this);

    double dValue = 0;
    VclPtr<FormattedField> pField = GetAs<FormattedField>();
    if (pField)
        dValue = pField->GetFormatter().GetValue();

    return css::uno::Any(dValue);
}

sal_Bool SVTXAccessibleNumericField::setCurrentValue(const css::uno::Any& aNumber)
{
    OExternalLockGuard aGuard(this);

    VclPtr<FormattedField> pField = GetAs<FormattedField>();
    if (!pField)
        return false;

    double dValue = 0;
    aNumber >>= dValue;
    pField->GetFormatter().SetValue(dValue);
    return true;
}

css::uno::Any SAL_CALL SVTXAccessibleNumericField::getMaximumValue()
{
    OExternalLockGuard aGuard(this);

    double dValue = 0;
    VclPtr<FormattedField> pField = GetAs<FormattedField>();
    if (pField)
        dValue = pField->GetFormatter().GetMaxValue();

    return css::uno::Any(dValue);
}

css::uno::Any SAL_CALL SVTXAccessibleNumericField::getMinimumValue()
{
    OExternalLockGuard aGuard(this);

    double dValue = 0;
    VclPtr<FormattedField> pField = GetAs<FormattedField>();
    if (pField)
        dValue = pField->GetFormatter().GetMinValue();

    return css::uno::Any(dValue);
}

css::uno::Any SAL_CALL SVTXAccessibleNumericField::getMinimumIncrement()
{
    OExternalLockGuard aGuard(this);

    double dValue = 0;
    VclPtr<FormattedField> pField = GetAs<FormattedField>();
    if (pField)
        dValue = pField->GetFormatter().GetSpinSize();

    return css::uno::Any(dValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
