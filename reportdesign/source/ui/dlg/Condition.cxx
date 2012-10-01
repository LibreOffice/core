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
#include "RptResId.hrc"
#include "helpids.hrc"
#include "reportformula.hxx"
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/ui/ImageType.hpp>

#define ITEMID_COLOR
#define ITEMID_BRUSH
#include <svx/tbcontrl.hxx>
#include <svx/svxids.hrc>
#include <svx/xtable.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/imgdef.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bmpacc.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>
#include <svtools/valueset.hxx>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

ConditionField::ConditionField( Condition* _pParent, const ResId& _rResId ) : Edit(_pParent,_rResId)
,m_pParent(_pParent)
,m_aFormula(this)
{
    m_pSubEdit = new Edit(this,0);
    SetSubEdit(m_pSubEdit);
    m_pSubEdit->EnableRTL( sal_False );
    m_pSubEdit->SetPosPixel( Point() );

    m_aFormula.SetText(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("...")));
    m_aFormula.SetClickHdl( LINK( this, ConditionField, OnFormula ) );
    m_aFormula.Show();
    m_pSubEdit->Show();
    Resize();
}
// -----------------------------------------------------------------------------
ConditionField::~ConditionField()
{
    SetSubEdit(NULL);
    delete m_pSubEdit;
}
// -----------------------------------------------------------------------------
void ConditionField::Resize()
{
    Edit::Resize();
    const Size aSize = GetOutputSizePixel();
    const Size aButtonSize( LogicToPixel( Size( 12, 0 ), MAP_APPFONT ).Width(),aSize.Height());
    const Point aButtonPos(aSize.Width() - aButtonSize.Width(), 0);
    m_aFormula.SetPosSizePixel(aButtonPos,aButtonSize);
    m_pSubEdit->SetPosSizePixel(Point(0,0),Size(aButtonPos.X() ,aSize.Height()));
}
// -----------------------------------------------------------------------------
IMPL_LINK( ConditionField, OnFormula, Button*, /*_pClickedButton*/ )
{
    ::rtl::OUString sFormula(m_pSubEdit->GetText());
    const sal_Int32 nLen = sFormula.getLength();
    if ( nLen )
    {
        ReportFormula aFormula( sFormula );
        sFormula = aFormula.getCompleteFormula();
    }
    uno::Reference< awt::XWindow> xInspectorWindow = VCLUnoHelper::GetInterface(this);
    uno::Reference< beans::XPropertySet> xProp(m_pParent->getController().getRowSet(),uno::UNO_QUERY);
    if ( rptui::openDialogFormula_nothrow( sFormula, m_pParent->getController().getContext(),xInspectorWindow,xProp ) )
    {
        ReportFormula aFormula( sFormula );
        m_pSubEdit->SetText(aFormula.getUndecoratedContent());
    }
    return 0L;
}
//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================
#ifndef WB_NO_DIRECTSELECT
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#endif

#define PALETTE_X 10
#define PALETTE_Y 10
#define PALETTE_SIZE (PALETTE_X * PALETTE_Y)
class OColorPopup : public FloatingWindow
{
    DECL_LINK( SelectHdl, void * );
    Condition* m_pCondition;
    sal_uInt16      m_nSlotId;
public:
    OColorPopup(Window* _pParent,Condition* _pCondition);
    ValueSet        m_aColorSet;

    virtual void KeyInput( const KeyEvent& rKEvt );
    virtual void Resize();

