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
#pragma once
#if 1

#include <sfx2/basedlgs.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <swdbdata.hxx>
#include "sharedconnection.hxx"


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

class SwAddressListDialog : public SfxModalDialog
{
    FixedInfo       m_aDescriptionFI;

    FixedInfo       m_aListFT;
    HeaderBar       m_aListHB;
    SvTabListBox    m_aListLB;

    PushButton      m_aLoadListPB;
    PushButton      m_aCreateListPB;
    PushButton      m_aFilterPB;
    PushButton      m_aEditPB;
    PushButton      m_aTablePB;

    FixedLine       m_aSeparatorFL;

    OKButton        m_aOK;
    CancelButton    m_aCancel;
    HelpButton      m_aHelp;

    String          m_sName;
    String          m_sTable;
    String          m_sConnecting;

    String          m_sCreatedURL;
    SvTreeListEntry*    m_pCreatedDataSource;

    bool            m_bInSelectHdl;

    SwMailMergeAddressBlockPage* m_pAddressPage;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseContext> m_xDBContext;

    SwDBData                                                                   m_aDBData;

    void DetectTablesAndQueries(SvTreeListEntry* pSelect, bool bWidthDialog);

    DECL_LINK(FilterHdl_Impl, void *);
    DECL_LINK(LoadHdl_Impl, void *);
    DECL_LINK(CreateHdl_Impl, PushButton*);
    DECL_LINK(ListBoxSelectHdl_Impl, void *);
    DECL_LINK(EditHdl_Impl, PushButton*);
    DECL_LINK(TableSelectHdl_Impl, PushButton*);
    DECL_LINK(OKHdl_Impl, void *);

    DECL_STATIC_LINK(SwAddressListDialog, StaticListBoxSelectHdl_Impl, SvTreeListEntry*);

public:
    SwAddressListDialog(SwMailMergeAddressBlockPage* pParent);
    ~SwAddressListDialog();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>
                        GetSource();

    SharedConnection    GetConnection();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>
                        GetColumnsSupplier();

    const SwDBData&     GetDBData() const       {return m_aDBData;}
    ::rtl::OUString     GetFilter();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
