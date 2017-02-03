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

#include "tp_RangeChooser.hxx"
#include "Strings.hrc"
#include "ResId.hxx"
#include "macros.hxx"
#include "DataSourceHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeTemplateProvider.hxx"
#include "DialogModel.hxx"
#include "RangeSelectionHelper.hxx"
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <svtools/miscopt.hxx>

namespace
{
    void lcl_ShowChooserButton(
        PushButton& rChooserButton,
        bool bShow)
    {
        if( rChooserButton.IsVisible() != bShow )
        {
            rChooserButton.Show( bShow );
        }
    }

    void lcl_enableRangeChoosing( bool bEnable, Dialog * pDialog )
    {
        if( pDialog )
        {
            pDialog->Show( !bEnable );
            pDialog->SetModalInputMode( !bEnable );
        }
    }

} // anonymous namespace

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Sequence;

RangeChooserTabPage::RangeChooserTabPage( vcl::Window* pParent
        , DialogModel & rDialogModel
        , ChartTypeTemplateProvider* pTemplateProvider
        , Dialog * pParentDialog
        , bool bHideDescription /* = false */ )
        : OWizardPage( pParent
        ,"tp_RangeChooser"
        ,"modules/schart/ui/tp_RangeChooser.ui")
        , m_nChangingControlCalls(0)
        , m_bIsDirty(false)
        , m_aLastValidRangeString()
        , m_xCurrentChartTypeTemplate(nullptr)
        , m_pTemplateProvider(pTemplateProvider)
        , m_rDialogModel( rDialogModel )
        , m_pParentDialog( pParentDialog )
        , m_pTabPageNotifiable( dynamic_cast< TabPageNotifiable * >( pParentDialog ))
{
    get(m_pFT_Caption, "FT_CAPTION_FOR_WIZARD");
    get(m_pFT_Range, "FT_RANGE");
    get(m_pED_Range, "ED_RANGE");
    get(m_pIB_Range, "IB_RANGE");
    get(m_pRB_Rows, "RB_DATAROWS");
    get(m_pRB_Columns, "RB_DATACOLS");
    get(m_pCB_FirstRowAsLabel, "CB_FIRST_ROW_ASLABELS");
    get(m_pCB_FirstColumnAsLabel, "CB_FIRST_COLUMN_ASLABELS");
    get(m_pFTTitle, "STR_PAGE_DATA_RANGE");// OH:remove later with dialog title
    get(m_pFL_TimeBased, "separator1");
    get(m_pCB_TimeBased, "CB_TIME_BASED");
    get(m_pFT_TimeStart, "label1");
    get(m_pEd_TimeStart, "ED_TIME_BASED_START");
    get(m_pFT_TimeEnd, "label2");
    get(m_pEd_TimeEnd, "ED_TIME_BASED_END");

    m_pFT_Caption->Show(!bHideDescription);

    this->SetText( m_pFTTitle->GetText());// OH:remove later with dialog

    // set defaults as long as DetectArguments does not work
    m_pRB_Columns->Check();
    m_pCB_FirstColumnAsLabel->Check();
    m_pCB_FirstRowAsLabel->Check();

    // BM: Note, that the range selection is not available, if there is no view.
    // This happens for charts having their own embedded spreadsheet.  If you
    // force to get the range selection here, this would mean when entering this
    // page the calc view would be created in this case.  So, I enable the
    // button here, and in the worst case nothing happens when it is pressed.
    // Not nice, but I see no better solution for the moment.
    m_pIB_Range->SetClickHdl( LINK( this, RangeChooserTabPage, ChooseRangeHdl ));

    // #i75179# enable setting the background to a different color
    m_pED_Range->SetStyle( m_pED_Range->GetStyle() | WB_FORCECTRLBACKGROUND );

    m_pED_Range->SetUpdateDataHdl( LINK( this, RangeChooserTabPage, ControlChangedHdl ));
    m_pED_Range->SetModifyHdl( LINK( this, RangeChooserTabPage, ControlEditedHdl ));
    m_pRB_Rows->SetToggleHdl( LINK( this, RangeChooserTabPage, ControlChangedRadioHdl ) );
    m_pCB_FirstRowAsLabel->SetToggleHdl( LINK( this, RangeChooserTabPage, ControlChangedCheckBoxHdl ) );
    m_pCB_FirstColumnAsLabel->SetToggleHdl( LINK( this, RangeChooserTabPage, ControlChangedCheckBoxHdl ) );
    m_pCB_TimeBased->SetToggleHdl( LINK( this, RangeChooserTabPage, ControlChangedCheckBoxHdl ) );
    m_pEd_TimeStart->SetModifyHdl( LINK( this, RangeChooserTabPage, ControlChangedHdl ) );
    m_pEd_TimeEnd->SetModifyHdl( LINK( this, RangeChooserTabPage, ControlChangedHdl ) );

    SvtMiscOptions aOpts;
    if ( !aOpts.IsExperimentalMode() )
    {
        m_pFL_TimeBased->Hide();
        m_pCB_TimeBased->Hide();
        m_pFT_TimeStart->Hide();
        m_pEd_TimeStart->Hide();
        m_pFT_TimeEnd->Hide();
        m_pEd_TimeEnd->Hide();
    }
}

