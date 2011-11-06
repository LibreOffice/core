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


#ifndef _SVXSWAFOPT_HXX
#define _SVXSWAFOPT_HXX

#include <vcl/font.hxx>
#include "editeng/editengdllapi.h"

class SvStringsISortDtor;
class SmartTagMgr;

// Klasse fuer Optionen vom Autoformat
struct EDITENG_DLLPUBLIC SvxSwAutoFmtFlags
{
    Font aBulletFont;
    Font aByInputBulletFont;
    const SvStringsISortDtor* pAutoCmpltList;  // only valid inside the Dialog!!!
    SmartTagMgr* pSmartTagMgr;

    sal_Unicode cBullet;
    sal_Unicode cByInputBullet;

    sal_uInt16 nAutoCmpltWordLen, nAutoCmpltListLen;
    sal_uInt16 nAutoCmpltExpandKey;

    sal_uInt8 nRightMargin;

    sal_Bool bAutoCorrect : 1;
    sal_Bool bCptlSttSntnc : 1;
    sal_Bool bCptlSttWrd : 1;
    sal_Bool bChkFontAttr : 1;

    sal_Bool bChgUserColl : 1;
    sal_Bool bChgEnumNum : 1;

    sal_Bool bAFmtByInput : 1;
    sal_Bool bDelEmptyNode : 1;
    sal_Bool bSetNumRule : 1;

    sal_Bool bChgOrdinalNumber : 1;
    sal_Bool bChgToEnEmDash : 1;
    sal_Bool bAddNonBrkSpace : 1;
    sal_Bool bChgWeightUnderl : 1;
    sal_Bool bSetINetAttr : 1;

    sal_Bool bSetBorder : 1;
    sal_Bool bCreateTable : 1;
    sal_Bool bReplaceStyles : 1;
    sal_Bool bDummy : 1;

    sal_Bool bWithRedlining : 1;

    sal_Bool bRightMargin : 1;

    sal_Bool bAutoCompleteWords : 1;
    sal_Bool bAutoCmpltCollectWords : 1;
    sal_Bool bAutoCmpltEndless : 1;
// -- under NT hier starts a new long
    sal_Bool bAutoCmpltAppendBlanc : 1;
    sal_Bool bAutoCmpltShowAsTip : 1;

    sal_Bool bAFmtDelSpacesAtSttEnd : 1;
    sal_Bool bAFmtDelSpacesBetweenLines : 1;
    sal_Bool bAFmtByInpDelSpacesAtSttEnd : 1;
    sal_Bool bAFmtByInpDelSpacesBetweenLines : 1;

    sal_Bool bAutoCmpltKeepList : 1;

    // some dummies for any new options
    sal_Bool bDummy6 : 1,
         bDummy7 : 1,
         bDummy8 : 1
         ;

    SvxSwAutoFmtFlags();
    SvxSwAutoFmtFlags( const SvxSwAutoFmtFlags& rAFFlags ) { *this = rAFFlags; }
    SvxSwAutoFmtFlags& operator=( const SvxSwAutoFmtFlags& );
};

#endif

