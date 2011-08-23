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
#ifndef _SVXSWAFOPT_HXX
#define _SVXSWAFOPT_HXX

#include <vcl/font.hxx>
class SvStringsISortDtor;
namespace binfilter {


// Klasse fuer Optionen vom Autoformat
struct SvxSwAutoFmtFlags
{
    Font aBulletFont;
    Font aByInputBulletFont;
    const SvStringsISortDtor* pAutoCmpltList;  // only valid inside the Dialog!!!

    sal_Unicode cBullet;
    sal_Unicode cByInputBullet;

    USHORT nAutoCmpltWordLen, nAutoCmpltListLen;
    USHORT nAutoCmpltExpandKey;

    BYTE nRightMargin;

    BOOL bReplaceQuote : 1;
    BOOL bAutoCorrect : 1;
    BOOL bCptlSttSntnc : 1;
    BOOL bCptlSttWrd : 1;
    BOOL bChkFontAttr : 1;

    BOOL bChgUserColl : 1;
    BOOL bChgEnumNum : 1;

    BOOL bAFmtByInput : 1;
    BOOL bDelEmptyNode : 1;
    BOOL bSetNumRule : 1;

    BOOL bChgFracionSymbol : 1;
    BOOL bChgOrdinalNumber : 1;
    BOOL bChgToEnEmDash : 1;
    BOOL bChgWeightUnderl : 1;
    BOOL bSetINetAttr : 1;

    BOOL bSetBorder : 1;
    BOOL bCreateTable : 1;
    BOOL bReplaceStyles : 1;
    BOOL bDummy : 1;

    BOOL bWithRedlining : 1;

    BOOL bRightMargin : 1;

    BOOL bAutoCompleteWords : 1;
    BOOL bAutoCmpltCollectWords : 1;
    BOOL bAutoCmpltEndless : 1;
// -- under NT hier starts a new long
    BOOL bAutoCmpltAppendBlanc : 1;
    BOOL bAutoCmpltShowAsTip : 1;

    BOOL bAFmtDelSpacesAtSttEnd : 1;
    BOOL bAFmtDelSpacesBetweenLines : 1;
    BOOL bAFmtByInpDelSpacesAtSttEnd : 1;
    BOOL bAFmtByInpDelSpacesBetweenLines : 1;

    BOOL bAutoCmpltKeepList : 1;

    // some dummies for any new options
    BOOL bDummy5 : 1,
         bDummy6 : 1,
         bDummy7 : 1,
         bDummy8 : 1
         ;

    SvxSwAutoFmtFlags( const SvxSwAutoFmtFlags& rAFFlags ) { *this = rAFFlags; }
    SvxSwAutoFmtFlags& operator=( const SvxSwAutoFmtFlags& );
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
