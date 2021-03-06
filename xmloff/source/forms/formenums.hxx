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

#pragma once

#include <xmloff/xmlement.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/form/TabulatorCycle.hpp>
#include <tools/gen.hxx>

namespace xmloff
{
extern const SvXMLEnumMapEntry<css::form::FormSubmitEncoding> aSubmitEncodingMap[];
extern const SvXMLEnumMapEntry<css::form::FormSubmitMethod> aSubmitMethodMap[];
extern const SvXMLEnumMapEntry<sal_Int32> aCommandTypeMap[];
extern const SvXMLEnumMapEntry<css::form::NavigationBarMode> aNavigationTypeMap[];
extern const SvXMLEnumMapEntry<css::form::TabulatorCycle> aTabulatorCycleMap[];
extern const SvXMLEnumMapEntry<css::form::FormButtonType> aFormButtonTypeMap[];
extern const SvXMLEnumMapEntry<css::form::ListSourceType> aListSourceTypeMap[];
extern const SvXMLEnumMapEntry<TriState> aCheckStateMap[];
extern const SvXMLEnumMapEntry<sal_Int16> aTextAlignMap[];
extern const SvXMLEnumMapEntry<sal_uInt16> aBorderTypeMap[];
extern const SvXMLEnumMapEntry<sal_uInt16> aFontEmphasisMap[];
extern const SvXMLEnumMapEntry<sal_uInt16> aFontReliefMap[];
extern const SvXMLEnumMapEntry<sal_Int16> aListLinkageMap[];
extern const SvXMLEnumMapEntry<sal_Int32> aOrientationMap[];
extern const SvXMLEnumMapEntry<sal_Int16> aVisualEffectMap[];
extern const SvXMLEnumMapEntry<sal_Int16> aImagePositionMap[];
extern const SvXMLEnumMapEntry<sal_uInt16> aImageAlignMap[];
extern const SvXMLEnumMapEntry<sal_uInt16> aScaleModeMap[];

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
