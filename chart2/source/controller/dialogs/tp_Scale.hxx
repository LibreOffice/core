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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_SCALE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_SCALE_HXX

#include <sfx2/tabdlg.hxx>

namespace chart
{

class ScaleTabPage : public SfxTabPage
{
public:
    ScaleTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~ScaleTabPage() override;

    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* rInAttrs );
    virtual bool FillItemSet( SfxItemSet* rOutAttrs ) override;
    virtual void Reset( const SfxItemSet* rInAttrs ) override;
    using TabPage::DeactivatePage;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pItemSet ) override;

    void SetNumFormatter( SvNumberFormatter* pFormatter );
    void SetNumFormat();

    void ShowAxisOrigin( bool bShowOrigin );

private:
    double              fMin;
    double              fMax;
    double              fStepMain;
    sal_Int32           nStepHelp;
    double              fOrigin;
    sal_Int32           m_nTimeResolution;
    sal_Int32           m_nMainTimeUnit;
    sal_Int32           m_nHelpTimeUnit;
    int                 m_nAxisType;
    bool                m_bAllowDateAxis;
    SvNumberFormatter*  pNumFormatter;

    bool                m_bShowAxisOrigin;

    std::unique_ptr<weld::CheckButton> m_xCbxReverse;
    std::unique_ptr<weld::CheckButton> m_xCbxLogarithm;
    std::unique_ptr<weld::Widget> m_xBxType;
    std::unique_ptr<weld::ComboBox> m_xLB_AxisType;
    std::unique_ptr<weld::Widget> m_xBxMinMax;
    std::unique_ptr<weld::FormattedSpinButton> m_xFmtFldMin;
    std::unique_ptr<weld::CheckButton> m_xCbxAutoMin;
    std::unique_ptr<weld::FormattedSpinButton> m_xFmtFldMax;
    std::unique_ptr<weld::CheckButton> m_xCbxAutoMax;
    std::unique_ptr<weld::Widget> m_xBxResolution;
    std::unique_ptr<weld::ComboBox> m_xLB_TimeResolution;
    std::unique_ptr<weld::CheckButton> m_xCbx_AutoTimeResolution;
    std::unique_ptr<weld::Label> m_xTxtMain;
    std::unique_ptr<weld::FormattedSpinButton> m_xFmtFldStepMain;
    std::unique_ptr<weld::SpinButton> m_xMt_MainDateStep;
    std::unique_ptr<weld::ComboBox> m_xLB_MainTimeUnit;
    std::unique_ptr<weld::CheckButton> m_xCbxAutoStepMain;
    std::unique_ptr<weld::Label> m_xTxtHelpCount;
    std::unique_ptr<weld::Label> m_xTxtHelp;
    std::unique_ptr<weld::SpinButton> m_xMtStepHelp;
    std::unique_ptr<weld::ComboBox> m_xLB_HelpTimeUnit;
    std::unique_ptr<weld::CheckButton> m_xCbxAutoStepHelp;
    std::unique_ptr<weld::FormattedSpinButton> m_xFmtFldOrigin;
    std::unique_ptr<weld::CheckButton> m_xCbxAutoOrigin;
    std::unique_ptr<weld::Widget> m_xBxOrigin;

    void EnableControls();

    DECL_LINK(SelectAxisTypeHdl, weld::ComboBox&, void);
    DECL_LINK(EnableValueHdl, weld::ToggleButton&, void);

    /** shows a warning window due to an invalid input.

        @param pResIdMessage
            The resource identifier that represents the localized warning text.
            If this is nullptr, no warning is shown and false is returned.

        @param pControl
            If non-NULL, contains a pointer to the control in which the
            erroneous value was in. This method gives this control the focus
            and selects its content.

        @return false, if nResIdMessage was 0, true otherwise
     */
    bool ShowWarning(const char* pResIdMessage, weld::Widget* pControl);

    void HideAllControls();
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
