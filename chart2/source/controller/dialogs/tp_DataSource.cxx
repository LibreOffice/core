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
#include "Strings.hrc"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "macros.hxx"
#include "ChartTypeTemplateProvider.hxx"
#include "RangeSelectionHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "tp_DataSourceControls.hxx"
#include "ControllerLockGuard.hxx"
#include "DataSourceHelper.hxx"
#include <com/sun/star/sheet/XRangeSelection.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>

#include <vcl/msgbox.hxx>
#include <rtl/ustrbuf.hxx>

#include <functional>
#include <algorithm>
#include <map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

const OUString lcl_aLabelRole( "label" );

OUString lcl_GetRoleLBEntry(
    const OUString & rRole, const OUString & rRange )
{
    OUStringBuffer aEntry( rRole );
    aEntry.append( "\t" );
    aEntry.append( OUString(
        ::chart::DialogModel::ConvertRoleFromInternalToUI( rRole )) );
    aEntry.append( "\t" );
    aEntry.append(OUString( rRange ));

    OUString sFoo = aEntry.makeStringAndClear();
    return sFoo;
}

void lcl_UpdateCurrentRange(
    SvTabListBox & rOutListBox,
    const OUString & rRole, const OUString & rRange )
{
    SvTreeListEntry * pEntry = rOutListBox.FirstSelected();
    if( pEntry )
        rOutListBox.SetEntryText( lcl_GetRoleLBEntry( rRole, rRange ), pEntry );
}

bool lcl_UpdateCurrentSeriesName(
    SvTreeListBox & rOutListBox )
{
    bool bResult = false;
    ::chart::SeriesEntry * pEntry = dynamic_cast< ::chart::SeriesEntry * >( rOutListBox.FirstSelected());
    if( pEntry &&
        pEntry->m_xDataSeries.is() &&
        pEntry->m_xChartType.is())
    {
        OUString aLabel( ::chart::DataSeriesHelper::getDataSeriesLabel(
                           pEntry->m_xDataSeries,
                           pEntry->m_xChartType->getRoleOfSequenceForSeriesLabel()));
        if( !aLabel.isEmpty())
        {
            rOutListBox.SetEntryText( pEntry, aLabel );
            bResult = true;
        }
    }
    return bResult;
}

OUString lcl_GetSelectedRole( const SvTabListBox & rRoleListBox, bool bUITranslated = false )
{
    OUString aResult;
    SvTreeListEntry * pEntry = rRoleListBox.FirstSelected();
    if( pEntry )
        aResult = SvTabListBox::GetEntryText( pEntry, bUITranslated ? 1 : 0 );
    return aResult;
}

OUString lcl_GetSelectedRolesRange( const SvTabListBox & rRoleListBox )
{
    OUString aResult;
    SvTreeListEntry * pEntry = rRoleListBox.FirstSelected();
    if( pEntry )
        aResult = SvTabListBox::GetEntryText( pEntry, 2 );
    return aResult;
}

OUString lcl_GetSequenceNameForLabel( ::chart::SeriesEntry * pEntry )
{
    OUString aResult( "values-y" );
    if( pEntry &&
        pEntry->m_xChartType.is())
    {
        aResult = pEntry->m_xChartType->getRoleOfSequenceForSeriesLabel();
    }
    return aResult;
}

static long lcl_pRoleListBoxTabs[] =
    {   3,        // Number of Tabs
        0, 0, 75
    };

