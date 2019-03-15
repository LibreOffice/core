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
#include "ChartTypeTemplateProvider.hxx"
#include <RangeSelectionHelper.hxx>
#include <DataSeriesHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <DataSourceHelper.hxx>
#include "DialogModel.hxx"
#include <TabPageNotifiable.hxx>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>

#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

const OUString lcl_aLabelRole( "label" );

void lcl_UpdateCurrentRange(weld::TreeView& rOutListBox, const OUString & rRole,
                            const OUString& rRange)
{
    int nEntry = rOutListBox.get_selected_index();
    if (nEntry != -1)
    {
        rOutListBox.set_text(nEntry, ::chart::DialogModel::ConvertRoleFromInternalToUI(rRole), 0);
        rOutListBox.set_text(nEntry, rRange, 1);
        ::chart::SeriesEntry* pEntry = reinterpret_cast<::chart::SeriesEntry*>(rOutListBox.get_id(nEntry).toInt64());
        pEntry->m_sRole = rRole;
    }
}

bool lcl_UpdateCurrentSeriesName(weld::TreeView& rOutListBox)
{
    int nEntry = rOutListBox.get_selected_index();
    if (nEntry == -1)
        return false;

    bool bResult = false;
    ::chart::SeriesEntry * pEntry = reinterpret_cast<::chart::SeriesEntry*>(rOutListBox.get_id(nEntry).toInt64());
    if (pEntry->m_xDataSeries.is() && pEntry->m_xChartType.is())
    {
        OUString aLabel(::chart::DataSeriesHelper::getDataSeriesLabel(
                        pEntry->m_xDataSeries,
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
    OUString aResult;
    int nEntry = rRoleListBox.get_selected_index();
    if (nEntry != -1)
    {
        if (bUITranslated)
            return rRoleListBox.get_text(nEntry);
        ::chart::SeriesEntry* pEntry = reinterpret_cast<::chart::SeriesEntry*>(rRoleListBox.get_id(nEntry).toInt64());
        return pEntry->m_sRole;
    }
    return aResult;
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

void lcl_enableRangeChoosing( bool bEnable, Dialog * pDialog )
{
    if( pDialog )
    {
        pDialog->SetModalInputMode( !bEnable );
        pDialog->Show( !bEnable );
    }
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
    const Reference< chart2::data::XLabeledDataSequence > & xLSequence,
    const Reference< chart2::data::XDataSource > & xSource )
{
    Reference< data::XDataSink > xSink( xSource, uno::UNO_QUERY );
    if( xSink.is())
    {
        Sequence< Reference< chart2::data::XLabeledDataSequence > > aData( xSource->getDataSequences());
        aData.realloc( aData.getLength() + 1 );
        aData[ aData.getLength() - 1 ] = xLSequence;
        xSink->setData( aData );
    }
}

Reference< chart2::data::XLabeledDataSequence > lcl_findLSequenceWithOnlyLabel(
    const Reference< chart2::data::XDataSource > & xDataSource )
{
    Reference< chart2::data::XLabeledDataSequence > xResult;
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences( xDataSource->getDataSequences());

    for( sal_Int32 i=0; i<aSequences.getLength(); ++i )
    {
        // no values are set but a label exists
        if( ! aSequences[i]->getValues().is() &&
            aSequences[i]->getLabel().is())
        {
            xResult.set( aSequences[i] );
            break;
        }
    }

    return xResult;
}

} //  anonymous namespace

namespace chart
{

DataSourceTabPage::DataSourceTabPage(TabPageParent pParent, DialogModel & rDialogModel,
                                     ChartTypeTemplateProvider* pTemplateProvider,
                                     Dialog * pParentDialog,
                                     bool bHideDescription /* = false */)
    : ::svt::OWizardPage(pParent, "modules/schart/ui/tp_DataSource.ui", "tp_DataSource")
    , m_pTemplateProvider(pTemplateProvider)
    , m_rDialogModel(rDialogModel)
    , m_pCurrentRangeChoosingField( nullptr )
    , m_bIsDirty( false )
    , m_pParentDialog( pParentDialog )
    , m_pParentController(pParent.pController)
    , m_pTabPageNotifiable(pParentDialog ? dynamic_cast<TabPageNotifiable*>(pParentDialog)
                                         : dynamic_cast<TabPageNotifiable*>(m_pParentController))
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
    SetText( SchResId( STR_OBJECT_DATASERIES_PLURAL ) );

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
    std::vector<int> aWidths;
    aWidths.push_back(m_xLB_ROLE->get_approximate_digit_width() * 20);
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
    m_xLB_ROLE->append(OUString::number(reinterpret_cast<sal_Int64>(pEntry)),
                       ::chart::DialogModel::ConvertRoleFromInternalToUI(rRole));
    m_xLB_ROLE->set_text(m_xLB_ROLE->n_children() - 1, rRange, 1);
}

DataSourceTabPage::~DataSourceTabPage()
{
    disposeOnce();
}

void DataSourceTabPage::dispose()
{
    m_pParentDialog.clear();
    ::svt::OWizardPage::dispose();
}

void DataSourceTabPage::ActivatePage()
{
    OWizardPage::ActivatePage();
    updateControlsFromDialogModel();
    m_xLB_SERIES->grab_focus();
}

void DataSourceTabPage::initializePage()
{
}

void DataSourceTabPage::DeactivatePage()
{
    commitPage();
    svt::OWizardPage::DeactivatePage();
}

void DataSourceTabPage::commitPage()
{
    commitPage(::svt::WizardTypes::eFinish);
}

bool DataSourceTabPage::commitPage( ::svt::WizardTypes::CommitPageReason /*eReason*/ )
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
    rEdit.set_error(!bIsValid);
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
    Reference< XDataSeries > xSelected;
    SeriesEntry* pEntry = nullptr;
    int nEntry = m_xLB_SERIES->get_selected_index();
    if (nEntry != -1)
    {
        pEntry = reinterpret_cast<SeriesEntry*>(m_xLB_SERIES->get_id(nEntry).toInt64());
        xSelected.set(pEntry->m_xDataSeries);
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
                const OUString aReplacementStr( "%NUMBER" );
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
        pEntry->m_xDataSeries.set(series.second.first);
        pEntry->m_xChartType.set(series.second.second);
        m_xLB_SERIES->append(OUString::number(reinterpret_cast<sal_Int64>(pEntry)), aLabel);
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
        pSeriesEntry = reinterpret_cast<SeriesEntry*>(m_xLB_SERIES->get_id(nSeriesEntry).toInt64());
    bool bHasSelectedEntry = (pSeriesEntry != nullptr);

    int nRoleIndex = m_xLB_ROLE->get_selected_index();
    if (bHasSelectedEntry)
    {
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
    if (nEntry != -1)
    {
        OUString aSelectedRoleUI = lcl_GetSelectedRole( *m_xLB_ROLE, true );
        OUString aSelectedRange = lcl_GetSelectedRolesRange( *m_xLB_ROLE );

        // replace role in fixed text label
        const OUString aReplacementStr( "%VALUETYPE" );
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

        lcl_enableRangeChoosing( true, m_pParentDialog );
        lcl_enableRangeChoosing( true, m_pParentController );
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
    lcl_enableRangeChoosing(true, m_pParentDialog);
    lcl_enableRangeChoosing(true, m_pParentController);
    m_rDialogModel.getRangeSelectionHelper()->chooseRange(
        m_rDialogModel.getCategoriesRange(), aStr, *this );
}

IMPL_LINK_NOARG(DataSourceTabPage, AddButtonClickedHdl, weld::Button&, void)
{
    m_rDialogModel.startControllerLockTimer();
    int nEntry = m_xLB_SERIES->get_selected_index();
    Reference< XDataSeries > xSeriesToInsertAfter;
    Reference< XChartType > xChartTypeForNewSeries;
    if( m_pTemplateProvider )
            m_rDialogModel.setTemplate( m_pTemplateProvider->getCurrentTemplate());

    if (nEntry != -1)
    {
        ::chart::SeriesEntry* pEntry = reinterpret_cast<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(nEntry).toInt64());
        xSeriesToInsertAfter.set(pEntry->m_xDataSeries);
        xChartTypeForNewSeries.set(pEntry->m_xChartType);
    }
    else
    {
        std::vector< Reference< XDataSeriesContainer > > aCntVec(
            m_rDialogModel.getAllDataSeriesContainers());
        if( ! aCntVec.empty())
            xChartTypeForNewSeries.set( aCntVec.front(), uno::UNO_QUERY );
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
    if (nEntry != -1)
    {
        SeriesEntry* pEntry = reinterpret_cast<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(nEntry).toInt64());
        Reference< XDataSeries > xNewSelSeries;
        SeriesEntry * pNewSelEntry = nullptr;
        if (nEntry + 1 < m_xLB_SERIES->n_children())
            pNewSelEntry = reinterpret_cast<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(nEntry + 1).toInt64());
        else if (nEntry > 0)
            pNewSelEntry = reinterpret_cast<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(nEntry - 1).toInt64());
        if (pNewSelEntry)
            xNewSelSeries.set(pNewSelEntry->m_xDataSeries);

        m_rDialogModel.deleteSeries( pEntry->m_xDataSeries, pEntry->m_xChartType );
        setDirty();

        m_xLB_SERIES->remove(nEntry);
        fillSeriesListBox();

        // select previous or next series
        if (xNewSelSeries.is())
        {
            for (int i = 0; i < m_xLB_SERIES->n_children(); ++i)
            {
                pEntry = reinterpret_cast<::chart::SeriesEntry*>(m_xLB_SERIES->get_id(i).toInt64());
                if (pEntry->m_xDataSeries == xNewSelSeries)
                {
                    m_xLB_SERIES->select(i);
                    break;
                }
            }
        }
        SeriesSelectionChangedHdl(*m_xLB_SERIES);
    }
}

IMPL_LINK_NOARG(DataSourceTabPage, UpButtonClickedHdl, weld::Button&, void)
{
    m_rDialogModel.startControllerLockTimer();

    int nEntry = m_xLB_SERIES->get_selected_index();
    SeriesEntry* pEntry = nullptr;
    if (nEntry != -1)
        pEntry = reinterpret_cast<SeriesEntry*>(m_xLB_SERIES->get_id(nEntry).toInt64());

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
        pEntry = reinterpret_cast<SeriesEntry*>(m_xLB_SERIES->get_id(nEntry).toInt64());

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
    ToTop();
    GrabFocus();
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
    lcl_enableRangeChoosing(false, m_pParentDialog);
    lcl_enableRangeChoosing(false, m_pParentController);
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
        Reference< data::XLabeledDataSequence > xLabeledSeq( m_rDialogModel.getCategories() );
        if( xDataProvider.is())
        {
            OUString aRange(m_xEDT_CATEGORIES->get_text());
            if (!aRange.isEmpty())
            {
                // create or change categories
                if( !xLabeledSeq.is())
                {
                    xLabeledSeq.set( DataSourceHelper::createLabeledDataSequence() );
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
                xLabeledSeq.set(nullptr);
                m_rDialogModel.setCategories( xLabeledSeq );
            }
        }
    }

    int nSeriesEntry = m_xLB_SERIES->get_selected_index();
    SeriesEntry* pSeriesEntry = nullptr;
    if (nSeriesEntry != -1)
        pSeriesEntry = reinterpret_cast<SeriesEntry*>(m_xLB_SERIES->get_id(nSeriesEntry).toInt64());
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

                Reference< data::XDataSource > xSource( pSeriesEntry->m_xDataSeries, uno::UNO_QUERY_THROW );
                Reference< data::XLabeledDataSequence > xLabeledSeq(
                    DataSeriesHelper::getDataSequenceByRole( xSource, aSequenceRole ));

                if( xDataProvider.is())
                {
                    if( bIsLabel )
                    {
                        if( ! xLabeledSeq.is())
                        {
                            // check if there is already an "orphan" label sequence
                            xLabeledSeq.set( lcl_findLSequenceWithOnlyLabel( xSource ));
                            if( ! xLabeledSeq.is())
                            {
                                // no corresponding labeled data sequence for label found
                                xLabeledSeq.set( DataSourceHelper::createLabeledDataSequence() );
                                lcl_addLSequenceToDataSource( xLabeledSeq, xSource );
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
                                        xLabeledSeq.set( lcl_findLSequenceWithOnlyLabel( xSource ));
                                    if( ! xLabeledSeq.is())
                                    {
                                        xLabeledSeq.set( DataSourceHelper::createLabeledDataSequence() );
                                        lcl_addLSequenceToDataSource( xLabeledSeq, xSource );
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
            Reference< util::XModifiable > xModifiable( m_rDialogModel.getChartModel(), uno::UNO_QUERY );
            if( xModifiable.is() )
                xModifiable->setModified( true );
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
