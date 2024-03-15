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

#include "tp_DataSource.hxx"
#include <strings.hrc>
#include <ResId.hxx>
#include <ChartType.hxx>
#include <ChartTypeTemplateProvider.hxx>
#include <ChartTypeTemplate.hxx>
#include <ChartModel.hxx>
#include <RangeSelectionHelper.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <DataSourceHelper.hxx>
#include <LabeledDataSequence.hxx>
#include "DialogModel.hxx"
#include <o3tl/safeint.hxx>
#include <TabPageNotifiable.hxx>
#include <com/sun/star/chart2/data/XDataProvider.hpp>

#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;

namespace
{

constexpr OUString lcl_aLabelRole( u"label"_ustr );

void lcl_UpdateCurrentRange(weld::TreeView& rOutListBox, const OUString & rRole,
                            const OUString& rRange)
{
    int nEntry = rOutListBox.get_selected_index();
    if (nEntry != -1)
    {
        rOutListBox.set_text(nEntry, ::chart::DialogModel::ConvertRoleFromInternalToUI(rRole), 0);
        rOutListBox.set_text(nEntry, rRange, 1);
        ::chart::SeriesEntry* pEntry = weld::fromId<::chart::SeriesEntry*>(rOutListBox.get_id(nEntry));
        pEntry->m_sRole = rRole;
    }
}

bool lcl_UpdateCurrentSeriesName(weld::TreeView& rOutListBox)
{
    int nEntry = rOutListBox.get_selected_index();
    if (nEntry == -1)
        return false;

    bool bResult = false;
    ::chart::SeriesEntry * pEntry = weld::fromId<::chart::SeriesEntry*>(rOutListBox.get_id(nEntry));
    if (pEntry->m_xDataSeries.is() && pEntry->m_xChartType.is())
    {
        OUString aLabel(pEntry->m_xDataSeries->getLabelForRole(
                        pEntry->m_xChartType->getRoleOfSequenceForSeriesLabel()));
        if (!aLabel.isEmpty())
        {
            rOutListBox.set_text(nEntry, aLabel);
            bResult = true;
        }
    }
    return bResult;
}

OUString lcl_GetSelectedRole(const weld::TreeView& rRoleListBox, bool bUITranslated = false)
{
    int nEntry = rRoleListBox.get_selected_index();
    if (nEntry != -1)
    {
        if (bUITranslated)
            return rRoleListBox.get_text(nEntry);
        ::chart::SeriesEntry* pEntry = weld::fromId<::chart::SeriesEntry*>(rRoleListBox.get_id(nEntry));
        return pEntry->m_sRole;
    }
    return OUString();
}

OUString lcl_GetSelectedRolesRange( const weld::TreeView& rRoleListBox )
{
    OUString aResult;
    int nEntry = rRoleListBox.get_selected_index();
    if (nEntry != -1)
        aResult = rRoleListBox.get_text(nEntry, 1);
    return aResult;
}

OUString lcl_GetSequenceNameForLabel(const ::chart::SeriesEntry* pEntry)
{
    OUString aResult("values-y");
    if (pEntry && pEntry->m_xChartType.is())
        aResult = pEntry->m_xChartType->getRoleOfSequenceForSeriesLabel();
    return aResult;
}

void lcl_enableRangeChoosing(bool bEnable, weld::DialogController* pDialog)
{
    if (!pDialog)
        return;
    weld::Dialog* pDlg = pDialog->getDialog();
    pDlg->set_modal(!bEnable);
    pDlg->set_visible(!bEnable);
}

void lcl_addLSequenceToDataSource(
    const uno::Reference< chart2::data::XLabeledDataSequence > & xLSequence,
    const Reference< ::chart::DataSeries > & xSource )
{
    if( xSource.is())
    {
        std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aData = xSource->getDataSequences2();
        aData.push_back( xLSequence );
        xSource->setData( aData );
    }
}

uno::Reference< chart2::data::XLabeledDataSequence > lcl_findLSequenceWithOnlyLabel(
    const rtl::Reference< ::chart::DataSeries > & xDataSource )
{
    uno::Reference< chart2::data::XLabeledDataSequence > xResult;

    for( uno::Reference< chart2::data::XLabeledDataSequence > const & labeledDataSeq : xDataSource->getDataSequences2() )
    {
        // no values are set but a label exists
        if( ! labeledDataSeq->getValues().is() &&
            labeledDataSeq->getLabel().is())
        {
            xResult = labeledDataSeq;
            break;
        }
    }

    return xResult;
}

} //  anonymous namespace

