/*************************************************************************
 *
 *  $RCSfile: animations.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:27:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

