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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CHANGEDB_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CHANGEDB_HXX

#include <svx/stddlg.hxx>
#include <vcl/weld.hxx>
#include "dbtree.hxx"

class SwFieldMgr;
class SwView;
class SwWrtShell;
struct SwDBData;

// exchange database at fields
class SwChangeDBDlg : public SfxDialogController
{
    SwWrtShell      *pSh;

    std::unique_ptr<weld::TreeView> m_xUsedDBTLB;
    std::unique_ptr<SwDBTreeList> m_xAvailDBTLB;
    std::unique_ptr<weld::Button> m_xAddDBPB;
    std::unique_ptr<weld::Label> m_xDocDBNameFT;
    std::unique_ptr<weld::Button> m_xDefineBT;

    void TreeSelect();

    DECL_LINK(TreeSelectHdl, weld::TreeView&, void);
    DECL_LINK(ButtonHdl, weld::Button&, void);
    DECL_LINK(AddDBHdl, weld::Button&, void);

    void            UpdateFields();
    void            FillDBPopup();
    std::unique_ptr<weld::TreeIter> Insert(const OUString& rDBName);
    void            ShowDBName(const SwDBData& rDBData);

public:
    SwChangeDBDlg(SwView const & rVw);
    virtual short run() override;
    virtual ~SwChangeDBDlg() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
