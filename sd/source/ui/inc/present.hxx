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

#ifndef INCLUDED_SD_SOURCE_UI_INC_PRESENT_HXX
#define INCLUDED_SD_SOURCE_UI_INC_PRESENT_HXX

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>

class SfxItemSet;
class List;
class SdCustomShowList;

/**
 * Dialog to define options and to start the presentation
 */
class SdStartPresentationDlg : public ModalDialog
{
private:

    VclPtr<RadioButton>        aRbtAll;
    VclPtr<RadioButton>        aRbtAtDia;
    VclPtr<RadioButton>        aRbtCustomshow;
    VclPtr<ListBox>            aLbDias;
    VclPtr<ListBox>            aLbCustomshow;

    VclPtr<RadioButton>        aRbtStandard;
    VclPtr<RadioButton>        aRbtWindow;
    VclPtr<RadioButton>        aRbtAuto;
    VclPtr<TimeField>          aTmfPause;
    VclPtr<CheckBox>           aCbxAutoLogo;

    VclPtr<CheckBox>           aCbxManuel;
    VclPtr<CheckBox>           aCbxMousepointer;
    VclPtr<CheckBox>           aCbxPen;
    VclPtr<CheckBox>           aCbxNavigator;
    VclPtr<CheckBox>           aCbxAnimationAllowed;
    VclPtr<CheckBox>           aCbxChangePage;
    VclPtr<CheckBox>           aCbxAlwaysOnTop;

    VclPtr<FixedText>          maFtMonitor;
    VclPtr<ListBox>            maLBMonitor;

    SdCustomShowList*   pCustomShowList;
    const SfxItemSet&   rOutAttrs;
    sal_Int32           mnMonitors;

    VclPtr<FixedText>          msMonitor;
    VclPtr<FixedText>          msAllMonitors;
    VclPtr<FixedText>          msMonitorExternal;
    VclPtr<FixedText>          msExternal;

                        DECL_LINK_TYPED( ChangeRangeHdl, Button*, void );
                        DECL_LINK_TYPED( ClickWindowPresentationHdl, Button*, void );
                        DECL_LINK( ChangePauseHdl, void * );

    void                InitMonitorSettings();
    enum DisplayType {
        EXTERNAL_IS_NUMBER,
        MONITOR_NORMAL,
        MONITOR_IS_EXTERNAL,
    };
    sal_Int32           InsertDisplayEntry(const rtl::OUString &aName,
                                           sal_Int32            nDisplay);
    OUString            GetDisplayName( sal_Int32 nDisplay,
                                        DisplayType eType );
public:
                        SdStartPresentationDlg( vcl::Window* pWindow,
                                const SfxItemSet& rInAttrs,
                                const std::vector<OUString> &rPageNames,
                                SdCustomShowList* pCSList );
    virtual             ~SdStartPresentationDlg();
    virtual void        dispose() override;

    void                GetAttr( SfxItemSet& rOutAttrs );
};

#endif // INCLUDED_SD_SOURCE_UI_INC_PRESENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
