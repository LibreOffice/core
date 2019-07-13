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

#include <i18nutil/transliteration.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/weld.hxx>

class SfxItemSet;

class SvxJSearchOptionsPage : public SfxTabPage
{
private:
    std::unique_ptr<weld::CheckButton> m_xMatchCase;
    std::unique_ptr<weld::CheckButton> m_xMatchFullHalfWidth;
    std::unique_ptr<weld::CheckButton> m_xMatchHiraganaKatakana;
    std::unique_ptr<weld::CheckButton> m_xMatchContractions;
    std::unique_ptr<weld::CheckButton> m_xMatchMinusDashChoon;
    std::unique_ptr<weld::CheckButton> m_xMatchRepeatCharMarks;
    std::unique_ptr<weld::CheckButton> m_xMatchVariantFormKanji;
    std::unique_ptr<weld::CheckButton> m_xMatchOldKanaForms;
    std::unique_ptr<weld::CheckButton> m_xMatchDiziDuzu;
    std::unique_ptr<weld::CheckButton> m_xMatchBavaHafa;
    std::unique_ptr<weld::CheckButton> m_xMatchTsithichiDhizi;
    std::unique_ptr<weld::CheckButton> m_xMatchHyuiyuByuvyu;
    std::unique_ptr<weld::CheckButton> m_xMatchSesheZeje;
    std::unique_ptr<weld::CheckButton> m_xMatchIaiya;
    std::unique_ptr<weld::CheckButton> m_xMatchKiku;
    std::unique_ptr<weld::CheckButton> m_xMatchProlongedSoundMark;

    std::unique_ptr<weld::CheckButton> m_xIgnorePunctuation;
    std::unique_ptr<weld::CheckButton> m_xIgnoreWhitespace;
    std::unique_ptr<weld::CheckButton> m_xIgnoreMiddleDot;

    TransliterationFlags nTransliterationFlags;
    bool bSaveOptions;

    TransliterationFlags GetTransliterationFlags_Impl();

public:
    SvxJSearchOptionsPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxJSearchOptionsPage() override;
    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rSet);

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;

    void                EnableSaveOptions( bool bVal )  { bSaveOptions = bVal; }

    TransliterationFlags GetTransliterationFlags() const { return nTransliterationFlags; }
    void                SetTransliterationFlags( TransliterationFlags nSettings );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
