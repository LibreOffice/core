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
#ifndef CHART2_DATASOURCETABPAGE_HXX
#define CHART2_DATASOURCETABPAGE_HXX

#include <svtools/wizardmachine.hxx>

#include "DialogModel.hxx"
#include "TabPageNotifiable.hxx"

// header for class Edit
#include <vcl/edit.hxx>
// header for class ListBox
#include <vcl/lstbox.hxx>
// header for class PushButton, OKButton, CancelButton, HelpButton
#include <vcl/button.hxx>
// header for class FixedText, FixedLine
#include <vcl/fixed.hxx>
// header for class SvTabListBox
#include <svtools/svtabbx.hxx>
// header for class SvTreeListBox
#include <svtools/treelistbox.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/sheet/XRangeSelection.hpp>

#include <utility>
#include <vector>
#include <memory>

#include "RangeSelectionListener.hxx"
#include "RangeSelectionButton.hxx"
#include "RangeEdit.hxx"

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
        Window * pParent,
        DialogModel & rDialogModel,
        ChartTypeTemplateProvider* pTemplateProvider,
        Dialog * pParentDialog,
        bool bHideDescription = false );
    virtual ~DataSourceTabPage();

    void commitPage();

protected:
    // OWizardPage
    virtual void ActivatePage();
    virtual sal_Bool commitPage( ::svt::WizardTypes::CommitPageReason eReason );

    //TabPage
    virtual void DeactivatePage();

    virtual void        initializePage();

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
    virtual void listeningFinished( const ::rtl::OUString & rNewRange );
    virtual void disposingRangeSelection();

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

    /** @return </sal_True>, if the edit field contains a valid range entry. if no
        XCellRangesAccess can be obtained, </sal_True> is returned.
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
    FixedText               m_aFT_CAPTION;
    FixedText               m_aFT_SERIES;
    ::std::auto_ptr< SvTreeListBox > m_apLB_SERIES;
    PushButton              m_aBTN_ADD;
    PushButton              m_aBTN_REMOVE;
    PushButton              m_aBTN_UP;
    PushButton              m_aBTN_DOWN;

    FixedText               m_aFT_ROLE;
    SvTabListBox            m_aLB_ROLE;
    FixedText               m_aFT_RANGE;
    RangeEdit               m_aEDT_RANGE;
    RangeSelectionButton    m_aIMB_RANGE_MAIN;

    FixedText               m_aFT_CATEGORIES;
    FixedText               m_aFT_DATALABELS;//used for xy charts
    RangeEdit               m_aEDT_CATEGORIES;
    RangeSelectionButton    m_aIMB_RANGE_CAT;

    ::rtl::OUString         m_aFixedTextRange;

    ChartTypeTemplateProvider * m_pTemplateProvider;
    DialogModel &               m_rDialogModel;
    Edit *                      m_pCurrentRangeChoosingField;
    bool                        m_bIsDirty;
    sal_Int32                   m_nLastChartTypeGroupIndex;

    Dialog *                    m_pParentDialog;
    TabPageNotifiable *         m_pTabPageNotifiable;
};

} //  namespace chart

// CHART2_DATASOURCETABPAGE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
