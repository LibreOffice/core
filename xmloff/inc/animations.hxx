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

#include <xmloff/xmltoken.hxx>

template <typename EnumT> struct SvXMLEnumMapEntry;

namespace xmloff
{
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_Fill[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_FillDefault[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_Restart[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_RestartDefault[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_Endsync[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_CalcMode[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_AdditiveMode[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_TransformType[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_TransitionType[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_TransitionSubType[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_EventTrigger[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_EffectPresetClass[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_EffectNodeType[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_SubItem[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_IterateType[];
extern const SvXMLEnumMapEntry<sal_Int16> aAnimations_EnumMap_Command[];

struct ImplAttributeNameConversion
{
    token::XMLTokenEnum meXMLToken;
    OUString maAPIName;
};

extern const struct ImplAttributeNameConversion* getAnimationAttributeNamesConversionList();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