void lcl_enableRangeChoosing( bool bEnable, Dialog * pDialog )
{
    if( pDialog )
    {
        pDialog->Show( !bEnable );
        pDialog->SetModalInputMode( !bEnable );
    }
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

DataSourceTabPage::DataSourceTabPage(
    vcl::Window * pParent,
    DialogModel & rDialogModel,
    ChartTypeTemplateProvider* pTemplateProvider,
    Dialog * pParentDialog,
    bool bHideDescription /* = false */ ) :
        ::svt::OWizardPage( pParent
                           ,"tp_DataSource"
                           ,"modules/schart/ui/tp_DataSource.ui"),
    m_pTemplateProvider( pTemplateProvider ),
    m_rDialogModel( rDialogModel ),

    m_pCurrentRangeChoosingField( nullptr ),
    m_bIsDirty( false ),
    m_pParentDialog( pParentDialog ),
    m_pTabPageNotifiable( dynamic_cast< TabPageNotifiable * >( pParentDialog ))
{

    get(m_pFT_CAPTION     ,"FT_CAPTION_FOR_WIZARD");
    get(m_pFT_SERIES      ,"FT_SERIES");

    get(m_pLB_SERIES     ,"LB_SERIES");

    get(m_pBTN_ADD        ,"BTN_ADD");
    get(m_pBTN_REMOVE     ,"BTN_REMOVE");
    get(m_pBTN_UP         ,"BTN_UP");
    get(m_pBTN_DOWN       ,"BTN_DOWN");
    get(m_pFT_ROLE        ,"FT_ROLE");
    get(m_pLB_ROLE        ,"LB_ROLE");
    get(m_pFT_RANGE       ,"FT_RANGE");
    get(m_pEDT_RANGE      ,"EDT_RANGE");
    get(m_pIMB_RANGE_MAIN ,"IMB_RANGE_MAIN");
    get(m_pFT_CATEGORIES  ,"FT_CATEGORIES");
    get(m_pFT_DATALABELS  ,"FT_DATALABELS");
    get(m_pEDT_CATEGORIES ,"EDT_CATEGORIES");
    get(m_pIMB_RANGE_CAT  ,"IMB_RANGE_CAT");

    m_pFT_CAPTION->Show(!bHideDescription);

    m_aFixedTextRange = m_pFT_RANGE->GetText();
    this->SetText( SCH_RESSTR( STR_OBJECT_DATASERIES_PLURAL ) );

    // set handlers
    m_pLB_SERIES->SetSelectHdl( LINK( this, DataSourceTabPage, SeriesSelectionChangedHdl ));

    m_pLB_ROLE->SetSelectHdl( LINK( this, DataSourceTabPage, RoleSelectionChangedHdl ));

    m_pIMB_RANGE_MAIN->SetClickHdl( LINK( this, DataSourceTabPage, MainRangeButtonClickedHdl ));
    m_pIMB_RANGE_CAT->SetClickHdl( LINK( this, DataSourceTabPage, CategoriesRangeButtonClickedHdl ));

    m_pBTN_ADD->SetClickHdl( LINK( this, DataSourceTabPage, AddButtonClickedHdl ));
    m_pBTN_REMOVE->SetClickHdl( LINK( this, DataSourceTabPage, RemoveButtonClickedHdl ));

    m_pBTN_UP->SetClickHdl( LINK( this, DataSourceTabPage, UpButtonClickedHdl ));
    m_pBTN_DOWN->SetClickHdl( LINK( this, DataSourceTabPage, DownButtonClickedHdl ));

    m_pEDT_RANGE->SetModifyHdl( LINK( this, DataSourceTabPage, RangeModifiedHdl ));
    m_pEDT_CATEGORIES->SetModifyHdl( LINK( this, DataSourceTabPage, RangeModifiedHdl ));
    m_pEDT_RANGE->SetUpdateDataHdl( LINK( this, DataSourceTabPage, RangeUpdateDataHdl ));
    m_pEDT_CATEGORIES->SetUpdateDataHdl( LINK( this, DataSourceTabPage, RangeUpdateDataHdl ));

    // #i75179# enable setting the background to a different color
    m_pEDT_RANGE->SetStyle( m_pEDT_RANGE->GetStyle() | WB_FORCECTRLBACKGROUND );
    m_pEDT_CATEGORIES->SetStyle( m_pEDT_CATEGORIES->GetStyle() | WB_FORCECTRLBACKGROUND );

    // set symbol font for arrows
    // note: StarSymbol is substituted to OpenSymbol for OOo
    vcl::Font aSymbolFont( m_pBTN_UP->GetFont());
    aSymbolFont.SetName( "StarSymbol" );
    m_pBTN_UP->SetControlFont( aSymbolFont );
    m_pBTN_DOWN->SetControlFont( aSymbolFont );

    // set button text
    sal_Unicode cBlackUpPointingTriangle( 0x25b2 );
    sal_Unicode cBlackDownPointingTriangle( 0x25bc );
    m_pBTN_UP->SetText( OUString( cBlackUpPointingTriangle ));
    m_pBTN_DOWN->SetText( OUString( cBlackDownPointingTriangle ));

    // init controls
    m_pLB_ROLE->SetTabs( lcl_pRoleListBoxTabs );
    m_pLB_ROLE->Show();

    updateControlsFromDialogModel();

    // select first series
    if( m_pLB_SERIES->First())
        m_pLB_SERIES->Select( m_pLB_SERIES->First());
    m_pLB_SERIES->GrabFocus();

    m_pBTN_UP->SetAccessibleName(SCH_RESSTR(STR_BUTTON_UP));
    m_pBTN_DOWN->SetAccessibleName(SCH_RESSTR(STR_BUTTON_DOWN));
}

DataSourceTabPage::~DataSourceTabPage()
{
    disposeOnce();
}

void DataSourceTabPage::dispose()
{
    m_pFT_CAPTION.clear();
    m_pFT_SERIES.clear();
    m_pLB_SERIES.clear();
    m_pBTN_ADD.clear();
    m_pBTN_REMOVE.clear();
    m_pBTN_UP.clear();
    m_pBTN_DOWN.clear();
    m_pFT_ROLE.clear();
    m_pLB_ROLE.clear();
    m_pFT_RANGE.clear();
    m_pEDT_RANGE.clear();
    m_pIMB_RANGE_MAIN.clear();
    m_pFT_CATEGORIES.clear();
    m_pFT_DATALABELS.clear();
    m_pEDT_CATEGORIES.clear();
    m_pIMB_RANGE_CAT.clear();
    m_pCurrentRangeChoosingField.clear();
    m_pParentDialog.clear();
    ::svt::OWizardPage::dispose();
}

void DataSourceTabPage::ActivatePage()
{
    OWizardPage::ActivatePage();
    updateControlsFromDialogModel();
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

bool DataSourceTabPage::isRangeFieldContentValid( Edit & rEdit )
{
    OUString aRange( rEdit.GetText());
    bool bIsValid = ( aRange.isEmpty() ) ||
        m_rDialogModel.getRangeSelectionHelper()->verifyCellRange( aRange );

    if( bIsValid )
    {
        rEdit.SetControlForeground();
        rEdit.SetControlBackground();
    }
    else
    {
        rEdit.SetControlBackground( RANGE_SELECTION_INVALID_RANGE_BACKGROUND_COLOR );
        rEdit.SetControlForeground( RANGE_SELECTION_INVALID_RANGE_FOREGROUND_COLOR );
    }

    return bIsValid;
}

bool DataSourceTabPage::isValid()
{
    bool bRoleRangeValid = true;
    bool bCategoriesRangeValid = true;
    bool bHasSelectedEntry = (m_pLB_SERIES->FirstSelected() != nullptr);

    if( bHasSelectedEntry )
        bRoleRangeValid = isRangeFieldContentValid( *m_pEDT_RANGE );
    if( m_pEDT_CATEGORIES->IsEnabled() )
        bCategoriesRangeValid = isRangeFieldContentValid( *m_pEDT_CATEGORIES );
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
    SeriesSelectionChangedHdl( nullptr );

    // categories
    m_pEDT_CATEGORIES->SetText( m_rDialogModel.getCategoriesRange() );

    updateControlState();
}

void DataSourceTabPage::fillSeriesListBox()
{
    m_pLB_SERIES->SetUpdateMode( false );

    Reference< XDataSeries > xSelected;
    SeriesEntry * pEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->FirstSelected());
    if( pEntry )
        xSelected.set( pEntry->m_xDataSeries );

    bool bHasSelectedEntry = (pEntry != nullptr);
    SvTreeListEntry * pSelectedEntry = nullptr;
    m_pLB_SERIES->Clear();

    ::std::vector< DialogModel::tSeriesWithChartTypeByName > aSeries(
        m_rDialogModel.getAllDataSeriesWithLabel() );

    sal_Int32 nUnnamedSeriesIndex = 1;
    for( ::std::vector< DialogModel::tSeriesWithChartTypeByName >::const_iterator aIt = aSeries.begin();
         aIt != aSeries.end(); ++aIt )
    {
        OUString aLabel( (*aIt).first );
        if( aLabel.isEmpty())
        {
            if( nUnnamedSeriesIndex > 1 )
            {
                OUString aResString(::chart::SchResId( STR_DATA_UNNAMED_SERIES_WITH_INDEX ).toString());

                // replace index of unnamed series
                const OUString aReplacementStr( "%NUMBER" );
                sal_Int32 nIndex = aResString.indexOf( aReplacementStr );
                if( nIndex != -1 )
                    aLabel = aResString.replaceAt(
                                         nIndex, aReplacementStr.getLength(),
                                         OUString::number(nUnnamedSeriesIndex));
            }
            if( aLabel.isEmpty() )
                aLabel = ::chart::SchResId( STR_DATA_UNNAMED_SERIES ).toString();

            ++nUnnamedSeriesIndex;
        }
        pEntry = dynamic_cast< SeriesEntry * >(
            m_pLB_SERIES->InsertEntry( aLabel ));
        if( pEntry )
        {
            pEntry->m_xDataSeries.set( (*aIt).second.first );
            pEntry->m_xChartType.set(  (*aIt).second.second );
            if( bHasSelectedEntry && ((*aIt).second.first == xSelected))
                pSelectedEntry = pEntry;
        }
    }

    if( bHasSelectedEntry && pSelectedEntry )
        m_pLB_SERIES->Select( pSelectedEntry );

    m_pLB_SERIES->SetUpdateMode( true );
}

void DataSourceTabPage::fillRoleListBox()
{
    SeriesEntry * pSeriesEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->FirstSelected());
    bool bHasSelectedEntry = (pSeriesEntry != nullptr);

    SvTreeListEntry * pRoleEntry =  m_pLB_ROLE->FirstSelected();
    sal_uLong nRoleIndex = SAL_MAX_UINT32;
    if( pRoleEntry )
        nRoleIndex = m_pLB_ROLE->GetModel()->GetAbsPos( pRoleEntry );

    if( bHasSelectedEntry )
    {
        DialogModel::tRolesWithRanges aRoles(
            DialogModel::getRolesWithRanges(
                pSeriesEntry->m_xDataSeries,
                lcl_GetSequenceNameForLabel( pSeriesEntry ),
                pSeriesEntry->m_xChartType ));

        // fill role list
        m_pLB_ROLE->SetUpdateMode( false );
        m_pLB_ROLE->Clear();
        m_pLB_ROLE->RemoveSelection();

        for( DialogModel::tRolesWithRanges::const_iterator aIt( aRoles.begin());
             aIt != aRoles.end(); ++ aIt )
        {
            m_pLB_ROLE->InsertEntry( lcl_GetRoleLBEntry( aIt->first, aIt->second ));
        }

        // series may contain no roles, check listbox size before selecting entries
        if( m_pLB_ROLE->GetEntryCount() > 0 )
        {
            if( nRoleIndex >= m_pLB_ROLE->GetEntryCount())
                nRoleIndex = 0;
            m_pLB_ROLE->Select( m_pLB_ROLE->GetEntry( nRoleIndex ));
        }

        m_pLB_ROLE->SetUpdateMode( true );
    }
}

