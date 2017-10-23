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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_ADDRESSLISTDIALOG_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_ADDRESSLISTDIALOG_HXX

#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <swdbdata.hxx>
#include <sharedconnection.hxx>

namespace com{namespace sun{namespace star{
    namespace container{
        class XNameAccess;
    }
    namespace sdb{
        class XDatabaseContext;
    }
    namespace sdbc{
        class XDataSource;
    }
    namespace sdbcx{
        class XColumnsSupplier;
    }
}}}
class SwMailMergeAddressBlockPage;
class SwAddrSourceLB;

class SwAddressListDialog : public SfxModalDialog
{
    VclPtr<FixedText>      m_pDescriptionFI;

    VclPtr<SwAddrSourceLB> m_pListLB;

    VclPtr<PushButton>     m_pLoadListPB;
    VclPtr<PushButton>     m_pCreateListPB;
    VclPtr<PushButton>     m_pFilterPB;
    VclPtr<PushButton>     m_pEditPB;
    VclPtr<PushButton>     m_pTablePB;

    VclPtr<OKButton>       m_pOK;

    OUString        m_sName;
    OUString        m_sTable;
    OUString        m_sConnecting;

    SvTreeListEntry*    m_pCreatedDataSource;

    bool            m_bInSelectHdl;

    VclPtr<SwMailMergeAddressBlockPage> m_pAddressPage;

    css::uno::Reference< css::sdb::XDatabaseContext> m_xDBContext;

    SwDBData                                         m_aDBData;

    void DetectTablesAndQueries(SvTreeListEntry* pSelect, bool bWidthDialog);

    DECL_LINK(FilterHdl_Impl, Button*, void);
    DECL_LINK(LoadHdl_Impl, Button*, void);
    DECL_LINK(CreateHdl_Impl, Button*, void);
    DECL_LINK(ListBoxSelectHdl_Impl, SvTreeListBox*, void);
    DECL_LINK(EditHdl_Impl, Button*, void);
    DECL_LINK(TableSelectHdl_Impl, Button*, void);
    DECL_LINK(OKHdl_Impl, Button*, void);

    DECL_LINK(StaticListBoxSelectHdl_Impl, void*, void);

public:
    SwAddressListDialog(SwMailMergeAddressBlockPage* pParent);
    virtual ~SwAddressListDialog() override;
    virtual void dispose() override;

    css::uno::Reference< css::sdbc::XDataSource>
                        GetSource();

    SharedConnection    GetConnection();

    css::uno::Reference< css::sdbcx::XColumnsSupplier>
                        GetColumnsSupplier();

    const SwDBData&     GetDBData() const       {return m_aDBData;}
    OUString     GetFilter();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
