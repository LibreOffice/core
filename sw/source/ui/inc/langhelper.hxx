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
#ifndef _SWLANGHELPER_HXX
#define _SWLANGHELPER_HXX

class SwWrtShell;
class SwView;
class EditEngine;
class EditView;
class OutlinerView;
class SfxItemSet;
struct ESelection;

namespace SwLangHelper
{
    extern sal_uInt16 GetLanguageStatus( OutlinerView* pOLV, SfxItemSet& rSet );
    extern bool SetLanguageStatus( OutlinerView* pOLV, SfxRequest &rReq, SwView &rView, SwWrtShell &rSh );

    extern void SetLanguage( SwWrtShell &rWrtSh, const String &rLangText, bool bIsForSelection, SfxItemSet &rCoreSet );
//  extern void SetLanguage( SwWrtShell &rWrtSh, EditEngine* pEditEngine, ESelection aSelection, const String &rLangText, bool bIsForSelection, SfxItemSet &rCoreSet );
    extern void SetLanguage( SwWrtShell &rWrtSh, OutlinerView* pOLV, ESelection aSelection, const String &rLangText, bool bIsForSelection, SfxItemSet &rCoreSet );
    extern void SetLanguage_None( SwWrtShell &rWrtSh, bool bIsForSelection, SfxItemSet &rCoreSet );
//  extern void SetLanguage_None( SwWrtShell &rWrtSh, EditEngine* pEditEngine, ESelection aSelection, bool bIsForSelection, SfxItemSet &rCoreSet  );
    extern void SetLanguage_None( SwWrtShell &rWrtSh, OutlinerView* pOLV, ESelection aSelection, bool bIsForSelection, SfxItemSet &rCoreSet  );
    extern void ResetLanguages( SwWrtShell &rWrtSh, bool bIsForSelection );
//  extern void ResetLanguages( SwWrtShell &rWrtSh, EditEngine* pEditEngine, ESelection aSelection, bool bIsForSelection );
    extern void ResetLanguages( SwWrtShell &rWrtSh, OutlinerView* pOLV, ESelection aSelection, bool bIsForSelection );

    // document
    extern void SelectCurrentPara( SwWrtShell &rWrtSh );
    // EditView
    extern void SelectPara( EditView &rEditView, const ESelection &rCurSel );

    extern String GetTextForLanguageGuessing( EditEngine* rEditEngine, ESelection aDocSelection );
    extern String GetTextForLanguageGuessing( SwWrtShell &rSh );

    extern LanguageType GetLanguage( SfxItemSet aSet, sal_uInt16 nLangWhichId );
    extern LanguageType GetLanguage( SwWrtShell &rSh, sal_uInt16 nLangWhichId );

    extern LanguageType GetCurrentLanguage( SfxItemSet aSet, sal_uInt16 nScriptType );
    extern LanguageType GetCurrentLanguage( SwWrtShell &rSh );
}

#endif
