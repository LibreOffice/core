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
#include <DataSourceHelper.hxx>
#include "ChartTypeTemplateProvider.hxx"
#include "DialogModel.hxx"
#include <RangeSelectionHelper.hxx>
#include <TabPageNotifiable.hxx>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svtools/miscopt.hxx>
#include <osl/diagnose.h>

namespace
{
    void lcl_ShowChooserButton(
        weld::Button& rChooserButton,
        bool bShow)
    {
        if( rChooserButton.get_visible() != bShow )
        {
            rChooserButton.set_visible( bShow );
        }
    }

    void lcl_enableRangeChoosing(bool bEnable, Dialog * pDialog)
    {
        if( pDialog )
        {
            pDialog->SetModalInputMode( !bEnable );
            pDialog->Show(!bEnable);
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

} // anonymous namespace

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Sequence;

RangeChooserTabPage::RangeChooserTabPage(TabPageParent pParent, DialogModel & rDialogModel,
                                         ChartTypeTemplateProvider* pTemplateProvider,
                                         Dialog* pParentDialog, bool bHideDescription /* = false */)
    : OWizardPage(pParent, "modules/schart/ui/tp_RangeChooser.ui", "tp_RangeChooser")
    , m_nChangingControlCalls(0)
    , m_bIsDirty(false)
    , m_aLastValidRangeString()
    , m_pTemplateProvider(pTemplateProvider)
    , m_rDialogModel( rDialogModel )
    , m_pParentDialog(pParentDialog)
    , m_pParentController(pParent.pController)
    , m_pTabPageNotifiable(pParentDialog ? dynamic_cast<TabPageNotifiable*>(pParentDialog)
                                         : dynamic_cast<TabPageNotifiable*>(m_pParentController))
    , m_xFT_Caption(m_xBuilder->weld_label("FT_CAPTION_FOR_WIZARD"))
    , m_xFT_Range(m_xBuilder->weld_label("FT_RANGE"))
    , m_xED_Range(m_xBuilder->weld_entry("ED_RANGE"))
    , m_xIB_Range(m_xBuilder->weld_button("IB_RANGE"))
    , m_xRB_Rows(m_xBuilder->weld_radio_button("RB_DATAROWS"))
    , m_xRB_Columns(m_xBuilder->weld_radio_button("RB_DATACOLS"))
    , m_xCB_FirstRowAsLabel(m_xBuilder->weld_check_button("CB_FIRST_ROW_ASLABELS"))
    , m_xCB_FirstColumnAsLabel(m_xBuilder->weld_check_button("CB_FIRST_COLUMN_ASLABELS"))
    , m_xFTTitle(m_xBuilder->weld_label("STR_PAGE_DATA_RANGE"))
    , m_xFL_TimeBased(m_xBuilder->weld_widget("separator1"))
    , m_xCB_TimeBased(m_xBuilder->weld_check_button("CB_TIME_BASED"))
    , m_xFT_TimeStart(m_xBuilder->weld_label("label1"))
    , m_xEd_TimeStart(m_xBuilder->weld_entry("ED_TIME_BASED_START"))
    , m_xFT_TimeEnd(m_xBuilder->weld_label("label2"))
    , m_xEd_TimeEnd(m_xBuilder->weld_entry("ED_TIME_BASED_END"))
{
    m_xFT_Caption->set_visible(!bHideDescription);

    SetText(m_xFTTitle->get_label());// OH:remove later with dialog

    // set defaults as long as DetectArguments does not work
    m_xRB_Columns->set_active(true);
    m_xCB_FirstColumnAsLabel->set_active(true);
    m_xCB_FirstRowAsLabel->set_active(true);

    // BM: Note, that the range selection is not available, if there is no view.
    // This happens for charts having their own embedded spreadsheet.  If you
    // force to get the range selection here, this would mean when entering this
    // page the calc view would be created in this case.  So, I enable the
    // button here, and in the worst case nothing happens when it is pressed.
    // Not nice, but I see no better solution for the moment.
    m_xIB_Range->connect_clicked( LINK( this, RangeChooserTabPage, ChooseRangeHdl ));

    m_xED_Range->connect_changed( LINK( this, RangeChooserTabPage, ControlEditedHdl ));
    m_xRB_Rows->connect_toggled( LINK( this, RangeChooserTabPage, ControlChangedRadioHdl ) );
    m_xCB_FirstRowAsLabel->connect_toggled( LINK( this, RangeChooserTabPage, ControlChangedCheckBoxHdl ) );
    m_xCB_FirstColumnAsLabel->connect_toggled( LINK( this, RangeChooserTabPage, ControlChangedCheckBoxHdl ) );
    m_xCB_TimeBased->connect_toggled( LINK( this, RangeChooserTabPage, ControlChangedCheckBoxHdl ) );
    m_xEd_TimeStart->connect_changed( LINK( this, RangeChooserTabPage, ControlChangedHdl ) );
    m_xEd_TimeEnd->connect_changed( LINK( this, RangeChooserTabPage, ControlChangedHdl ) );

    SvtMiscOptions aOpts;
    if ( !aOpts.IsExperimentalMode() )
    {
        m_xFL_TimeBased->hide();
        m_xCB_TimeBased->hide();
        m_xFT_TimeStart->hide();
        m_xEd_TimeStart->hide();
        m_xFT_TimeEnd->hide();
        m_xEd_TimeEnd->hide();
    }
}

RangeChooserTabPage::~RangeChooserTabPage()
{
    disposeOnce();
}

void RangeChooserTabPage::dispose()
{
    m_pParentDialog.clear();
    OWizardPage::dispose();
}

void RangeChooserTabPage::ActivatePage()
{
    OWizardPage::ActivatePage();
    initControlsFromModel();
    m_xED_Range->grab_focus();
}

void RangeChooserTabPage::initControlsFromModel()
{
    m_nChangingControlCalls++;

    if(m_pTemplateProvider)
        m_xCurrentChartTypeTemplate = m_pTemplateProvider->getCurrentTemplate();

    bool bUseColumns = !m_xRB_Rows->get_active();
    bool bFirstCellAsLabel = bUseColumns ? m_xCB_FirstRowAsLabel->get_active() : m_xCB_FirstColumnAsLabel->get_active();
    bool bHasCategories = bUseColumns ? m_xCB_FirstColumnAsLabel->get_active() : m_xCB_FirstRowAsLabel->get_active();

    bool bIsValid = m_rDialogModel.allArgumentsForRectRangeDetected();
    if( bIsValid )
        m_rDialogModel.detectArguments(m_aLastValidRangeString, bUseColumns, bFirstCellAsLabel, bHasCategories );
    else
        m_aLastValidRangeString.clear();

    m_xED_Range->set_text( m_aLastValidRangeString );

    m_xRB_Rows->set_active( !bUseColumns );
    m_xRB_Columns->set_active(  bUseColumns );

    m_xCB_FirstRowAsLabel->set_active( m_xRB_Rows->get_active()?bHasCategories:bFirstCellAsLabel  );
    m_xCB_FirstColumnAsLabel->set_active( m_xRB_Columns->get_active()?bHasCategories:bFirstCellAsLabel  );

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
        bool bFirstCellAsLabel = ( m_xCB_FirstColumnAsLabel->get_active() && !m_xRB_Columns->get_active() )
            || ( m_xCB_FirstRowAsLabel->get_active()    && !m_xRB_Rows->get_active() );
        bool bHasCategories = ( m_xCB_FirstColumnAsLabel->get_active() && m_xRB_Columns->get_active() )
            || ( m_xCB_FirstRowAsLabel->get_active()    && m_xRB_Rows->get_active() );
        bool bTimeBased = m_xCB_TimeBased->get_active();

        Sequence< beans::PropertyValue > aArguments(
            DataSourceHelper::createArguments(
                m_xRB_Columns->get_active(), bFirstCellAsLabel, bHasCategories ) );

        if(bTimeBased)
        {
            aArguments.realloc( aArguments.getLength() + 1 );
            aArguments[aArguments.getLength() - 1] =
                beans::PropertyValue( "TimeBased", -1, uno::Any(bTimeBased),
                        beans::PropertyState_DIRECT_VALUE );
        }

        // only if range is valid
        if( m_aLastValidRangeString == m_xED_Range->get_text())
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
                sal_Int32 nStart = m_xEd_TimeStart->get_text().toInt32();
                sal_Int32 nEnd = m_xEd_TimeEnd->get_text().toInt32();
                m_rDialogModel.setTimeBasedRange(true, nStart, nEnd);
            }
        }

