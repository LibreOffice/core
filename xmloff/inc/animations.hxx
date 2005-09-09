/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animations.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:02:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_ANIMATIONS_HXX
#define _XMLOFF_ANIMATIONS_HXX

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

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

