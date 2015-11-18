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

#ifndef INCLUDED_VCL_UNX_GTK_A11Y_ATKTEXTATTRIBUTES_HXX
#define INCLUDED_VCL_UNX_GTK_A11Y_ATKTEXTATTRIBUTES_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>

#include <atk/atk.h>

AtkAttributeSet*
attribute_set_new_from_property_values(
    const css::uno::Sequence< css::beans::PropertyValue >& rAttributeList,
    bool run_attributes_only,
    AtkText *text);

AtkAttributeSet*
attribute_set_new_from_extended_attributes(
    const css::uno::Reference< css::accessibility::XAccessibleExtendedAttributes >& rExtendedAttributes );

bool
attribute_set_map_to_property_values(
    AtkAttributeSet* attribute_set,
    css::uno::Sequence< css::beans::PropertyValue >& rValueList );

AtkAttributeSet* attribute_set_prepend_misspelled( AtkAttributeSet* attribute_set );
// #i92232#
AtkAttributeSet* attribute_set_prepend_tracked_change_insertion( AtkAttributeSet* attribute_set );
AtkAttributeSet* attribute_set_prepend_tracked_change_deletion( AtkAttributeSet* attribute_set );
AtkAttributeSet* attribute_set_prepend_tracked_change_formatchange( AtkAttributeSet* attribute_set );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
