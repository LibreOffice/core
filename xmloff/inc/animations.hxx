/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_ANIMATIONS_HXX
#define _XMLOFF_ANIMATIONS_HXX

#include <xmloff/xmltoken.hxx>

struct SvXMLEnumMapEntry;

namespace xmloff
{
const sal_uInt16 Animations_EnumMap_Fill    = 0;
const sal_uInt16 Animations_EnumMap_FillDefault = 1;
const sal_uInt16 Animations_EnumMap_Restart = 2;
const sal_uInt16 Animations_EnumMap_RestartDefault = 3;
const sal_uInt16 Animations_EnumMap_Endsync = 4;
const sal_uInt16 Animations_EnumMap_CalcMode = 5;
const sal_uInt16 Animations_EnumMap_AdditiveMode = 6;
const sal_uInt16 Animations_EnumMap_TransformType = 7;
const sal_uInt16 Animations_EnumMap_TransitionType = 8;
const sal_uInt16 Animations_EnumMap_TransitionSubType = 9;
const sal_uInt16 Animations_EnumMap_EventTrigger = 10;
const sal_uInt16 Animations_EnumMap_EffectPresetClass = 11;
const sal_uInt16 Animations_EnumMap_EffectNodeType = 12;
const sal_uInt16 Animations_EnumMap_SubItem = 13;
const sal_uInt16 Animations_EnumMap_IterateType = 14;
const sal_uInt16 Animations_EnumMap_Command = 15;

extern SvXMLEnumMapEntry* getAnimationsEnumMap( sal_uInt16 nMap );

struct ImplAttributeNameConversion
{
    token::XMLTokenEnum meXMLToken;
    const sal_Char* mpAPIName;
};

extern struct ImplAttributeNameConversion* getAnimationAttributeNamesConversionList();

}

#endif  //  _XMLOFF_ANIMATIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