void DataSourceTabPage::updateControlState()
{
    SvTreeListEntry * pSeriesEntry = m_pLB_SERIES->FirstSelected();
    bool bHasSelectedSeries = (pSeriesEntry != nullptr);
    bool bHasValidRole = false;
    bool bHasRangeChooser = m_rDialogModel.getRangeSelectionHelper()->hasRangeSelection();

    if( bHasSelectedSeries )
    {
        SvTreeListEntry * pRoleEntry =  m_pLB_ROLE->FirstSelected();
        bHasValidRole = (pRoleEntry != nullptr);
    }

    m_pBTN_ADD->Enable();
    m_pBTN_REMOVE->Enable( bHasSelectedSeries );

    m_pBTN_UP->Enable( bHasSelectedSeries && (pSeriesEntry != m_pLB_SERIES->First()));
    m_pBTN_DOWN->Enable( bHasSelectedSeries && (pSeriesEntry != m_pLB_SERIES->Last()));

    bool bHasCategories = m_rDialogModel.isCategoryDiagram();

    m_pFT_DATALABELS->Show(!bHasCategories);
    m_pFT_CATEGORIES->Show( bHasCategories);
    bool bShowIB = bHasRangeChooser;

    m_pIMB_RANGE_CAT->Show(bShowIB);

    m_pFT_SERIES->Enable();
    m_pLB_SERIES->Enable();

    m_pFT_ROLE->Enable( bHasSelectedSeries );
    m_pLB_ROLE->Enable( bHasSelectedSeries );

    m_pFT_RANGE->Enable( bHasValidRole );
    m_pEDT_RANGE->Enable( bHasValidRole );

    m_pIMB_RANGE_MAIN->Show(bShowIB);

    isValid();
}

