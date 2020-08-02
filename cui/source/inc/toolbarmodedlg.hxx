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

#ifndef INCLUDED_CUI_INC_TOOLBARMODEDIALOG_HXX
#define INCLUDED_CUI_INC_TOOLBARMODEDIALOG_HXX

#include <vcl/weld.hxx>

class ToolbarmodeDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Image> m_pImage;
    std::unique_ptr<weld::Button> m_pApply;
    std::unique_ptr<weld::RadioButton> m_pRadioButtons[9];
    std::unique_ptr<weld::RadioButton> m_pSingle;
    std::unique_ptr<weld::RadioButton> m_pSidebar;
    std::unique_ptr<weld::RadioButton> m_pNotebookBarTabbed;
    std::unique_ptr<weld::RadioButton> m_pNotebookBarTabbedCompact;
    std::unique_ptr<weld::RadioButton> m_pNotebookBarGroupedBarCompact;
    std::unique_ptr<weld::RadioButton> m_pNotebookBarGroupedBarFull;
    std::unique_ptr<weld::RadioButton> m_pNotebookBarSingle;
    std::unique_ptr<weld::RadioButton> m_pNotebookBarGroups;
    std::unique_ptr<weld::Label> m_pInfoLabel;

    void UpdateImage(OUString aFileName);
    DECL_LINK(SelectToolbarmode, weld::ToggleButton&, void);
    DECL_LINK(OnApplyClick, weld::Button&, void);

public:
    ToolbarmodeDialog(weld::Window* pWindow);
    virtual ~ToolbarmodeDialog() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