RangeChooserTabPage::~RangeChooserTabPage()
{
    disposeOnce();
}

void RangeChooserTabPage::dispose()
{
    m_pFT_Caption.clear();
    m_pFT_Range.clear();
    m_pED_Range.clear();
    m_pIB_Range.clear();
    m_pRB_Rows.clear();
    m_pRB_Columns.clear();
    m_pCB_FirstRowAsLabel.clear();
    m_pCB_FirstColumnAsLabel.clear();
    m_pFTTitle.clear();
    m_pFL_TimeBased.clear();
    m_pCB_TimeBased.clear();
    m_pFT_TimeStart.clear();
    m_pEd_TimeStart.clear();
    m_pFT_TimeEnd.clear();
    m_pEd_TimeEnd.clear();
    m_pParentDialog.clear();
    OWizardPage::dispose();
}


void RangeChooserTabPage::ActivatePage()
{
    OWizardPage::ActivatePage();
    initControlsFromModel();
}

void RangeChooserTabPage::initControlsFromModel()
{
    m_nChangingControlCalls++;

    if(m_pTemplateProvider)
        m_xCurrentChartTypeTemplate = m_pTemplateProvider->getCurrentTemplate();

    bool bUseColumns = ! m_pRB_Rows->IsChecked();
    bool bFirstCellAsLabel = bUseColumns ? m_pCB_FirstRowAsLabel->IsChecked() : m_pCB_FirstColumnAsLabel->IsChecked();
    bool bHasCategories = bUseColumns ? m_pCB_FirstColumnAsLabel->IsChecked() : m_pCB_FirstRowAsLabel->IsChecked();

    bool bIsValid = m_rDialogModel.allArgumentsForRectRangeDetected();
    if( bIsValid )
        m_rDialogModel.detectArguments(m_aLastValidRangeString, bUseColumns, bFirstCellAsLabel, bHasCategories );
    else
        m_aLastValidRangeString.clear();

    m_pED_Range->SetText( m_aLastValidRangeString );

    m_pRB_Rows->Check( !bUseColumns );
    m_pRB_Columns->Check(  bUseColumns );

    m_pCB_FirstRowAsLabel->Check( m_pRB_Rows->IsChecked()?bHasCategories:bFirstCellAsLabel  );
    m_pCB_FirstColumnAsLabel->Check( m_pRB_Columns->IsChecked()?bHasCategories:bFirstCellAsLabel  );

    isValid();

    m_nChangingControlCalls--;
}

void RangeChooserTabPage::DeactivatePage()
{
    commitPage();
    svt::OWizardPage::DeactivatePage();
}

void RangeChooserTabPage::commitPage()
{
    commitPage(::svt::WizardTypes::eFinish);
}