    void StartSelection();
    void SetSlotId(sal_uInt16 _nSlotId);
};
// -----------------------------------------------------------------------------
OColorPopup::OColorPopup(Window* _pParent,Condition* _pCondition)
:FloatingWindow(_pParent, WinBits( WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK|WB_DIALOGCONTROL ))
,m_pCondition(_pCondition)
,m_nSlotId(0)
,m_aColorSet( this, WinBits( WB_ITEMBORDER | WB_NAMEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT) )
{
    m_aColorSet.SetHelpId( HID_RPT_POPUP_COLOR_CTRL );
    SetHelpId( HID_RPT_POPUP_COLOR );
    const Size aSize12( 13, 13 );
    short i = 0;
    XColorListRef pColorList( XColorList::CreateStdColorList() );
    long nCount = pColorList->Count();
    XColorEntry* pEntry = NULL;
    Color aColWhite( COL_WHITE );
    String aStrWhite( ModuleRes(STR_COLOR_WHITE) );

    if ( nCount > PALETTE_SIZE )
        // Show scrollbar if more than PALLETTE_SIZE colors are available
        m_aColorSet.SetStyle( m_aColorSet.GetStyle() | WB_VSCROLL );

    for ( i = 0; i < nCount; i++ )
    {
        pEntry = pColorList->GetColor(i);
        m_aColorSet.InsertItem( i+1, pEntry->GetColor(), pEntry->GetName() );
    }

    while ( i < PALETTE_SIZE )
    {
        // fill empty elements if less then PALLETTE_SIZE colors are available
        m_aColorSet.InsertItem( i+1, aColWhite, aStrWhite );
        i++;
    }

    m_aColorSet.SetSelectHdl( LINK( this, OColorPopup, SelectHdl ) );
    m_aColorSet.SetColCount( PALETTE_X );
    m_aColorSet.SetLineCount( PALETTE_Y );
    Size aSize = m_aColorSet.CalcWindowSizePixel( aSize12 );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    SetOutputSizePixel( aSize );
    m_aColorSet.Show();
}
// -----------------------------------------------------------------------------
void OColorPopup::KeyInput( const KeyEvent& rKEvt )
{
    m_aColorSet.KeyInput(rKEvt);
}

// -----------------------------------------------------------------------------
void OColorPopup::Resize()
{
    Size aSize = GetOutputSizePixel();
    aSize.Width()  -= 4;
    aSize.Height() -= 4;
    m_aColorSet.SetPosSizePixel( Point(2,2), aSize );
}

