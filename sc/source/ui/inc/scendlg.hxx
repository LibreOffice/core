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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SCENDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SCENDLG_HXX

#include <vcl/weld.hxx>

enum class ScScenarioFlags;

class ColorListBox;

class ScNewScenarioDlg : public weld::GenericDialogController
{
public:
    ScNewScenarioDlg(weld::Window* pParent, const OUString& rName, bool bEdit, bool bSheetProtected);
    virtual ~ScNewScenarioDlg() override;
    void SetScenarioData( const OUString& rName, const OUString& rComment,
                          const Color& rColor, ScScenarioFlags nFlags );

    void GetScenarioData(OUString& rName, OUString& rComment,
                          Color& rColor, ScScenarioFlags &rFlags) const;

private:
    const OUString      aDefScenarioName;
    bool const                bIsEdit;
    std::unique_ptr<weld::Entry> m_xEdName;
    std::unique_ptr<weld::TextView> m_xEdComment;
    std::unique_ptr<weld::CheckButton> m_xCbShowFrame;
    std::unique_ptr<ColorListBox> m_xLbColor;
    std::unique_ptr<weld::CheckButton> m_xCbTwoWay;
    std::unique_ptr<weld::CheckButton> m_xCbCopyAll;
    std::unique_ptr<weld::CheckButton> m_xCbProtect;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Label> m_xAltTitle;
    std::unique_ptr<weld::Label> m_xCreatedFt;
    std::unique_ptr<weld::Label> m_xOnFt;

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(EnableHdl, weld::ToggleButton&, void);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_SCENDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
