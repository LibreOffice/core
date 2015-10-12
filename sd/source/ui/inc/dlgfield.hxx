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

#ifndef INCLUDED_SD_SOURCE_UI_INC_DLGFIELD_HXX
#define INCLUDED_SD_SOURCE_UI_INC_DLGFIELD_HXX

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
    VclPtr<RadioButton> m_pRbtFix;
    VclPtr<RadioButton> m_pRbtVar;
    VclPtr<SvxLanguageBox> m_pLbLanguage;
    VclPtr<ListBox> m_pLbFormat;
    SfxItemSet          maInputSet;

    const SvxFieldData* pField;

    void                FillFormatList();
    void                FillControls();

    DECL_LINK_TYPED( LanguageChangeHdl, ListBox&, void );

public:
    SdModifyFieldDlg( vcl::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet );
    virtual ~SdModifyFieldDlg();
    virtual void dispose() override;

    SvxFieldData*       GetField();
    SfxItemSet          GetItemSet();
};

#endif // INCLUDED_SD_SOURCE_UI_INC_DLGFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
