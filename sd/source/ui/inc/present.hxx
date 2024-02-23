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

#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>

class SfxItemSet;
class SdCustomShowList;

/**
 * Dialog to define optionsm_xnd to start the presentation
 */
class SdStartPresentationDlg : public weld::GenericDialogController
{
private:
    SdCustomShowList*   pCustomShowList;
    const SfxItemSet&   rOutAttrs;
    sal_Int32           mnMonitors;

    std::unique_ptr<weld::RadioButton> m_xRbtAll;
    std::unique_ptr<weld::RadioButton> m_xRbtAtDia;
    std::unique_ptr<weld::RadioButton> m_xRbtCustomshow;
    std::unique_ptr<weld::ComboBox> m_xLbDias;
    std::unique_ptr<weld::ComboBox> m_xLbCustomshow;

    std::unique_ptr<weld::RadioButton> m_xRbtStandard;
    std::unique_ptr<weld::RadioButton> m_xRbtWindow;
    std::unique_ptr<weld::RadioButton> m_xRbtAuto;
    std::unique_ptr<weld::FormattedSpinButton> m_xTmfPause;
    std::unique_ptr<weld::TimeFormatter> m_xFormatter;
    std::unique_ptr<weld::CheckButton> m_xCbxAutoLogo;

    std::unique_ptr<weld::CheckButton> m_xCbxManuel;
    std::unique_ptr<weld::CheckButton> m_xCbxMousepointer;
    std::unique_ptr<weld::CheckButton> m_xCbxPen;
    std::unique_ptr<weld::CheckButton> m_xCbxAnimationAllowed;
    std::unique_ptr<weld::CheckButton> m_xCbxChangePage;
    std::unique_ptr<weld::CheckButton> m_xCbxAlwaysOnTop;
    std::unique_ptr<weld::CheckButton> m_xCbxShowNavigationButton;
    std::unique_ptr<weld::ComboBox>    m_xLbNavigationButtonsSize;
    std::unique_ptr<weld::Label>       m_xFtNavigationButtonsSize;
    std::unique_ptr<weld::Frame> m_xFrameEnableRemote;
    std::unique_ptr<weld::CheckButton> m_xCbxEnableRemote;
    std::unique_ptr<weld::CheckButton> m_xCbxEnableRemoteInsecure;
    std::unique_ptr<weld::CheckButton> m_xCbxInteractiveMode;

    std::unique_ptr<weld::ComboBox> m_xLbConsole;

    std::unique_ptr<weld::Label> m_xFtMonitor;
    std::unique_ptr<weld::ComboBox> m_xLBMonitor;

    std::unique_ptr<weld::Label> m_xMonitor;
    std::unique_ptr<weld::Label> m_xAllMonitors;
    std::unique_ptr<weld::Label> m_xMonitorExternal;
    std::unique_ptr<weld::Label> m_xExternal;

    DECL_LINK(ChangeRemoteHdl, weld::Toggleable&, void);
    DECL_LINK(ChangeRangeHdl, weld::Toggleable&, void);
    DECL_LINK(ClickWindowPresentationHdl, weld::Toggleable&, void);
    void ChangePause();
    DECL_LINK(ChangePauseHdl, weld::FormattedSpinButton&, void);

    void                InitMonitorSettings();
    enum DisplayType {
        EXTERNAL_IS_NUMBER,
        MONITOR_NORMAL,
        MONITOR_IS_EXTERNAL,
    };
    sal_Int32           InsertDisplayEntry(const OUString &aName,
                                           sal_Int32            nDisplay);
    OUString            GetDisplayName( sal_Int32 nDisplay,
                                        DisplayType eType );
public:
                        SdStartPresentationDlg(weld::Window* pWindow,
                                               const SfxItemSet& rInAttrs,
                                               const std::vector<OUString> &rPageNames,
                                               SdCustomShowList* pCSList);
    virtual             ~SdStartPresentationDlg() override;
    virtual short run() override;
    void                GetAttr( SfxItemSet& rOutAttrs );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
