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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTJSEARCH_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTJSEARCH_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <sfx2/tabdlg.hxx>

class Window;
class SfxItemSet;



class SvxJSearchOptionsPage : public SfxTabPage
{
private:
    CheckBox*   m_pMatchCase;
    CheckBox*   m_pMatchFullHalfWidth;
    CheckBox*   m_pMatchHiraganaKatakana;
    CheckBox*   m_pMatchContractions;
    CheckBox*   m_pMatchMinusDashChoon;
    CheckBox*   m_pMatchRepeatCharMarks;
    CheckBox*   m_pMatchVariantFormKanji;
    CheckBox*   m_pMatchOldKanaForms;
    CheckBox*   m_pMatchDiziDuzu;
    CheckBox*   m_pMatchBavaHafa;
    CheckBox*   m_pMatchTsithichiDhizi;
    CheckBox*   m_pMatchHyuiyuByuvyu;
    CheckBox*   m_pMatchSesheZeje;
    CheckBox*   m_pMatchIaiya;
    CheckBox*   m_pMatchKiku;
    CheckBox*   m_pMatchProlongedSoundMark;

    CheckBox*   m_pIgnorePunctuation;
    CheckBox*   m_pIgnoreWhitespace;
    CheckBox*   m_pIgnoreMiddleDot;

    sal_Int32       nTransliterationFlags;
    sal_Bool        bSaveOptions;

    sal_Int32               GetTransliterationFlags_Impl();

protected:
                        SvxJSearchOptionsPage( Window* pParent, const SfxItemSet& rSet );

public:
                        ~SvxJSearchOptionsPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );

    virtual void        Reset( const SfxItemSet& rSet );
    virtual bool        FillItemSet( SfxItemSet& rSet );

    sal_Bool                IsSaveOptions() const           { return bSaveOptions; }
    void                EnableSaveOptions( sal_Bool bVal )  { bSaveOptions = bVal; }

    sal_Int32               GetTransliterationFlags() const { return nTransliterationFlags; }
    void                SetTransliterationFlags( sal_Int32 nSettings );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
