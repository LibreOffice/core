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
#ifndef INCLUDED_CUI_SOURCE_INC_OPTLINGU_HXX
#define INCLUDED_CUI_SOURCE_INC_OPTLINGU_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com{namespace sun{namespace star{
namespace beans{
    class XPropertySet;
}
namespace linguistic2{
    class XDictionary;
    class XDictionaryList;
    class XLinguProperties;
}}}}

class SvTreeListEntry;
class SvxLinguData_Impl;

// define ----------------------------------------------------------------

#define GROUP_MODULES   (sal_uInt16(0x0008))

// forward ---------------------------------------------------------------

class SvxEditModulesDlg : public weld::GenericDialogController
{
    OUString            sSpell;
    OUString            sHyph;
    OUString            sThes;
    OUString            sGrammar;

    std::unique_ptr<SvxLinguData_Impl>  pDefaultLinguData;
    SvxLinguData_Impl&  rLinguData;

    std::unique_ptr<weld::TreeView> m_xModulesCLB;
    std::unique_ptr<weld::Button> m_xPrioUpPB;
    std::unique_ptr<weld::Button> m_xPrioDownPB;
    std::unique_ptr<weld::Button> m_xBackPB;
    std::unique_ptr<weld::LinkButton> m_xMoreDictsLink;
    std::unique_ptr<weld::Button> m_xClosePB;
    std::unique_ptr<LanguageBox> m_xLanguageLB;

    DECL_LINK( SelectHdl_Impl, weld::TreeView&, void );
    DECL_LINK( UpDownHdl_Impl, weld::Button&, void );
    DECL_LINK( ClickHdl_Impl, weld::Button&, void );
    DECL_LINK( BackHdl_Impl, weld::Button&, void );
    DECL_LINK( LangSelectListBoxHdl_Impl, weld::ComboBox&, void );
    typedef std::pair<int, int> row_col;
    DECL_LINK( BoxCheckButtonHdl_Impl, const row_col&, void );
    void LangSelectHdl_Impl(const LanguageBox* pBox);

public:
    SvxEditModulesDlg(weld::Window* pParent, SvxLinguData_Impl& rData);
    virtual ~SvxEditModulesDlg() override;
};

// class SvxLinguTabPage -------------------------------------------------

class SvxLinguTabPage : public SfxTabPage
{
    friend class VclPtr<SvxLinguTabPage>;
private:
    OUString            sCapitalWords;
    OUString            sWordsWithDigits;
    OUString            sSpellSpecial;
    OUString            sSpellAuto;
    OUString            sGrammarAuto;
    OUString            sNumMinWordlen;
    OUString            sNumPreBreak;
    OUString            sNumPostBreak;
    OUString            sHyphAuto;
    OUString            sHyphSpecial;

    int nUPN_HYPH_MIN_WORD_LENGTH;
    int nUPN_HYPH_MIN_LEADING;
    int nUPN_HYPH_MIN_TRAILING;

    css::uno::Reference<
        css::linguistic2::XLinguProperties >     xProp;

    css::uno::Reference<
        css::linguistic2::XDictionaryList >      xDicList;
    css::uno::Sequence<
        css::uno::Reference<
            css::linguistic2::XDictionary > >    aDics;

    std::unique_ptr<SvxLinguData_Impl>  pLinguData;

    std::unique_ptr<weld::Label> m_xLinguModulesFT;
    std::unique_ptr<weld::TreeView> m_xLinguModulesCLB;
    std::unique_ptr<weld::Button> m_xLinguModulesEditPB;
    std::unique_ptr<weld::Label> m_xLinguDicsFT;
    std::unique_ptr<weld::TreeView> m_xLinguDicsCLB;
    std::unique_ptr<weld::Button> m_xLinguDicsNewPB;
    std::unique_ptr<weld::Button> m_xLinguDicsEditPB;
    std::unique_ptr<weld::Button> m_xLinguDicsDelPB;
    std::unique_ptr<weld::TreeView> m_xLinguOptionsCLB;
    std::unique_ptr<weld::Button> m_xLinguOptionsEditPB;
    std::unique_ptr<weld::LinkButton> m_xMoreDictsLink;

    SvxLinguTabPage(TabPageParent pParent, const SfxItemSet& rCoreSet);

    void    AddDicBoxEntry( const css::uno::Reference< css::linguistic2::XDictionary > &rxDic, sal_uInt16 nIdx );
    static sal_uInt32 GetDicUserData( const css::uno::Reference< css::linguistic2::XDictionary > &rxDic, sal_uInt16 nIdx );

    DECL_LINK( SelectHdl_Impl, weld::TreeView&, void );
    DECL_LINK( ClickHdl_Impl, weld::Button&, void );
    DECL_LINK( BoxDoubleClickHdl_Impl, weld::TreeView&, void );
    typedef std::pair<int, int> row_col;
    DECL_LINK( ModulesBoxCheckButtonHdl_Impl, const row_col&, void );
    DECL_LINK( DicsBoxCheckButtonHdl_Impl, const row_col&, void );
    DECL_LINK( PostDblClickHdl_Impl, void *, void);

    void                UpdateModulesBox_Impl();
    void                UpdateDicBox_Impl();

public:
    virtual             ~SvxLinguTabPage() override;
    virtual void        dispose() override;
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    void                HideGroups( sal_uInt16 nGrp );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