namespace chart
{

DataSourceTabPage::DataSourceTabPage(weld::Container* pPage, weld::DialogController* pController,
                                     DialogModel & rDialogModel,
                                     ChartTypeTemplateProvider* pTemplateProvider,
                                     bool bHideDescription /* = false */)
    : ::vcl::OWizardPage(pPage, pController, "modules/schart/ui/tp_DataSource.ui", "tp_DataSource")
    , m_pTemplateProvider(pTemplateProvider)
    , m_rDialogModel(rDialogModel)
    , m_pCurrentRangeChoosingField( nullptr )
    , m_bIsDirty( false )
    , m_pTabPageNotifiable(dynamic_cast<TabPageNotifiable*>(pController))
    , m_xFT_CAPTION(m_xBuilder->weld_label("FT_CAPTION_FOR_WIZARD"))
    , m_xFT_SERIES(m_xBuilder->weld_label("FT_SERIES"))
    , m_xLB_SERIES(m_xBuilder->weld_tree_view("LB_SERIES"))
    , m_xBTN_ADD(m_xBuilder->weld_button("BTN_ADD"))
    , m_xBTN_REMOVE(m_xBuilder->weld_button("BTN_REMOVE"))
    , m_xBTN_UP(m_xBuilder->weld_button("BTN_UP"))
    , m_xBTN_DOWN(m_xBuilder->weld_button("BTN_DOWN"))
    , m_xFT_ROLE(m_xBuilder->weld_label("FT_ROLE"))
    , m_xLB_ROLE(m_xBuilder->weld_tree_view("LB_ROLE"))
    , m_xFT_RANGE(m_xBuilder->weld_label("FT_RANGE"))
    , m_xEDT_RANGE(m_xBuilder->weld_entry("EDT_RANGE"))
    , m_xIMB_RANGE_MAIN(m_xBuilder->weld_button("IMB_RANGE_MAIN"))
    , m_xFT_CATEGORIES(m_xBuilder->weld_label("FT_CATEGORIES"))
    , m_xFT_DATALABELS(m_xBuilder->weld_label("FT_DATALABELS"))
    , m_xEDT_CATEGORIES(m_xBuilder->weld_entry("EDT_CATEGORIES"))
    , m_xIMB_RANGE_CAT(m_xBuilder->weld_button("IMB_RANGE_CAT"))
{
    m_xLB_SERIES->set_size_request(m_xLB_SERIES->get_approximate_digit_width() * 25,
                                   m_xLB_SERIES->get_height_rows(10));
    m_xLB_ROLE->set_size_request(m_xLB_ROLE->get_approximate_digit_width() * 60,
                                 m_xLB_ROLE->get_height_rows(5));
    m_xFT_CAPTION->set_visible(!bHideDescription);

    m_aFixedTextRange = m_xFT_RANGE->get_label();
    SetPageTitle(SchResId(STR_OBJECT_DATASERIES_PLURAL));

    // set handlers
    m_xLB_SERIES->connect_changed(LINK(this, DataSourceTabPage, SeriesSelectionChangedHdl));
    m_xLB_ROLE->connect_changed(LINK(this, DataSourceTabPage, RoleSelectionChangedHdl));

    m_xIMB_RANGE_MAIN->connect_clicked(LINK(this, DataSourceTabPage, MainRangeButtonClickedHdl));
    m_xIMB_RANGE_CAT->connect_clicked(LINK(this, DataSourceTabPage, CategoriesRangeButtonClickedHdl));

    m_xBTN_ADD->connect_clicked(LINK(this, DataSourceTabPage, AddButtonClickedHdl));
    m_xBTN_REMOVE->connect_clicked(LINK(this, DataSourceTabPage, RemoveButtonClickedHdl));

    m_xBTN_UP->connect_clicked(LINK(this, DataSourceTabPage, UpButtonClickedHdl));
    m_xBTN_DOWN->connect_clicked(LINK(this, DataSourceTabPage, DownButtonClickedHdl));

    m_xEDT_RANGE->connect_changed(LINK(this, DataSourceTabPage, RangeModifiedHdl));
    m_xEDT_CATEGORIES->connect_changed(LINK( this, DataSourceTabPage, RangeModifiedHdl));

    // init controls
    std::vector<int> aWidths { o3tl::narrowing<int>(m_xLB_ROLE->get_approximate_digit_width() * 20) };
    m_xLB_ROLE->set_column_fixed_widths(aWidths);
    m_xLB_ROLE->show();

    updateControlsFromDialogModel();

    // select first series
    if (m_xLB_SERIES->n_children())
        m_xLB_SERIES->select(0);
}

void DataSourceTabPage::InsertRoleLBEntry(const OUString& rRole, const OUString& rRange)
{
    m_aEntries.emplace_back(new SeriesEntry);
    SeriesEntry* pEntry = m_aEntries.back().get();
    pEntry->m_sRole = rRole;
    m_xLB_ROLE->append(weld::toId(pEntry),
                       ::chart::DialogModel::ConvertRoleFromInternalToUI(rRole));
    m_xLB_ROLE->set_text(m_xLB_ROLE->n_children() - 1, rRange, 1);
}

DataSourceTabPage::~DataSourceTabPage()
{
}

void DataSourceTabPage::Activate()
{
    OWizardPage::Activate();
    updateControlsFromDialogModel();
    m_xLB_SERIES->grab_focus();
}

void DataSourceTabPage::initializePage()
{
}

void DataSourceTabPage::Deactivate()
{
    commitPage();
    vcl::OWizardPage::Deactivate();
}

void DataSourceTabPage::commitPage()
{
    commitPage(::vcl::WizardTypes::eFinish);
}

bool DataSourceTabPage::commitPage( ::vcl::WizardTypes::CommitPageReason /*eReason*/ )
{
    //ranges may have been edited in the meanwhile (dirty is true in that case here)
    if( isValid() )
    {
        updateModelFromControl();
        return true; //return false if this page should not be left
    }
    else
        return false;
}

bool DataSourceTabPage::isRangeFieldContentValid(weld::Entry& rEdit )
{
    OUString aRange(rEdit.get_text());
    bool bIsValid = aRange.isEmpty() ||
        m_rDialogModel.getRangeSelectionHelper()->verifyCellRange(aRange);
    rEdit.set_message_type(bIsValid ? weld::EntryMessageType::Normal : weld::EntryMessageType::Error);
    return bIsValid;
}

bool DataSourceTabPage::isValid()
{
    bool bRoleRangeValid = true;
    bool bCategoriesRangeValid = true;
    bool bHasSelectedEntry = (m_xLB_SERIES->get_selected_index() != -1);

    if (bHasSelectedEntry)
        bRoleRangeValid = isRangeFieldContentValid(*m_xEDT_RANGE);
    if (m_xEDT_CATEGORIES->get_sensitive())
        bCategoriesRangeValid = isRangeFieldContentValid( *m_xEDT_CATEGORIES );
    bool bValid = ( bRoleRangeValid && bCategoriesRangeValid );

    if( m_pTabPageNotifiable )
    {
        if( bValid )
            m_pTabPageNotifiable->setValidPage( this );
        else
            m_pTabPageNotifiable->setInvalidPage( this );
    }

    return bValid;
}

void DataSourceTabPage::setDirty()
{
    m_bIsDirty = true;
}

void DataSourceTabPage::updateControlsFromDialogModel()
{
    // series
    fillSeriesListBox();
    SeriesSelectionChangedHdl(*m_xLB_SERIES);

    // categories
    m_xEDT_CATEGORIES->set_text(m_rDialogModel.getCategoriesRange());

    updateControlState();
}

void DataSourceTabPage::fillSeriesListBox()
{
    rtl::Reference< DataSeries > xSelected;
    SeriesEntry* pEntry = nullptr;
    int nEntry = m_xLB_SERIES->get_selected_index();
    if (nEntry != -1)
    {
        pEntry = weld::fromId<SeriesEntry*>(m_xLB_SERIES->get_id(nEntry));
        xSelected = pEntry->m_xDataSeries;
    }

    bool bHasSelectedEntry = (pEntry != nullptr);
    int nSelectedEntry = -1;

    m_xLB_SERIES->freeze();
    m_xLB_SERIES->clear();

    std::vector< DialogModel::tSeriesWithChartTypeByName > aSeries(
        m_rDialogModel.getAllDataSeriesWithLabel() );

    sal_Int32 nUnnamedSeriesIndex = 1;
    nEntry = 0;
    for (auto const& series : aSeries)
    {
        OUString aLabel(series.first);
        if (aLabel.isEmpty())
        {
            if( nUnnamedSeriesIndex > 1 )
            {
                OUString aResString(::chart::SchResId( STR_DATA_UNNAMED_SERIES_WITH_INDEX ));

                // replace index of unnamed series
                static constexpr OUString aReplacementStr( u"%NUMBER"_ustr );
                sal_Int32 nIndex = aResString.indexOf( aReplacementStr );
                if( nIndex != -1 )
                    aLabel = aResString.replaceAt(
                                         nIndex, aReplacementStr.getLength(),
                                         OUString::number(nUnnamedSeriesIndex));
            }
            if( aLabel.isEmpty() )
                aLabel = ::chart::SchResId( STR_DATA_UNNAMED_SERIES );

            ++nUnnamedSeriesIndex;
        }

        m_aEntries.emplace_back(new SeriesEntry);
        pEntry = m_aEntries.back().get();
        pEntry->m_xDataSeries = series.second.first;
        pEntry->m_xChartType = series.second.second;
        m_xLB_SERIES->append(weld::toId(pEntry), aLabel);
        if (bHasSelectedEntry && series.second.first == xSelected)
            nSelectedEntry = nEntry;
        ++nEntry;
    }

    m_xLB_SERIES->thaw();

    if (bHasSelectedEntry && nSelectedEntry != -1)
        m_xLB_SERIES->select(nSelectedEntry);
}

void DataSourceTabPage::fillRoleListBox()
{
    int nSeriesEntry = m_xLB_SERIES->get_selected_index();
    SeriesEntry* pSeriesEntry = nullptr;
    if (nSeriesEntry != -1)
        pSeriesEntry = weld::fromId<SeriesEntry*>(m_xLB_SERIES->get_id(nSeriesEntry));
    bool bHasSelectedEntry = (pSeriesEntry != nullptr);

    int nRoleIndex = m_xLB_ROLE->get_selected_index();
    if (!bHasSelectedEntry)
        return;

    DialogModel::tRolesWithRanges aRoles(
        DialogModel::getRolesWithRanges(
            pSeriesEntry->m_xDataSeries,
            lcl_GetSequenceNameForLabel( pSeriesEntry ),
            pSeriesEntry->m_xChartType ));

    // fill role list
    m_xLB_ROLE->freeze();
    m_xLB_ROLE->clear();

    for (auto const& elemRole : aRoles)
    {
        InsertRoleLBEntry(elemRole.first, elemRole.second);
    }

    m_xLB_ROLE->thaw();

    // series may contain no roles, check listbox size before selecting entries
    if (m_xLB_ROLE->n_children() > 0)
    {
        if (nRoleIndex == -1 || nRoleIndex >= m_xLB_ROLE->n_children())
            nRoleIndex = 0;
        m_xLB_ROLE->select(nRoleIndex);
    }
}

void DataSourceTabPage::updateControlState()
{
    int nSeriesEntry = m_xLB_SERIES->get_selected_index();
    bool bHasSelectedSeries = nSeriesEntry != -1;
    bool bHasValidRole = false;
    bool bHasRangeChooser = m_rDialogModel.getRangeSelectionHelper()->hasRangeSelection();

    if( bHasSelectedSeries )
    {
        int nRoleEntry = m_xLB_ROLE->get_selected_index();
        bHasValidRole = nRoleEntry != -1;
    }

    m_xBTN_ADD->set_sensitive(true);
    m_xBTN_REMOVE->set_sensitive(bHasSelectedSeries);

    m_xBTN_UP->set_sensitive(bHasSelectedSeries && (nSeriesEntry != 0));
    m_xBTN_DOWN->set_sensitive(bHasSelectedSeries && (nSeriesEntry != m_xLB_SERIES->n_children() - 1));

    bool bHasCategories = m_rDialogModel.isCategoryDiagram();

    m_xFT_DATALABELS->set_visible(!bHasCategories);
    m_xFT_CATEGORIES->set_visible( bHasCategories);
    bool bShowIB = bHasRangeChooser;

    m_xIMB_RANGE_CAT->set_visible(bShowIB);

    m_xFT_ROLE->set_sensitive(bHasSelectedSeries);
    m_xLB_ROLE->set_sensitive(bHasSelectedSeries);

    m_xFT_RANGE->set_sensitive(bHasValidRole);
    m_xEDT_RANGE->set_sensitive(bHasValidRole);

    m_xFT_SERIES->set_sensitive(true);
    m_xLB_SERIES->set_sensitive(true);

    m_xIMB_RANGE_MAIN->set_visible(bShowIB);

    isValid();
}

IMPL_LINK_NOARG(DataSourceTabPage, SeriesSelectionChangedHdl, weld::TreeView&, void)
{
    m_rDialogModel.startControllerLockTimer();
    if (m_xLB_SERIES->get_selected_index() != -1)
    {
        fillRoleListBox();
        RoleSelectionChangedHdl(*m_xLB_ROLE);
    }
    updateControlState();
}

IMPL_LINK_NOARG(DataSourceTabPage, RoleSelectionChangedHdl, weld::TreeView&, void)
{
    m_rDialogModel.startControllerLockTimer();
    int nEntry = m_xLB_ROLE->get_selected_index();
    if (nEntry == -1)
        return;

    OUString aSelectedRoleUI = lcl_GetSelectedRole( *m_xLB_ROLE, true );
    OUString aSelectedRange = lcl_GetSelectedRolesRange( *m_xLB_ROLE );

    // replace role in fixed text label
    static constexpr OUString aReplacementStr( u"%VALUETYPE"_ustr );
    sal_Int32 nIndex = m_aFixedTextRange.indexOf( aReplacementStr );
    if( nIndex != -1 )
    {
        m_xFT_RANGE->set_label(
            m_aFixedTextRange.replaceAt(
                        nIndex, aReplacementStr.getLength(), aSelectedRoleUI ));
    }

    m_xEDT_RANGE->set_text(aSelectedRange);
    isValid();
}

IMPL_LINK_NOARG(DataSourceTabPage, MainRangeButtonClickedHdl, weld::Button&, void)
{
    OSL_ASSERT( m_pCurrentRangeChoosingField == nullptr );
    m_pCurrentRangeChoosingField = m_xEDT_RANGE.get();
    if (!m_xEDT_RANGE->get_text().isEmpty() &&
        !updateModelFromControl( m_pCurrentRangeChoosingField))
        return;

    int nEntry = m_xLB_SERIES->get_selected_index();
    bool bHasSelectedEntry = (nEntry != -1);

    OUString aSelectedRolesRange = lcl_GetSelectedRolesRange(*m_xLB_ROLE);

    if (bHasSelectedEntry && (m_xLB_ROLE->get_selected_index() != -1))
    {
        OUString aUIStr(SchResId(STR_DATA_SELECT_RANGE_FOR_SERIES));

        // replace role
        OUString aReplacement( "%VALUETYPE" );
        sal_Int32 nIndex = aUIStr.indexOf( aReplacement );
        if( nIndex != -1 )
        {
            aUIStr = aUIStr.replaceAt( nIndex, aReplacement.getLength(),
                                       lcl_GetSelectedRole( *m_xLB_ROLE, true ));
        }
        // replace series name
        aReplacement = "%SERIESNAME";
        nIndex = aUIStr.indexOf( aReplacement );
        if( nIndex != -1 )
        {
            aUIStr = aUIStr.replaceAt(nIndex, aReplacement.getLength(),
                                      m_xLB_SERIES->get_text(nEntry));
        }

        lcl_enableRangeChoosing(true, m_pDialogController);
        m_rDialogModel.getRangeSelectionHelper()->chooseRange( aSelectedRolesRange, aUIStr, *this );
    }
    else
        m_pCurrentRangeChoosingField = nullptr;
}

IMPL_LINK_NOARG(DataSourceTabPage, CategoriesRangeButtonClickedHdl, weld::Button&, void)
{
    OSL_ASSERT( m_pCurrentRangeChoosingField == nullptr );
    m_pCurrentRangeChoosingField = m_xEDT_CATEGORIES.get();
    if( !m_xEDT_CATEGORIES->get_text().isEmpty() &&
        ! updateModelFromControl( m_pCurrentRangeChoosingField ))
        return;

    OUString aStr(SchResId(m_xFT_CATEGORIES->get_visible() ? STR_DATA_SELECT_RANGE_FOR_CATEGORIES : STR_DATA_SELECT_RANGE_FOR_DATALABELS));
    lcl_enableRangeChoosing(true, m_pDialogController);
    m_rDialogModel.getRangeSelectionHelper()->chooseRange(
        m_rDialogModel.getCategoriesRange(), aStr, *this );
}

IMPL_LINK_NOARG(DataSourceTabPage, AddButtonClickedHdl, weld::Button&, void)
{
    m_rDialogModel.startControllerLockTimer();
    int nEntry = m_xLB_SERIES->get_selected_index();
    rtl::Reference< DataSeries > xSeriesToInsertAfter;
    rtl::Reference< ChartType > xChartTypeForNewSeries;
    if( m_pTemplateProvider )
            m_rDialogModel.setTemplate( m_pTemplateProvider->getCurrentTemplate());

    if (nEntry != -1)
    {
        ::chart::SeriesEntry* pEntry = weld::fromId<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(nEntry));
        xSeriesToInsertAfter = pEntry->m_xDataSeries;
        xChartTypeForNewSeries = pEntry->m_xChartType;
    }
    else
    {
        std::vector< rtl::Reference< ChartType > > aCntVec(
            m_rDialogModel.getAllDataSeriesContainers());
        if( ! aCntVec.empty())
            xChartTypeForNewSeries = aCntVec.front();
    }
    OSL_ENSURE( xChartTypeForNewSeries.is(), "Cannot insert new series" );

