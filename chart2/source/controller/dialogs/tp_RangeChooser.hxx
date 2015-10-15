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

#include "RangeSelectionListener.hxx"
#include "TabPageNotifiable.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <svtools/wizardmachine.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <com/sun/star/sheet/XRangeSelection.hpp>

namespace chart
{

/**
*/
class ChartTypeTemplateProvider;
class DialogModel;

class RangeChooserTabPage : public svt::OWizardPage, public RangeSelectionListenerParent
{
public:

    RangeChooserTabPage( vcl::Window* pParent
                , DialogModel & rDialogModel
                , ChartTypeTemplateProvider* pTemplateProvider
                , Dialog * pParentDialog
                , bool bHideDescription = false );
    virtual ~RangeChooserTabPage();
    virtual void dispose() override;

    //RangeSelectionListenerParent
    virtual void listeningFinished( const OUString & rNewRange ) override;
    virtual void disposingRangeSelection() override;

    void commitPage();

protected: //methods

    //OWizardPage
    virtual void ActivatePage() override;
    virtual bool commitPage( ::svt::WizardTypes::CommitPageReason eReason ) override;

    //TabPage
    virtual void DeactivatePage() override;

    void initControlsFromModel();
    void changeDialogModelAccordingToControls();
    bool isValid();
    void setDirty();

    DECL_LINK_TYPED( ChooseRangeHdl, Button*, void );
    DECL_LINK_TYPED( ControlChangedHdl, Edit&, void );
    DECL_LINK_TYPED( ControlChangedCheckBoxHdl, CheckBox&, void );
    DECL_LINK_TYPED( ControlChangedRadioHdl, RadioButton&, void );
    DECL_LINK_TYPED( ControlEditedHdl, Edit&, void );

protected: //member

    VclPtr<FixedText>       m_pFT_Caption;
    VclPtr<FixedText>       m_pFT_Range;
    VclPtr<Edit>            m_pED_Range;
    VclPtr<PushButton>      m_pIB_Range;

    VclPtr<RadioButton>     m_pRB_Rows;
    VclPtr<RadioButton>     m_pRB_Columns;

    VclPtr<CheckBox>        m_pCB_FirstRowAsLabel;
    VclPtr<CheckBox>        m_pCB_FirstColumnAsLabel;
    VclPtr<FixedText>       m_pFTTitle;

    VclPtr<FixedLine>       m_pFL_TimeBased;
    VclPtr<CheckBox>        m_pCB_TimeBased;
    VclPtr<FixedText>       m_pFT_TimeStart;
    VclPtr<Edit>            m_pEd_TimeStart;
    VclPtr<FixedText>       m_pFT_TimeEnd;
    VclPtr<Edit>            m_pEd_TimeEnd;

    sal_Int32       m_nChangingControlCalls;
    bool            m_bIsDirty;

    ::com::sun::star::uno::Reference<
         ::com::sun::star::chart2::XChartDocument >         m_xChartDocument;
    ::com::sun::star::uno::Reference<
         ::com::sun::star::chart2::data::XDataProvider >    m_xDataProvider;

    OUString                                                m_aLastValidRangeString;
    ::com::sun::star::uno::Reference<
         ::com::sun::star::chart2::XChartTypeTemplate >     m_xCurrentChartTypeTemplate;
    ChartTypeTemplateProvider*                              m_pTemplateProvider;

    DialogModel &                                           m_rDialogModel;
    VclPtr<Dialog>                                          m_pParentDialog;
    TabPageNotifiable *                                     m_pTabPageNotifiable;

};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