        //@todo warn user that the selected range is not valid
        //@todo better: disable OK-Button if range is invalid
    }
}

bool RangeChooserTabPage::isValid()
{
    OUString aRange( m_xED_Range->get_text());
    bool bFirstCellAsLabel = ( m_xCB_FirstColumnAsLabel->get_active() && !m_xRB_Columns->get_active() )
        || ( m_xCB_FirstRowAsLabel->get_active()    && !m_xRB_Rows->get_active() );
    bool bHasCategories = ( m_xCB_FirstColumnAsLabel->get_active() && m_xRB_Columns->get_active() )
        || ( m_xCB_FirstRowAsLabel->get_active()    && m_xRB_Rows->get_active() );
    bool bIsValid = ( aRange.isEmpty() ) ||
        m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
            DataSourceHelper::createArguments(
                aRange, Sequence< sal_Int32 >(), m_xRB_Columns->get_active(), bFirstCellAsLabel, bHasCategories ));

    if( bIsValid )
    {
        m_xED_Range->set_error(false);
        m_xED_Range->set_error(false);
        if( m_pTabPageNotifiable )
            m_pTabPageNotifiable->setValidPage( this );
        m_aLastValidRangeString = aRange;
    }
    else
    {
        m_xED_Range->set_error(true);
        m_xED_Range->set_error(true);
        if( m_pTabPageNotifiable )
            m_pTabPageNotifiable->setInvalidPage( this );
    }

    // enable/disable controls
    // #i79531# if the range is valid but an action of one of these buttons
    // would render it invalid, the button should be disabled
    if( bIsValid )
    {
        bool bDataInColumns = m_xRB_Columns->get_active();
        bool bIsSwappedRangeValid = m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
            DataSourceHelper::createArguments(
                aRange, Sequence< sal_Int32 >(), ! bDataInColumns, bHasCategories, bFirstCellAsLabel ));
        m_xRB_Rows->set_sensitive( bIsSwappedRangeValid );
        m_xRB_Columns->set_sensitive( bIsSwappedRangeValid );

        m_xCB_FirstRowAsLabel->set_sensitive(
            m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
                DataSourceHelper::createArguments(
                    aRange, Sequence< sal_Int32 >(), m_xRB_Columns->get_active(),
                    bDataInColumns ? ! bFirstCellAsLabel : bFirstCellAsLabel,
                    bDataInColumns ? bHasCategories : ! bHasCategories )));
        m_xCB_FirstColumnAsLabel->set_sensitive(
            m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
                DataSourceHelper::createArguments(
                    aRange, Sequence< sal_Int32 >(), m_xRB_Columns->get_active(),
                    bDataInColumns ? bFirstCellAsLabel : ! bFirstCellAsLabel,
                    bDataInColumns ? ! bHasCategories : bHasCategories )));
    }
    else
    {
        m_xRB_Rows->set_sensitive( bIsValid );
        m_xRB_Columns->set_sensitive( bIsValid );
        m_xCB_FirstRowAsLabel->set_sensitive( bIsValid );
        m_xCB_FirstColumnAsLabel->set_sensitive( bIsValid );
    }

    bool bShowIB = m_rDialogModel.getRangeSelectionHelper()->hasRangeSelection();
    lcl_ShowChooserButton( *m_xIB_Range, bShowIB );

    return bIsValid;
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlEditedHdl, weld::Entry&, void)
{
    setDirty();
    isValid();
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlChangedRadioHdl, weld::ToggleButton&, void)
{
    ControlChangedHdl(*m_xED_Range);
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlChangedCheckBoxHdl, weld::ToggleButton&, void)
{
    ControlChangedHdl(*m_xED_Range);
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlChangedHdl, weld::Entry&, void)
{
    setDirty();
    if( isValid())
        changeDialogModelAccordingToControls();
}

IMPL_LINK_NOARG(RangeChooserTabPage, ChooseRangeHdl, weld::Button&, void)
{
    OUString aRange = m_xED_Range->get_text();
    OUString aTitle = m_xFTTitle->get_label();

    lcl_enableRangeChoosing( true, m_pParentDialog );
    lcl_enableRangeChoosing( true, m_pParentController );
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
    m_xED_Range->set_text(aRange);
    m_xED_Range->grab_focus();

    setDirty();
    if( isValid())
        changeDialogModelAccordingToControls();

    lcl_enableRangeChoosing( false, m_pParentDialog );
    lcl_enableRangeChoosing( false, m_pParentController );
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
