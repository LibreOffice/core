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
        virtual void    SetTabs() SAL_OVERRIDE;
        virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
        virtual void    Resize() SAL_OVERRIDE;

    public:
        SvxFontSubstCheckListBox(SvSimpleTableContainer& rParent, WinBits nBits)
            : SvSimpleTable(rParent, nBits)
        {
        }

        inline void     *GetUserData(sal_uLong nPos) { return GetEntry(nPos)->GetUserData(); }
        inline void     SetUserData(sal_uLong nPos, void *pData ) { GetEntry(nPos)->SetUserData(pData); }

        bool            IsChecked(sal_uLong nPos, sal_uInt16 nCol = 0);
        bool            IsChecked(SvTreeListEntry* pEntry, sal_uInt16 nCol = 0);
        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, bool bChecked);
        void            CheckEntry(SvTreeListEntry* pEntry, sal_uInt16 nCol, bool bChecked);
        SvButtonState   GetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol ) const;
        void            SetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol, SvButtonState );

        void setColSizes();
};

// class SvxFontSubstTabPage ----------------------------------------------------
class SvtFontSubstConfig;
class SvxFontSubstTabPage : public SfxTabPage
{
    CheckBox*                   m_pUseTableCB;
    VclContainer*               m_pReplacements;
    FontNameBox*                m_pFont1CB;
    FontNameBox*                m_pFont2CB;
    PushButton*                 m_pApply;
    PushButton*                 m_pDelete;

    SvxFontSubstCheckListBox*   m_pCheckLB;

    ListBox*                    m_pFontNameLB;
    CheckBox*                   m_pNonPropFontsOnlyCB;
    ListBox*                    m_pFontHeightLB;

    OUString                    m_sAutomatic;

    SvtFontSubstConfig*         pConfig;

    Color           aTextColor;

    SvLBoxButtonData*   pCheckButtonData;

    DECL_LINK(SelectHdl, vcl::Window *pWin = 0);
    DECL_LINK(NonPropFontsHdl, CheckBox* pBox);

    SvTreeListEntry*    CreateEntry(OUString& rFont1, OUString& rFont2);
    void            CheckEnable();


    SvxFontSubstTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxFontSubstTabPage();

public:
    static SfxTabPage*  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
};


#endif // INCLUDED_CUI_SOURCE_OPTIONS_FONTSUBS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
