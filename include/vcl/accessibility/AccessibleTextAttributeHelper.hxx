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

#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/beans/PropertyValue.hdl>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

class VCL_DLLPUBLIC AccessibleTextAttributeHelper
{
public:
    /** Converts UNO text attribute properties to a string holding
     *  the corresponding IAccessible2 text attributes.
     * @param rUnoAttributes A sequence holding the UNO text attributes.
     * @returns String holding the corresponding IAccessible2 text properties.
     */
    static OUString ConvertUnoToIAccessible2TextAttributes(
        const css::uno::Sequence<css::beans::PropertyValue>& rUnoAttributes);

    /**
     * Get the IAccessible2 text attributes and the span of the attributes at the given index.
     * @param xText The interace to query for the information.
     * @param nOffset Character offset for which to retrieve the information.
     * @param rStartOffset Out param that is set to the start index of the attribute run.
     * @param rEndOffset Out param that is set to the end index of the attribute run.
     * @return IAccessible2 text attributes at the given character offset.
     */
    static OUString
    GetIAccessible2TextAttributes(css::uno::Reference<css::accessibility::XAccessibleText> xText,
                                  sal_Int32 nOffset, sal_Int32& rStartOffset,
                                  sal_Int32& rEndOffset);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
