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
#ifndef INCLUDED_SVX_LANGBOX_HXX
#define INCLUDED_SVX_LANGBOX_HXX

#include <memory>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <svx/svxdllapi.h>
#include <vcl/weld.hxx>
#include <o3tl/typed_flags_set.hxx>

enum class SvxLanguageListFlags
{
    EMPTY             = 0x0000,
    ALL               = 0x0001,
    WESTERN           = 0x0002,
    CTL               = 0x0004,
    CJK               = 0x0008,
    FBD_CHARS         = 0x0010,
    ONLY_KNOWN        = 0x0020,  // list only locales provided by I18N
    SPELL_USED        = 0x0040,
};
namespace o3tl
{
    template<> struct typed_flags<SvxLanguageListFlags> : is_typed_flags<SvxLanguageListFlags, 0x007f> {};
}

// load language strings from resource
SVXCORE_DLLPUBLIC OUString    GetDicInfoStr( const OUString& rName, const LanguageType nLang, bool bNeg );

class SVXCORE_DLLPUBLIC SvxLanguageBox
{
public:
    enum class EditedAndValid
    {
        No,
        Valid,
        Invalid
    };

private:
    std::unique_ptr<weld::ComboBox> m_xControl;
    Link<weld::ComboBox&, void> m_aChangeHdl;
    OUString m_aAllString;
    std::unique_ptr<css::uno::Sequence<sal_Int16>> m_xSpellUsedLang;
    LanguageType m_eSavedLanguage;
    EditedAndValid  m_eEditedAndValid;
    bool m_bHasLangNone;
    bool m_bLangNoneIsLangAll;
    bool m_bWithCheckmark;

    SVX_DLLPRIVATE weld::ComboBoxEntry BuildEntry(const LanguageType nLangType, sal_Int16 nType = css::i18n::ScriptType::WEAK);
    SVX_DLLPRIVATE void AddLanguages(const std::vector< LanguageType >& rLanguageTypes, SvxLanguageListFlags nLangList,
                                     std::vector<weld::ComboBoxEntry>& rEntries);
    SVX_DLLPRIVATE void InsertLanguage(const LanguageType nLangType, sal_Int16 nType);

    SVX_DLLPRIVATE int ImplTypeToPos(LanguageType eType) const;
    DECL_LINK(ChangeHdl, weld::ComboBox&, void);
public:
    SvxLanguageBox(std::unique_ptr<weld::ComboBox> pControl);
    void            SetLanguageList( SvxLanguageListFlags nLangList,
                            bool bHasLangNone, bool bLangNoneIsLangAll = false,
                            bool bCheckSpellAvail = false, bool bDefaultLangExist = false,
                            LanguageType eDefaultLangType = LANGUAGE_NONE,
                            sal_Int16 nDefaultType = 0 );
    void            InsertLanguage(const LanguageType nLangType);

    EditedAndValid      GetEditedAndValid() const { return m_eEditedAndValid;}
    sal_Int32           SaveEditedAsEntry();

    void connect_changed(const Link<weld::ComboBox&, void>& rLink) { m_aChangeHdl = rLink; }
    void connect_focus_in(const Link<weld::Widget&, void>& rLink) { m_xControl->connect_focus_in(rLink); }
    void grab_focus() { m_xControl->grab_focus(); }
    void save_active_id() { m_eSavedLanguage = get_active_id(); }
    LanguageType get_saved_active_id() const { return m_eSavedLanguage; }
    bool get_active_id_changed_from_saved() const { return m_eSavedLanguage != get_active_id(); }
    void hide() { m_xControl->hide(); }
    void set_visible(bool bShow) { m_xControl->set_visible(bShow); }
    void set_sensitive(bool bSensitive) { m_xControl->set_sensitive(bSensitive); }
    void set_active(int nPos) { m_xControl->set_active(nPos); }
    int get_active() const { return m_xControl->get_active(); }
    void set_active_id(const LanguageType eLangType);
    OUString get_active_text() const { return m_xControl->get_active_text(); }
    bool get_visible() const { return m_xControl->get_visible(); }
    LanguageType get_active_id() const;
    int find_id(const LanguageType eLangType) const;
    LanguageType get_id(int nPos) const;
    void set_id(int nPos, const LanguageType eLangType);
    void remove_id(const LanguageType eLangType);
    void append(const LanguageType eLangType, const OUString& rStr);
    int find_text(const OUString& rStr) const { return m_xControl->find_text(rStr); }
    OUString get_text(int nPos) const { return m_xControl->get_text(nPos); }
    int get_count() const { return m_xControl->get_count(); }
    weld::ComboBox* get_widget() { return m_xControl.get(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