// -----------------------------------------------------------------------------
void OColorPopup::StartSelection()
{
    m_aColorSet.StartSelection();
}
// -----------------------------------------------------------------------------
void OColorPopup::SetSlotId(sal_uInt16 _nSlotId)
{
    m_nSlotId = _nSlotId;
    if ( SID_ATTR_CHAR_COLOR_BACKGROUND == _nSlotId || SID_BACKGROUND_COLOR == _nSlotId )
    {
        m_aColorSet.SetStyle( m_aColorSet.GetStyle() | WB_NONEFIELD );
        m_aColorSet.SetText( String(ModuleRes( STR_TRANSPARENT )) );
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK_NOARG(OColorPopup, SelectHdl)
{
    sal_uInt16 nItemId = m_aColorSet.GetSelectItemId();
    Color aColor( nItemId == 0 ? Color( COL_TRANSPARENT ) : m_aColorSet.GetItemColor( nItemId ) );

    /*  #i33380# Moved the following line above the Dispatch() calls.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    m_aColorSet.SetNoSelection();

    if ( IsInPopupMode() )
        EndPopupMode();

    m_pCondition->ApplyCommand( m_nSlotId, aColor );
    return 0;
}

// =============================================================================
// = Condition
// =============================================================================
// -----------------------------------------------------------------------------
Condition::Condition( Window* _pParent, IConditionalFormatAction& _rAction, ::rptui::OReportController& _rController )
    :Control(_pParent, ModuleRes(WIN_CONDITION))
    ,m_rController( _rController )
    ,m_rAction( _rAction )
    ,m_aHeader(this,            ModuleRes(FL_CONDITION_HEADER))
    ,m_aConditionType(this,       ModuleRes(LB_COND_TYPE))
    ,m_aOperationList( this,    ModuleRes(LB_OP))
    ,m_aCondLHS(this,           ModuleRes(ED_CONDITION_LHS))
    ,m_aOperandGlue(this,            ModuleRes(FT_AND))
    ,m_aCondRHS(this,           ModuleRes(ED_CONDITION_RHS))
    ,m_aActions(this,           ModuleRes(TB_FORMAT))
    ,m_aPreview(this,           ModuleRes(CRTL_FORMAT_PREVIEW))
    ,m_aMoveUp( this,           ModuleRes( BTN_MOVE_UP ) )
    ,m_aMoveDown( this,         ModuleRes( BTN_MOVE_DOWN ) )
    ,m_aAddCondition( this,     ModuleRes( BTN_ADD_CONDITION ) )
    ,m_aRemoveCondition( this,  ModuleRes( BTN_REMOVE_CONDITION ) )
    ,m_pColorFloat(NULL)
    ,m_pBtnUpdaterFontColor(NULL)
    ,m_pBtnUpdaterBackgroundColor(NULL)
    ,m_nCondIndex( 0 )
    ,m_nLastKnownWindowWidth( -1 )
    ,m_bInDestruction( false )
{
    FreeResource();
    m_aActions.SetStyle(m_aActions.GetStyle()|WB_LINESPACING);
    m_aCondLHS.GrabFocus();

    m_aConditionType.SetSelectHdl( LINK( this, Condition, OnTypeSelected ) );

    m_aOperationList.SetDropDownLineCount( 10 );
    m_aOperationList.SetSelectHdl( LINK( this, Condition, OnOperationSelected ) );

    m_aActions.SetSelectHdl(LINK(this, Condition, OnFormatAction));
    m_aActions.SetDropdownClickHdl( LINK( this, Condition, DropdownClick ) );
    setToolBox(&m_aActions);

    m_aMoveUp.SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_aMoveDown.SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_aAddCondition.SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_aRemoveCondition.SetClickHdl( LINK( this, Condition, OnConditionAction ) );

    m_aMoveUp.SetStyle( m_aMoveUp.GetStyle() | WB_NOPOINTERFOCUS );
    m_aMoveDown.SetStyle( m_aMoveDown.GetStyle() | WB_NOPOINTERFOCUS );
    m_aAddCondition.SetStyle( m_aMoveUp.GetStyle() | WB_NOPOINTERFOCUS | WB_CENTER | WB_VCENTER );
    m_aRemoveCondition.SetStyle( m_aMoveDown.GetStyle() | WB_NOPOINTERFOCUS | WB_CENTER | WB_VCENTER );

    Font aFont( m_aAddCondition.GetFont() );
    aFont.SetWeight( WEIGHT_BOLD );
    m_aAddCondition.SetFont( aFont );
    m_aRemoveCondition.SetFont( aFont );

    m_aOperandGlue.SetStyle( m_aOperandGlue.GetStyle() | WB_VCENTER );

    m_aConditionType.SelectEntryPos( 0 );
    m_aOperationList.SelectEntryPos( 0 );

    // the toolbar got its site automatically, ensure that the preview is positioned
    // right of it
    Size aRelatedControls( LogicToPixel( Size( RELATED_CONTROLS, 0 ), MAP_APPFONT ) );
    Point aToolbarPos( m_aActions.GetPosPixel() );
    Size aToolbarSize( m_aActions.GetSizePixel() );
    m_aPreview.setPosSizePixel( aToolbarPos.X() + aToolbarSize.Width() + 2 * aRelatedControls.Width(),
        0, 0, 0, WINDOW_POSSIZE_X );

    // ensure the toolbar is vertically centered, relative to the preview
    Size aPreviewSize( m_aPreview.GetSizePixel() );
    m_aActions.setPosSizePixel( 0, aToolbarPos.Y() + ( aPreviewSize.Height() - aToolbarSize.Height() ) / 2, 0, 0, WINDOW_POSSIZE_Y );

    m_pBtnUpdaterBackgroundColor = new ::svx::ToolboxButtonColorUpdater(
                                            SID_BACKGROUND_COLOR, SID_BACKGROUND_COLOR, &m_aActions );
    m_pBtnUpdaterFontColor = new ::svx::ToolboxButtonColorUpdater(
                                            SID_ATTR_CHAR_COLOR2, SID_ATTR_CHAR_COLOR2, &m_aActions, TBX_UPDATER_MODE_CHAR_COLOR_NEW );

    Show();

    impl_layoutAll();

    ConditionalExpressionFactory::getKnownConditionalExpressions( m_aConditionalExpressions );
}

// -----------------------------------------------------------------------------
Condition::~Condition()
{
    m_bInDestruction = true;

    delete m_pColorFloat;
    delete m_pBtnUpdaterFontColor;
    delete m_pBtnUpdaterBackgroundColor;
}
// -----------------------------------------------------------------------------
IMPL_LINK( Condition, DropdownClick, ToolBox*, /*pToolBar*/ )
{
    sal_uInt16 nId( m_aActions.GetCurItemId() );
    if ( !m_pColorFloat )
        m_pColorFloat = new OColorPopup(&m_aActions,this);

    sal_uInt16 nTextId = 0;
    switch(nId)
    {
        case SID_ATTR_CHAR_COLOR2:
            nTextId = STR_CHARCOLOR;
            break;
        case SID_BACKGROUND_COLOR:
            nTextId = STR_CHARBACKGROUND;
            break;
        default:
            break;
    }
    if ( nTextId )
        m_pColorFloat->SetText(String(ModuleRes(nTextId)));
    m_pColorFloat->SetSlotId(nId);
    m_pColorFloat->SetPosPixel(m_aActions.GetItemPopupPosition(nId,m_pColorFloat->GetSizePixel()));
    m_pColorFloat->StartPopupMode(&m_aActions);
    m_pColorFloat->StartSelection();

    return 1;
}
//------------------------------------------------------------------
IMPL_LINK( Condition, OnFormatAction, ToolBox*, /*NOTINTERESTEDIN*/ )
{
    Color aCol(COL_AUTO);
    ApplyCommand(m_aActions.GetCurItemId(),aCol);
    return 0L;
}

//------------------------------------------------------------------
IMPL_LINK( Condition, OnConditionAction, Button*, _pClickedButton )
{
    if ( _pClickedButton == &m_aMoveUp )
        m_rAction.moveConditionUp( getConditionIndex() );
    else if ( _pClickedButton == &m_aMoveDown )
        m_rAction.moveConditionDown( getConditionIndex() );
    else if ( _pClickedButton == &m_aAddCondition )
        m_rAction.addCondition( getConditionIndex() );
    else if ( _pClickedButton == &m_aRemoveCondition )
        m_rAction.deleteCondition( getConditionIndex() );
    return 0L;
}

//------------------------------------------------------------------------------
void Condition::ApplyCommand( sal_uInt16 _nCommandId, const ::Color& _rColor)
{
    if ( _nCommandId == SID_ATTR_CHAR_COLOR2 )
        m_pBtnUpdaterFontColor->Update( _rColor );
    else if ( _nCommandId == SID_BACKGROUND_COLOR )
        m_pBtnUpdaterBackgroundColor->Update( _rColor );

    m_rAction.applyCommand( m_nCondIndex, _nCommandId, _rColor );
}
//------------------------------------------------------------------------------
ImageList Condition::getImageList(sal_Int16 _eBitmapSet) const
{
    sal_Int16 nN = IMG_CONDFORMAT_DLG_SC;
    if ( _eBitmapSet == SFX_SYMBOLS_SIZE_LARGE )
        nN = IMG_CONDFORMAT_DLG_LC;
    return ImageList(ModuleRes(nN));
}
//------------------------------------------------------------------
void Condition::resizeControls(const Size& _rDiff)
{
    // we use large images so we must change them
    if ( _rDiff.Width() || _rDiff.Height() )
    {
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void Condition::Paint( const Rectangle& rRect )
{
    Control::Paint(rRect);

    // draw border
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    ColorChanger aColors( this, rStyleSettings.GetShadowColor(), rStyleSettings.GetDialogColor() );
    DrawRect( impl_getToolBarBorderRect() );
}
// -----------------------------------------------------------------------------
void Condition::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        // Check if we need to get new images for normal/high contrast mode
        checkImageList();
    }
    else if ( nType == STATE_CHANGE_TEXT )
    {
        // The physical toolbar changed its outlook and shows another logical toolbar!
        // We have to set the correct high contrast mode on the new tbx manager.
        checkImageList();
    }
}
// -----------------------------------------------------------------------------
void Condition::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
        ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
        ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
    {
        // Check if we need to get new images for normal/high contrast mode
        checkImageList();
    }
}

// -----------------------------------------------------------------------------
void Condition::GetFocus()
{
    Control::GetFocus();
    if ( !m_bInDestruction )
        m_aCondLHS.GrabFocus();
}

// -----------------------------------------------------------------------------
void Condition::Resize()
{
    Control::Resize();
    impl_layoutAll();
}

// -----------------------------------------------------------------------------
Rectangle Condition::impl_getToolBarBorderRect() const
{
    const Point aToolbarPos( m_aActions.GetPosPixel() );
    const Size aToolbarSize( m_aActions.GetSizePixel() );
    const Size aRelatedControls = LogicToPixel( Size( RELATED_CONTROLS, RELATED_CONTROLS ), MAP_APPFONT );

    Rectangle aBorderRect( aToolbarPos, aToolbarSize );
    aBorderRect.Left() -= aRelatedControls.Width();
    aBorderRect.Top() -= aRelatedControls.Height();
    aBorderRect.Right() += aRelatedControls.Width();
    aBorderRect.Bottom() += aRelatedControls.Height();

    return aBorderRect;
}

// -----------------------------------------------------------------------------
void Condition::impl_layoutAll()
{
    // if our width changed, resize/-position some controls
    const Size aSize( GetOutputSizePixel() );
    if ( aSize.Width() == m_nLastKnownWindowWidth )
        return;

    m_nLastKnownWindowWidth = aSize.Width();

    const Size aRelatedControls( LogicToPixel( Size( RELATED_CONTROLS, RELATED_CONTROLS ), MAP_APPFONT ) );
    const Size aUnrelatedControls( LogicToPixel( Size( UNRELATED_CONTROLS, 0 ), MAP_APPFONT ) );
    const Point aRow1( LogicToPixel( Point( 0, ROW_1_POS ), MAP_APPFONT ) );
    const Point aRow3( LogicToPixel( Point( 0, ROW_3_POS ), MAP_APPFONT ) );

    // resize the header line
    m_aHeader.setPosSizePixel( 0, 0, aSize.Width() - 2 * aRelatedControls.Width(), 0, WINDOW_POSSIZE_WIDTH );

    // position the up/down buttons
    const Size aButtonSize( LogicToPixel( Size( IMAGE_BUTTON_WIDTH, IMAGE_BUTTON_HEIGHT ), MAP_APPFONT ) );
    Point aButtonPos( aSize.Width() - aUnrelatedControls.Width() - aButtonSize.Width(), aRow1.Y() );
    m_aMoveUp.setPosSizePixel( aButtonPos.X(), aButtonPos.Y(), aButtonSize.Width(), aButtonSize.Height() );
    aButtonPos.Move( 0, aButtonSize.Height() + aRelatedControls.Height() );
    m_aMoveDown.setPosSizePixel( aButtonPos.X(), aButtonPos.Y(), aButtonSize.Width(), aButtonSize.Height() );

    // resize the preview
    const long nNewPreviewRight = aButtonPos.X() - aRelatedControls.Width();

    const Point aPreviewPos( m_aPreview.GetPosPixel() );
    OSL_ENSURE( aPreviewPos.X() < nNewPreviewRight, "Condition::impl_layoutAll: being *that* small should not be allowed!" );
    m_aPreview.setPosSizePixel( 0, 0, nNewPreviewRight - aPreviewPos.X(), 0, WINDOW_POSSIZE_WIDTH );

    // position the add/remove buttons
    aButtonPos = Point( nNewPreviewRight - aButtonSize.Width(), aRow3.Y() );
    m_aRemoveCondition.setPosSizePixel( aButtonPos.X(), aButtonPos.Y(), aButtonSize.Width(), aButtonSize.Height() );
    aButtonPos.Move( -( aButtonSize.Width() + aRelatedControls.Width() ), 0 );
    m_aAddCondition.setPosSizePixel( aButtonPos.X(), aButtonPos.Y(), aButtonSize.Width(), aButtonSize.Height() );

    // layout the operands input controls
    impl_layoutOperands();
}

// -----------------------------------------------------------------------------
IMPL_LINK( Condition, OnTypeSelected, ListBox*, /*_pNotInterestedIn*/ )
{
    impl_layoutOperands();
    return 0L;
}

// -----------------------------------------------------------------------------
IMPL_LINK( Condition, OnOperationSelected, ListBox*, /*_pNotInterestedIn*/ )
{
    impl_layoutOperands();
    return 0L;
}

// -----------------------------------------------------------------------------
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

    const Size aRelatedControls( LogicToPixel( Size( RELATED_CONTROLS, 0 ), MAP_APPFONT ) );
    const Rectangle aPreviewRect( m_aPreview.GetPosPixel(), m_aPreview.GetSizePixel() );

    // the "condition type" list box
    const Rectangle aCondTypeRect( m_aConditionType.GetPosPixel(), m_aConditionType.GetSizePixel() );
    const Point aOpListPos( aCondTypeRect.Right() + aRelatedControls.Width(), aCondTypeRect.Top() );
    const Size aOpListSize( LogicToPixel( Size( COND_OP_WIDTH, 60 ), MAP_APPFONT ) );
    m_aOperationList.setPosSizePixel( aOpListPos.X(), aOpListPos.Y(),aOpListSize.Width(), aOpListSize.Height() );
    m_aOperationList.Show( !bIsExpression );

    // the LHS input field
    Point aLHSPos( aOpListPos.X() + aOpListSize.Width() + aRelatedControls.Width(), aOpListPos.Y() );
    if ( bIsExpression )
        aLHSPos.X() = aOpListPos.X();
    Size aLHSSize( LogicToPixel( Size( EDIT_WIDTH, EDIT_HEIGHT ), MAP_APPFONT ) );
    if ( !bHaveRHS )
        aLHSSize.Width() = aPreviewRect.Right() - aLHSPos.X();
    m_aCondLHS.setPosSizePixel( aLHSPos.X(), aLHSPos.Y(), aLHSSize.Width(), aLHSSize.Height() );

    if ( bHaveRHS )
    {
        // the "and" text being the glue between LHS and RHS
        const Point aOpGluePos( aLHSPos.X() + aLHSSize.Width() + aRelatedControls.Width(), aLHSPos.Y() );
        const Size aOpGlueSize( m_aOperandGlue.GetTextWidth( m_aOperandGlue.GetText() ) + aRelatedControls.Width(), aLHSSize.Height() );
        m_aOperandGlue.setPosSizePixel( aOpGluePos.X(), aOpGluePos.Y(), aOpGlueSize.Width(), aOpGlueSize.Height() );

        // the RHS input field
        const Point aRHSPos( aOpGluePos.X() + aOpGlueSize.Width() + aRelatedControls.Width(), aOpGluePos.Y() );
        const Size aRHSSize( aPreviewRect.Right() - aRHSPos.X(), aLHSSize.Height() );
        m_aCondRHS.setPosSizePixel( aRHSPos.X(), aRHSPos.Y(), aRHSSize.Width(), aRHSSize.Height() );
    }

    m_aOperandGlue.Show( bHaveRHS );
    m_aCondRHS.Show( bHaveRHS );
}

