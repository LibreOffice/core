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

namespace vcl { class Window; }
class SfxItemSet;



class SvxJSearchOptionsPage : public SfxTabPage
{
private:
    VclPtr<CheckBox>   m_pMatchCase;
    VclPtr<CheckBox>   m_pMatchFullHalfWidth;
    VclPtr<CheckBox>   m_pMatchHiraganaKatakana;
    VclPtr<CheckBox>   m_pMatchContractions;
    VclPtr<CheckBox>   m_pMatchMinusDashChoon;
    VclPtr<CheckBox>   m_pMatchRepeatCharMarks;
    VclPtr<CheckBox>   m_pMatchVariantFormKanji;
    VclPtr<CheckBox>   m_pMatchOldKanaForms;
    VclPtr<CheckBox>   m_pMatchDiziDuzu;
    VclPtr<CheckBox>   m_pMatchBavaHafa;
    VclPtr<CheckBox>   m_pMatchTsithichiDhizi;
    VclPtr<CheckBox>   m_pMatchHyuiyuByuvyu;
    VclPtr<CheckBox>   m_pMatchSesheZeje;
    VclPtr<CheckBox>   m_pMatchIaiya;
    VclPtr<CheckBox>   m_pMatchKiku;
    VclPtr<CheckBox>   m_pMatchProlongedSoundMark;

    VclPtr<CheckBox>   m_pIgnorePunctuation;
    VclPtr<CheckBox>   m_pIgnoreWhitespace;
    VclPtr<CheckBox>   m_pIgnoreMiddleDot;

    sal_Int32       nTransliterationFlags;
    bool        bSaveOptions;

    sal_Int32               GetTransliterationFlags_Impl();

public:
                        SvxJSearchOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxJSearchOptionsPage();
    virtual void dispose() override;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;

    bool                IsSaveOptions() const           { return bSaveOptions; }
    void                EnableSaveOptions( bool bVal )  { bSaveOptions = bVal; }

    sal_Int32               GetTransliterationFlags() const { return nTransliterationFlags; }
    void                SetTransliterationFlags( sal_Int32 nSettings );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
