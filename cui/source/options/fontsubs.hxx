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
#ifndef _SVX_FONT_SUBSTITUTION_HXX
#define _SVX_FONT_SUBSTITUTION_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <svx/simptabl.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/ctrlbox.hxx>

// class SvxFontSubstCheckListBox ------------------------------------------

class SvxFontSubstCheckListBox : public SvxSimpleTable
{
    friend class SvxFontSubstTabPage;
    using SvxSimpleTable::SetTabs;
    using SvTreeListBox::GetCheckButtonState;
    using SvTreeListBox::SetCheckButtonState;

    protected:
        virtual void    SetTabs();
        virtual void    KeyInput( const KeyEvent& rKEvt );

    public:
        SvxFontSubstCheckListBox(SvxSimpleTableContainer& rParent, WinBits nBits = WB_BORDER)
            : SvxSimpleTable(rParent, nBits)
        {
        }

        inline void     *GetUserData(sal_uLong nPos) { return GetEntry(nPos)->GetUserData(); }
        inline void     SetUserData(sal_uLong nPos, void *pData ) { GetEntry(nPos)->SetUserData(pData); }

        sal_Bool            IsChecked(sal_uLong nPos, sal_uInt16 nCol = 0);
        sal_Bool            IsChecked(SvTreeListEntry* pEntry, sal_uInt16 nCol = 0);
        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked);
        void            CheckEntry(SvTreeListEntry* pEntry, sal_uInt16 nCol, sal_Bool bChecked);
        SvButtonState   GetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol ) const;
        void            SetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol, SvButtonState );
};

// class SvxFontSubstTabPage ----------------------------------------------------
class SvtFontSubstConfig;
class SvxFontSubstTabPage : public SfxTabPage
{
    CheckBox                    aUseTableCB;
    FixedText                   aFont1FT;
    FontNameBox                 aFont1CB;
    FixedText                   aFont2FT;
    FontNameBox                 aFont2CB;
    ToolBox                     aNewDelTBX;
    SvxSimpleTableContainer m_aCheckLBContainer;
    SvxFontSubstCheckListBox    aCheckLB;

    FixedLine                   aSourceViewFontsFL;
    FixedText                   aFontNameFT;
    ListBox                     aFontNameLB;
    CheckBox                    aNonPropFontsOnlyCB;
    FixedText                   aFontHeightFT;
    ListBox                     aFontHeightLB;

    ImageList                   aImageList;
    String                      sAutomatic;

    SvtFontSubstConfig*         pConfig;

    String          sHeader1;
    String          sHeader2;
    String          sHeader3;
    String          sHeader4;

    Color           aTextColor;

    SvLBoxButtonData*   pCheckButtonData;

    DECL_LINK(SelectHdl, Window *pWin = 0);
    DECL_LINK(NonPropFontsHdl, CheckBox* pBox);

    SvTreeListEntry*    CreateEntry(String& rFont1, String& rFont2);
    void            CheckEnable();


    SvxFontSubstTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxFontSubstTabPage();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};


#endif // _SVX_FONT_SUBSTITUTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
