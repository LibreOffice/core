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

#include <RangeSelectionListener.hxx>

namespace com { namespace sun { namespace star { namespace chart2 { class XChartType; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XDataSeries; } } } }

namespace chart { class TabPageNotifiable; }

namespace chart
{

class ChartTypeTemplateProvider;
class DialogModel;

class SeriesEntry
{
public:
    OUString m_sRole;

    /// the corresponding data series
    css::uno::Reference< css::chart2::XDataSeries > m_xDataSeries;

    /// the chart type that contains the series (via XDataSeriesContainer)
    css::uno::Reference< css::chart2::XChartType > m_xChartType;
};

class DataSourceTabPage final :
        public ::svt::OWizardPage,
        public RangeSelectionListenerParent
{
public:
    explicit DataSourceTabPage(TabPageParent pParent,
                               DialogModel & rDialogModel,
                               ChartTypeTemplateProvider* pTemplateProvider,
                               Dialog * pParentDialog,
                               bool bHideDescription = false);
    virtual ~DataSourceTabPage() override;

    virtual void ActivatePage() override;

    void commitPage();

private:
    // OWizardPage
    virtual void dispose() override;
    virtual bool commitPage( ::svt::WizardTypes::CommitPageReason eReason ) override;

    //TabPage
    virtual void DeactivatePage() override;

    virtual void        initializePage() override;

    DECL_LINK( SeriesSelectionChangedHdl, weld::TreeView&, void );
    DECL_LINK( RoleSelectionChangedHdl, weld::TreeView&, void );
    DECL_LINK( MainRangeButtonClickedHdl, weld::Button&, void );
    DECL_LINK( CategoriesRangeButtonClickedHdl, weld::Button&, void );
    DECL_LINK( AddButtonClickedHdl, weld::Button&, void );
    DECL_LINK( RemoveButtonClickedHdl, weld::Button&, void );
    DECL_LINK( RangeModifiedHdl, weld::Entry&, void );
    DECL_LINK( UpButtonClickedHdl, weld::Button&, void );
    DECL_LINK( DownButtonClickedHdl, weld::Button&, void );

    // ____ RangeSelectionListenerParent ____
    virtual void listeningFinished( const OUString & rNewRange ) override;
    virtual void disposingRangeSelection() override;

    void InsertRoleLBEntry(const OUString& rRole, const OUString& rRange);

    void updateControlState();

    /** updates the internal data according to the content of the given edit
        field.  If pField is 0, all relevant fields are used

        @return
            <TRUE/> if the text from the field is a valid format to the internal
            data was valid
     */
    bool updateModelFromControl(const weld::Entry* pField = nullptr);

    /** @return </sal_True>, if the edit field contains a valid range entry. If no
        XCellRangesAccess can be obtained, </sal_False> is returned.
     */
    bool isRangeFieldContentValid(weld::Entry& rEdit);

    /** @return </sal_True>, if the tab-page is in a consistent (commitable) state
     */
    bool isValid();
    void setDirty();

    void updateControlsFromDialogModel();

    void fillSeriesListBox();
    void fillRoleListBox();

    std::vector<std::unique_ptr<SeriesEntry>> m_aEntries;

    OUString       m_aFixedTextRange;

    ChartTypeTemplateProvider * m_pTemplateProvider;
    DialogModel &               m_rDialogModel;
    weld::Entry*                m_pCurrentRangeChoosingField;
    bool                        m_bIsDirty;

    VclPtr<Dialog>              m_pParentDialog;
    weld::DialogController*     m_pParentController;
    TabPageNotifiable *         m_pTabPageNotifiable;

    std::unique_ptr<weld::Label> m_xFT_CAPTION;
    std::unique_ptr<weld::Label> m_xFT_SERIES;
    std::unique_ptr<weld::TreeView> m_xLB_SERIES;
    std::unique_ptr<weld::Button> m_xBTN_ADD;
    std::unique_ptr<weld::Button> m_xBTN_REMOVE;
    std::unique_ptr<weld::Button> m_xBTN_UP;
    std::unique_ptr<weld::Button> m_xBTN_DOWN;
    std::unique_ptr<weld::Label> m_xFT_ROLE;
    std::unique_ptr<weld::TreeView> m_xLB_ROLE;
    std::unique_ptr<weld::Label> m_xFT_RANGE;
    std::unique_ptr<weld::Entry> m_xEDT_RANGE;
    std::unique_ptr<weld::Button> m_xIMB_RANGE_MAIN;
    std::unique_ptr<weld::Label> m_xFT_CATEGORIES;
    std::unique_ptr<weld::Label> m_xFT_DATALABELS;//used for xy charts
    std::unique_ptr<weld::Entry> m_xEDT_CATEGORIES;
    std::unique_ptr<weld::Button> m_xIMB_RANGE_CAT;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_DATASOURCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