bool RangeChooserTabPage::commitPage( ::svt::WizardTypes::CommitPageReason /*eReason*/ )
{
    //ranges may have been edited in the meanwhile (dirty is true in that case here)
    if( isValid() )
    {
        changeDialogModelAccordingToControls();
        return true; // return false if this page should not be left
    }
    else
        return false;
}

void RangeChooserTabPage::changeDialogModelAccordingToControls()
{
    if(m_nChangingControlCalls>0)
        return;

    if( !m_xCurrentChartTypeTemplate.is() )
    {
        if(m_pTemplateProvider)
            m_xCurrentChartTypeTemplate.set( m_pTemplateProvider->getCurrentTemplate());
        if( !m_xCurrentChartTypeTemplate.is())
        {
            OSL_FAIL( "Need a template to change data source" );
            return;
        }
    }

    if( m_bIsDirty )
    {
        bool bFirstCellAsLabel = ( m_pCB_FirstColumnAsLabel->IsChecked() && !m_pRB_Columns->IsChecked() )
            || ( m_pCB_FirstRowAsLabel->IsChecked()    && !m_pRB_Rows->IsChecked() );
        bool bHasCategories = ( m_pCB_FirstColumnAsLabel->IsChecked() && m_pRB_Columns->IsChecked() )
            || ( m_pCB_FirstRowAsLabel->IsChecked()    && m_pRB_Rows->IsChecked() );
        bool bTimeBased = m_pCB_TimeBased->IsChecked();

        Sequence< beans::PropertyValue > aArguments(
            DataSourceHelper::createArguments(
                m_pRB_Columns->IsChecked(), bFirstCellAsLabel, bHasCategories ) );

        if(bTimeBased)
        {
            aArguments.realloc( aArguments.getLength() + 1 );
            aArguments[aArguments.getLength() - 1] =
                beans::PropertyValue( "TimeBased", -1, uno::Any(bTimeBased),
                        beans::PropertyState_DIRECT_VALUE );
        }

        // only if range is valid
        if( m_aLastValidRangeString.equals(m_pED_Range->GetText()))
        {
            m_rDialogModel.setTemplate( m_xCurrentChartTypeTemplate );
            aArguments.realloc( aArguments.getLength() + 1 );
            aArguments[aArguments.getLength() - 1] =
                beans::PropertyValue( "CellRangeRepresentation" , -1,
                                      uno::Any( m_aLastValidRangeString ),
                                      beans::PropertyState_DIRECT_VALUE );
            m_rDialogModel.setData( aArguments );
            m_bIsDirty = false;

            if(bTimeBased)
            {
                sal_Int32 nStart = m_pEd_TimeStart->GetText().toInt32();
                sal_Int32 nEnd = m_pEd_TimeEnd->GetText().toInt32();
                m_rDialogModel.setTimeBasedRange(true, nStart, nEnd);
            }
        }

        //@todo warn user that the selected range is not valid
        //@todo better: disable OK-Button if range is invalid
    }
}