IMPL_LINK_NOARG_TYPED(DataSourceTabPage, SeriesSelectionChangedHdl, SvTreeListBox*, void)
{
    m_rDialogModel.startControllerLockTimer();
    if( m_pLB_SERIES->FirstSelected())
    {
        fillRoleListBox();
        RoleSelectionChangedHdl( nullptr );
    }
    updateControlState();
}

IMPL_LINK_NOARG_TYPED(DataSourceTabPage, RoleSelectionChangedHdl, SvTreeListBox*, void)
{
    m_rDialogModel.startControllerLockTimer();
    SvTreeListEntry * pEntry = m_pLB_ROLE->FirstSelected();
    if( pEntry )
    {
        OUString aSelectedRoleUI = lcl_GetSelectedRole( *m_pLB_ROLE, true );
        OUString aSelectedRange = lcl_GetSelectedRolesRange( *m_pLB_ROLE );

        // replace role in fixed text label
        const OUString aReplacementStr( "%VALUETYPE" );
        sal_Int32 nIndex = m_aFixedTextRange.indexOf( aReplacementStr );
        if( nIndex != -1 )
        {
            m_pFT_RANGE->SetText(
                m_aFixedTextRange.replaceAt(
                            nIndex, aReplacementStr.getLength(), aSelectedRoleUI ));
        }

        m_pEDT_RANGE->SetText( aSelectedRange );
        isValid();
    }
}

