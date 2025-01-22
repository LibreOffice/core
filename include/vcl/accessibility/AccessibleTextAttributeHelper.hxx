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
#include <o3tl/typed_flags_set.hxx>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

/**
 * According to the IAccessible2 specification, some of the attributes that LibreOffice
 * handles as text attributes are mapped to IAccessible2 text attributes as well,
 * but others should be reported as object attributes (e.g. text alignment is reported
 * via the "text-align" object attribute on the paragraph object).
 *
 * https://github.com/LinuxA11y/IAccessible2/blob/master/spec/textattributes.md
 * https://github.com/LinuxA11y/IAccessible2/blob/master/spec/objectattributes.md
 *
 * This enum class is used to specify the type(s) of attributes of interest.
 */
enum class IA2AttributeType
{
    None = 0x0000,
    ObjectAttributes = 0x0001,
    TextAttributes = 0x0002
};

template <> struct o3tl::typed_flags<IA2AttributeType> : is_typed_flags<IA2AttributeType, 0x003>
{
};

namespace AccessibleTextAttributeHelper
{
/**
     * Get the IAccessible2 text attributes and the span of the attributes at the given index.
     * @param xText The interface to query for the information.
     * @param eAttributeType: The type(s) of attributes of interest.
     * @param nOffset Character offset for which to retrieve the information.
     * @param rStartOffset Out param that is set to the start index of the attribute run.
     * @param rEndOffset Out param that is set to the end index of the attribute run.
     * @return IAccessible2 text attributes at the given character offset.
     */
OUString VCL_DLLPUBLIC
GetIAccessible2TextAttributes(const css::uno::Reference<css::accessibility::XAccessibleText>& xText,
                              IA2AttributeType eAttributeType, sal_Int32 nOffset,
                              sal_Int32& rStartOffset, sal_Int32& rEndOffset);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
