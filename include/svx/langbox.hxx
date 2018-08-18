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
#include <svx/svxdllapi.h>
#include <vcl/image.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
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
    SPELL_AVAIL       = 0x0020,
    HYPH_AVAIL        = 0x0040,
    THES_AVAIL        = 0x0080,
    ONLY_KNOWN        = 0x0100,  // list only locales provided by I18N
    SPELL_USED        = 0x0200,
    HYPH_USED         = 0x0400,
    THES_USED         = 0x0800,
    ALSO_PRIMARY_ONLY = 0x1000,  // Do not exclude primary-only
                                 // languages that do not form a
                                 // locale, such as Arabic as
                                 // opposed to Arabic-Egypt.
};
namespace o3tl
{
    template<> struct typed_flags<SvxLanguageListFlags> : is_typed_flags<SvxLanguageListFlags, 0x1fff> {};
}

// load language strings from resource
SVX_DLLPUBLIC OUString    GetDicInfoStr( const OUString& rName, const LanguageType nLang, bool bNeg );

class SVX_DLLPUBLIC SvxLanguageBoxBase
{
private:
    SvxLanguageBoxBase(const SvxLanguageBoxBase&) = delete;
    SvxLanguageBoxBase& operator=(const SvxLanguageBoxBase&) = delete;
public:
    explicit SvxLanguageBoxBase();
    virtual ~SvxLanguageBoxBase();

    void            SetLanguageList( SvxLanguageListFlags nLangList,
                            bool bHasLangNone, bool bLangNoneIsLangAll = false,
                            bool bCheckSpellAvail = false );

    void            AddLanguages( const std::vector< LanguageType >& rLanguageTypes, SvxLanguageListFlags nLangList );

    sal_Int32       InsertLanguage( const LanguageType eLangType );
    void            InsertDefaultLanguage( sal_Int16 nType );
    void            InsertSystemLanguage();
    void            InsertLanguage( const LanguageType eLangType,
                                    bool bCheckEntry );
    void            RemoveLanguage( const LanguageType eLangType );
    void            SelectLanguage( const LanguageType eLangType );
    LanguageType    GetSelectedLanguage() const;
    bool            IsLanguageSelected( const LanguageType eLangType ) const;

    void                SetNoSelectionLBB();
    void                HideLBB();
    void                DisableLBB();
    void                SaveValueLBB();
    sal_Int32           GetSelectedEntryPosLBB() const;
    void*               GetEntryDataLBB( sal_Int32  nPos ) const;
    sal_Int32           GetSavedValueLBB() const;

protected:
    Image                   m_aNotCheckedImage;
    Image                   m_aCheckedImage;
    OUString                m_aAllString;
    std::unique_ptr<css::uno::Sequence< sal_Int16 >>
                            m_pSpellUsedLang;
    bool                    m_bHasLangNone;
    bool                    m_bLangNoneIsLangAll;
    bool                    m_bWithCheckmark;

    SVX_DLLPRIVATE void                 ImplLanguageBoxBaseInit();
    SVX_DLLPRIVATE sal_Int32            ImplInsertLanguage(LanguageType, sal_Int32 nPos, sal_Int16 nType);
    SVX_DLLPRIVATE sal_Int32            ImplTypeToPos( LanguageType eType ) const;

    SVX_DLLPRIVATE virtual sal_Int32    ImplInsertImgEntry( const OUString& rEntry, sal_Int32  nPos, bool bChecked ) = 0;
    SVX_DLLPRIVATE virtual void         ImplRemoveEntryAt( sal_Int32 nPos ) = 0;

    SVX_DLLPRIVATE virtual void         ImplClear() = 0;
    SVX_DLLPRIVATE virtual sal_Int32    ImplInsertEntry( const OUString& rEntry, sal_Int32 nPos ) = 0;
    SVX_DLLPRIVATE virtual void         ImplSetEntryData( sal_Int32 nPos, void* pData ) = 0;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetSelectedEntryPos() const = 0;
    SVX_DLLPRIVATE virtual void*        ImplGetEntryData( sal_Int32 nPos ) const = 0;
    SVX_DLLPRIVATE virtual void         ImplSelectEntryPos( sal_Int32 nPos, bool bSelect ) = 0;
    SVX_DLLPRIVATE virtual bool         ImplIsEntryPosSelected( sal_Int32 nPos ) const = 0;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetEntryPos( const void* pData ) const = 0;
    SVX_DLLPRIVATE virtual void         ImplSetNoSelection() = 0;
    SVX_DLLPRIVATE virtual void         ImplHide() = 0;
    SVX_DLLPRIVATE virtual void         ImplDisable() = 0;
    SVX_DLLPRIVATE virtual void         ImplSaveValue() = 0;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetSavedValue() const = 0;
};