IMPL_LINK_NOARG_TYPED(DataSourceTabPage, MainRangeButtonClickedHdl, Button*, void)
{
    OSL_ASSERT( m_pCurrentRangeChoosingField == nullptr );
    m_pCurrentRangeChoosingField = m_pEDT_RANGE;
    if( !m_pEDT_RANGE->GetText().isEmpty() &&
        ! updateModelFromControl( m_pCurrentRangeChoosingField ))
        return;

    SeriesEntry * pEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->FirstSelected());
    bool bHasSelectedEntry = (pEntry != nullptr);

    OUString aSelectedRolesRange = lcl_GetSelectedRolesRange( *m_pLB_ROLE );

    if( bHasSelectedEntry && (m_pLB_ROLE->FirstSelected() != nullptr))
    {
        OUString aUIStr(SCH_RESSTR(STR_DATA_SELECT_RANGE_FOR_SERIES));

        // replace role
        OUString aReplacement( "%VALUETYPE" );
        sal_Int32 nIndex = aUIStr.indexOf( aReplacement );
        if( nIndex != -1 )
        {
            aUIStr = aUIStr.replaceAt( nIndex, aReplacement.getLength(),
                                       lcl_GetSelectedRole( *m_pLB_ROLE, true ));
        }
        // replace series name
        aReplacement = "%SERIESNAME";
        nIndex = aUIStr.indexOf( aReplacement );
        if( nIndex != -1 )
        {
            aUIStr = aUIStr.replaceAt( nIndex, aReplacement.getLength(),
                                       OUString( m_pLB_SERIES->GetEntryText( pEntry )));
        }

        lcl_enableRangeChoosing( true, m_pParentDialog );
        m_rDialogModel.getRangeSelectionHelper()->chooseRange( aSelectedRolesRange, aUIStr, *this );
    }
    else
        m_pCurrentRangeChoosingField = nullptr;
}

