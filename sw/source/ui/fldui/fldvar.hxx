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

#include "fldpage.hxx"
#include <condedit.hxx>
#include <numfmtlb.hxx>

class SwFieldVarPage;

class SwFieldVarPage : public SwFieldPage
{
    std::unique_ptr<weld::TreeView> m_xTypeLB;
    std::unique_ptr<weld::Widget> m_xSelection;
    std::unique_ptr<weld::TreeView> m_xSelectionLB;
    std::unique_ptr<weld::Label> m_xNameFT;
    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::Label> m_xValueFT;
    std::unique_ptr<ConditionEdit<weld::TextView>> m_xValueED;
    std::unique_ptr<weld::Widget> m_xFormat;
    std::unique_ptr<SwNumFormatTreeView> m_xNumFormatLB;
    std::unique_ptr<weld::TreeView> m_xFormatLB;
    std::unique_ptr<weld::Widget> m_xChapterFrame;
    std::unique_ptr<weld::ComboBox> m_xChapterLevelLB;
    std::unique_ptr<weld::CheckButton> m_xInvisibleCB;
    std::unique_ptr<weld::Label> m_xSeparatorFT;
    std::unique_ptr<weld::Entry> m_xSeparatorED;
    std::unique_ptr<weld::Button> m_xNewPB;
    std::unique_ptr<weld::Button> m_xDelPB;

    OUString            m_sOldValueFT;
    OUString            m_sOldNameFT;

    sal_uInt32          m_nOldFormat;
    bool                m_bInit;

    DECL_LINK( TypeHdl, weld::TreeView&, void );
    DECL_LINK( SubTypeListBoxHdl, weld::TreeView&, void );
    DECL_LINK( ModifyHdl, weld::Entry&, void );
    DECL_LINK( TBClickHdl, weld::Button&, void );
    DECL_LINK( ChapterHdl, weld::ComboBox&, void );
    DECL_LINK( SeparatorHdl, weld::Entry&, void );
    DECL_LINK( SubTypeInsertHdl, weld::TreeView&, bool );
    void SubTypeHdl(const weld::TreeView*);

    void                UpdateSubType();
    void                FillFormatLB(SwFieldTypesEnum nTypeId);

protected:
    virtual sal_uInt16      GetGroup() override;

public:
    SwFieldVarPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* pSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);
    virtual ~SwFieldVarPage() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
