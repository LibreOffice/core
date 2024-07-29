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

#include <sfx2/tabdlg.hxx>
#include <sfx2/AccessibilityIssue.hxx>
#include <map>

class SvxAccessibilityOptionsTabPage : public SfxTabPage
{
    std::unique_ptr<weld::CheckButton> m_xAccessibilityTool;
    std::unique_ptr<weld::CheckButton> m_xTextSelectionInReadonly;
    std::unique_ptr<weld::Widget> m_xTextSelectionInReadonlyImg;
    std::unique_ptr<weld::ComboBox> m_xAnimatedGraphics;
    std::unique_ptr<weld::Widget> m_xAnimatedGraphicsImg;
    std::unique_ptr<weld::Label> m_xAnimatedGraphicsLabel;
    std::unique_ptr<weld::ComboBox> m_xAnimatedOthers;
    std::unique_ptr<weld::Widget> m_xAnimatedOthersImg;
    std::unique_ptr<weld::Label> m_xAnimatedOthersLabel;
    std::unique_ptr<weld::ComboBox> m_xAnimatedTexts;
    std::unique_ptr<weld::Widget> m_xAnimatedTextsImg;
    std::unique_ptr<weld::Label> m_xAnimatedTextsLabel;
    std::unique_ptr<weld::ComboBox> m_xHighContrast;
    std::unique_ptr<weld::Widget> m_xHighContrastImg;
    std::unique_ptr<weld::Label> m_xHighContrastLabel;
    std::unique_ptr<weld::CheckButton> m_xAutomaticFontColor;
    std::unique_ptr<weld::Widget> m_xAutomaticFontColorImg;
    std::unique_ptr<weld::CheckButton> m_xPagePreviews;
    std::unique_ptr<weld::Widget> m_xPagePreviewsImg;
    std::unique_ptr<weld::TreeView> m_xOptionsLB;
    std::unique_ptr<weld::Button> m_xDefaultPB;

    // saved options after "Reset"; used in "FillItemSet" for comparison
    std::map<sfx::AccessibilityIssueID, TriState> m_aSavedOptions;

    DECL_LINK(UseAsDefaultHdl, weld::Button&, void);

public:
    SvxAccessibilityOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SvxAccessibilityOptionsTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual OUString GetAllStrings() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
