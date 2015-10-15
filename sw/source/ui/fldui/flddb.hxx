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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDDB_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDDB_HXX

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>

#include "condedit.hxx"
#include "dbtree.hxx"
#include "numfmtlb.hxx"

#include "fldpage.hxx"

class SwFieldDBPage : public SwFieldPage
{
    VclPtr<ListBox>            m_pTypeLB;
    VclPtr<SwDBTreeList>       m_pDatabaseTLB;

    VclPtr<PushButton>         m_pAddDBPB;

    VclPtr<VclContainer>       m_pCondition;
    VclPtr<ConditionEdit>      m_pConditionED;
    VclPtr<VclContainer>       m_pValue;
    VclPtr<Edit>               m_pValueED;
    VclPtr<RadioButton>        m_pDBFormatRB;
    VclPtr<RadioButton>        m_pNewFormatRB;
    VclPtr<NumFormatListBox>   m_pNumFormatLB;
    VclPtr<ListBox>            m_pFormatLB;
    VclPtr<VclContainer>       m_pFormat;

    OUString            m_sOldDBName;
    OUString            m_sOldTableName;
    OUString            m_sOldColumnName;
    sal_uLong           m_nOldFormat;
    sal_uInt16          m_nOldSubType;
    Link<ListBox&,void> m_aOldNumSelectHdl;

    DECL_LINK_TYPED( TypeListBoxHdl, ListBox&, void );
    DECL_LINK_TYPED( NumSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( TreeSelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( ModifyHdl, Edit&, void );
    DECL_LINK_TYPED( AddDBHdl, Button*, void );
    void TypeHdl(ListBox*);

    void                CheckInsert();

    using SwFieldPage::SetWrtShell;

protected:
    virtual sal_uInt16      GetGroup() override;

public:
                        SwFieldDBPage(vcl::Window* pParent, const SfxItemSet& rSet);

                        virtual ~SwFieldDBPage();
    virtual void        dispose() override;

    static VclPtr<SfxTabPage>  Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
    void                ActivateMailMergeAddress();

    void                SetWrtShell(SwWrtShell& rSh);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
