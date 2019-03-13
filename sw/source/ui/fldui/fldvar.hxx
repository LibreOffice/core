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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDVAR_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDVAR_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/edit.hxx>

#include "fldpage.hxx"
#include <condedit.hxx>
#include <numfmtlb.hxx>

class SwFieldVarPage;

class SelectionListBox : public ListBox
{
    bool            bCallAddSelection;

    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

public:
    SelectionListBox(vcl::Window* pParent, WinBits nStyle);

    //  detect selection via Ctrl or Alt and evaluate with SelectHdl
    bool            IsCallAddSelection() const {return bCallAddSelection;}
    void            ResetCallAddSelection() {bCallAddSelection = false;}
};

class SwFieldVarPage : public SwFieldPage
{
    friend class SelectionListBox;

    VclPtr<ListBox>            m_pTypeLB;
    VclPtr<VclContainer>       m_pSelection;
    VclPtr<SelectionListBox>   m_pSelectionLB;
    VclPtr<FixedText>          m_pNameFT;
    VclPtr<Edit>               m_pNameED;
    VclPtr<FixedText>          m_pValueFT;
    VclPtr<ConditionEdit>      m_pValueED;
    VclPtr<VclContainer>       m_pFormat;
    VclPtr<NumFormatListBox>   m_pNumFormatLB;
    VclPtr<ListBox>            m_pFormatLB;
    VclPtr<VclContainer>       m_pChapterFrame;
    VclPtr<ListBox>            m_pChapterLevelLB;
    VclPtr<CheckBox>           m_pInvisibleCB;
    VclPtr<FixedText>          m_pSeparatorFT;
    VclPtr<Edit>               m_pSeparatorED;
    VclPtr<PushButton>         m_pNewPB;
    VclPtr<PushButton>         m_pDelPB;

    OUString            sOldValueFT;
    OUString            sOldNameFT;

    sal_uInt32          nOldFormat;
    bool                bInit;

    DECL_LINK( TypeHdl, ListBox&, void );
    DECL_LINK( SubTypeListBoxHdl, ListBox&, void );
    DECL_LINK( ModifyHdl, Edit&, void );
    DECL_LINK( TBClickHdl, Button*, void );
    DECL_LINK( ChapterHdl, ListBox&, void );
    DECL_LINK( SeparatorHdl, Edit&, void );
    void SubTypeHdl(ListBox const *);

    void                UpdateSubType();
    void                FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup() override;

public:
                        SwFieldVarPage(vcl::Window* pParent, const SfxItemSet* pSet);

                        virtual ~SwFieldVarPage() override;
    virtual void        dispose() override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
