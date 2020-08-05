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

#include <RangeSelectionListener.hxx>

#include <vcl/wizardmachine.hxx>

namespace chart { class TabPageNotifiable; }
namespace com::sun::star::chart2 { class XChartTypeTemplate; }

namespace chart
{

class ChartTypeTemplateProvider;
class DialogModel;

class RangeChooserTabPage final : public vcl::OWizardPage, public RangeSelectionListenerParent
{
public:

    RangeChooserTabPage(weld::Container* pPage, weld::DialogController* pController, DialogModel & rDialogModel,
                        ChartTypeTemplateProvider* pTemplateProvider,
                        bool bHideDescription = false);
    virtual ~RangeChooserTabPage() override;

    //RangeSelectionListenerParent
    virtual void listeningFinished( const OUString & rNewRange ) override;
    virtual void disposingRangeSelection() override;

    virtual void Activate() override;

    void commitPage();

private:

    //OWizardPage
    virtual bool commitPage( ::vcl::WizardTypes::CommitPageReason eReason ) override;

    //TabPage
    virtual void Deactivate() override;

    void initControlsFromModel();
    void changeDialogModelAccordingToControls();
    bool isValid();
    void setDirty();

    DECL_LINK( ChooseRangeHdl, weld::Button&, void );
    DECL_LINK( ControlChangedHdl, weld::Entry&, void );
    DECL_LINK( ControlChangedCheckBoxHdl, weld::ToggleButton&, void );
    DECL_LINK( ControlChangedRadioHdl, weld::ToggleButton&, void );
    DECL_LINK( ControlEditedHdl, weld::Entry&, void );

    sal_Int32       m_nChangingControlCalls;
    bool            m_bIsDirty;

    OUString                                                m_aLastValidRangeString;
    css::uno::Reference< css::chart2::XChartTypeTemplate >  m_xCurrentChartTypeTemplate;
    ChartTypeTemplateProvider*                              m_pTemplateProvider;

    DialogModel &                                           m_rDialogModel;
    TabPageNotifiable *                                     m_pTabPageNotifiable;

    std::unique_ptr<weld::Label> m_xFT_Caption;
    std::unique_ptr<weld::Label> m_xFT_Range;
    std::unique_ptr<weld::Entry> m_xED_Range;
    std::unique_ptr<weld::Button> m_xIB_Range;
    std::unique_ptr<weld::RadioButton> m_xRB_Rows;
    std::unique_ptr<weld::RadioButton> m_xRB_Columns;
    std::unique_ptr<weld::CheckButton> m_xCB_FirstRowAsLabel;
    std::unique_ptr<weld::CheckButton> m_xCB_FirstColumnAsLabel;
    std::unique_ptr<weld::Label> m_xFTTitle;
    std::unique_ptr<weld::Widget> m_xFL_TimeBased;
    std::unique_ptr<weld::CheckButton> m_xCB_TimeBased;
    std::unique_ptr<weld::Label> m_xFT_TimeStart;
    std::unique_ptr<weld::Entry> m_xEd_TimeStart;
    std::unique_ptr<weld::Label> m_xFT_TimeEnd;
    std::unique_ptr<weld::Entry> m_xEd_TimeEnd;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
