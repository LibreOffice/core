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