    m_rDialogModel.insertSeriesAfter( xSeriesToInsertAfter, xChartTypeForNewSeries );
    setDirty();

    fillSeriesListBox();
    // note the box was cleared and refilled, so nEntry is invalid now

    int nSelEntry = m_xLB_SERIES->get_selected_index();
    if (nSelEntry != -1)
    {
        ++nSelEntry;
        if (nSelEntry < m_xLB_SERIES->n_children())
            m_xLB_SERIES->select(nSelEntry);
    }
    SeriesSelectionChangedHdl(*m_xLB_SERIES);
}

IMPL_LINK_NOARG(DataSourceTabPage, RemoveButtonClickedHdl, weld::Button&, void)
{
    m_rDialogModel.startControllerLockTimer();
    int nEntry = m_xLB_SERIES->get_selected_index();
    if (nEntry == -1)
        return;

    SeriesEntry* pEntry = weld::fromId<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(nEntry));
    rtl::Reference< DataSeries > xNewSelSeries;
    SeriesEntry * pNewSelEntry = nullptr;
    if (nEntry + 1 < m_xLB_SERIES->n_children())
        pNewSelEntry = weld::fromId<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(nEntry + 1));
    else if (nEntry > 0)
        pNewSelEntry = weld::fromId<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(nEntry - 1));
    if (pNewSelEntry)
        xNewSelSeries = pNewSelEntry->m_xDataSeries;

    m_rDialogModel.deleteSeries( pEntry->m_xDataSeries, pEntry->m_xChartType );
    setDirty();

    m_xLB_SERIES->remove(nEntry);
    fillSeriesListBox();

    // select previous or next series
    if (xNewSelSeries.is())
    {
        for (int i = 0; i < m_xLB_SERIES->n_children(); ++i)
        {
            pEntry = weld::fromId<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(i));
            if (pEntry->m_xDataSeries == xNewSelSeries)
            {
                m_xLB_SERIES->select(i);
                break;
            }
        }
    }
    SeriesSelectionChangedHdl(*m_xLB_SERIES);
}

