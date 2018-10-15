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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_RANGECHOOSER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_RANGECHOOSER_HXX

#include <RangeSelectionListener.hxx>
#include <TabPageNotifiable.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <svtools/wizardmachine.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <com/sun/star/sheet/XRangeSelection.hpp>

namespace chart
{

class ChartTypeTemplateProvider;
class DialogModel;

class RangeChooserTabPage final : public svt::OWizardPage, public RangeSelectionListenerParent
{
public:

    RangeChooserTabPage(TabPageParent pParent, DialogModel & rDialogModel,
                        ChartTypeTemplateProvider* pTemplateProvider,
                        Dialog * pParentDialog, bool bHideDescription = false);
    virtual ~RangeChooserTabPage() override;
    virtual void dispose() override;

    //RangeSelectionListenerParent
    virtual void listeningFinished( const OUString & rNewRange ) override;
    virtual void disposingRangeSelection() override;

    void commitPage();

private:

    //OWizardPage
    virtual void ActivatePage() override;
    virtual bool commitPage( ::svt::WizardTypes::CommitPageReason eReason ) override;

    //TabPage
    virtual void DeactivatePage() override;

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
    VclPtr<Dialog>                                          m_pParentDialog;
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
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
