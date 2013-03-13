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


#ifndef _SD_DLGFIELD_HXX
#define _SD_DLGFIELD_HXX


#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <svx/langbox.hxx>
class SvxFieldData;

/**
 * dialog to adjust field-commands
 */
class SdModifyFieldDlg : public ModalDialog
{
private:
    FixedLine           aGrpType;
    RadioButton         aRbtFix;
    RadioButton         aRbtVar;
    FixedText           maFtLanguage;
    SvxLanguageBox      maLbLanguage;
    FixedText           aFtFormat;
    ListBox             aLbFormat;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    SfxItemSet          maInputSet;

    const SvxFieldData* pField;

    void                FillFormatList();
    void                FillControls();

    DECL_LINK( LanguageChangeHdl, void * );

public:
    SdModifyFieldDlg( Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet );
    ~SdModifyFieldDlg() {}

    SvxFieldData*       GetField();
    SfxItemSet          GetItemSet();
};



#endif      // _SD_DLGFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