IMPL_LINK_NOARG(DataSourceTabPage, UpButtonClickedHdl, weld::Button&, void)
{
    m_rDialogModel.startControllerLockTimer();

    int nEntry = m_xLB_SERIES->get_selected_index();
    SeriesEntry* pEntry = nullptr;
    if (nEntry != -1)
        pEntry = weld::fromId<SeriesEntry*>(m_xLB_SERIES->get_id(nEntry));

    bool bHasSelectedEntry = (pEntry != nullptr);

    if (bHasSelectedEntry)
    {
        m_rDialogModel.moveSeries( pEntry->m_xDataSeries, DialogModel::MoveDirection::Up );
        setDirty();
        fillSeriesListBox();
        SeriesSelectionChangedHdl(*m_xLB_SERIES);
    }
}

IMPL_LINK_NOARG(DataSourceTabPage, DownButtonClickedHdl, weld::Button&, void)
{
    m_rDialogModel.startControllerLockTimer();

    int nEntry = m_xLB_SERIES->get_selected_index();
    SeriesEntry* pEntry = nullptr;
    if (nEntry != -1)
        pEntry = weld::fromId<SeriesEntry*>(m_xLB_SERIES->get_id(nEntry));

    bool bHasSelectedEntry = (pEntry != nullptr);

    if (bHasSelectedEntry)
    {
        m_rDialogModel.moveSeries( pEntry->m_xDataSeries, DialogModel::MoveDirection::Down );
        setDirty();
        fillSeriesListBox();
        SeriesSelectionChangedHdl(*m_xLB_SERIES);
    }
}

