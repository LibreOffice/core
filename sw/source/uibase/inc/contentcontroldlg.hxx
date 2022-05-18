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
#include <vcl/weld.hxx>

class SwWrtShell;
class SwContentControl;
class SwContentControlListItem;

/**
 * The content control dialog allows editing the properties of the content control under the
 * cursor.
 */
class SwContentControlDlg final : public SfxDialogController
{
    SwWrtShell& m_rWrtShell;
    std::shared_ptr<SwContentControl> m_pContentControl;
    std::vector<SwContentControlListItem> m_aSavedListItems;

    std::unique_ptr<weld::CheckButton> m_xShowingPlaceHolderCB;
    std::unique_ptr<weld::Frame> m_xListItemsFrame;
    std::unique_ptr<weld::TreeView> m_xListItems;
    std::unique_ptr<weld::Box> m_xListItemButtons;
    std::unique_ptr<weld::Button> m_xInsertBtn;
    std::unique_ptr<weld::Button> m_xRenameBtn;
    std::unique_ptr<weld::Button> m_xDeleteBtn;
    std::unique_ptr<weld::Button> m_xMoveUpBtn;
    std::unique_ptr<weld::Button> m_xMoveDownBtn;

    std::unique_ptr<weld::Button> m_xOk;

    DECL_LINK(InsertHdl, weld::Button&, void);
    DECL_LINK(RenameHdl, weld::Button&, void);
    DECL_LINK(DeleteHdl, weld::Button&, void);
    DECL_LINK(MoveUpHdl, weld::Button&, void);
    DECL_LINK(MoveDownHdl, weld::Button&, void);
    DECL_LINK(SelectionChangedHdl, weld::TreeView&, void);
    DECL_LINK(OkHdl, weld::Button&, void);

public:
    SwContentControlDlg(weld::Window* pParent, SwWrtShell& rSh);
    virtual ~SwContentControlDlg() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
