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
#include "UITools.hxx"
#include "CondFormat.hxx"
#include "CondFormat.hrc"
#include "RptResId.hrc"
#include "ReportController.hxx"
#include "ModuleHelper.hxx"
#include "ColorChanger.hxx"
#include "helpids.hrc"
#include "reportformula.hxx"
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

IMPL_LINK_TYPED( ConditionField, OnFormula, Button*, _pClickedButton, void )
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

// class SvxColorWindow_Impl --------------------------------------------------
#ifndef WB_NO_DIRECTSELECT
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#endif

#define PALETTE_X 10
#define PALETTE_Y 10
#define PALETTE_SIZE (PALETTE_X * PALETTE_Y)
class OColorPopup : public FloatingWindow
{
    DECL_LINK_TYPED( SelectHdl, ValueSet*, void );
    VclPtr<Condition>      m_pCondition;
    sal_uInt16             m_nSlotId;
public:
    OColorPopup(vcl::Window* _pParent,Condition* _pCondition);
    virtual ~OColorPopup();
    virtual void dispose() override;
    VclPtr<ValueSet>        m_aColorSet;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void Resize() override;

    void StartSelection();
    void SetSlotId(sal_uInt16 _nSlotId);
};

OColorPopup::OColorPopup(vcl::Window* _pParent,Condition* _pCondition)
:FloatingWindow(_pParent, WinBits( WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK|WB_DIALOGCONTROL ))
,m_pCondition(_pCondition)
,m_nSlotId(0)
,m_aColorSet( VclPtr<ValueSet>::Create(this, WinBits( WB_ITEMBORDER | WB_NAMEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT)) )
{
    m_aColorSet->SetHelpId( HID_RPT_POPUP_COLOR_CTRL );
    SetHelpId( HID_RPT_POPUP_COLOR );
    const Size aSize12( 13, 13 );
    short i = 0;
    XColorListRef pColorList( XColorList::CreateStdColorList() );
    long nCount = pColorList->Count();
    Color aColWhite( COL_WHITE );
    OUString aStrWhite( ModuleRes(STR_COLOR_WHITE) );

    if ( nCount > PALETTE_SIZE )
        // Show scrollbar if more than PALLETTE_SIZE colors are available
        m_aColorSet->SetStyle( m_aColorSet->GetStyle() | WB_VSCROLL );

    for ( i = 0; i < nCount; i++ )
    {
        XColorEntry* pEntry = pColorList->GetColor(i);
        m_aColorSet->InsertItem( i+1, pEntry->GetColor(), pEntry->GetName() );
    }

    while ( i < PALETTE_SIZE )
    {
        // fill empty elements if less then PALLETTE_SIZE colors are available
        m_aColorSet->InsertItem( i+1, aColWhite, aStrWhite );
        i++;
    }

    m_aColorSet->SetSelectHdl( LINK( this, OColorPopup, SelectHdl ) );
    m_aColorSet->SetColCount( PALETTE_X );
    m_aColorSet->SetLineCount( PALETTE_Y );
    Size aSize = m_aColorSet->CalcWindowSizePixel( aSize12 );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    SetOutputSizePixel( aSize );
    m_aColorSet->Show();
}

OColorPopup::~OColorPopup()
{
    disposeOnce();
}

void OColorPopup::dispose()
{
    disposeBuilder();
    m_aColorSet.clear();
    m_pCondition.clear();
    FloatingWindow::dispose();
}

void OColorPopup::KeyInput( const KeyEvent& rKEvt )
{
    m_aColorSet->KeyInput(rKEvt);
}

void OColorPopup::Resize()
{
    Size aSize = GetOutputSizePixel();
    aSize.Width()  -= 4;
    aSize.Height() -= 4;
    m_aColorSet->SetPosSizePixel( Point(2,2), aSize );
}

void OColorPopup::StartSelection()
{
    m_aColorSet->StartSelection();
}

void OColorPopup::SetSlotId(sal_uInt16 _nSlotId)
{
    m_nSlotId = _nSlotId;
    if ( SID_ATTR_CHAR_COLOR_BACKGROUND == _nSlotId || SID_BACKGROUND_COLOR == _nSlotId )
    {
        m_aColorSet->SetStyle( m_aColorSet->GetStyle() | WB_NONEFIELD );
        m_aColorSet->SetText( OUString(ModuleRes( STR_TRANSPARENT )) );
    }
}