IMPL_LINK_NOARG_TYPED(DataSourceTabPage, CategoriesRangeButtonClickedHdl, Button*, void)
{
    OSL_ASSERT( m_pCurrentRangeChoosingField == nullptr );
    m_pCurrentRangeChoosingField = m_pEDT_CATEGORIES;
    if( !m_pEDT_CATEGORIES->GetText().isEmpty() &&
        ! updateModelFromControl( m_pCurrentRangeChoosingField ))
        return;

    OUString aStr( SCH_RESSTR( m_pFT_CATEGORIES->IsVisible() ? STR_DATA_SELECT_RANGE_FOR_CATEGORIES : STR_DATA_SELECT_RANGE_FOR_DATALABELS ));
    lcl_enableRangeChoosing( true, m_pParentDialog );
    m_rDialogModel.getRangeSelectionHelper()->chooseRange(
        m_rDialogModel.getCategoriesRange(), aStr, *this );
}

IMPL_LINK_NOARG_TYPED(DataSourceTabPage, AddButtonClickedHdl, Button*, void)
{
    m_rDialogModel.startControllerLockTimer();
    SeriesEntry * pEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->FirstSelected());
    Reference< XDataSeries > xSeriesToInsertAfter;
    Reference< XChartType > xChartTypeForNewSeries;
    if( m_pTemplateProvider )
            m_rDialogModel.setTemplate( m_pTemplateProvider->getCurrentTemplate());

    if( pEntry )
    {
        xSeriesToInsertAfter.set( pEntry->m_xDataSeries );
        xChartTypeForNewSeries.set( pEntry->m_xChartType );
    }
    else
    {
        ::std::vector< Reference< XDataSeriesContainer > > aCntVec(
            m_rDialogModel.getAllDataSeriesContainers());
        if( ! aCntVec.empty())
            xChartTypeForNewSeries.set( aCntVec.front(), uno::UNO_QUERY );
    }
    OSL_ENSURE( xChartTypeForNewSeries.is(), "Cannot insert new series" );

    m_rDialogModel.insertSeriesAfter( xSeriesToInsertAfter, xChartTypeForNewSeries );
    setDirty();

    fillSeriesListBox();
    // note the box was cleared and refilled, so pEntry is invalid now
    SvTreeListEntry * pSelEntry = m_pLB_SERIES->FirstSelected();
    if( pSelEntry )
    {
        SvTreeListEntry * pNextEntry = m_pLB_SERIES->Next( pSelEntry );
        if( pNextEntry )
            m_pLB_SERIES->Select( pNextEntry );
    }
    SeriesSelectionChangedHdl( nullptr );
}

IMPL_LINK_NOARG_TYPED(DataSourceTabPage, RemoveButtonClickedHdl, Button*, void)
{
    m_rDialogModel.startControllerLockTimer();
    SeriesEntry * pEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->FirstSelected());
    if( pEntry )
    {
        Reference< XDataSeries > xNewSelSeries;
        SeriesEntry * pNewSelEntry = dynamic_cast< SeriesEntry * >(m_pLB_SERIES->Next( pEntry ));
        if( pNewSelEntry )
            xNewSelSeries.set( pNewSelEntry->m_xDataSeries );
        else
        {
            pNewSelEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->Prev( pEntry ));
            if( pNewSelEntry )
                xNewSelSeries.set( pNewSelEntry->m_xDataSeries );
        }

        m_rDialogModel.deleteSeries( pEntry->m_xDataSeries, pEntry->m_xChartType );
        setDirty();

        m_pLB_SERIES->RemoveSelection();
        fillSeriesListBox();

        // select previous or next series
        //@improve: see methods GetModel()->GetAbsPos()/GetEntry() for absolute list positions
        if( xNewSelSeries.is())
        {
            pEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->First());
            while( pEntry )
            {
                if( pEntry->m_xDataSeries == xNewSelSeries )
                {
                    m_pLB_SERIES->Select( pEntry );
                    break;
                }
                pEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->Next( pEntry ));
            }
        }
        SeriesSelectionChangedHdl( nullptr );
    }
}

