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

class SfxViewFrame;
struct SfxVersionInfo;

class SfxVersionTableDtor;
class SfxVersionDialog final : public SfxDialogController
{
    SfxViewFrame* m_pViewFrame;
    bool m_bIsSaveVersionOnClose;
    std::unique_ptr<SfxVersionTableDtor> m_pTable;
    std::unique_ptr<weld::Button> m_xSaveButton;
    std::unique_ptr<weld::CheckButton> m_xSaveCheckBox;
    std::unique_ptr<weld::Button> m_xOpenButton;
    std::unique_ptr<weld::Button> m_xViewButton;
    std::unique_ptr<weld::Button> m_xDeleteButton;
    std::unique_ptr<weld::Button> m_xCompareButton;
    std::unique_ptr<weld::Button> m_xCmisButton;
    std::unique_ptr<weld::TreeView> m_xVersionBox;

    DECL_LINK(DClickHdl_Impl, weld::TreeView&, bool);
    DECL_LINK(SelectHdl_Impl, weld::TreeView&, void);
    DECL_LINK(ButtonHdl_Impl, weld::Button&, void);
    DECL_LINK(ToggleHdl_Impl, weld::Toggleable&, void);
    void Init_Impl();
    void Open_Impl();

public:
    SfxVersionDialog(weld::Window* pParent, SfxViewFrame* pFrame, bool);
    virtual ~SfxVersionDialog() override;
    bool IsSaveVersionOnClose() const { return m_bIsSaveVersionOnClose; }
};

class SfxViewVersionDialog_Impl final : public SfxDialogController
{
private:
    SfxVersionInfo& m_rInfo;

    std::unique_ptr<weld::Label> m_xDateTimeText;
    std::unique_ptr<weld::Label> m_xSavedByText;
    std::unique_ptr<weld::TextView> m_xEdit;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Button> m_xCancelButton;
    std::unique_ptr<weld::Button> m_xCloseButton;

    DECL_LINK(ButtonHdl, weld::Button&, void);

public:
    SfxViewVersionDialog_Impl(weld::Window* pParent, SfxVersionInfo& rInfo, bool bEdit);
};

class SfxCmisVersionsDialog final : public SfxDialogController
{
    SfxViewFrame* m_pViewFrame;
    std::unique_ptr<SfxVersionTableDtor> m_pTable;

    std::unique_ptr<weld::Button> m_xOpenButton;
    std::unique_ptr<weld::Button> m_xViewButton;
    std::unique_ptr<weld::Button> m_xDeleteButton;
    std::unique_ptr<weld::Button> m_xCompareButton;
    std::unique_ptr<weld::TreeView> m_xVersionBox;

    void LoadVersions();

public:
    SfxCmisVersionsDialog(weld::Window* pParent, SfxViewFrame* pFrame);
    virtual ~SfxCmisVersionsDialog() override;
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
