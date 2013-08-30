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
#include "tp_RangeChooser.hrc"
#include "Strings.hrc"
#include "ResId.hxx"
#include "macros.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "DataSourceHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeTemplateProvider.hxx"
#include "DialogModel.hxx"
#include "RangeSelectionHelper.hxx"
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>

namespace
{
void lcl_ShowChooserButton(
    ::chart::RangeSelectionButton & rChooserButton,
    Edit & rEditField,
    sal_Bool bShow )
{
    if( rChooserButton.IsVisible() != bShow )
    {
        rChooserButton.Show( bShow );
        sal_Int32 nWidhtDiff = 10 + 2;
        if( bShow )
            nWidhtDiff = -nWidhtDiff;
        Size aSize = rChooserButton.PixelToLogic( rEditField.GetSizePixel(), MAP_APPFONT );
        aSize.setWidth( aSize.getWidth() + nWidhtDiff );
        rEditField.SetSizePixel( rChooserButton.LogicToPixel( aSize, MAP_APPFONT ));
    }
}
void lcl_enableRangeChoosing( bool bEnable, Dialog * pDialog )
{
    if( pDialog )
    {
        pDialog->Show( bEnable ? sal_False : sal_True );
        pDialog->SetModalInputMode( bEnable ? sal_False : sal_True );
    }
}
void lcl_shiftControlY( Control & rControl, long nYOffset )
{
    Point aPos( rControl.GetPosPixel());
    aPos.setY( aPos.getY() + nYOffset );
    rControl.SetPosPixel( aPos );
}
} // anonymous namespace

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

RangeChooserTabPage::RangeChooserTabPage( Window* pParent
        , DialogModel & rDialogModel
        , ChartTypeTemplateProvider* pTemplateProvider
        , Dialog * pParentDialog
        , bool bHideDescription /* = false */ )

        : OWizardPage( pParent, SchResId(TP_RANGECHOOSER) )

        , m_aFT_Caption( this, SchResId( FT_CAPTION_FOR_WIZARD ) )
        , m_aFT_Range( this, SchResId( FT_RANGE ) )
        , m_aED_Range( this, SchResId( ED_RANGE ) )
        , m_aIB_Range( this, SchResId( IB_RANGE ) )
        , m_aRB_Rows( this, SchResId( RB_DATAROWS ) )
        , m_aRB_Columns( this, SchResId( RB_DATACOLS ) )
        , m_aCB_FirstRowAsLabel( this, SchResId( CB_FIRST_ROW_ASLABELS ) )
        , m_aCB_FirstColumnAsLabel( this, SchResId( CB_FIRST_COLUMN_ASLABELS ) )
        , m_nChangingControlCalls(0)
        , m_bIsDirty(false)
        , m_xDataProvider( 0 )
        , m_aLastValidRangeString()
        , m_xCurrentChartTypeTemplate(0)
        , m_pTemplateProvider(pTemplateProvider)
        , m_rDialogModel( rDialogModel )
        , m_pParentDialog( pParentDialog )
        , m_pTabPageNotifiable( dynamic_cast< TabPageNotifiable * >( pParentDialog ))
{
    FreeResource();

    if( bHideDescription )
    {
        // note: the offset should be a negative value for shifting upwards, the
        // 4 is for the offset difference between a wizard page and a tab-page
        long nYOffset = - ( m_aFT_Range.GetPosPixel().getY() - m_aFT_Caption.GetPosPixel().getY() + 4 );
        m_aFT_Caption.Hide();

        // shift all controls by the offset space saved by hiding the caption
        lcl_shiftControlY( m_aFT_Range, nYOffset );
        lcl_shiftControlY( m_aED_Range, nYOffset );
        lcl_shiftControlY( m_aIB_Range, nYOffset );
        lcl_shiftControlY( m_aRB_Rows, nYOffset );
        lcl_shiftControlY( m_aRB_Columns, nYOffset );
        lcl_shiftControlY( m_aCB_FirstRowAsLabel, nYOffset );
        lcl_shiftControlY( m_aCB_FirstColumnAsLabel, nYOffset );
    }
    else
    {
        // make font of caption bold
        Font aFont( m_aFT_Caption.GetControlFont() );
        aFont.SetWeight( WEIGHT_BOLD );
        m_aFT_Caption.SetControlFont( aFont );

        // no mnemonic
        m_aFT_Caption.SetStyle( m_aFT_Caption.GetStyle() | WB_NOLABEL );
    }

    this->SetText( SCH_RESSTR(STR_PAGE_DATA_RANGE) );
    m_aIB_Range.SetQuickHelpText( SCH_RESSTR(STR_TIP_SELECT_RANGE) );

    // set defaults as long as DetectArguments does not work
    m_aRB_Columns.Check();
    m_aCB_FirstColumnAsLabel.Check();
    m_aCB_FirstRowAsLabel.Check();

    // BM: Note, that the range selection is not available, if there is no view.
    // This happens for charts having their own embedded spreadsheet.  If you
    // force to get the range selection here, this would mean when entering this
    // page the calc view would be created in this case.  So, I enable the
    // button here, and in the worst case nothing happens when it is pressed.
    // Not nice, but I see no better solution for the moment.
    m_aIB_Range.SetClickHdl( LINK( this, RangeChooserTabPage, ChooseRangeHdl ));
    m_aED_Range.SetKeyInputHdl( LINK( this, RangeChooserTabPage, ChooseRangeHdl ));

    // #i75179# enable setting the background to a different color
    m_aED_Range.SetStyle( m_aED_Range.GetStyle() | WB_FORCECTRLBACKGROUND );

    m_aED_Range.SetUpdateDataHdl( LINK( this, RangeChooserTabPage, ControlChangedHdl ));
    m_aED_Range.SetModifyHdl( LINK( this, RangeChooserTabPage, ControlEditedHdl ));
    m_aRB_Rows.SetToggleHdl( LINK( this, RangeChooserTabPage, ControlChangedHdl ) );
    m_aCB_FirstRowAsLabel.SetToggleHdl( LINK( this, RangeChooserTabPage, ControlChangedHdl ) );
    m_aCB_FirstColumnAsLabel.SetToggleHdl( LINK( this, RangeChooserTabPage, ControlChangedHdl ) );
}

