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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPTABLE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPTABLE_HXX

#include <sfx2/tabdlg.hxx>

class ScTablePage : public SfxTabPage
{
    friend class VclPtr<ScTablePage>;
    static const sal_uInt16 pPageTableRanges[];
public:
    static  VclPtr<SfxTabPage> Create          ( TabPageParent pParent,
                                          const SfxItemSet* rCoreSet );
    static  const sal_uInt16* GetRanges () { return pPageTableRanges; }
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;
    virtual void        DataChanged     ( const DataChangedEvent& rDCEvt ) override;

    virtual         ~ScTablePage() override;
private:
    ScTablePage(TabPageParent pParent, const SfxItemSet& rCoreSet);
    void            ShowImage();

private:
    sal_uInt16              m_nOrigScalePageWidth;
    sal_uInt16              m_nOrigScalePageHeight;

    std::unique_ptr<weld::RadioButton> m_xBtnTopDown;
    std::unique_ptr<weld::RadioButton> m_xBtnLeftRight;
    std::unique_ptr<weld::Image> m_xBmpPageDir;
    std::unique_ptr<weld::CheckButton> m_xBtnPageNo;
    std::unique_ptr<weld::SpinButton> m_xEdPageNo;

    std::unique_ptr<weld::CheckButton> m_xBtnHeaders;
    std::unique_ptr<weld::CheckButton> m_xBtnGrid;
    std::unique_ptr<weld::CheckButton> m_xBtnNotes;
    std::unique_ptr<weld::CheckButton> m_xBtnObjects;
    std::unique_ptr<weld::CheckButton> m_xBtnCharts;
    std::unique_ptr<weld::CheckButton> m_xBtnDrawings;
    std::unique_ptr<weld::CheckButton> m_xBtnFormulas;
    std::unique_ptr<weld::CheckButton> m_xBtnNullVals;

    std::unique_ptr<weld::ComboBox> m_xLbScaleMode;
    std::unique_ptr<weld::Widget> m_xBxScaleAll;
    std::unique_ptr<weld::MetricSpinButton> m_xEdScaleAll;
    std::unique_ptr<weld::Widget> m_xGrHeightWidth;
    std::unique_ptr<weld::SpinButton> m_xEdScalePageWidth;
    std::unique_ptr<weld::CheckButton> m_xCbScalePageWidth;
    std::unique_ptr<weld::SpinButton> m_xEdScalePageHeight;
    std::unique_ptr<weld::CheckButton> m_xCbScalePageHeight;
    std::unique_ptr<weld::Widget> m_xBxScalePageNum;
    std::unique_ptr<weld::SpinButton> m_xEdScalePageNum;

private:

    // Handler:
    DECL_LINK(PageDirHdl, weld::ToggleButton&, void);
    DECL_LINK(PageNoHdl, weld::ToggleButton&, void);
    void PageNoHdl(const weld::ToggleButton* pBtn);
    DECL_LINK(ScaleHdl, weld::ComboBox&, void);
    DECL_LINK(ToggleHdl, weld::ToggleButton&, void);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