IMPL_LINK_NOARG_TYPED(DataSourceTabPage, UpButtonClickedHdl, Button*, void)
{
    m_rDialogModel.startControllerLockTimer();
    SeriesEntry * pEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->FirstSelected());
    bool bHasSelectedEntry = (pEntry != nullptr);

    if( bHasSelectedEntry )
    {
        m_rDialogModel.moveSeries( pEntry->m_xDataSeries, DialogModel::MOVE_UP );
        setDirty();
        fillSeriesListBox();
        SeriesSelectionChangedHdl(nullptr);
    }
}

IMPL_LINK_NOARG_TYPED(DataSourceTabPage, DownButtonClickedHdl, Button*, void)
{
    m_rDialogModel.startControllerLockTimer();
    SeriesEntry * pEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->FirstSelected());
    bool bHasSelectedEntry = (pEntry != nullptr);

    if( bHasSelectedEntry )
    {
        m_rDialogModel.moveSeries( pEntry->m_xDataSeries, DialogModel::MOVE_DOWN );
        setDirty();
        fillSeriesListBox();
        SeriesSelectionChangedHdl(nullptr);
    }
}

IMPL_LINK_TYPED( DataSourceTabPage, RangeModifiedHdl, Edit&, rEdit, void )
{
    // note: isValid sets the color of the edit field
    if( isRangeFieldContentValid( rEdit ))
    {
        setDirty();
        updateModelFromControl( &rEdit );
        if( &rEdit == m_pEDT_RANGE )
        {
            if( ! lcl_UpdateCurrentSeriesName( *m_pLB_SERIES ))
                fillSeriesListBox();
        }
    }

    // enable/disable OK button
    isValid();
}