// -----------------------------------------------------------------------------
void Condition::impl_setCondition( const ::rtl::OUString& _rConditionFormula )
{
    // determine the condition's type and comparison operation
    ConditionType eType( eFieldValueComparison );
    ComparisonOperation eOperation( eBetween );

    // LHS and RHS, matched below
    ::rtl::OUString sLHS, sRHS;

    if ( !_rConditionFormula.isEmpty() )
    {
        // the unprefixed expression which forms the condition
        ReportFormula aFormula( _rConditionFormula );
        OSL_ENSURE( aFormula.getType() == ReportFormula::Expression, "Condition::setCondition: illegal formula!" );
        ::rtl::OUString sExpression;
        if ( aFormula.getType() == ReportFormula::Expression )
            sExpression = aFormula.getExpression();
        // as fallback, if the below matching does not succeed, assume
        // the whole expression is the LHS
        eType = eExpression;
        sLHS = sExpression;

        // the data field (or expression) to which our control is bound
        const ReportFormula aFieldContentFormula( m_rAction.getDataField() );
        const ::rtl::OUString sUnprefixedFieldContent( aFieldContentFormula.getBracketedFieldOrExpression() );

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
    m_aConditionType.SelectEntryPos( (sal_uInt16)eType );
    m_aOperationList.SelectEntryPos( (sal_uInt16)eOperation );
    m_aCondLHS.SetText( sLHS );
    m_aCondRHS.SetText( sRHS );

    // re-layout
    impl_layoutOperands();
}

// -----------------------------------------------------------------------------
void Condition::setCondition( const uno::Reference< report::XFormatCondition >& _rxCondition )
{
    OSL_PRECOND( _rxCondition.is(), "Condition::setCondition: empty condition object!" );
    if ( !_rxCondition.is() )
        return;

    ::rtl::OUString sConditionFormula;
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

// -----------------------------------------------------------------------------
void Condition::updateToolbar(const uno::Reference< report::XReportControlFormat >& _xReportControlFormat)
{
    OSL_ENSURE(_xReportControlFormat.is(),"XReportControlFormat is NULL!");
    if ( _xReportControlFormat.is() )
    {
        sal_uInt16 nItemCount = m_aActions.GetItemCount();
        for (sal_uInt16 j = 0; j< nItemCount; ++j)
        {
            sal_uInt16 nItemId = m_aActions.GetItemId(j);
            m_aActions.CheckItem( nItemId, m_rController.isFormatCommandEnabled( nItemId, _xReportControlFormat ) );
        }

        try
        {
            Font aBaseFont( Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont() );
            SvxFont aFont( VCLUnoHelper::CreateFont( _xReportControlFormat->getFontDescriptor(), aBaseFont ) );
            aFont.SetHeight( OutputDevice::LogicToLogic( Size( 0, (sal_Int32)aFont.GetHeight() ), MAP_POINT, MAP_TWIP ).Height());
            aFont.SetEmphasisMark( static_cast< FontEmphasisMark >( _xReportControlFormat->getControlTextEmphasis() ) );
            aFont.SetRelief( static_cast< FontRelief >( _xReportControlFormat->getCharRelief() ) );
            aFont.SetColor( _xReportControlFormat->getCharColor() );
            m_aPreview.SetFont( aFont, aFont, aFont );
            m_aPreview.SetBackColor( _xReportControlFormat->getControlBackground() );
            m_aPreview.SetTextLineColor( Color( _xReportControlFormat->getCharUnderlineColor() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------
void Condition::fillFormatCondition(const uno::Reference< report::XFormatCondition >& _xCondition)
{
    const ConditionType eType( impl_getCurrentConditionType() );
    const ComparisonOperation eOperation( impl_getCurrentComparisonOperation() );

    const ::rtl::OUString sLHS( m_aCondLHS.GetText() );
    const ::rtl::OUString sRHS( m_aCondRHS.GetText() );

    ::rtl::OUString sUndecoratedFormula( sLHS );

    if ( eType == eFieldValueComparison )
    {
        ReportFormula aFieldContentFormula( m_rAction.getDataField() );
        ::rtl::OUString sUnprefixedFieldContent( aFieldContentFormula.getBracketedFieldOrExpression() );

        PConditionalExpression pFactory( m_aConditionalExpressions[ eOperation ] );
        sUndecoratedFormula = pFactory->assembleExpression( sUnprefixedFieldContent, sLHS, sRHS );
    }

    ReportFormula aFormula( ReportFormula::Expression, sUndecoratedFormula );
    _xCondition->setFormula( aFormula.getCompleteFormula() );
}
// -----------------------------------------------------------------------------
void Condition::setConditionIndex( size_t _nCondIndex, size_t _nCondCount )
{
    m_nCondIndex = _nCondIndex;
    String sHeader( ModuleRes( STR_NUMBERED_CONDITION ) );
    sHeader.SearchAndReplaceAscii( "$number$", String::CreateFromInt32( _nCondIndex + 1 ) );
    m_aHeader.SetText( sHeader );

    m_aMoveUp.Enable( _nCondIndex > 0 );
    OSL_PRECOND( _nCondCount > 0, "Condition::setConditionIndex: having no conditions at all is nonsense!" );
    m_aMoveDown.Enable( _nCondIndex < _nCondCount - 1 );
}

// -----------------------------------------------------------------------------
bool Condition::isEmpty() const
{
    return m_aCondLHS.GetText().Len() == 0;
}

// =============================================================================
} // rptui
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
