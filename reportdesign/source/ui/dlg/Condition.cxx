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
#include <reportformula.hxx>

#include <svx/PaletteManager.hxx>
#include <svx/svxids.hrc>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <tools/diagnose_ex.h>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

ConditionField::ConditionField(Condition* pParent, std::unique_ptr<weld::Entry> xSubEdit,
                               std::unique_ptr<weld::Button> xFormula)
    : m_pParent(pParent)
    , m_xSubEdit(std::move(xSubEdit))
    , m_xFormula(std::move(xFormula))
{
    m_xFormula->set_label("...");
    m_xFormula->connect_clicked( LINK( this, ConditionField, OnFormula ) );
}

IMPL_LINK_NOARG(ConditionField, OnFormula, weld::Button&, void)
{
    OUString sFormula(m_xSubEdit->get_text());
    const sal_Int32 nLen = sFormula.getLength();
    if ( nLen )
    {
        ReportFormula aFormula( sFormula );
        sFormula = aFormula.getCompleteFormula();
    }
    uno::Reference< awt::XWindow> xInspectorWindow = m_pParent->GetXWindow();
    uno::Reference< beans::XPropertySet> xProp(m_pParent->getController().getRowSet(),uno::UNO_QUERY);
    if ( rptui::openDialogFormula_nothrow( sFormula, m_pParent->getController().getContext(),xInspectorWindow,xProp ) )
    {
        ReportFormula aFormula( sFormula );
        m_xSubEdit->set_text(aFormula.getUndecoratedContent());
    }
}

ConditionColorWrapper::ConditionColorWrapper(Condition* pControl, sal_uInt16 nSlotId)
    : mpControl(pControl)
    , mnSlotId(nSlotId)
{
}

void ConditionColorWrapper::operator()(
    [[maybe_unused]] const OUString& /*rCommand*/, const NamedColor& rNamedColor)
{
    mpControl->ApplyCommand(mnSlotId, rNamedColor);
}

// = Condition
Condition::Condition(weld::Container* pParent, weld::Window* pDialog, IConditionalFormatAction& _rAction, ::rptui::OReportController& _rController)
    : m_xPaletteManager(std::make_shared<PaletteManager>())
    , m_aBackColorWrapper(this, SID_BACKGROUND_COLOR)
    , m_aForeColorWrapper(this, SID_ATTR_CHAR_COLOR2)
    , m_rController(_rController)
    , m_rAction(_rAction)
    , m_nCondIndex(0)
    , m_pDialog(pDialog)
    , m_xBuilder(Application::CreateBuilder(pParent, "modules/dbreport/ui/conditionwin.ui"))
    , m_xContainer(m_xBuilder->weld_container("ConditionWin"))
    , m_xHeader(m_xBuilder->weld_label("headerLabel"))
    , m_xConditionType(m_xBuilder->weld_combo_box("typeCombobox"))
    , m_xOperationList(m_xBuilder->weld_combo_box("opCombobox"))
    , m_xOperandGlue(m_xBuilder->weld_label("andLabel"))
    , m_xActions(m_xBuilder->weld_toolbar("formatToolbox"))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, "previewDrawingarea", m_aPreview))
    , m_xMoveUp(m_xBuilder->weld_button("upButton"))
    , m_xMoveDown(m_xBuilder->weld_button("downButton"))
    , m_xAddCondition(m_xBuilder->weld_button("addButton"))
    , m_xRemoveCondition(m_xBuilder->weld_button("removeButton"))
{
    m_xCondLHS.reset(new ConditionField(this, m_xBuilder->weld_entry("lhsEntry"), m_xBuilder->weld_button("lhsButton")));
    m_xCondRHS.reset(new ConditionField(this, m_xBuilder->weld_entry("rhsEntry"), m_xBuilder->weld_button("rhsButton")));

    m_xCondLHS->grab_focus();

    m_xConditionType->connect_changed( LINK( this, Condition, OnTypeSelected ) );

    m_xOperationList->connect_changed( LINK( this, Condition, OnOperationSelected ) );

    m_xActions->connect_clicked(LINK(this, Condition, OnFormatAction));

    m_xMoveUp->connect_clicked( LINK( this, Condition, OnConditionAction ) );
    m_xMoveDown->connect_clicked( LINK( this, Condition, OnConditionAction ) );
    m_xAddCondition->connect_clicked( LINK( this, Condition, OnConditionAction ) );
    m_xRemoveCondition->connect_clicked( LINK( this, Condition, OnConditionAction ) );

    m_xConditionType->set_active(0);
    m_xOperationList->set_active(0);

    SetBackgroundDropdownClick();
    SetForegroundDropdownClick();

    m_xContainer->show();

    ConditionalExpressionFactory::getKnownConditionalExpressions( m_aConditionalExpressions );
}

sal_uInt16 Condition::mapToolbarItemToSlotId(std::string_view rItemId)
{
    if (rItemId == "bold")
        return SID_ATTR_CHAR_WEIGHT;
    if (rItemId == "italic")
        return SID_ATTR_CHAR_POSTURE;
    if (rItemId == "underline")
        return SID_ATTR_CHAR_UNDERLINE;
    if (rItemId == "background")
        return SID_BACKGROUND_COLOR;
    if (rItemId == "foreground")
        return SID_ATTR_CHAR_COLOR2;
    if (rItemId == "fontdialog")
        return SID_CHAR_DLG;
    return 0;
}

Condition::~Condition()
{
}