class SVX_DLLPUBLIC SvxLanguageBox : public ListBox, public SvxLanguageBoxBase
{
public:
    SvxLanguageBox( vcl::Window* pParent, WinBits nBits );

private:
    SVX_DLLPRIVATE virtual sal_Int32    ImplInsertImgEntry( const OUString& rEntry, sal_Int32  nPos, bool bChecked ) override;
    SVX_DLLPRIVATE virtual void         ImplRemoveEntryAt( sal_Int32 nPos ) override;

    SVX_DLLPRIVATE virtual void         ImplClear() override;
    SVX_DLLPRIVATE virtual sal_Int32    ImplInsertEntry( const OUString& rEntry, sal_Int32 nPos ) override;
    SVX_DLLPRIVATE virtual void         ImplSetEntryData( sal_Int32 nPos, void* pData ) override;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetSelectedEntryPos() const override;
    SVX_DLLPRIVATE virtual void*        ImplGetEntryData( sal_Int32 nPos ) const override;
    SVX_DLLPRIVATE virtual void         ImplSelectEntryPos( sal_Int32 nPos, bool bSelect ) override;
    SVX_DLLPRIVATE virtual bool         ImplIsEntryPosSelected( sal_Int32 nPos ) const override;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetEntryPos( const void* pData ) const override;
    SVX_DLLPRIVATE virtual void         ImplSetNoSelection() override;
    SVX_DLLPRIVATE virtual void         ImplHide() override;
    SVX_DLLPRIVATE virtual void         ImplDisable() override;
    SVX_DLLPRIVATE virtual void         ImplSaveValue() override;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetSavedValue() const override;
};

class SVX_DLLPUBLIC LanguageBox
{
private:
    std::unique_ptr<weld::ComboBoxText> m_xControl;
    Link<weld::ComboBoxText&, void> m_aChangeHdl;
    OUString m_aAllString;
    bool m_bHasLangNone;
    bool m_bLangNoneIsLangAll;

    SVX_DLLPRIVATE int ImplTypeToPos(LanguageType eType) const;
    SVX_DLLPRIVATE void ImplClear();
    DECL_LINK(ChangeHdl, weld::ComboBoxText&, void);
public:
    LanguageBox(std::unique_ptr<weld::ComboBoxText> pControl);
    void            SetLanguageList( SvxLanguageListFlags nLangList,
                            bool bHasLangNone, bool bLangNoneIsLangAll = false );
    void            AddLanguages( const std::vector< LanguageType >& rLanguageTypes, SvxLanguageListFlags nLangList );
    void            InsertLanguage(const LanguageType nLangType);
    void            SelectLanguage( const LanguageType eLangType );
    LanguageType    GetSelectedLanguage() const;
    void            SelectEntryPos(int nPos) { m_xControl->set_active(nPos); }

    void connect_changed(const Link<weld::ComboBoxText&, void>& rLink) { m_aChangeHdl = rLink; }
    void save_value() { m_xControl->save_value(); }
    bool get_value_changed_from_saved() const { return m_xControl->get_value_changed_from_saved(); }
    void hide() { m_xControl->hide(); }
};

class SVX_DLLPUBLIC SvxLanguageComboBox : public ComboBox, public SvxLanguageBoxBase
{
public:
    SvxLanguageComboBox( vcl::Window* pParent, WinBits nBits );

    enum class EditedAndValid
    {
        No,
        Valid,
        Invalid
    };

    EditedAndValid      GetEditedAndValid() const { return meEditedAndValid;}
    sal_Int32           SaveEditedAsEntry();


private:
    sal_Int32       mnSavedValuePos;
    EditedAndValid  meEditedAndValid;

    SVX_DLLPRIVATE virtual sal_Int32    ImplInsertImgEntry( const OUString& rEntry, sal_Int32  nPos, bool bChecked ) override;
    SVX_DLLPRIVATE virtual void         ImplRemoveEntryAt( sal_Int32 nPos ) override;

    SVX_DLLPRIVATE virtual void         ImplClear() override;
    SVX_DLLPRIVATE virtual sal_Int32    ImplInsertEntry( const OUString& rEntry, sal_Int32 nPos ) override;
    SVX_DLLPRIVATE virtual void         ImplSetEntryData( sal_Int32 nPos, void* pData ) override;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetSelectedEntryPos() const override;
    SVX_DLLPRIVATE virtual void*        ImplGetEntryData( sal_Int32 nPos ) const override;
    SVX_DLLPRIVATE virtual void         ImplSelectEntryPos( sal_Int32 nPos, bool bSelect ) override;
    SVX_DLLPRIVATE virtual bool         ImplIsEntryPosSelected( sal_Int32 nPos ) const override;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetEntryPos( const void* pData ) const override;
    SVX_DLLPRIVATE virtual void         ImplSetNoSelection() override;
    SVX_DLLPRIVATE virtual void         ImplHide() override;
    SVX_DLLPRIVATE virtual void         ImplDisable() override;
    SVX_DLLPRIVATE virtual void         ImplSaveValue() override;
    SVX_DLLPRIVATE virtual sal_Int32    ImplGetSavedValue() const override;

    DECL_LINK( EditModifyHdl, Edit&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
