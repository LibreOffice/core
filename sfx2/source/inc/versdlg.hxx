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
    SfxVersionTableDtor*        m_pTable;
    bool                        m_bIsSaveVersionOnClose;

    DECL_LINK_TYPED(            DClickHdl_Impl, SvTreeListBox*, bool);
    DECL_LINK_TYPED(            SelectHdl_Impl, SvTreeListBox*, void);
    DECL_LINK_TYPED(            ButtonHdl_Impl, Button*, void );
    void                        Init_Impl();
    void                        Open_Impl();

public:
                                SfxVersionDialog ( SfxViewFrame* pFrame, bool );
    virtual                     ~SfxVersionDialog ();
    virtual void                dispose() override;
    bool                        IsSaveVersionOnClose() const { return m_bIsSaveVersionOnClose; }
};

class SfxViewVersionDialog_Impl : public SfxModalDialog
{
    VclPtr<FixedText>        m_pDateTimeText;
    VclPtr<FixedText>        m_pSavedByText;
    VclPtr<VclMultiLineEdit> m_pEdit;
    VclPtr<OKButton>         m_pOKButton;
    VclPtr<CancelButton>     m_pCancelButton;
    VclPtr<CloseButton>      m_pCloseButton;
    SfxVersionInfo&   m_rInfo;

    DECL_LINK_TYPED(ButtonHdl, Button*, void);

public:
    SfxViewVersionDialog_Impl(vcl::Window *pParent, SfxVersionInfo& rInfo, bool bEdit);
    virtual ~SfxViewVersionDialog_Impl();
    virtual void dispose() override;
};

class SfxCmisVersionsDialog : public SfxModalDialog
{
    VclPtr<SfxVersionsTabListBox_Impl> m_pVersionBox;
    VclPtr<PushButton>                 m_pOpenButton;
    VclPtr<PushButton>                 m_pViewButton;
    VclPtr<PushButton>                 m_pDeleteButton;
    VclPtr<PushButton>                 m_pCompareButton;
    SfxViewFrame*               pViewFrame;
    SfxVersionTableDtor*        m_pTable;

    void                        LoadVersions();

public:
                                SfxCmisVersionsDialog ( SfxViewFrame* pFrame );
    virtual                     ~SfxCmisVersionsDialog ();
    virtual void                dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