void Condition::SetBackgroundDropdownClick()
{
    m_xBackColorFloat.reset(new ColorWindow(
                            OUString() /*m_aCommandURL*/,
                            m_xPaletteManager,
                            m_aColorStatus,
                            SID_BACKGROUND_COLOR,
                            nullptr,
                            MenuOrToolMenuButton(m_xActions.get(), "background"),
                            [this]{ return m_pDialog; },
                            m_aBackColorWrapper));

    m_xActions->set_item_popover("background", m_xBackColorFloat->getTopLevel());
}

void Condition::SetForegroundDropdownClick()
{
    m_xForeColorFloat.reset(new ColorWindow(
                            OUString() /*m_aCommandURL*/,
                            m_xPaletteManager,
                            m_aColorStatus,
                            SID_ATTR_CHAR_COLOR2,
                            nullptr,
                            MenuOrToolMenuButton(m_xActions.get(), "foreground"),
                            [this]{ return m_pDialog; },
                            m_aForeColorWrapper));

    m_xActions->set_item_popover("foreground", m_xForeColorFloat->getTopLevel());
}


IMPL_LINK(Condition, OnFormatAction, const OString&, rIdent, void)
{
    ApplyCommand(mapToolbarItemToSlotId(rIdent),
                 NamedColor(COL_AUTO, "#" + COL_AUTO.AsRGBHexString()));
}

IMPL_LINK(Condition, OnConditionAction, weld::Button&, rClickedButton, void)
{
    if ( &rClickedButton == m_xMoveUp.get() )
        m_rAction.moveConditionUp( getConditionIndex() );
    else if ( &rClickedButton == m_xMoveDown.get() )
        m_rAction.moveConditionDown( getConditionIndex() );
    else if ( &rClickedButton == m_xAddCondition.get() )
        m_rAction.addCondition( getConditionIndex() );
    else if ( &rClickedButton == m_xRemoveCondition.get() )
        m_rAction.deleteCondition( getConditionIndex() );
}

void Condition::ApplyCommand( sal_uInt16 _nCommandId, const NamedColor& rNamedColor )
{
    m_rAction.applyCommand( m_nCondIndex, _nCommandId, rNamedColor.first );
}

IMPL_LINK_NOARG( Condition, OnTypeSelected, weld::ComboBox&, void )
{
    impl_layoutOperands();
}

IMPL_LINK_NOARG( Condition, OnOperationSelected, weld::ComboBox&, void )
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
    m_xOperationList->set_visible( !bIsExpression );
    m_xOperandGlue->set_visible( bHaveRHS );
    m_xCondRHS->set_visible( bHaveRHS );
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
        for (const auto& [rOperation, rxConditionalExpression] : m_aConditionalExpressions)
        {
            if ( rxConditionalExpression->matchExpression( sExpression, sUnprefixedFieldContent, sLHS, sRHS ) )
            {
                eType = eFieldValueComparison;
                eOperation = rOperation;
                break;
            }
        }
    }

    // update UI
    m_xConditionType->set_active(static_cast<sal_uInt16>(eType));
    m_xOperationList->set_active(static_cast<sal_uInt16>(eOperation));
    m_xCondLHS->set_text( sLHS );
    m_xCondRHS->set_text( sRHS );

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
    updateToolbar( _rxCondition );
}


void Condition::updateToolbar(const uno::Reference< report::XReportControlFormat >& _xReportControlFormat)
{
    OString aItems[] = { "bold", "italic", "underline", "fontdialog" };

    OSL_ENSURE(_xReportControlFormat.is(),"XReportControlFormat is NULL!");
    if ( !_xReportControlFormat.is() )
        return;

    for (size_t j = 0; j < SAL_N_ELEMENTS(aItems); ++j)
    {
        m_xActions->set_item_active(aItems[j], OReportController::isFormatCommandEnabled(mapToolbarItemToSlotId(aItems[j]),
            _xReportControlFormat));
    }

    try
    {
        vcl::Font aBaseFont( Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont() );
        SvxFont aFont( VCLUnoHelper::CreateFont( _xReportControlFormat->getFontDescriptor(), aBaseFont ) );
        aFont.SetFontHeight(OutputDevice::LogicToLogic(Size(0, aFont.GetFontHeight()), MapMode(MapUnit::MapPoint), MapMode(MapUnit::MapTwip)).Height());
        aFont.SetEmphasisMark( static_cast< FontEmphasisMark >( _xReportControlFormat->getControlTextEmphasis() ) );
        aFont.SetRelief( static_cast< FontRelief >( _xReportControlFormat->getCharRelief() ) );
        aFont.SetColor( Color(ColorTransparency, _xReportControlFormat->getCharColor()) );
        m_aPreview.SetFont( aFont, aFont, aFont );
        m_aPreview.SetTextLineColor( Color( ColorTransparency, _xReportControlFormat->getCharUnderlineColor() ) );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}

void Condition::fillFormatCondition(const uno::Reference< report::XFormatCondition >& _xCondition)
{
    const ConditionType eType( impl_getCurrentConditionType() );
    const ComparisonOperation eOperation( impl_getCurrentComparisonOperation() );

    const OUString sLHS( m_xCondLHS->get_text() );
    const OUString sRHS( m_xCondRHS->get_text() );

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
    m_xHeader->set_label( sHeader );

    m_xMoveUp->set_sensitive(_nCondIndex > 0);
    OSL_PRECOND( _nCondCount > 0, "Condition::setConditionIndex: having no conditions at all is nonsense!" );
    m_xMoveDown->set_sensitive(_nCondIndex < _nCondCount - 1);
}

bool Condition::isEmpty() const
{
    return m_xCondLHS->get_text().isEmpty();
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