RangeChooserTabPage::~RangeChooserTabPage()
{
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

    bool bUseColumns = ! m_aRB_Rows.IsChecked();
    bool bFirstCellAsLabel = bUseColumns ? m_aCB_FirstRowAsLabel.IsChecked() : m_aCB_FirstColumnAsLabel.IsChecked();
    bool bHasCategories = bUseColumns ? m_aCB_FirstColumnAsLabel.IsChecked() : m_aCB_FirstRowAsLabel.IsChecked();

    bool bIsValid = m_rDialogModel.allArgumentsForRectRangeDetected();
    if( bIsValid )
        m_rDialogModel.detectArguments(
            m_aLastValidRangeString, bUseColumns, bFirstCellAsLabel, bHasCategories );
    else
        m_aLastValidRangeString = String::EmptyString();

    m_aED_Range.SetText( m_aLastValidRangeString );

    m_aRB_Rows.Check( !bUseColumns );
    m_aRB_Columns.Check(  bUseColumns );

    m_aCB_FirstRowAsLabel.Check( m_aRB_Rows.IsChecked()?bHasCategories:bFirstCellAsLabel  );
    m_aCB_FirstColumnAsLabel.Check( m_aRB_Columns.IsChecked()?bHasCategories:bFirstCellAsLabel  );

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

sal_Bool RangeChooserTabPage::commitPage( ::svt::WizardTypes::CommitPageReason /*eReason*/ )
{
    //ranges may have been edited in the meanwhile (dirty is true in that case here)
    if( isValid() )
    {
        changeDialogModelAccordingToControls();
        return sal_True;//return false if this page should not be left
    }
    else
        return sal_False;
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
        sal_Bool bFirstCellAsLabel = ( m_aCB_FirstColumnAsLabel.IsChecked() && !m_aRB_Columns.IsChecked() )
            || ( m_aCB_FirstRowAsLabel.IsChecked()    && !m_aRB_Rows.IsChecked() );
        sal_Bool bHasCategories = ( m_aCB_FirstColumnAsLabel.IsChecked() && m_aRB_Columns.IsChecked() )
            || ( m_aCB_FirstRowAsLabel.IsChecked()    && m_aRB_Rows.IsChecked() );

        Sequence< beans::PropertyValue > aArguments(
            DataSourceHelper::createArguments(
                m_aRB_Columns.IsChecked(), bFirstCellAsLabel, bHasCategories ) );

        // only if range is valid
        if( m_aLastValidRangeString.equals(m_aED_Range.GetText()))
        {
            m_rDialogModel.setTemplate( m_xCurrentChartTypeTemplate );
            aArguments.realloc( aArguments.getLength() + 1 );
            aArguments[aArguments.getLength() - 1] =
                beans::PropertyValue( "CellRangeRepresentation" , -1,
                                      uno::makeAny( m_aLastValidRangeString ),
                                      beans::PropertyState_DIRECT_VALUE );
            m_rDialogModel.setData( aArguments );
            m_bIsDirty = false;
        }

        //@todo warn user that the selected range is not valid
        //@todo better: disable OK-Button if range is invalid
    }
}

