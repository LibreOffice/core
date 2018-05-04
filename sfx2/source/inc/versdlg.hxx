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

#ifndef INCLUDED_SFX2_SOURCE_INC_VERSDLG_HXX
#define INCLUDED_SFX2_SOURCE_INC_VERSDLG_HXX

#include <sfx2/basedlgs.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/svtabbx.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/weld.hxx>

class SfxViewFrame;
struct SfxVersionInfo;

class SfxVersionsTabListBox_Impl : public SvSimpleTable
{
public:
    SfxVersionsTabListBox_Impl(SvSimpleTableContainer& rParent, WinBits nBits)
        : SvSimpleTable(rParent, nBits)
    {
    }
    void setColSizes();
    virtual void Resize() override;
    virtual void KeyInput(const KeyEvent& rKeyEvent) override;
};

class SfxVersionTableDtor;
class SfxVersionDialog : public SfxModalDialog
{
    VclPtr<PushButton>                 m_pSaveButton;
    VclPtr<CheckBox>                   m_pSaveCheckBox;
    VclPtr<SfxVersionsTabListBox_Impl> m_pVersionBox;
    VclPtr<PushButton>                 m_pOpenButton;
    VclPtr<PushButton>                 m_pViewButton;
    VclPtr<PushButton>                 m_pDeleteButton;
    VclPtr<PushButton>                 m_pCompareButton;
    VclPtr<PushButton>                 m_pCmisButton;
    SfxViewFrame*               pViewFrame;
    std::unique_ptr<SfxVersionTableDtor> m_pTable;
    bool                        m_bIsSaveVersionOnClose;

    DECL_LINK(            DClickHdl_Impl, SvTreeListBox*, bool);
    DECL_LINK(            SelectHdl_Impl, SvTreeListBox*, void);
    DECL_LINK(            ButtonHdl_Impl, Button*, void );
    void                        Init_Impl();
    void                        Open_Impl();

public:
                                SfxVersionDialog ( SfxViewFrame* pFrame, bool );
    virtual                     ~SfxVersionDialog () override;
    virtual void                dispose() override;
    bool                        IsSaveVersionOnClose() const { return m_bIsSaveVersionOnClose; }
};

class SfxViewVersionDialog_Impl : public weld::GenericDialogController
{
private:
    SfxVersionInfo&   m_rInfo;

    std::unique_ptr<weld::Label> m_xDateTimeText;
    std::unique_ptr<weld::Label> m_xSavedByText;
    std::unique_ptr<weld::TextView> m_xEdit;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Button> m_xCancelButton;
    std::unique_ptr<weld::Button> m_xCloseButton;

    DECL_LINK(ButtonHdl, weld::Button&, void);

public:
    SfxViewVersionDialog_Impl(weld::Window *pParent, SfxVersionInfo& rInfo, bool bEdit);
};

class SfxCmisVersionsDialog : public SfxModalDialog
{
    VclPtr<SfxVersionsTabListBox_Impl> m_pVersionBox;
    VclPtr<PushButton>                 m_pOpenButton;
    VclPtr<PushButton>                 m_pViewButton;
    VclPtr<PushButton>                 m_pDeleteButton;
    VclPtr<PushButton>                 m_pCompareButton;
    SfxViewFrame*               pViewFrame;
    std::unique_ptr<SfxVersionTableDtor> m_pTable;

    void                        LoadVersions();

public:
                                SfxCmisVersionsDialog ( SfxViewFrame* pFrame );
    virtual                     ~SfxCmisVersionsDialog () override;
    virtual void                dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