IMPL_LINK(DataSourceTabPage, RangeModifiedHdl, weld::Entry&, rEdit, void)
{
    // note: isValid sets the color of the edit field
    if( isRangeFieldContentValid( rEdit ))
    {
        setDirty();
        updateModelFromControl( &rEdit );
        if (&rEdit == m_xEDT_RANGE.get())
        {
            if( ! lcl_UpdateCurrentSeriesName( *m_xLB_SERIES ))
                fillSeriesListBox();
        }
    }

    // enable/disable OK button
    isValid();
}

void DataSourceTabPage::listeningFinished(
    const OUString & rNewRange )
{
    // rNewRange becomes invalid after removing the listener
    OUString aRange( rNewRange );

    m_rDialogModel.startControllerLockTimer();

    // stop listening
    m_rDialogModel.getRangeSelectionHelper()->stopRangeListening();

    // change edit field
    if( m_pCurrentRangeChoosingField )
    {
        m_pCurrentRangeChoosingField->set_text(aRange);
        m_pCurrentRangeChoosingField->grab_focus();
    }

    if (m_pCurrentRangeChoosingField == m_xEDT_RANGE.get())
    {
        m_xEDT_RANGE->set_text(aRange);
        setDirty();
    }
    else if (m_pCurrentRangeChoosingField == m_xEDT_CATEGORIES.get())
    {
        m_xEDT_CATEGORIES->set_text(aRange);
        setDirty();
    }

    updateModelFromControl(m_pCurrentRangeChoosingField);
    if (!lcl_UpdateCurrentSeriesName(*m_xLB_SERIES))
        fillSeriesListBox();

    m_pCurrentRangeChoosingField = nullptr;

    updateControlState();
    lcl_enableRangeChoosing(false, m_pDialogController);
}

