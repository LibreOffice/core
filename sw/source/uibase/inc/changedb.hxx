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

#include <sfx2/basedlgs.hxx>
#include "dbtree.hxx"

class SwView;
class SwWrtShell;
struct SwDBData;

// exchange database at fields
class SwChangeDBDlg final : public SfxDialogController
{
    SwWrtShell      *m_pSh;

    std::unique_ptr<weld::TreeView> m_xUsedDBTLB;
    std::unique_ptr<SwDBTreeList> m_xAvailDBTLB;
    std::unique_ptr<weld::Button> m_xAddDBPB;
    std::unique_ptr<weld::Label> m_xDocDBNameFT;
    std::unique_ptr<weld::Button> m_xDefineBT;

    void TreeSelect();

    DECL_LINK(TreeSelectHdl, weld::TreeView&, void);
    DECL_LINK(ButtonHdl, weld::Button&, void);
    DECL_LINK(AddDBHdl, weld::Button&, void);

    void            FillDBPopup();
    std::unique_ptr<weld::TreeIter> Insert(std::u16string_view rDBName);
    void            ShowDBName(const SwDBData& rDBData);

public:
    SwChangeDBDlg(SwView const & rVw);
    virtual ~SwChangeDBDlg() override;

    void            UpdateFields();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