bool RangeChooserTabPage::isValid()
{
    OUString aRange( m_aED_Range.GetText());
    sal_Bool bFirstCellAsLabel = ( m_aCB_FirstColumnAsLabel.IsChecked() && !m_aRB_Columns.IsChecked() )
        || ( m_aCB_FirstRowAsLabel.IsChecked()    && !m_aRB_Rows.IsChecked() );
    sal_Bool bHasCategories = ( m_aCB_FirstColumnAsLabel.IsChecked() && m_aRB_Columns.IsChecked() )
        || ( m_aCB_FirstRowAsLabel.IsChecked()    && m_aRB_Rows.IsChecked() );
    bool bIsValid = ( aRange.isEmpty() ) ||
        m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
            DataSourceHelper::createArguments(
                aRange, Sequence< sal_Int32 >(), m_aRB_Columns.IsChecked(), bFirstCellAsLabel, bHasCategories ));

    if( bIsValid )
    {
        m_aED_Range.SetControlForeground();
        m_aED_Range.SetControlBackground();
        if( m_pTabPageNotifiable )
            m_pTabPageNotifiable->setValidPage( this );
        m_aLastValidRangeString = aRange;
    }
    else
    {
        m_aED_Range.SetControlBackground( RANGE_SELECTION_INVALID_RANGE_BACKGROUND_COLOR );
        m_aED_Range.SetControlForeground( RANGE_SELECTION_INVALID_RANGE_FOREGROUND_COLOR );
        if( m_pTabPageNotifiable )
            m_pTabPageNotifiable->setInvalidPage( this );
    }

    // enable/disable controls
    // #i79531# if the range is valid but an action of one of these buttons
    // would render it invalid, the button should be disabled
    if( bIsValid )
    {
        bool bDataInColumns = m_aRB_Columns.IsChecked();
        bool bIsSwappedRangeValid = m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
            DataSourceHelper::createArguments(
                aRange, Sequence< sal_Int32 >(), ! bDataInColumns, bHasCategories, bFirstCellAsLabel ));
        m_aRB_Rows.Enable( bIsSwappedRangeValid );
        m_aRB_Columns.Enable( bIsSwappedRangeValid );

        m_aCB_FirstRowAsLabel.Enable(
            m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
                DataSourceHelper::createArguments(
                    aRange, Sequence< sal_Int32 >(), m_aRB_Columns.IsChecked(),
                    bDataInColumns ? ! bFirstCellAsLabel : bFirstCellAsLabel,
                    bDataInColumns ? bHasCategories : ! bHasCategories )));
        m_aCB_FirstColumnAsLabel.Enable(
            m_rDialogModel.getRangeSelectionHelper()->verifyArguments(
                DataSourceHelper::createArguments(
                    aRange, Sequence< sal_Int32 >(), m_aRB_Columns.IsChecked(),
                    bDataInColumns ? bFirstCellAsLabel : ! bFirstCellAsLabel,
                    bDataInColumns ? ! bHasCategories : bHasCategories )));
    }
    else
    {
        m_aRB_Rows.Enable( bIsValid );
        m_aRB_Columns.Enable( bIsValid );
        m_aCB_FirstRowAsLabel.Enable( bIsValid );
        m_aCB_FirstColumnAsLabel.Enable( bIsValid );
    }
    sal_Bool bShowIB = m_rDialogModel.getRangeSelectionHelper()->hasRangeSelection();
    lcl_ShowChooserButton( m_aIB_Range, m_aED_Range, bShowIB );

    return bIsValid;
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlEditedHdl)
{
    setDirty();
    isValid();
    return 0;
}

IMPL_LINK_NOARG(RangeChooserTabPage, ControlChangedHdl)
{
    setDirty();
    if( isValid())
        changeDialogModelAccordingToControls();
    return 0;
}

IMPL_LINK_NOARG(RangeChooserTabPage, ChooseRangeHdl)
{
    OUString aRange = m_aED_Range.GetText();
    // using assignment for broken gcc 3.3
    OUString aTitle = SCH_RESSTR( STR_PAGE_DATA_RANGE );

    lcl_enableRangeChoosing( true, m_pParentDialog );
    m_rDialogModel.getRangeSelectionHelper()->chooseRange( aRange, aTitle, *this );

    return 0;
}

void RangeChooserTabPage::listeningFinished( const OUString & rNewRange )
{
    //user has selected a new range

    OUString aRange( rNewRange );

    m_rDialogModel.startControllerLockTimer();

    // stop listening
    m_rDialogModel.getRangeSelectionHelper()->stopRangeListening();

    //update dialog state
    ToTop();
    GrabFocus();
    m_aED_Range.SetText( String( aRange ) );
    m_aED_Range.GrabFocus();

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
