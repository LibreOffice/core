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

#include "Condition.hxx"
#include <UITools.hxx>
#include <CondFormat.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <ReportController.hxx>
#include <ColorChanger.hxx>
#include <reportformula.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/ui/ImageType.hpp>

#include <svx/tbcontrl.hxx>
#include <svx/svxids.hrc>
#include <svx/xtable.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/imgdef.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/settings.hxx>

#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>
#include <svtools/valueset.hxx>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

ConditionField::ConditionField(Condition* pParent, Edit* pSubEdit, PushButton *pFormula)
    : m_pParent(pParent)
    , m_pSubEdit(pSubEdit)
    , m_pFormula(pFormula)
{
    m_pSubEdit->EnableRTL( false );

    m_pFormula->SetText("...");
    m_pFormula->SetClickHdl( LINK( this, ConditionField, OnFormula ) );
}

IMPL_LINK( ConditionField, OnFormula, Button*, _pClickedButton, void )
{
    OUString sFormula(m_pSubEdit->GetText());
    const sal_Int32 nLen = sFormula.getLength();
    if ( nLen )
    {
        ReportFormula aFormula( sFormula );
        sFormula = aFormula.getCompleteFormula();
    }
    uno::Reference< awt::XWindow> xInspectorWindow = VCLUnoHelper::GetInterface(_pClickedButton);
    uno::Reference< beans::XPropertySet> xProp(m_pParent->getController().getRowSet(),uno::UNO_QUERY);
    if ( rptui::openDialogFormula_nothrow( sFormula, m_pParent->getController().getContext(),xInspectorWindow,xProp ) )
    {
        ReportFormula aFormula( sFormula );
        m_pSubEdit->SetText(aFormula.getUndecoratedContent());
    }
}

ConditionColorWrapper::ConditionColorWrapper(Condition* pControl)
    : mxControl(pControl)
    , mnSlotId(0)
{
}

void ConditionColorWrapper::dispose()
{
    mxControl.clear();
}

void ConditionColorWrapper::operator()(const OUString& /*rCommand*/, const NamedColor& rNamedColor)
{
    mxControl->ApplyCommand(mnSlotId, rNamedColor.first);
}

// = Condition


Condition::Condition( vcl::Window* _pParent, IConditionalFormatAction& _rAction, ::rptui::OReportController& _rController )
    : VclHBox(_pParent)
    , m_xPaletteManager(new PaletteManager)
    , m_aColorWrapper(this)
    , m_rController(_rController)
    , m_rAction(_rAction)
    , m_pBtnUpdaterFontColor(nullptr)
    , m_pBtnUpdaterBackgroundColor(nullptr)
    , m_nCondIndex(0)
    , m_bInDestruction(false)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "modules/dbreport/ui/conditionwin.ui"));

    get(m_pHeader, "headerLabel");
    get(m_pConditionType, "typeCombobox");
    get(m_pOperationList, "opCombobox");
    m_pCondLHS.reset( new ConditionField(this, get<Edit>("lhsEntry"), get<PushButton>("lhsButton")) );
    get(m_pOperandGlue, "andLabel");
    m_pCondRHS.reset( new ConditionField(this, get<Edit>("rhsEntry"), get<PushButton>("rhsButton")) );
    get(m_pActions, "formatToolbox");
    get(m_pPreview, "previewDrawingarea");
    get(m_pMoveUp, "upButton");
    get(m_pMoveDown, "downButton");
    get(m_pAddCondition, "addButton");
    get(m_pRemoveCondition, "removeButton");

    m_pActions->SetLineSpacing(true);
    m_pCondLHS->GrabFocus();

    m_pConditionType->SetSelectHdl( LINK( this, Condition, OnTypeSelected ) );

    m_pOperationList->SetDropDownLineCount( 10 );
    m_pOperationList->SetSelectHdl( LINK( this, Condition, OnOperationSelected ) );

    m_pActions->SetSelectHdl(LINK(this, Condition, OnFormatAction));
    m_pActions->SetDropdownClickHdl( LINK( this, Condition, DropdownClick ) );
    setToolBox(m_pActions);

    m_pMoveUp->SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_pMoveDown->SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_pAddCondition->SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_pRemoveCondition->SetClickHdl( LINK( this, Condition, OnConditionAction ) );

    vcl::Font aFont( m_pAddCondition->GetFont() );
    aFont.SetWeight( WEIGHT_BOLD );
    m_pAddCondition->SetFont( aFont );
    m_pRemoveCondition->SetFont( aFont );

    m_pOperandGlue->SetStyle( m_pOperandGlue->GetStyle() | WB_VCENTER );

    m_pConditionType->SelectEntryPos( 0 );
    m_pOperationList->SelectEntryPos( 0 );

    m_nBoldId = m_pActions->GetItemId(".uno:Bold");
    m_nItalicId = m_pActions->GetItemId(".uno:Italic");
    m_nUnderLineId = m_pActions->GetItemId(".uno:Underline");
    m_nBackgroundColorId = m_pActions->GetItemId(".uno:BackgroundColor");
    m_nFontColorId = m_pActions->GetItemId(".uno:FontColor");
    m_nFontDialogId = m_pActions->GetItemId(".uno:FontDialog");

    m_pBtnUpdaterBackgroundColor.reset( new svx::ToolboxButtonColorUpdater(
                                            SID_BACKGROUND_COLOR, m_nBackgroundColorId, m_pActions ) );
    m_pBtnUpdaterFontColor.reset( new svx::ToolboxButtonColorUpdater(
                                            SID_ATTR_CHAR_COLOR2, m_nFontColorId, m_pActions ) );

    Show();

    ConditionalExpressionFactory::getKnownConditionalExpressions( m_aConditionalExpressions );
}

