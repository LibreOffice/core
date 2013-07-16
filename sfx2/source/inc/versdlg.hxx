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

#ifndef _VERSDLG_HXX
#define _VERSDLG_HXX

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
    virtual void Resize();
    virtual void KeyInput(const KeyEvent& rKeyEvent);
};

class SfxVersionTableDtor;
class SfxVersionDialog : public SfxModalDialog
{
    PushButton*                 m_pSaveButton;
    CheckBox*                   m_pSaveCheckBox;
    SfxVersionsTabListBox_Impl* m_pVersionBox;
    PushButton*                 m_pOpenButton;
    PushButton*                 m_pViewButton;
    PushButton*                 m_pDeleteButton;
    PushButton*                 m_pCompareButton;
    SfxViewFrame*               pViewFrame;
    SfxVersionTableDtor*        m_pTable;
    bool                        m_bIsSaveVersionOnClose;

    DECL_LINK(DClickHdl_Impl, void *);
    DECL_LINK(SelectHdl_Impl, void *);
    DECL_LINK(                  ButtonHdl_Impl, Button* );
    void                        Init_Impl();
    void                        Open_Impl();

public:
                                SfxVersionDialog ( SfxViewFrame* pFrame, sal_Bool );
    virtual                     ~SfxVersionDialog ();
    bool                        IsSaveVersionOnClose() const { return m_bIsSaveVersionOnClose; }
};

class SfxViewVersionDialog_Impl : public SfxModalDialog
{
    FixedText*        m_pDateTimeText;
    FixedText*        m_pSavedByText;
    VclMultiLineEdit* m_pEdit;
    OKButton*         m_pOKButton;
    CancelButton*     m_pCancelButton;
    CloseButton*      m_pCloseButton;
    SfxVersionInfo&   m_rInfo;

    DECL_LINK(ButtonHdl, Button*);

public:
    SfxViewVersionDialog_Impl(Window *pParent, SfxVersionInfo& rInfo, bool bEdit);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