bool RangeChooserTabPage::isValid()
{
    OUString aRange( m_pED_Range->GetText());
    bool bFirstCellAsLabel = ( m_pCB_FirstColumnAsLabel->IsChecked() && !m_pRB_Columns->IsChecked() )
        || ( m_pCB_FirstRowAsLabel->IsChecked()    && !m_pRB_Rows->IsChecked() );
    bool bHasCategories = ( m_pCB_FirstColumnAsLabel->IsChecked() && m_pRB_Columns->IsChecked() )
        || ( m_pCB_FirstRowAsLabel->IsChecked()    && m_pRB_Rows->IsChecked() );
    bool bIsValid = ( aRange.isEmpty() ) ||
        m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
            DataSourceHelper::createArguments(
                aRange, Sequence< sal_Int32 >(), m_pRB_Columns->IsChecked(), bFirstCellAsLabel, bHasCategories ));

    if( bIsValid )
    {
        m_pED_Range->SetControlForeground();
        m_pED_Range->SetControlBackground();
        if( m_pTabPageNotifiable )
            m_pTabPageNotifiable->setValidPage( this );
        m_aLastValidRangeString = aRange;
    }
    else
    {
        m_pED_Range->SetControlBackground( RANGE_SELECTION_INVALID_RANGE_BACKGROUND_COLOR );
        m_pED_Range->SetControlForeground( RANGE_SELECTION_INVALID_RANGE_FOREGROUND_COLOR );
        if( m_pTabPageNotifiable )
            m_pTabPageNotifiable->setInvalidPage( this );
    }

    // enable/disable controls
    // #i79531# if the range is valid but an action of one of these buttons
    // would render it invalid, the button should be disabled
    if( bIsValid )
    {
        bool bDataInColumns = m_pRB_Columns->IsChecked();
        bool bIsSwappedRangeValid = m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
            DataSourceHelper::createArguments(
                aRange, Sequence< sal_Int32 >(), ! bDataInColumns, bHasCategories, bFirstCellAsLabel ));
        m_pRB_Rows->Enable( bIsSwappedRangeValid );
        m_pRB_Columns->Enable( bIsSwappedRangeValid );

        m_pCB_FirstRowAsLabel->Enable(
            m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
                DataSourceHelper::createArguments(
                    aRange, Sequence< sal_Int32 >(), m_pRB_Columns->IsChecked(),
                    bDataInColumns ? ! bFirstCellAsLabel : bFirstCellAsLabel,
                    bDataInColumns ? bHasCategories : ! bHasCategories )));
        m_pCB_FirstColumnAsLabel->Enable(
            m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
                DataSourceHelper::createArguments(
                    aRange, Sequence< sal_Int32 >(), m_pRB_Columns->IsChecked(),
                    bDataInColumns ? bFirstCellAsLabel : ! bFirstCellAsLabel,
                    bDataInColumns ? ! bHasCategories : bHasCategories )));
    }
    else
    {
        m_pRB_Rows->Enable( bIsValid );
        m_pRB_Columns->Enable( bIsValid );
        m_pCB_FirstRowAsLabel->Enable( bIsValid );
        m_pCB_FirstColumnAsLabel->Enable( bIsValid );
    }

    bool bShowIB = m_rDialogModel.getRangeSelectionHelper()->hasRangeSelection();
    lcl_ShowChooserButton( *m_pIB_Range, bShowIB );

    return bIsValid;
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlEditedHdl, Edit&, void)
{
    setDirty();
    isValid();
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlChangedRadioHdl, RadioButton&, void)
{
    ControlChangedHdl(*m_pED_Range);
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlChangedCheckBoxHdl, CheckBox&, void)
{
    ControlChangedHdl(*m_pED_Range);
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlChangedHdl, Edit&, void)
{
    setDirty();
    if( isValid())
        changeDialogModelAccordingToControls();
}

IMPL_LINK_NOARG(RangeChooserTabPage, ChooseRangeHdl, Button*, void)
{
    OUString aRange = m_pED_Range->GetText();
    OUString aTitle = m_pFTTitle->GetText();

    lcl_enableRangeChoosing( true, m_pParentDialog );
    m_rDialogModel.getRangeSelectionHelper()->chooseRange( aRange, aTitle, *this );
}

void RangeChooserTabPage::listeningFinished( const OUString & rNewRange )
{
    //user has selected a new range

    // rNewRange becomes invalid after removing the listener
    OUString aRange( rNewRange );

    m_rDialogModel.startControllerLockTimer();

    // stop listening
    m_rDialogModel.getRangeSelectionHelper()->stopRangeListening();

    //update dialog state
    ToTop();
    GrabFocus();
    m_pED_Range->SetText( aRange );
    m_pED_Range->GrabFocus();

    setDirty();
    if( isValid())
        changeDialogModelAccordingToControls();

    lcl_enableRangeChoosing( false, m_pParentDialog );
}
void RangeChooserTabPage::disposingRangeSelection()
{
    m_rDialogModel.getRangeSelectionHelper()->stopRangeListening( false );
}

void RangeChooserTabPage::setDirty()
{
    if( m_nChangingControlCalls == 0 )
        m_bIsDirty = true;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