IMPL_LINK_TYPED( DataSourceTabPage, RangeUpdateDataHdl, Edit&, rEdit, void )
{
    // note: isValid sets the color of the edit field
    if( isRangeFieldContentValid( rEdit ))
    {
        setDirty();
        updateModelFromControl( &rEdit );
        if( &rEdit == m_pEDT_RANGE )
        {
            if( ! lcl_UpdateCurrentSeriesName( *m_pLB_SERIES ))
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
        m_pCurrentRangeChoosingField->SetText( aRange );
        m_pCurrentRangeChoosingField->GrabFocus();
    }

    if( m_pCurrentRangeChoosingField == m_pEDT_RANGE )
    {
        m_pEDT_RANGE->SetText( aRange );
        setDirty();
    }
    else if( m_pCurrentRangeChoosingField == m_pEDT_CATEGORIES )
    {
        m_pEDT_CATEGORIES->SetText( aRange );
        setDirty();
    }

    updateModelFromControl( m_pCurrentRangeChoosingField );
    if( ! lcl_UpdateCurrentSeriesName( *m_pLB_SERIES ))
        fillSeriesListBox();

    m_pCurrentRangeChoosingField = nullptr;

    updateControlState();
    lcl_enableRangeChoosing( false, m_pParentDialog );
}

void DataSourceTabPage::disposingRangeSelection()
{
    m_rDialogModel.getRangeSelectionHelper()->stopRangeListening( false );
}

bool DataSourceTabPage::updateModelFromControl( Edit * pField )
{
    if( !m_bIsDirty )
        return true;

    ControllerLockGuardUNO aLockedControllers( m_rDialogModel.getChartModel() );

    // @todo: validity check of field content
    bool bResult = true;
    bool bAll = (pField == nullptr);
    Reference< data::XDataProvider > xDataProvider( m_rDialogModel.getDataProvider());

    if( bAll || (pField == m_pEDT_CATEGORIES) )
    {
        Reference< data::XLabeledDataSequence > xLabeledSeq( m_rDialogModel.getCategories() );
        if( xDataProvider.is())
        {
            OUString aRange( m_pEDT_CATEGORIES->GetText());
            if( !aRange.isEmpty())
            {
                // create or change categories
                if( !xLabeledSeq.is())
                {
                    xLabeledSeq.set( DataSourceHelper::createLabeledDataSequence( Reference< uno::XComponentContext >(nullptr)));
                    m_rDialogModel.setCategories( xLabeledSeq );
                }
                try
                {
                    xLabeledSeq->setValues( xDataProvider->createDataSequenceByRangeRepresentation( aRange ));
                }
                catch( const uno::Exception & ex )
                {
                    // should work as validation should have happened before
                    ASSERT_EXCEPTION( ex );
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

    SeriesEntry * pSeriesEntry = dynamic_cast< SeriesEntry * >( m_pLB_SERIES->FirstSelected());
    bool bHasSelectedEntry = (pSeriesEntry != nullptr);

    if( bHasSelectedEntry )
    {
        if( bAll || (pField == m_pEDT_RANGE) )
        {
            try
            {
                OUString aSelectedRole = lcl_GetSelectedRole( *m_pLB_ROLE );
                OUString aRange( m_pEDT_RANGE->GetText());
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
                                xLabeledSeq.set( DataSourceHelper::createLabeledDataSequence( Reference< uno::XComponentContext >(nullptr)));
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
                                catch( const uno::Exception & ex )
                                {
                                    // should work as validation should have happened before
                                    ASSERT_EXCEPTION( ex );
                                }
                                if( xNewSeq.is())
                                {
                                    // update range name by the full string provided
                                    // by the data provider. E.g. "a1" might become
                                    // "$Sheet1.$A$1"
                                    aRange = xNewSeq->getSourceRangeRepresentation();
                                    Reference< beans::XPropertySet > xProp( xNewSeq, uno::UNO_QUERY_THROW );
                                    xProp->setPropertyValue( "Role" , uno::makeAny( lcl_aLabelRole ));
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
                            catch( const uno::Exception & ex )
                            {
                                // should work as validation should have happened before
                                ASSERT_EXCEPTION( ex );
                            }
                            if( xNewSeq.is())
                            {
                                // update range name by the full string provided
                                // by the data provider. E.g. "a1:e1" might become
                                // "$Sheet1.$A$1:$E$1"
                                aRange = xNewSeq->getSourceRangeRepresentation();

                                Reference< beans::XPropertySet > xProp( xNewSeq, uno::UNO_QUERY_THROW );
                                xProp->setPropertyValue( "Role" , uno::makeAny( aSelectedRole ));
                                if( !xLabeledSeq.is())
                                {
                                    if( aSelectedRole == aSequenceNameForLabel )
                                        xLabeledSeq.set( lcl_findLSequenceWithOnlyLabel( xSource ));
                                    if( ! xLabeledSeq.is())
                                    {
                                        xLabeledSeq.set( DataSourceHelper::createLabeledDataSequence( Reference< uno::XComponentContext >(nullptr)));
                                        lcl_addLSequenceToDataSource( xLabeledSeq, xSource );
                                    }
                                }
                                xLabeledSeq->setValues( xNewSeq );
                            }
                        }
                    }
                }

                lcl_UpdateCurrentRange( *m_pLB_ROLE, aSelectedRole, aRange );
            }
            catch( const uno::Exception & ex )
            {
                bResult = false;
                ASSERT_EXCEPTION( ex );
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
                xModifiable->setModified( sal_True );
            const DialogModelTimeBasedInfo& rInfo = m_rDialogModel.getTimeBasedInfo();
            if(rInfo.bTimeBased)
            {
                m_rDialogModel.setTimeBasedRange(rInfo.bTimeBased, rInfo.nStart, rInfo.nEnd);
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return bResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
