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

    RadioButton*        aRbtAll;
    RadioButton*        aRbtAtDia;
    RadioButton*        aRbtCustomshow;
    ListBox*            aLbDias;
    ListBox*            aLbCustomshow;

    RadioButton*        aRbtStandard;
    RadioButton*        aRbtWindow;
    RadioButton*        aRbtAuto;
    TimeField*          aTmfPause;
    CheckBox*           aCbxAutoLogo;

    CheckBox*           aCbxManuel;
    CheckBox*           aCbxMousepointer;
    CheckBox*           aCbxPen;
    CheckBox*           aCbxNavigator;
    CheckBox*           aCbxAnimationAllowed;
    CheckBox*           aCbxChangePage;
    CheckBox*           aCbxAlwaysOnTop;

    FixedText*          maFtMonitor;
    ListBox*            maLBMonitor;

    SdCustomShowList*   pCustomShowList;
    const SfxItemSet&   rOutAttrs;
    sal_Int32           mnMonitors;

    FixedText*          msMonitor;
    FixedText*          msAllMonitors;
    FixedText*          msMonitorExternal;
    FixedText*          msExternal;

                        DECL_LINK( ChangeRangeHdl, void * );
                        DECL_LINK( ClickWindowPresentationHdl, void * );
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

    void                GetAttr( SfxItemSet& rOutAttrs );
};

#endif // INCLUDED_SD_SOURCE_UI_INC_PRESENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