IMPL_LINK_NOARG_TYPED(OColorPopup, SelectHdl, ValueSet*, void)
{
    sal_uInt16 nItemId = m_aColorSet->GetSelectItemId();
    Color aColor( nItemId == 0 ? Color( COL_TRANSPARENT ) : m_aColorSet->GetItemColor( nItemId ) );

    /*  #i33380# Moved the following line above the Dispatch() calls.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    m_aColorSet->SetNoSelection();

    if ( IsInPopupMode() )
        EndPopupMode();

    m_pCondition->ApplyCommand( m_nSlotId, aColor );
}


// = Condition


Condition::Condition( vcl::Window* _pParent, IConditionalFormatAction& _rAction, ::rptui::OReportController& _rController )
    :VclHBox(_pParent)
    ,m_rController( _rController )
    ,m_rAction( _rAction )
    ,m_pColorFloat(nullptr)
    ,m_pBtnUpdaterFontColor(nullptr)
    ,m_pBtnUpdaterBackgroundColor(nullptr)
    ,m_nCondIndex( 0 )
    ,m_bInDestruction( false )
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), "modules/dbreport/ui/conditionwin.ui");

    get(m_pHeader, "headerLabel");
    get(m_pConditionType, "typeCombobox");
    get(m_pOperationList, "opCombobox");
    m_pCondLHS = new ConditionField(this, get<Edit>("lhsEntry"), get<PushButton>("lhsButton"));
    get(m_pOperandGlue, "andLabel");
    m_pCondRHS = new ConditionField(this, get<Edit>("rhsEntry"), get<PushButton>("rhsButton"));
    get(m_pActions, "formatToolbox");
    get(m_pPreview, "previewDrawingarea");
    get(m_pMoveUp, "upButton");
    get(m_pMoveDown, "downButton");
    get(m_pAddCondition, "addButton");
    get(m_pRemoveCondition, "removeButton");

    m_pActions->SetStyle(m_pActions->GetStyle()|WB_LINESPACING);
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

    m_pMoveUp->SetStyle( m_pMoveUp->GetStyle() | WB_NOPOINTERFOCUS );
    m_pMoveDown->SetStyle( m_pMoveDown->GetStyle() | WB_NOPOINTERFOCUS );
    m_pAddCondition->SetStyle( m_pAddCondition->GetStyle() | WB_NOPOINTERFOCUS );
    m_pRemoveCondition->SetStyle( m_pRemoveCondition->GetStyle() | WB_NOPOINTERFOCUS );

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

    m_pBtnUpdaterBackgroundColor = new svx::ToolboxButtonColorUpdater(
                                            SID_BACKGROUND_COLOR, m_nBackgroundColorId, m_pActions );
    m_pBtnUpdaterFontColor = new svx::ToolboxButtonColorUpdater(
                                            SID_ATTR_CHAR_COLOR2, m_nFontColorId, m_pActions );

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

    delete m_pBtnUpdaterFontColor;
    delete m_pCondLHS;
    delete m_pCondRHS;
    delete m_pBtnUpdaterBackgroundColor;
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
    disposeBuilder();
    VclHBox::dispose();
}

IMPL_LINK_NOARG_TYPED( Condition, DropdownClick, ToolBox*, void )
{
    sal_uInt16 nId( m_pActions->GetCurItemId() );
    if ( !m_pColorFloat )
        m_pColorFloat = VclPtr<OColorPopup>::Create(m_pActions,this);

    sal_uInt16 nTextId = 0;
    if (nId == m_nFontColorId)
    {
        nTextId = STR_CHARCOLOR;
    }
    else if (nId == m_nBackgroundColorId)
    {
        nTextId = STR_CHARBACKGROUND;
    }
    if ( nTextId )
        m_pColorFloat->SetText(OUString(ModuleRes(nTextId)));
    m_pColorFloat->SetSlotId(mapToolbarItemToSlotId(nId));
    m_pColorFloat->SetPosPixel(m_pActions->GetItemPopupPosition(nId,m_pColorFloat->GetSizePixel()));
    m_pColorFloat->StartPopupMode(m_pActions);
    m_pColorFloat->StartSelection();
}

IMPL_LINK_NOARG_TYPED( Condition, OnFormatAction, ToolBox*, void )
{
    Color aCol(COL_AUTO);
    ApplyCommand(mapToolbarItemToSlotId(m_pActions->GetCurItemId()),aCol);
}

IMPL_LINK_TYPED( Condition, OnConditionAction, Button*, _pClickedButton, void )
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

IMPL_LINK_NOARG_TYPED( Condition, OnTypeSelected, ListBox&, void )
{
    impl_layoutOperands();
}


IMPL_LINK_NOARG_TYPED( Condition, OnOperationSelected, ListBox&, void )
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
    m_pConditionType->SelectEntryPos( (sal_uInt16)eType );
    m_pOperationList->SelectEntryPos( (sal_uInt16)eOperation );
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
        DBG_UNHANDLED_EXCEPTION();
    }
    impl_setCondition( sConditionFormula );
    updateToolbar( _rxCondition.get() );
}


void Condition::updateToolbar(const uno::Reference< report::XReportControlFormat >& _xReportControlFormat)
{
    OSL_ENSURE(_xReportControlFormat.is(),"XReportControlFormat is NULL!");
    if ( _xReportControlFormat.is() )
    {
        sal_uInt16 nItemCount = m_pActions->GetItemCount();
        for (sal_uInt16 j = 0; j< nItemCount; ++j)
        {
            sal_uInt16 nItemId = m_pActions->GetItemId(j);
            m_pActions->CheckItem( nItemId, OReportController::isFormatCommandEnabled(mapToolbarItemToSlotId(nItemId),
                _xReportControlFormat ) );
        }

        try
        {
            vcl::Font aBaseFont( Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont() );
            SvxFont aFont( VCLUnoHelper::CreateFont( _xReportControlFormat->getFontDescriptor(), aBaseFont ) );
            aFont.SetHeight( OutputDevice::LogicToLogic( Size( 0, (sal_Int32)aFont.GetHeight() ), MAP_POINT, MAP_TWIP ).Height());
            aFont.SetEmphasisMark( static_cast< FontEmphasisMark >( _xReportControlFormat->getControlTextEmphasis() ) );
            aFont.SetRelief( static_cast< FontRelief >( _xReportControlFormat->getCharRelief() ) );
            aFont.SetColor( _xReportControlFormat->getCharColor() );
            m_pPreview->SetFont( aFont, aFont, aFont );
            m_pPreview->SetBackColor( _xReportControlFormat->getControlBackground() );
            m_pPreview->SetTextLineColor( Color( _xReportControlFormat->getCharUnderlineColor() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
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
    OUString sHeader( ModuleRes( STR_NUMBERED_CONDITION ) );
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