sal_uInt16 Condition::mapToolbarItemToSlotId(sal_uInt16 nItemId) const
{
    if (nItemId == m_nBoldId)
        return SID_ATTR_CHAR_WEIGHT;
    if (nItemId == m_nItalicId)
        return SID_ATTR_CHAR_POSTURE;
    if (nItemId == m_nUnderLineId)
        return SID_ATTR_CHAR_UNDERLINE;
    if (nItemId == m_nBackgroundColorId)
        return SID_BACKGROUND_COLOR;
    if (nItemId == m_nFontColorId)
        return SID_ATTR_CHAR_COLOR2;
    if (nItemId == m_nFontDialogId)
        return SID_CHAR_DLG;
    return 0;
}

Condition::~Condition()
{
    disposeOnce();
}

void Condition::dispose()
{
    m_bInDestruction = true;

    m_pBtnUpdaterFontColor.reset();
    m_pCondLHS.reset();
    m_pCondRHS.reset();
    m_pBtnUpdaterBackgroundColor.reset();
    m_pHeader.clear();
    m_pConditionType.clear();
    m_pOperationList.clear();
    m_pOperandGlue.clear();
    m_pActions.clear();
    m_pPreview.clear();
    m_pMoveUp.clear();
    m_pMoveDown.clear();
    m_pAddCondition.clear();
    m_pRemoveCondition.clear();
    m_pColorFloat.disposeAndClear();
    m_aColorWrapper.dispose();
    disposeBuilder();
    VclHBox::dispose();
}

IMPL_LINK(Condition, DropdownClick, ToolBox*, pToolBox, void)
{
    sal_uInt16 nId( m_pActions->GetCurItemId() );
    m_pColorFloat.disposeAndClear();
    sal_uInt16 nSlotId(mapToolbarItemToSlotId(nId));
    m_aColorWrapper.SetSlotId(nSlotId);
    m_pColorFloat = VclPtr<SvxColorWindow>::Create(
                           OUString() /*m_aCommandURL*/,
                           m_xPaletteManager,
                           m_aBorderColorStatus,
                           nSlotId,
                           nullptr,
                           pToolBox,
                           false,
                           m_aColorWrapper);

    m_pColorFloat->EnableDocking();
    vcl::Window::GetDockingManager()->StartPopupMode(pToolBox, m_pColorFloat, FloatWinPopupFlags::GrabFocus);
}

IMPL_LINK_NOARG( Condition, OnFormatAction, ToolBox*, void )
{
    ApplyCommand(mapToolbarItemToSlotId(m_pActions->GetCurItemId()),COL_AUTO);
}

