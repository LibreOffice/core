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
#ifndef _SWFLDDB_HXX
#define _SWFLDDB_HXX

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>

#include "condedit.hxx"
#include "dbtree.hxx"
#include "numfmtlb.hxx"

#include "fldpage.hxx"

class SwFldDBPage : public SwFldPage
{
    ListBox*            m_pTypeLB;
    SwDBTreeList*       m_pDatabaseTLB;

    PushButton*         m_pAddDBPB;

    VclContainer*       m_pCondition;
    ConditionEdit*      m_pConditionED;
    VclContainer*       m_pValue;
    Edit*               m_pValueED;
    RadioButton*        m_pDBFormatRB;
    RadioButton*        m_pNewFormatRB;
    NumFormatListBox*   m_pNumFormatLB;
    ListBox*            m_pFormatLB;
    VclContainer*       m_pFormat;

    String              sOldDBName;
    String              sOldTableName;
    String              sOldColumnName;
    sal_uLong               nOldFormat;
    sal_uInt16              nOldSubType;
    Link                aOldNumSelectHdl;

    DECL_LINK( TypeHdl, ListBox* );
    DECL_LINK( NumSelectHdl, NumFormatListBox* pLB = 0);
    DECL_LINK( TreeSelectHdl, SvTreeListBox* pBox );
    DECL_LINK(ModifyHdl, void *);
    DECL_LINK(AddDBHdl, void *);

    void                CheckInsert();

    using SwFldPage::SetWrtShell;

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldDBPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldDBPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
    void                ActivateMailMergeAddress();

    void                SetWrtShell(SwWrtShell& rSh);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
