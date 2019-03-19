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

#include <condedit.hxx>
#include <dbtree.hxx>
#include <numfmtlb.hxx>

#include "fldpage.hxx"

class SwFieldDBPage : public SwFieldPage
{
    OUString            m_sOldDBName;
    OUString            m_sOldTableName;
    OUString            m_sOldColumnName;
    sal_uLong           m_nOldFormat;
    sal_uInt16          m_nOldSubType;

    std::unique_ptr<weld::TreeView> m_xTypeLB;
    std::unique_ptr<SwDBTreeList> m_xDatabaseTLB;
    std::unique_ptr<weld::Button> m_xAddDBPB;
    std::unique_ptr<weld::Widget> m_xCondition;
    std::unique_ptr<SwConditionEdit> m_xConditionED;
    std::unique_ptr<weld::Widget> m_xValue;
    std::unique_ptr<weld::Entry> m_xValueED;
    std::unique_ptr<weld::RadioButton> m_xDBFormatRB;
    std::unique_ptr<weld::RadioButton> m_xNewFormatRB;
    std::unique_ptr<SwNumFormatListBox> m_xNumFormatLB;
    std::unique_ptr<weld::ComboBox> m_xFormatLB;
    std::unique_ptr<weld::Widget> m_xFormat;

    DECL_LINK( TypeListBoxHdl, weld::TreeView&, void );
    DECL_LINK( NumSelectHdl, weld::ComboBox&, void );
    DECL_LINK( TreeSelectHdl, weld::TreeView&, void );
    DECL_LINK( ModifyHdl, weld::Entry&, void );
    DECL_LINK( AddDBHdl, weld::Button&, void );
    void TypeHdl(const weld::TreeView*);

    void                CheckInsert();

    using SwFieldPage::SetWrtShell;

protected:
    virtual sal_uInt16      GetGroup() override;

public:
    SwFieldDBPage(TabPageParent pParent, const SfxItemSet* rSet);

    virtual ~SwFieldDBPage() override;
    virtual void        dispose() override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
    void                ActivateMailMergeAddress();

    void                SetWrtShell(SwWrtShell& rSh);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
