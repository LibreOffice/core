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

#ifndef _CHART2_TP_RANGECHOOSER_HXX
#define _CHART2_TP_RANGECHOOSER_HXX

#include "RangeSelectionListener.hxx"
#include "RangeSelectionButton.hxx"
#include "TabPageNotifiable.hxx"
#include "RangeEdit.hxx"

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

    RangeChooserTabPage( Window* pParent
                , DialogModel & rDialogModel
                , ChartTypeTemplateProvider* pTemplateProvider
                , Dialog * pParentDialog
                , bool bHideDescription = false );
virtual ~RangeChooserTabPage();

    //RangeSelectionListenerParent
    virtual void listeningFinished( const OUString & rNewRange );
    virtual void disposingRangeSelection();

    void commitPage();

protected: //methods

    //OWizardPage
    virtual void ActivatePage();
    virtual sal_Bool commitPage( ::svt::WizardTypes::CommitPageReason eReason );

    //TabPage
    virtual void DeactivatePage();

    void initControlsFromModel();
    void changeDialogModelAccordingToControls();
    bool isValid();
    void setDirty();

    DECL_LINK( ChooseRangeHdl, void* );
    DECL_LINK( ControlChangedHdl, void* );
    DECL_LINK( ControlEditedHdl, void* );

protected: //member

    FixedText       m_aFT_Caption;
    FixedText       m_aFT_Range;
    RangeEdit       m_aED_Range;
    RangeSelectionButton     m_aIB_Range;

    RadioButton     m_aRB_Rows;
    RadioButton     m_aRB_Columns;

    CheckBox        m_aCB_FirstRowAsLabel;
    CheckBox        m_aCB_FirstColumnAsLabel;

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
    Dialog *                                                m_pParentDialog;
    TabPageNotifiable *                                     m_pTabPageNotifiable;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
