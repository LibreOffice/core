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
#ifndef INCLUDED_SW_SOURCE_UI_INC_LANGHELPER_HXX
#define INCLUDED_SW_SOURCE_UI_INC_LANGHELPER_HXX

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

    extern void SetLanguage( SwWrtShell &rWrtSh, const OUString &rLangText, bool bIsForSelection, SfxItemSet &rCoreSet );
//  extern void SetLanguage( SwWrtShell &rWrtSh, EditEngine* pEditEngine, ESelection aSelection, const String &rLangText, bool bIsForSelection, SfxItemSet &rCoreSet );
    extern void SetLanguage( SwWrtShell &rWrtSh, OutlinerView* pOLV, ESelection aSelection, const OUString &rLangText, bool bIsForSelection, SfxItemSet &rCoreSet );
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

    extern OUString GetTextForLanguageGuessing( EditEngine* rEditEngine, ESelection aDocSelection );
    extern OUString GetTextForLanguageGuessing( SwWrtShell &rSh );

    extern LanguageType GetLanguage( SfxItemSet aSet, sal_uInt16 nLangWhichId );
    extern LanguageType GetLanguage( SwWrtShell &rSh, sal_uInt16 nLangWhichId );

    extern LanguageType GetCurrentLanguage( SfxItemSet aSet, sal_uInt16 nScriptType );
    extern LanguageType GetCurrentLanguage( SwWrtShell &rSh );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
