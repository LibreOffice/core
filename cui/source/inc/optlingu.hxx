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

#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixedhyper.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/checklbx.hxx>
#include <svx/langbox.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

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

#define GROUP_MODULES   ((sal_uInt16)0x0008)

// forward ---------------------------------------------------------------

class SvxEditModulesDlg : public ModalDialog
{
    VclPtr<SvxLanguageBox>     m_pLanguageLB;

    VclPtr<SvxCheckListBox>    m_pModulesCLB;
    VclPtr<PushButton>         m_pPrioUpPB;
    VclPtr<PushButton>         m_pPrioDownPB;
    VclPtr<PushButton>         m_pBackPB;
    VclPtr<FixedHyperlink>     m_pMoreDictsLink;

    VclPtr<CloseButton>        m_pClosePB;

    OUString            sSpell;
    OUString            sHyph;
    OUString            sThes;
    OUString            sGrammar;

    SvxLinguData_Impl*  pDefaultLinguData;
    SvxLinguData_Impl&  rLinguData;

    SvLBoxButtonData*   pCheckButtonData;

    SvTreeListEntry*    CreateEntry(OUString& rTxt, sal_uInt16 nCol);

    DECL_LINK_TYPED(SelectHdl_Impl, SvTreeListBox*, void);
    DECL_LINK_TYPED(UpDownHdl_Impl, Button*, void);
    DECL_LINK_TYPED(ClickHdl_Impl, Button*, void);
    DECL_LINK_TYPED(BackHdl_Impl, Button*, void);
    DECL_LINK_TYPED( LangSelectListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( BoxCheckButtonHdl_Impl2, SvLBoxButtonData*, void );
    DECL_LINK_TYPED( BoxCheckButtonHdl_Impl, SvTreeListBox*, void );
    DECL_LINK(OpenURLHdl_Impl, void *);
    void LangSelectHdl_Impl(ListBox*);

public:
    SvxEditModulesDlg(vcl::Window* pParent, SvxLinguData_Impl& rData);
    virtual ~SvxEditModulesDlg();
    virtual void dispose() SAL_OVERRIDE;
};

// class SvxLinguTabPage -------------------------------------------------

class SvxLinguTabPage : public SfxTabPage
{
    friend class VclPtr<SvxLinguTabPage>;
private:
    VclPtr<FixedText>          m_pLinguModulesFT;
    VclPtr<SvxCheckListBox>    m_pLinguModulesCLB;
    VclPtr<PushButton>         m_pLinguModulesEditPB;
    VclPtr<FixedText>          m_pLinguDicsFT;
    VclPtr<SvxCheckListBox>    m_pLinguDicsCLB;
    VclPtr<PushButton>         m_pLinguDicsNewPB;
    VclPtr<PushButton>         m_pLinguDicsEditPB;
    VclPtr<PushButton>         m_pLinguDicsDelPB;
    VclPtr<SvxCheckListBox>    m_pLinguOptionsCLB;
    VclPtr<PushButton>         m_pLinguOptionsEditPB;
    VclPtr<FixedHyperlink>     m_pMoreDictsLink;

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

    css::uno::Reference<
        css::linguistic2::XLinguProperties >     xProp;

    css::uno::Reference<
        css::linguistic2::XDictionaryList >      xDicList;
    css::uno::Sequence<
        css::uno::Reference<
            css::linguistic2::XDictionary > >    aDics;

    SvLBoxButtonData*   pCheckButtonData;

    SvxLinguData_Impl*  pLinguData;

    SvxLinguTabPage( vcl::Window* pParent, const SfxItemSet& rCoreSet );
    SvTreeListEntry*    CreateEntry(OUString& rTxt, sal_uInt16 nCol);

    void    AddDicBoxEntry( const css::uno::Reference< css::linguistic2::XDictionary > &rxDic, sal_uInt16 nIdx );
    static sal_uLong GetDicUserData( const css::uno::Reference< css::linguistic2::XDictionary > &rxDic, sal_uInt16 nIdx );

    DECL_LINK_TYPED( SelectHdl_Impl, SvTreeListBox*, void );
    DECL_LINK_TYPED( ClickHdl_Impl, Button *, void );
    DECL_LINK_TYPED( BoxDoubleClickHdl_Impl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( BoxCheckButtonHdl_Impl, SvTreeListBox*, void );
    DECL_LINK_TYPED( PostDblClickHdl_Impl, void *, void);
    DECL_LINK( OpenURLHdl_Impl, void *);

    void                UpdateModulesBox_Impl();
    void                UpdateDicBox_Impl();

public:
    virtual             ~SvxLinguTabPage();
    virtual void        dispose() SAL_OVERRIDE;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    void                HideGroups( sal_uInt16 nGrp );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
