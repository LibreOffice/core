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

#include <standard/svtaccessiblenumericfield.hxx>
#include <toolkit/awt/vclxwindows.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

SVTXAccessibleNumericField::SVTXAccessibleNumericField(VCLXWindow* pVCLWindow)
    : VCLXAccessibleEdit(pVCLWindow)
{
}

// XInterface
IMPLEMENT_FORWARD_XINTERFACE2(SVTXAccessibleNumericField, VCLXAccessibleTextComponent,
                              SVTXAccessibleNumericField_BASE)

// XTypeProvider
IMPLEMENT_FORWARD_XTYPEPROVIDER2(SVTXAccessibleNumericField, VCLXAccessibleTextComponent,
                                 SVTXAccessibleNumericField_BASE)

sal_Int16 SVTXAccessibleNumericField::getAccessibleRole() { return AccessibleRole::SPIN_BOX; }

css::uno::Any SAL_CALL SVTXAccessibleNumericField::getCurrentValue()
{
    OExternalLockGuard aGuard(this);

    double dValue = 0;
    SVTXNumericField* pField = static_cast<SVTXNumericField*>(GetVCLXWindow());
    if (pField)
        dValue = pField->getValue();

    return css::uno::Any(dValue);
}

sal_Bool SVTXAccessibleNumericField::setCurrentValue(const css::uno::Any& aNumber)
{
    OExternalLockGuard aGuard(this);

    SVTXNumericField* pField = static_cast<SVTXNumericField*>(GetVCLXWindow());
    if (!pField)
        return false;

    double dValue = 0;
    aNumber >>= dValue;
    pField->setValue(dValue);
    return true;
}

css::uno::Any SAL_CALL SVTXAccessibleNumericField::getMaximumValue()
{
    OExternalLockGuard aGuard(this);

    double dValue = 0;
    SVTXNumericField* pField = static_cast<SVTXNumericField*>(GetVCLXWindow());
    if (pField)
        dValue = pField->getMax();

    return css::uno::Any(dValue);
}

css::uno::Any SAL_CALL SVTXAccessibleNumericField::getMinimumValue()
{
    OExternalLockGuard aGuard(this);

    double dValue = 0;
    SVTXNumericField* pField = static_cast<SVTXNumericField*>(GetVCLXWindow());
    if (pField)
        dValue = pField->getMin();

    return css::uno::Any(dValue);
}

css::uno::Any SAL_CALL SVTXAccessibleNumericField::getMinimumIncrement()
{
    OExternalLockGuard aGuard(this);

    double dValue = 0;
    SVTXNumericField* pField = static_cast<SVTXNumericField*>(GetVCLXWindow());
    if (pField)
        dValue = pField->getSpinSize();

    return css::uno::Any(dValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
