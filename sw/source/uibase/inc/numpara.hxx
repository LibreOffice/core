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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NUMPARA_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NUMPARA_HXX

#include <sfx2/tabdlg.hxx>
#include <svl/style.hxx>

// with this TabPage numbering settings at the paragraph / paragraph style
// are performed.
class SwParagraphNumTabPage final : public SfxTabPage
{
    // #outlinelevel#
    const OUString msOutlineNumbering;

    bool                    m_bModified : 1;
    bool                    m_bCurNumrule : 1;

    std::unique_ptr<weld::Widget> m_xOutlineStartBX;
    std::unique_ptr<weld::ComboBox> m_xOutlineLvLB;
    std::unique_ptr<weld::Widget> m_xNumberStyleBX;
    std::unique_ptr<weld::ComboBox> m_xNumberStyleLB;
    std::unique_ptr<weld::Button> m_xEditNumStyleBtn;

    std::unique_ptr<weld::CheckButton> m_xNewStartCB;
    std::unique_ptr<weld::Widget> m_xNewStartBX;
    std::unique_ptr<weld::CheckButton> m_xNewStartNumberCB;
    std::unique_ptr<weld::SpinButton> m_xNewStartNF;

    std::unique_ptr<weld::Widget> m_xCountParaFram;
    std::unique_ptr<weld::CheckButton> m_xCountParaCB;
    std::unique_ptr<weld::CheckButton> m_xRestartParaCountCB;

    std::unique_ptr<weld::Widget> m_xRestartBX;
    std::unique_ptr<weld::SpinButton> m_xRestartNF;

    DECL_LINK(NewStartHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(StyleHdl_Impl, weld::ComboBox&,void);
    DECL_LINK(LineCountHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(EditNumStyleHdl_Impl, weld::Button&, void);
    DECL_LINK(EditNumStyleSelectHdl_Impl, weld::ComboBox&, void);

    static const WhichRangesContainer s_aPageRg;

    bool ExecuteEditNumStyle_Impl(sal_uInt16 nId, const OUString& rStr,
                                  SfxStyleFamily nFamily);

public:
    SwParagraphNumTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
    virtual ~SwParagraphNumTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                      const SfxItemSet* rSet );
    static WhichRangesContainer GetRanges() { return s_aPageRg; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                EnableNewStart();
    void                DisableOutline();
    void                DisableNumbering();

    weld::ComboBox& GetStyleBox() {return *m_xNumberStyleLB;};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
