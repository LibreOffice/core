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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTFLTR_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTFLTR_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svtools/simptabl.hxx>

class OfaMSFilterTabPage : public SfxTabPage
{
    VclPtr<CheckBox>       aWBasicCodeCB;
    VclPtr<CheckBox>       aWBasicWbctblCB;
    VclPtr<CheckBox>       aWBasicStgCB;
    VclPtr<CheckBox>       aEBasicCodeCB;
    VclPtr<CheckBox>       aEBasicExectblCB;
    VclPtr<CheckBox>       aEBasicStgCB;
    VclPtr<CheckBox>       aPBasicCodeCB;
    VclPtr<CheckBox>       aPBasicStgCB;


    DECL_LINK(LoadWordBasicCheckHdl_Impl, Button*, void);
    DECL_LINK(LoadExcelBasicCheckHdl_Impl, Button*, void);
public:
    OfaMSFilterTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMSFilterTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

enum class MSFltrPg2_CheckBoxEntries;

class OfaMSFilterTabPage2 : public SfxTabPage
{

    class MSFltrSimpleTable : public SvSimpleTable
    {
        using SvTreeListBox::GetCheckButtonState;
        using SvTreeListBox::SetCheckButtonState;
        using SvSimpleTable::SetTabs;

        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, bool bChecked);
        static SvButtonState GetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol );
        void            SetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol, SvButtonState );
    protected:
        virtual void    SetTabs() override;
        virtual void    HBarClick() override;
        virtual void    KeyInput( const KeyEvent& rKEvt ) override;

    public:
        explicit MSFltrSimpleTable(SvSimpleTableContainer& rParent)
            : SvSimpleTable(rParent, 0)
        {
        }
    };

    VclPtr<SvSimpleTableContainer> m_pCheckLBContainer;
    VclPtr<MSFltrSimpleTable> m_pCheckLB;
    OUString sHeader1, sHeader2;
    OUString sChgToFromMath,
           sChgToFromWriter,
           sChgToFromCalc,
           sChgToFromImpress,
           sChgToFromSmartArt;
    std::unique_ptr<SvLBoxButtonData> m_xCheckButtonData;

    VclPtr<RadioButton> aHighlightingRB;
    VclPtr<RadioButton> aShadingRB;

    virtual ~OfaMSFilterTabPage2() override;
    virtual void dispose() override;

    void                InsertEntry( const OUString& _rTxt, MSFltrPg2_CheckBoxEntries _nType );
    void                InsertEntry( const OUString& _rTxt, MSFltrPg2_CheckBoxEntries _nType,
                                     bool saveEnabled );
    SvTreeListEntry*    GetEntry4Type( MSFltrPg2_CheckBoxEntries _nType ) const;

public:
    OfaMSFilterTabPage2( vcl::Window* pParent, const SfxItemSet& rSet );
    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