void DataSourceTabPage::disposingRangeSelection()
{
    m_rDialogModel.getRangeSelectionHelper()->stopRangeListening( false );
}

bool DataSourceTabPage::updateModelFromControl(const weld::Entry* pField)
{
    if (!m_bIsDirty)
        return true;

    ControllerLockGuardUNO aLockedControllers( m_rDialogModel.getChartModel() );

    // @todo: validity check of field content
    bool bResult = true;
    bool bAll = (pField == nullptr);
    Reference< data::XDataProvider > xDataProvider( m_rDialogModel.getDataProvider());

    if (bAll || (pField == m_xEDT_CATEGORIES.get()))
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSeq( m_rDialogModel.getCategories() );
        if( xDataProvider.is())
        {
            OUString aRange(m_xEDT_CATEGORIES->get_text());
            if (!aRange.isEmpty())
            {
                // create or change categories
                if( !xLabeledSeq.is())
                {
                    xLabeledSeq = DataSourceHelper::createLabeledDataSequence();
                    m_rDialogModel.setCategories( xLabeledSeq );
                }
                try
                {
                    xLabeledSeq->setValues( xDataProvider->createDataSequenceByRangeRepresentation( aRange ));
                }
                catch( const uno::Exception & )
                {
                    // should work as validation should have happened before
                    DBG_UNHANDLED_EXCEPTION("chart2");
                }
            }
            else if( xLabeledSeq.is())
            {
                // clear existing categories
                xLabeledSeq.clear();
                m_rDialogModel.setCategories( xLabeledSeq );
            }
        }
    }

    int nSeriesEntry = m_xLB_SERIES->get_selected_index();
    SeriesEntry* pSeriesEntry = nullptr;
    if (nSeriesEntry != -1)
        pSeriesEntry = weld::fromId<SeriesEntry*>(m_xLB_SERIES->get_id(nSeriesEntry));
    bool bHasSelectedEntry = (pSeriesEntry != nullptr);

    if( bHasSelectedEntry )
    {
        if( bAll || (pField == m_xEDT_RANGE.get()) )
        {
            try
            {
                OUString aSelectedRole = lcl_GetSelectedRole( *m_xLB_ROLE );
                OUString aRange(m_xEDT_RANGE->get_text());
                OUString aSequenceRole( aSelectedRole );
                bool bIsLabel = (aSequenceRole == lcl_aLabelRole );
                OUString aSequenceNameForLabel( lcl_GetSequenceNameForLabel( pSeriesEntry ));

                if( bIsLabel )
                    aSequenceRole = aSequenceNameForLabel;

                uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSeq =
                    DataSeriesHelper::getDataSequenceByRole( pSeriesEntry->m_xDataSeries, aSequenceRole );

                if( xDataProvider.is())
                {
                    if( bIsLabel )
                    {
                        if( ! xLabeledSeq.is())
                        {
                            // check if there is already an "orphan" label sequence
                            xLabeledSeq = lcl_findLSequenceWithOnlyLabel( pSeriesEntry->m_xDataSeries );
                            if( ! xLabeledSeq.is())
                            {
                                // no corresponding labeled data sequence for label found
                                xLabeledSeq = DataSourceHelper::createLabeledDataSequence();
                                lcl_addLSequenceToDataSource( xLabeledSeq, pSeriesEntry->m_xDataSeries );
                            }
                        }
                        if( xLabeledSeq.is())
                        {
                            if( !aRange.isEmpty())
                            {
                                Reference< data::XDataSequence > xNewSeq;
                                try
                                {
                                    xNewSeq.set( xDataProvider->createDataSequenceByRangeRepresentation( aRange ));
                                }
                                catch( const uno::Exception & )
                                {
                                    // should work as validation should have happened before
                                    DBG_UNHANDLED_EXCEPTION("chart2");
                                }
                                if( xNewSeq.is())
                                {
                                    // update range name by the full string provided
                                    // by the data provider. E.g. "a1" might become
                                    // "$Sheet1.$A$1"
                                    aRange = xNewSeq->getSourceRangeRepresentation();
                                    Reference< beans::XPropertySet > xProp( xNewSeq, uno::UNO_QUERY_THROW );
                                    xProp->setPropertyValue( "Role" , uno::Any( lcl_aLabelRole ));

                                    //Labels should always include hidden cells, regardless of the setting chosen
                                    xProp->setPropertyValue( "IncludeHiddenCells", uno::Any(true));
                                    xLabeledSeq->setLabel( xNewSeq );
                                }
                            }
                            else
                            {
                                xLabeledSeq->setLabel( Reference< data::XDataSequence >());
                            }
                        }
                    }
                    else
                    {
                        if( !aRange.isEmpty())
                        {
                            Reference< data::XDataSequence > xNewSeq;
                            try
                            {
                                xNewSeq.set( xDataProvider->createDataSequenceByRangeRepresentation( aRange ));
                            }
                            catch( const uno::Exception & )
                            {
                                // should work as validation should have happened before
                                DBG_UNHANDLED_EXCEPTION("chart2");
                            }
                            if( xNewSeq.is())
                            {
                                // update range name by the full string provided
                                // by the data provider. E.g. "a1:e1" might become
                                // "$Sheet1.$A$1:$E$1"
                                aRange = xNewSeq->getSourceRangeRepresentation();

                                Reference< beans::XPropertySet > xProp( xNewSeq, uno::UNO_QUERY_THROW );
                                xProp->setPropertyValue( "Role" , uno::Any( aSelectedRole ));
                                if( !xLabeledSeq.is())
                                {
                                    if( aSelectedRole == aSequenceNameForLabel )
                                        xLabeledSeq = lcl_findLSequenceWithOnlyLabel( pSeriesEntry->m_xDataSeries );
                                    if( ! xLabeledSeq.is())
                                    {
                                        xLabeledSeq = DataSourceHelper::createLabeledDataSequence();
                                        lcl_addLSequenceToDataSource( xLabeledSeq, pSeriesEntry->m_xDataSeries );
                                    }
                                }
                                xLabeledSeq->setValues( xNewSeq );
                            }
                        }
                    }
                }

                lcl_UpdateCurrentRange( *m_xLB_ROLE, aSelectedRole, aRange );
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
                bResult = false;
            }
        }
    }

    // update View
    // @todo remove this when automatic view updates from calc, writer and own data sequences are available
    if( bResult )
    {
        try
        {
            if( m_rDialogModel.getChartModel() )
                m_rDialogModel.getChartModel()->setModified( true );
            const DialogModelTimeBasedInfo& rInfo = m_rDialogModel.getTimeBasedInfo();
            if(rInfo.bTimeBased)
            {
                m_rDialogModel.setTimeBasedRange(rInfo.bTimeBased, rInfo.nStart, rInfo.nEnd);
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return bResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
