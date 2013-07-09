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

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/svmedit.hxx>
#include <sfx2/basedlgs.hxx>

class SfxViewFrame;
struct SfxVersionInfo;

class SfxVersionsTabListBox_Impl  : public SvTabListBox
{
    virtual void                KeyInput( const KeyEvent& rKeyEvent );

public:
                                SfxVersionsTabListBox_Impl(
                                    Window* pParent, const ResId& );
};

class SfxVersionTableDtor;
class SfxVersionDialog : public SfxModalDialog
{
    FixedLine                   aNewGroup;
    PushButton                  aSaveButton;
    CheckBox                    aSaveCheckBox;
    FixedLine                   aExistingGroup;
    FixedText                   aDateTimeText;
    FixedText                   aSavedByText;
    FixedText                   aCommentText;
    SfxVersionsTabListBox_Impl  aVersionBox;
    CancelButton                aCloseButton;
    PushButton                  aOpenButton;
    PushButton                  aViewButton;
    PushButton                  aDeleteButton;
    PushButton                  aCompareButton;
    HelpButton                  aHelpButton;
    SfxViewFrame*               pViewFrame;
    SfxVersionTableDtor*        mpTable;
    sal_Bool                    mbIsSaveVersionOnClose;

    DECL_LINK(DClickHdl_Impl, void *);
    DECL_LINK(SelectHdl_Impl, void *);
    DECL_LINK(                  ButtonHdl_Impl, Button* );
    void                        Init_Impl();
    void                        Open_Impl();
    void                        RecalcDateColumn();

public:
                                SfxVersionDialog ( SfxViewFrame* pFrame, sal_Bool );
    virtual                     ~SfxVersionDialog ();
    sal_Bool                    IsSaveVersionOnClose() const { return mbIsSaveVersionOnClose; }
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
