/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

