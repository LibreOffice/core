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

#include <standard/vclxaccessiblefixedtext.hxx>

#include <vcl/window.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

// VCLXAccessibleFixedText

void VCLXAccessibleFixedText::FillAccessibleStateSet(sal_Int64& rStateSet)
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet(rStateSet);

    if (GetWindow() && GetWindow()->GetStyle() & WB_WORDBREAK)
        rStateSet |= AccessibleStateType::MULTI_LINE;
}

// XServiceInfo

OUString VCLXAccessibleFixedText::getImplementationName()
{
    return u"com.sun.star.comp.toolkit.AccessibleFixedText"_ustr;
}

Sequence<OUString> VCLXAccessibleFixedText::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.AccessibleFixedText"_ustr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