IMPL_LINK( Condition, OnConditionAction, Button*, _pClickedButton, void )
{
    if ( _pClickedButton == m_pMoveUp )
        m_rAction.moveConditionUp( getConditionIndex() );
    else if ( _pClickedButton == m_pMoveDown )
        m_rAction.moveConditionDown( getConditionIndex() );
    else if ( _pClickedButton == m_pAddCondition )
        m_rAction.addCondition( getConditionIndex() );
    else if ( _pClickedButton == m_pRemoveCondition )
        m_rAction.deleteCondition( getConditionIndex() );
}

void Condition::ApplyCommand( sal_uInt16 _nCommandId, const ::Color& _rColor)
{
    if ( _nCommandId == SID_ATTR_CHAR_COLOR2 )
        m_pBtnUpdaterFontColor->Update( _rColor );
    else if ( _nCommandId == SID_BACKGROUND_COLOR )
        m_pBtnUpdaterBackgroundColor->Update( _rColor );

    m_rAction.applyCommand( m_nCondIndex, _nCommandId, _rColor );
}

void Condition::setImageList(sal_Int16 /*_eBitmapSet*/)
{
}

void Condition::resizeControls(const Size& /*_rDiff*/)
{
}

void Condition::GetFocus()
{
    VclHBox::GetFocus();
    if ( !m_bInDestruction )
        m_pCondLHS->GrabFocus();
}

IMPL_LINK_NOARG( Condition, OnTypeSelected, ListBox&, void )
{
    impl_layoutOperands();
}


IMPL_LINK_NOARG( Condition, OnOperationSelected, ListBox&, void )
{
    impl_layoutOperands();
}

void Condition::impl_layoutOperands()
{
    const ConditionType eType( impl_getCurrentConditionType() );
    const ComparisonOperation eOperation( impl_getCurrentComparisonOperation() );

    const bool bIsExpression = ( eType == eExpression );
    const bool bHaveRHS =
            (   ( eType == eFieldValueComparison )
            &&  (   ( eOperation == eBetween )
                ||  ( eOperation == eNotBetween )
                )
            );

    // the "condition type" list box
    m_pOperationList->Show( !bIsExpression );
    m_pOperandGlue->Show( bHaveRHS );
    m_pCondRHS->Show( bHaveRHS );
}

void Condition::impl_setCondition( const OUString& _rConditionFormula )
{
    // determine the condition's type and comparison operation
    ConditionType eType( eFieldValueComparison );
    ComparisonOperation eOperation( eBetween );

    // LHS and RHS, matched below
    OUString sLHS, sRHS;

    if ( !_rConditionFormula.isEmpty() )
    {
        // the unprefixed expression which forms the condition
        ReportFormula aFormula( _rConditionFormula );
        OSL_ENSURE( aFormula.getType() == ReportFormula::Expression, "Condition::setCondition: illegal formula!" );
        OUString sExpression;
        if ( aFormula.getType() == ReportFormula::Expression )
            sExpression = aFormula.getExpression();
        // as fallback, if the below matching does not succeed, assume
        // the whole expression is the LHS
        eType = eExpression;
        sLHS = sExpression;

        // the data field (or expression) to which our control is bound
        const ReportFormula aFieldContentFormula( m_rAction.getDataField() );
        const OUString sUnprefixedFieldContent( aFieldContentFormula.getBracketedFieldOrExpression() );

        // check whether one of the Field Value Expression Factories recognizes the expression
        for (   ConditionalExpressions::const_iterator exp = m_aConditionalExpressions.begin();
                exp != m_aConditionalExpressions.end();
                ++exp
            )
        {
            if ( exp->second->matchExpression( sExpression, sUnprefixedFieldContent, sLHS, sRHS ) )
            {
                eType = eFieldValueComparison;
                eOperation = exp->first;
                break;
            }
        }
    }

    // update UI
    m_pConditionType->SelectEntryPos( static_cast<sal_uInt16>(eType) );
    m_pOperationList->SelectEntryPos( static_cast<sal_uInt16>(eOperation) );
    m_pCondLHS->SetText( sLHS );
    m_pCondRHS->SetText( sRHS );

    // re-layout
    impl_layoutOperands();
}


