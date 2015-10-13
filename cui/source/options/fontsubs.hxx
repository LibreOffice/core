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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_FONTSUBS_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_FONTSUBS_HXX

#include <sfx2/tabdlg.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/treelistentry.hxx>
#include <vcl/fixed.hxx>
#include <vcl/toolbox.hxx>

// class SvxFontSubstCheckListBox ------------------------------------------

class SvxFontSubstCheckListBox : public SvSimpleTable
{
    friend class SvxFontSubstTabPage;
    using SvSimpleTable::SetTabs;
    using SvTreeListBox::GetCheckButtonState;
    using SvTreeListBox::SetCheckButtonState;

    protected:
        virtual void    SetTabs() override;
        virtual void    KeyInput( const KeyEvent& rKEvt ) override;
        virtual void    Resize() override;

    public:
        SvxFontSubstCheckListBox(SvSimpleTableContainer& rParent, WinBits nBits)
            : SvSimpleTable(rParent, nBits)
        {
        }

        bool            IsChecked(sal_uLong nPos, sal_uInt16 nCol = 0);
        static bool     IsChecked(SvTreeListEntry* pEntry, sal_uInt16 nCol = 0);
        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, bool bChecked);
        void            CheckEntry(SvTreeListEntry* pEntry, sal_uInt16 nCol, bool bChecked);
        static SvButtonState GetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol );
        void            SetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol, SvButtonState );

        void setColSizes();
};

// class SvxFontSubstTabPage ----------------------------------------------------
class SvtFontSubstConfig;
class SvxFontSubstTabPage : public SfxTabPage
{
    VclPtr<CheckBox>                   m_pUseTableCB;
    VclPtr<VclContainer>               m_pReplacements;
    VclPtr<FontNameBox>                m_pFont1CB;
    VclPtr<FontNameBox>                m_pFont2CB;
    VclPtr<PushButton>                 m_pApply;
    VclPtr<PushButton>                 m_pDelete;

    VclPtr<SvxFontSubstCheckListBox>   m_pCheckLB;

    VclPtr<ListBox>                    m_pFontNameLB;
    VclPtr<CheckBox>                   m_pNonPropFontsOnlyCB;
    VclPtr<ListBox>                    m_pFontHeightLB;

    OUString                    m_sAutomatic;

    SvtFontSubstConfig*         pConfig;

    Color           aTextColor;

    SvLBoxButtonData*   pCheckButtonData;

    DECL_LINK(SelectHdl, vcl::Window *);
    DECL_LINK_TYPED(SelectComboBoxHdl, ComboBox&, void);
    DECL_LINK_TYPED(ClickHdl, Button*, void);
    DECL_LINK_TYPED(TreeListBoxSelectHdl, SvTreeListBox*, void);
    DECL_LINK_TYPED(NonPropFontsHdl, Button*, void);

    SvTreeListEntry*    CreateEntry(OUString& rFont1, OUString& rFont2);
    void            CheckEnable();


    virtual ~SvxFontSubstTabPage();
    virtual void dispose() override;

public:
    SvxFontSubstTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};


#endif // INCLUDED_CUI_SOURCE_OPTIONS_FONTSUBS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
