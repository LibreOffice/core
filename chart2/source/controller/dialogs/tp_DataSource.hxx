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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_DATASOURCE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_DATASOURCE_HXX

#include <svtools/wizardmachine.hxx>

#include "DialogModel.hxx"
#include "TabPageNotifiable.hxx"

#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/treelistbox.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/sheet/XRangeSelection.hpp>

#include <utility>
#include <vector>

#include "RangeSelectionListener.hxx"

namespace chart
{

class ChartTypeTemplateProvider;
class DialogModel;

class DataSourceTabPage :
        public ::svt::OWizardPage,
        public RangeSelectionListenerParent
{
public:
    explicit DataSourceTabPage(
        vcl::Window * pParent,
        DialogModel & rDialogModel,
        ChartTypeTemplateProvider* pTemplateProvider,
        Dialog * pParentDialog,
        bool bHideDescription = false );
    virtual ~DataSourceTabPage();
    virtual void dispose() SAL_OVERRIDE;

    void commitPage();

protected:
    // OWizardPage
    virtual void ActivatePage() SAL_OVERRIDE;
    virtual bool commitPage( ::svt::WizardTypes::CommitPageReason eReason ) SAL_OVERRIDE;

    //TabPage
    virtual void DeactivatePage() SAL_OVERRIDE;

    virtual void        initializePage() SAL_OVERRIDE;

    DECL_LINK( SeriesSelectionChangedHdl, void* );
    DECL_LINK( RoleSelectionChangedHdl, void* );
    DECL_LINK( MainRangeButtonClickedHdl, void* );
    DECL_LINK( CategoriesRangeButtonClickedHdl, void* );
    DECL_LINK( AddButtonClickedHdl, void* );
    DECL_LINK( RemoveButtonClickedHdl, void* );
    DECL_LINK( RangeModifiedHdl, Edit* );
    DECL_LINK( RangeUpdateDataHdl, Edit* );
    DECL_LINK( UpButtonClickedHdl, void* );
    DECL_LINK( DownButtonClickedHdl, void* );

    // ____ RangeSelectionListenerParent ____
    virtual void listeningFinished( const OUString & rNewRange ) SAL_OVERRIDE;
    virtual void disposingRangeSelection() SAL_OVERRIDE;

    void updateControlState();

protected:
    /** updates the internal data according to the content of the given edit
        field.  If pField is 0, all relevant fields are used

        @return
            <TRUE/> if the text from the field is a valid format to the internal
            data was valid
     */
    bool updateModelFromControl( Edit * pField = 0 );

    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelectionListener >
        getSelectionRangeListener();

    /** @return </sal_True>, if the edit field contains a valid range entry. If no
        XCellRangesAccess can be obtained, </sal_False> is returned.
     */
    bool isRangeFieldContentValid( Edit & rEdit );

    /** @return </sal_True>, if the tab-page is in a consistent (commitable) state
     */
    bool isValid();
    void setDirty();

    void updateControlsFromDialogModel();

    void fillSeriesListBox();
    void fillRoleListBox();

private:
    VclPtr<FixedText>     m_pFT_CAPTION;
    VclPtr<FixedText>     m_pFT_SERIES;
    VclPtr<SvTreeListBox> m_pLB_SERIES;
    VclPtr<PushButton>    m_pBTN_ADD;
    VclPtr<PushButton>    m_pBTN_REMOVE;
    VclPtr<PushButton>    m_pBTN_UP;
    VclPtr<PushButton>    m_pBTN_DOWN;

    VclPtr<FixedText>     m_pFT_ROLE;
    VclPtr<SvTabListBox>  m_pLB_ROLE;
    VclPtr<FixedText>     m_pFT_RANGE;
    VclPtr<Edit>          m_pEDT_RANGE;
    VclPtr<PushButton>    m_pIMB_RANGE_MAIN;

    VclPtr<FixedText>     m_pFT_CATEGORIES;
    VclPtr<FixedText>     m_pFT_DATALABELS;//used for xy charts
    VclPtr<Edit>          m_pEDT_CATEGORIES;
    VclPtr<PushButton>    m_pIMB_RANGE_CAT;

    OUString       m_aFixedTextRange;

    ChartTypeTemplateProvider * m_pTemplateProvider;
    DialogModel &               m_rDialogModel;
    VclPtr<Edit>                m_pCurrentRangeChoosingField;
    bool                        m_bIsDirty;
    sal_Int32                   m_nLastChartTypeGroupIndex;

    VclPtr<Dialog>              m_pParentDialog;
    TabPageNotifiable *         m_pTabPageNotifiable;
};

}

// INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_DATASOURCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