void Condition::setCondition( const uno::Reference< report::XFormatCondition >& _rxCondition )
{
    OSL_PRECOND( _rxCondition.is(), "Condition::setCondition: empty condition object!" );
    if ( !_rxCondition.is() )
        return;

    OUString sConditionFormula;
    try
    {
        if ( _rxCondition.is() )
            sConditionFormula =  _rxCondition->getFormula();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
    impl_setCondition( sConditionFormula );
    updateToolbar( _rxCondition.get() );
}


void Condition::updateToolbar(const uno::Reference< report::XReportControlFormat >& _xReportControlFormat)
{
    OSL_ENSURE(_xReportControlFormat.is(),"XReportControlFormat is NULL!");
    if ( _xReportControlFormat.is() )
    {
        ToolBox::ImplToolItems::size_type nItemCount = m_pActions->GetItemCount();
        for (ToolBox::ImplToolItems::size_type j = 0; j< nItemCount; ++j)
        {
            sal_uInt16 nItemId = m_pActions->GetItemId(j);
            m_pActions->CheckItem( nItemId, OReportController::isFormatCommandEnabled(mapToolbarItemToSlotId(nItemId),
                _xReportControlFormat ) );
        }

        try
        {
            vcl::Font aBaseFont( Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont() );
            SvxFont aFont( VCLUnoHelper::CreateFont( _xReportControlFormat->getFontDescriptor(), aBaseFont ) );
            aFont.SetFontHeight(OutputDevice::LogicToLogic(Size(0, aFont.GetFontHeight()), MapMode(MapUnit::MapPoint), MapMode(MapUnit::MapTwip)).Height());
            aFont.SetEmphasisMark( static_cast< FontEmphasisMark >( _xReportControlFormat->getControlTextEmphasis() ) );
            aFont.SetRelief( static_cast< FontRelief >( _xReportControlFormat->getCharRelief() ) );
            aFont.SetColor( Color(_xReportControlFormat->getCharColor()) );
            m_pPreview->SetFont( aFont, aFont, aFont );
            m_pPreview->SetBackColor( Color(_xReportControlFormat->getControlBackground()) );
            m_pPreview->SetTextLineColor( Color( _xReportControlFormat->getCharUnderlineColor() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
    }
}

void Condition::fillFormatCondition(const uno::Reference< report::XFormatCondition >& _xCondition)
{
    const ConditionType eType( impl_getCurrentConditionType() );
    const ComparisonOperation eOperation( impl_getCurrentComparisonOperation() );

    const OUString sLHS( m_pCondLHS->GetText() );
    const OUString sRHS( m_pCondRHS->GetText() );

    OUString sUndecoratedFormula( sLHS );

    if ( eType == eFieldValueComparison )
    {
        ReportFormula aFieldContentFormula( m_rAction.getDataField() );
        OUString sUnprefixedFieldContent( aFieldContentFormula.getBracketedFieldOrExpression() );

        PConditionalExpression pFactory( m_aConditionalExpressions[ eOperation ] );
        sUndecoratedFormula = pFactory->assembleExpression( sUnprefixedFieldContent, sLHS, sRHS );
    }

    ReportFormula aFormula( ReportFormula::Expression, sUndecoratedFormula );
    _xCondition->setFormula( aFormula.getCompleteFormula() );
}

void Condition::setConditionIndex( size_t _nCondIndex, size_t _nCondCount )
{
    m_nCondIndex = _nCondIndex;
    OUString sHeader( RptResId( STR_NUMBERED_CONDITION ) );
    sHeader = sHeader.replaceFirst( "$number$", OUString::number( _nCondIndex + 1) );
    m_pHeader->SetText( sHeader );

    m_pMoveUp->Enable( _nCondIndex > 0 );
    OSL_PRECOND( _nCondCount > 0, "Condition::setConditionIndex: having no conditions at all is nonsense!" );
    m_pMoveDown->Enable( _nCondIndex < _nCondCount - 1 );
}


bool Condition::isEmpty() const
{
    return m_pCondLHS->GetText().isEmpty();
}


} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
