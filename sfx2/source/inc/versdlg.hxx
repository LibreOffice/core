/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _VERSDLG_HXX
#define _VERSDLG_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/svmedit.hxx>
#include <sfx2/basedlgs.hxx>

class SfxObjectShell;
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
    LocaleDataWrapper*          mpLocaleWrapper;
    sal_Bool                    mbIsSaveVersionOnClose;

    DECL_LINK(                  DClickHdl_Impl, Control* );
    DECL_LINK(                  SelectHdl_Impl, Control* );
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
    FixedText                   aDateTimeText;
    FixedText                   aSavedByText;
    MultiLineEdit               aEdit;
    OKButton                    aOKButton;
    CancelButton                aCancelButton;
    PushButton                  aCloseButton;
    HelpButton                  aHelpButton;
    SfxVersionInfo*             pInfo;

    DECL_LINK(                  ButtonHdl, Button* );

public:
                                SfxViewVersionDialog_Impl( Window *pParent,
                                    SfxVersionInfo& rInfo, sal_Bool bEdit );
 };

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
