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


#include "optimizerdialog.hxx"

// -------------------
// - OptimizerDialog -
// -------------------
#include "pppoptimizer.hxx"
#include "graphiccollector.hxx"
#include "pagecollector.hxx"
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>

using namespace ::rtl;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;

// -----------------------------------------------------------------------------

void SetBold( OptimizerDialog& rOptimizerDialog, const OUString& rControl )
{
    FontDescriptor aFontDescriptor;
    if ( rOptimizerDialog.getControlProperty( rControl, TKGet( TK_FontDescriptor ) ) >>= aFontDescriptor )
    {
        aFontDescriptor.Weight = FontWeight::BOLD;
        rOptimizerDialog.setControlProperty( rControl, TKGet( TK_FontDescriptor ), Any( aFontDescriptor ) );
    }
}

// -----------------------------------------------------------------------------

OUString InsertSeparator( OptimizerDialog& rOptimizerDialog, const OUString& rControlName, sal_Int32 nOrientation,
                        sal_Int32 nPosX, sal_Int32 nPosY, sal_Int32 nWidth, sal_Int32 nHeight )
{
    OUString pNames[] = {
        TKGet( TK_Height ),
        TKGet( TK_Orientation ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( nHeight ),
        Any( nOrientation ),
        Any( nPosX ),
        Any( nPosY ),
        Any( sal_Int16( 0 ) ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rOptimizerDialog.insertControlModel( OUString( "com.sun.star.awt.UnoControlFixedLineModel"  ),
        rControlName, aNames, aValues );
    return rControlName;
}

// -----------------------------------------------------------------------------

OUString InsertButton( OptimizerDialog& rOptimizerDialog, const OUString& rControlName, Reference< XActionListener >& xActionListener,
    sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nTabIndex, sal_Bool bEnabled, PPPOptimizerTokenEnum nResID, sal_Int16 nPushButtonType )
{
    OUString pNames[] = {
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_Label ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_PushButtonType ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( bEnabled  ),
        Any( nHeight ),
        Any( rOptimizerDialog.getString( nResID ) ),
        Any( nXPos ),
        Any( nYPos ),
        Any( nPushButtonType ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };


    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rOptimizerDialog.insertButton( rControlName, xActionListener, aNames, aValues );
    return rControlName;
}

// -----------------------------------------------------------------------------

OUString InsertFixedText( OptimizerDialog& rOptimizerDialog, const OUString& rControlName, const OUString& rLabel,
                                sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Bool bMultiLine, sal_Bool bBold, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Height ),
        TKGet( TK_Label ),
        TKGet( TK_MultiLine ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( nHeight ),
        Any( rLabel ),
        Any( bMultiLine ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rOptimizerDialog.insertFixedText( rControlName, aNames, aValues );
    if ( bBold )
        SetBold( rOptimizerDialog, rControlName );
    return rControlName;
}

// -----------------------------------------------------------------------------

OUString InsertCheckBox( OptimizerDialog& rOptimizerDialog, const OUString& rControlName,
    const Reference< XItemListener > xItemListener, const OUString& rLabel,
        sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_Label ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( nHeight ),
        Any( rLabel ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XCheckBox > xCheckBox( rOptimizerDialog.insertCheckBox( rControlName, aNames, aValues ) );
    if ( xItemListener.is() )
        xCheckBox->addItemListener( xItemListener );
    return rControlName;
}

// -----------------------------------------------------------------------------

OUString InsertFormattedField( OptimizerDialog& rOptimizerDialog, const OUString& rControlName,
        const Reference< XTextListener > xTextListener, const Reference< XSpinListener > xSpinListener, sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth,
            double fEffectiveMin, double fEffectiveMax, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_EffectiveMax ),
        TKGet( TK_EffectiveMin ),
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Repeat ),
        TKGet( TK_Spin ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( fEffectiveMax ),
        Any( fEffectiveMin ),
        Any( sal_True ),
        Any( (sal_Int32)12 ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Bool)sal_True ),
        Any( (sal_Bool)sal_True ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XTextComponent > xTextComponent( rOptimizerDialog.insertFormattedField( rControlName, aNames, aValues ), UNO_QUERY_THROW );
    if ( xTextListener.is() )
        xTextComponent->addTextListener( xTextListener );
    if ( xSpinListener.is() )
    {
        Reference< XSpinField > xSpinField( xTextComponent, UNO_QUERY_THROW );
        xSpinField->addSpinListener( xSpinListener );
    }
    return rControlName;
}

// -----------------------------------------------------------------------------

OUString InsertComboBox( OptimizerDialog& rOptimizerDialog, const OUString& rControlName,
    const Reference< XTextListener > xTextListener, const sal_Bool bEnabled, const Sequence< OUString >& rItemList,
        sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Dropdown ),
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_LineCount ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_StringItemList ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( bEnabled ),
        Any( nHeight ),
        Any( (sal_Int16)8),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( rItemList ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XTextComponent > xTextComponent( rOptimizerDialog.insertComboBox( rControlName, aNames, aValues ), UNO_QUERY_THROW );
    if ( xTextListener.is() )
        xTextComponent->addTextListener( xTextListener );
    return rControlName;
}

// -----------------------------------------------------------------------------

OUString InsertRadioButton( OptimizerDialog& rOptimizerDialog, const OUString& rControlName, const Reference< XItemListener > xItemListener,
    const OUString& rLabel, sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Bool bMultiLine, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Height ),
        TKGet( TK_Label ),
        TKGet( TK_MultiLine ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( nHeight ),
        Any( rLabel ),
        Any( bMultiLine ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XRadioButton > xRadioButton( rOptimizerDialog.insertRadioButton( rControlName, aNames, aValues ) );
    if ( xItemListener.is() )
        xRadioButton->addItemListener( xItemListener );
    return rControlName;
}

// -----------------------------------------------------------------------------

OUString InsertListBox( OptimizerDialog& rOptimizerDialog, const OUString& rControlName,
    const Reference< XActionListener > xActionListener, const sal_Bool bEnabled, const Sequence< OUString >& rItemList,
        sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Dropdown ),
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_LineCount ),
        TKGet( TK_MultiSelection ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_StringItemList ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( bEnabled ),
        Any( nHeight ),
        Any( (sal_Int16)8),
        Any( sal_False ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( rItemList ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XListBox > xListBox( rOptimizerDialog.insertListBox( rControlName, aNames, aValues ) );
    if ( xListBox.is() )
        xListBox->addActionListener( xActionListener );
    return rControlName;
}

// -----------------------------------------------------------------------------

void OptimizerDialog::InitNavigationBar()
{
    sal_Int32   nCancelPosX = OD_DIALOG_WIDTH - BUTTON_WIDTH - 6;
    sal_Int32   nFinishPosX = nCancelPosX - 6 - BUTTON_WIDTH;
    sal_Int32   nNextPosX = nFinishPosX - 6 - BUTTON_WIDTH;
    sal_Int32   nBackPosX = nNextPosX - 3 - BUTTON_WIDTH;

    InsertSeparator( *this, TKGet( TK_lnNavSep1 ), 0, 0, DIALOG_HEIGHT - 26, OD_DIALOG_WIDTH, 1 );
    InsertSeparator( *this, TKGet( TK_lnNavSep2 ), 1, 85, 0, 1, BUTTON_POS_Y - 6 );

    InsertButton( *this, TKGet( TK_btnNavBack ), mxActionListener, nBackPosX, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_False, STR_BACK, PushButtonType_STANDARD );
    InsertButton( *this, TKGet( TK_btnNavNext ), mxActionListener, nNextPosX, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_True, STR_NEXT, PushButtonType_STANDARD );
    InsertButton( *this, TKGet( TK_btnNavFinish ), mxActionListener, nFinishPosX, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_True, STR_FINISH, PushButtonType_STANDARD );
    InsertButton( *this, TKGet( TK_btnNavCancel ), mxActionListener, nCancelPosX, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_True, STR_CANCEL, PushButtonType_STANDARD );

    setControlProperty( TKGet( TK_btnNavNext ), TKGet( TK_DefaultButton ), Any( sal_True ) );
}

// -----------------------------------------------------------------------------

void OptimizerDialog::UpdateControlStatesPage0()
{
    sal_uInt32 i;
    short nSelectedItem = -1;
    Sequence< OUString > aItemList;
    const std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        aItemList.realloc( rList.size() - 1 );
        for ( i = 1; i < rList.size(); i++ )
        {
            aItemList[ i - 1 ] = rList[ i ].maName;
            if ( nSelectedItem < 0 )
            {
                if ( rList[ i ] == rList[ 0 ] )
                    nSelectedItem = static_cast< short >( i - 1 );
            }
        }
    }
    sal_Bool bRemoveButtonEnabled = sal_False;
    Sequence< short > aSelectedItems;
    if ( nSelectedItem >= 0 )
    {
        aSelectedItems.realloc( 1 );
        aSelectedItems[ 0 ] = nSelectedItem;
        if ( nSelectedItem > 2 )    // only allowing to delete custom themes, the first can|t be deleted
            bRemoveButtonEnabled = sal_True;
    }
    setControlProperty( TKGet( TK_ListBox0Pg0 ), TKGet( TK_StringItemList ), Any( aItemList ) );
    setControlProperty( TKGet( TK_ListBox0Pg0 ), TKGet( TK_SelectedItems ), Any( aSelectedItems ) );
    setControlProperty( TKGet( TK_Button0Pg0 ), TKGet( TK_Enabled ), Any( bRemoveButtonEnabled ) );
}
void OptimizerDialog::InitPage0()
{
    Sequence< OUString > aItemList;
    std::vector< OUString > aControlList;
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText0Pg0 ), getString( STR_INTRODUCTION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText1Pg0 ), getString( STR_INTRODUCTION_T ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 100, sal_True, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertSeparator( *this, TKGet( TK_Separator1Pg0 ), 0, PAGE_POS_X + 6, DIALOG_HEIGHT - 66, PAGE_WIDTH - 12, 1 ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText2Pg0 ), getString( STR_CHOSE_SETTINGS ), PAGE_POS_X + 6, DIALOG_HEIGHT - 60, PAGE_WIDTH - 12, 8, sal_True, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertListBox(  *this, TKGet( TK_ListBox0Pg0 ), mxActionListenerListBox0Pg0, sal_True, aItemList, PAGE_POS_X + 6, DIALOG_HEIGHT - 48, ( OD_DIALOG_WIDTH - 50 ) - ( PAGE_POS_X + 6 ), 12, mnTabIndex++ ) );
    aControlList.push_back( InsertButton( *this, TKGet( TK_Button0Pg0 ), mxActionListener, OD_DIALOG_WIDTH - 46, DIALOG_HEIGHT - 49, 40, 14, mnTabIndex++, sal_True, STR_REMOVE, PushButtonType_STANDARD ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 0 );
    UpdateControlStatesPage0();
}

// -----------------------------------------------------------------------------

void OptimizerDialog::UpdateControlStatesPage1()
{
    sal_Bool bDeleteUnusedMasterPages( GetConfigProperty( TK_DeleteUnusedMasterPages, sal_False ) );
    sal_Bool bDeleteHiddenSlides( GetConfigProperty( TK_DeleteHiddenSlides, sal_False ) );
    sal_Bool bDeleteNotesPages( GetConfigProperty( TK_DeleteNotesPages, sal_False ) );

    setControlProperty( TKGet( TK_CheckBox0Pg3 ), TKGet( TK_State ), Any( (sal_Int16)bDeleteUnusedMasterPages ) );
    setControlProperty( TKGet( TK_CheckBox1Pg3 ), TKGet( TK_State ), Any( (sal_Int16)bDeleteNotesPages ) );
    setControlProperty( TKGet( TK_CheckBox2Pg3 ), TKGet( TK_State ), Any( (sal_Int16)bDeleteHiddenSlides ) );
}
void OptimizerDialog::InitPage1()
{
    Sequence< OUString > aCustomShowList;
    Reference< XModel > xModel( mxController->getModel() );
    if ( xModel.is() )
    {
        Reference< XCustomPresentationSupplier > aXCPSup( xModel, UNO_QUERY_THROW );
        Reference< XNameContainer > aXCont( aXCPSup->getCustomPresentations() );
        if ( aXCont.is() )
            aCustomShowList = aXCont->getElementNames();
    }
    std::vector< OUString > aControlList;
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText0Pg3 ), getString( STR_CHOOSE_SLIDES ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, TKGet( TK_CheckBox0Pg3 ), mxItemListener, getString( STR_DELETE_MASTER_PAGES ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, TKGet( TK_CheckBox2Pg3 ), mxItemListener, getString( STR_DELETE_HIDDEN_SLIDES ), PAGE_POS_X + 6, PAGE_POS_Y + 28, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, TKGet( TK_CheckBox3Pg3 ), mxItemListener, getString( STR_CUSTOM_SHOW ), PAGE_POS_X + 6, PAGE_POS_Y + 42, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( InsertListBox(  *this, TKGet( TK_ListBox0Pg3 ), mxActionListener, sal_True, aCustomShowList, PAGE_POS_X + 14, PAGE_POS_Y + 54, 150, 12, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, TKGet( TK_CheckBox1Pg3 ), mxItemListener, getString( STR_DELETE_NOTES_PAGES ), PAGE_POS_X + 6, PAGE_POS_Y + 70, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 1 );

    setControlProperty( TKGet( TK_CheckBox3Pg3 ), TKGet( TK_State ), Any( sal_False ) );
    setControlProperty( TKGet( TK_CheckBox3Pg3 ), TKGet( TK_Enabled ), Any( aCustomShowList.getLength() != 0 ) );
    setControlProperty( TKGet( TK_ListBox0Pg3 ), TKGet( TK_Enabled ), Any( sal_False ) );

    UpdateControlStatesPage1();
}

// -----------------------------------------------------------------------------

void OptimizerDialog::UpdateControlStatesPage2()
{
    sal_Bool bJPEGCompression( GetConfigProperty( TK_JPEGCompression, sal_False ) );
    sal_Bool bRemoveCropArea( GetConfigProperty( TK_RemoveCropArea, sal_False ) );
    sal_Bool bEmbedLinkedGraphics( GetConfigProperty( TK_EmbedLinkedGraphics, sal_True ) );
    sal_Int32 nJPEGQuality( GetConfigProperty( TK_JPEGQuality, (sal_Int32)90 ) );

    sal_Int32 nImageResolution( GetConfigProperty( TK_ImageResolution, (sal_Int32)0 ) );

    sal_Int32 nI0, nI1, nI2, nI3;
    nI0 = nI1 = nI2 = nI3 = 0;
    OUString aResolutionText;
    Sequence< OUString > aResolutionItemList( 4 );
    aResolutionItemList[ 0 ] = getString( STR_IMAGE_RESOLUTION_0 ).getToken( 1, ';', nI0 );
    aResolutionItemList[ 1 ] = getString( STR_IMAGE_RESOLUTION_1 ).getToken( 1, ';', nI1 );
    aResolutionItemList[ 2 ] = getString( STR_IMAGE_RESOLUTION_2 ).getToken( 1, ';', nI2 );
    aResolutionItemList[ 3 ] = getString( STR_IMAGE_RESOLUTION_3 ).getToken( 1, ';', nI3 );
    nI0 = nI1 = nI2 = nI3 = 0;
    if ( getString( STR_IMAGE_RESOLUTION_0 ).getToken( 0, ';', nI0 ).toInt32() == nImageResolution )
        aResolutionText = aResolutionItemList[ 0 ];
    else if ( getString( STR_IMAGE_RESOLUTION_1 ).getToken( 0, ';', nI1 ).toInt32() == nImageResolution )
        aResolutionText = aResolutionItemList[ 1 ];
    else if ( getString( STR_IMAGE_RESOLUTION_2 ).getToken( 0, ';', nI2 ).toInt32() == nImageResolution )
        aResolutionText = aResolutionItemList[ 2 ];
    else if ( getString( STR_IMAGE_RESOLUTION_3 ).getToken( 0, ';', nI3 ).toInt32() == nImageResolution )
        aResolutionText = aResolutionItemList[ 3 ];
    if ( aResolutionText.isEmpty() )
        aResolutionText = OUString::number( nImageResolution );

    setControlProperty( TKGet( TK_RadioButton0Pg1 ), TKGet( TK_State ), Any( (sal_Int16)( bJPEGCompression != sal_True ) ) );
    setControlProperty( TKGet( TK_RadioButton1Pg1 ), TKGet( TK_State ), Any( (sal_Int16)( bJPEGCompression != sal_False ) ) );
    setControlProperty( TKGet( TK_FixedText1Pg1  ), TKGet( TK_Enabled ), Any( bJPEGCompression ) );
    setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_Enabled ), Any( bJPEGCompression ) );
    setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( (double)nJPEGQuality ) );
    setControlProperty( TKGet( TK_CheckBox1Pg1 ), TKGet( TK_State ), Any( (sal_Int16)bRemoveCropArea ) );
    setControlProperty( TKGet( TK_ComboBox0Pg1 ), TKGet( TK_Text ), Any( aResolutionText ) );
    setControlProperty( TKGet( TK_CheckBox2Pg1 ), TKGet( TK_State ), Any( (sal_Int16)bEmbedLinkedGraphics ) );
}
void OptimizerDialog::InitPage2()
{
    sal_Int32 nI0, nI1, nI2, nI3;
    nI0 = nI1 = nI2 = nI3 = 0;
    Sequence< OUString > aResolutionItemList( 4 );
    aResolutionItemList[ 0 ] = getString( STR_IMAGE_RESOLUTION_0 ).getToken( 1, ';', nI0 );
    aResolutionItemList[ 1 ] = getString( STR_IMAGE_RESOLUTION_1 ).getToken( 1, ';', nI1 );
    aResolutionItemList[ 2 ] = getString( STR_IMAGE_RESOLUTION_2 ).getToken( 1, ';', nI2 );
    aResolutionItemList[ 3 ] = getString( STR_IMAGE_RESOLUTION_3 ).getToken( 1, ';', nI3 );

    std::vector< OUString > aControlList;
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText0Pg1 ), getString( STR_GRAPHIC_OPTIMIZATION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, TKGet( TK_RadioButton0Pg1 ), mxItemListener, getString( STR_LOSSLESS_COMPRESSION ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, TKGet( TK_RadioButton1Pg1 ), mxItemListener, getString( STR_JPEG_COMPRESSION ), PAGE_POS_X + 6, PAGE_POS_Y + 28, PAGE_WIDTH - 12, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText1Pg1 ), getString( STR_QUALITY ), PAGE_POS_X + 20, PAGE_POS_Y + 40, 72, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertFormattedField( *this, TKGet( TK_FormattedField0Pg1 ), mxTextListenerFormattedField0Pg1, mxSpinListenerFormattedField0Pg1, PAGE_POS_X + 106, PAGE_POS_Y + 38, 50, 0, 100, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText2Pg1 ), getString( STR_IMAGE_RESOLUTION ), PAGE_POS_X + 6, PAGE_POS_Y + 54, 94, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertComboBox(  *this, TKGet( TK_ComboBox0Pg1 ), mxTextListenerComboBox0Pg1, sal_True, aResolutionItemList, PAGE_POS_X + 106, PAGE_POS_Y + 52, 100, 12, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, TKGet( TK_CheckBox1Pg1 ), mxItemListener, getString( STR_REMOVE_CROP_AREA ), PAGE_POS_X + 6, PAGE_POS_Y + 68, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, TKGet( TK_CheckBox2Pg1 ), mxItemListener, getString( STR_EMBED_LINKED_GRAPHICS ), PAGE_POS_X + 6, PAGE_POS_Y + 82, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 2 );
    UpdateControlStatesPage2();
}

// -----------------------------------------------------------------------------

void OptimizerDialog::UpdateControlStatesPage3()
{
    sal_Bool bConvertOLEObjects( GetConfigProperty( TK_OLEOptimization, sal_False ) );
    sal_Int16 nOLEOptimizationType( GetConfigProperty( TK_OLEOptimizationType, (sal_Int16)0 ) );

    setControlProperty( TKGet( TK_CheckBox0Pg2 ), TKGet( TK_State ), Any( (sal_Int16)bConvertOLEObjects ) );
    setControlProperty( TKGet( TK_RadioButton0Pg2 ), TKGet( TK_Enabled ), Any( bConvertOLEObjects ) );
    setControlProperty( TKGet( TK_RadioButton0Pg2 ), TKGet( TK_State ), Any( (sal_Int16)( nOLEOptimizationType == 0 ) ) );
    setControlProperty( TKGet( TK_RadioButton1Pg2 ), TKGet( TK_Enabled ), Any( bConvertOLEObjects ) );
    setControlProperty( TKGet( TK_RadioButton1Pg2 ), TKGet( TK_State ), Any( (sal_Int16)( nOLEOptimizationType == 1 ) ) );
}
void OptimizerDialog::InitPage3()
{
    int nOLECount = 0;
    Reference< XModel > xModel( mxController->getModel() );
    Reference< XDrawPagesSupplier > xDrawPagesSupplier( xModel, UNO_QUERY_THROW );
    Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
    for ( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
    {
        Reference< XShapes > xShapes( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
        for ( sal_Int32 j = 0; j < xShapes->getCount(); j++ )
        {
            const OUString sOLE2Shape( "com.sun.star.drawing.OLE2Shape"  );
            Reference< XShape > xShape( xShapes->getByIndex( j ), UNO_QUERY_THROW );
            if ( xShape->getShapeType() == sOLE2Shape )
                nOLECount++;
        }
    }

    std::vector< OUString > aControlList;
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText0Pg2 ), getString( STR_OLE_OPTIMIZATION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, TKGet( TK_CheckBox0Pg2 ), mxItemListener, getString( STR_OLE_REPLACE ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, TKGet( TK_RadioButton0Pg2 ), mxItemListener, getString( STR_ALL_OLE_OBJECTS ), PAGE_POS_X + 14, PAGE_POS_Y + 28, PAGE_WIDTH - 22, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, TKGet( TK_RadioButton1Pg2 ), mxItemListener, getString( STR_ALIEN_OLE_OBJECTS_ONLY ), PAGE_POS_X + 14, PAGE_POS_Y + 40, PAGE_WIDTH - 22, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText1Pg2 ), nOLECount ? getString( STR_OLE_OBJECTS_DESC ) : getString( STR_NO_OLE_OBJECTS_DESC ), PAGE_POS_X + 6, PAGE_POS_Y + 64, PAGE_WIDTH - 22, 50, sal_True, sal_False, mnTabIndex++ ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 3 );
    UpdateControlStatesPage3();
}

// -----------------------------------------------------------------------------

static OUString ImpValueOfInMB( const sal_Int64& rVal, sal_Unicode nSeparator = '.' )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 20 );
    fVal += 0.05;
    OUStringBuffer aVal( OUString::number( fVal ) );
    sal_Int32 nX( OUString( aVal.getStr() ).indexOf( '.', 0 ) );
    if ( nX >= 0 )
    {
        aVal.setLength( nX + 2 );
        aVal[nX] = nSeparator;
    }
    aVal.append( OUString(" MB") );
    return aVal.makeStringAndClear();
}

void OptimizerDialog::UpdateControlStatesPage4()
{
    sal_Bool bSaveAs( GetConfigProperty( TK_SaveAs, sal_True ) );
    if ( mbIsReadonly )
    {
        setControlProperty( TKGet( TK_RadioButton0Pg4 ), TKGet( TK_State ), Any( (sal_Int16)( sal_False ) ) );
        setControlProperty( TKGet( TK_RadioButton1Pg4 ), TKGet( TK_State ), Any( (sal_Int16)( sal_True ) ) );
    }
    else
    {
        setControlProperty( TKGet( TK_RadioButton0Pg4 ), TKGet( TK_State ), Any( (sal_Int16)( bSaveAs == sal_False ) ) );
        setControlProperty( TKGet( TK_RadioButton1Pg4 ), TKGet( TK_State ), Any( (sal_Int16)( bSaveAs == sal_True ) ) );
    }
    setControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_Enabled ), Any( sal_False ) );

    sal_uInt32 w;
    Sequence< OUString > aItemList;
    const std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        aItemList.realloc( rList.size() - 1 );
        for ( w = 1; w < rList.size(); w++ )
            aItemList[ w - 1 ] = rList[ w ].maName;
    }
    setControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_StringItemList ), Any( aItemList ) );

    // now check if it is sensible to enable the combo box
    sal_Bool bSaveSettingsEnabled = sal_True;
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        for ( w = 1; w < rList.size(); w++ )
        {
            if ( rList[ w ] == rList[ 0 ] )
            {
                bSaveSettingsEnabled = sal_False;
                break;
            }
        }
    }
    sal_Int16 nInt16 = 0;
    getControlProperty( TKGet( TK_CheckBox1Pg4 ), TKGet( TK_State ) ) >>= nInt16;
    setControlProperty( TKGet( TK_CheckBox1Pg4 ), TKGet( TK_Enabled ), Any( bSaveSettingsEnabled ) );
    setControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_Enabled ), Any( bSaveSettingsEnabled && nInt16 ) );

    std::vector< OUString > aSummaryStrings;

    // taking care of deleted slides
    sal_Int32 nDeletedSlides = 0;
    OUString aCustomShowName;
    if ( getControlProperty( TKGet( TK_CheckBox3Pg3 ), TKGet( TK_State ) ) >>= nInt16 )
    {
        if ( nInt16 )
        {
            Sequence< short > aSelectedItems;
            Sequence< OUString > aStringItemList;
            Any aAny = getControlProperty( TKGet( TK_ListBox0Pg3 ), TKGet( TK_SelectedItems ) );
            if ( aAny >>= aSelectedItems )
            {
                if ( aSelectedItems.getLength() )
                {
                    sal_Int16 nSelectedItem = aSelectedItems[ 0 ];
                    aAny = getControlProperty( TKGet( TK_ListBox0Pg3 ), TKGet( TK_StringItemList ) );
                    if ( aAny >>= aStringItemList )
                    {
                        if ( aStringItemList.getLength() > nSelectedItem )
                            SetConfigProperty( TK_CustomShowName, Any( aStringItemList[ nSelectedItem ] ) );
                    }
                }
            }
        }
    }
    if ( !aCustomShowName.isEmpty() )
    {
        std::vector< Reference< XDrawPage > > vNonUsedPageList;
        PageCollector::CollectNonCustomShowPages( mxController->getModel(), aCustomShowName, vNonUsedPageList );
        nDeletedSlides += vNonUsedPageList.size();
    }
    if ( GetConfigProperty( TK_DeleteHiddenSlides, sal_False ) )
    {
        if ( !aCustomShowName.isEmpty() )
        {
            std::vector< Reference< XDrawPage > > vUsedPageList;
            PageCollector::CollectCustomShowPages( mxController->getModel(), aCustomShowName, vUsedPageList );
            std::vector< Reference< XDrawPage > >::iterator aIter( vUsedPageList.begin() );
            while( aIter != vUsedPageList.end() )
            {
                Reference< XPropertySet > xPropSet( *aIter, UNO_QUERY_THROW );
                sal_Bool bVisible = sal_True;
                const OUString sVisible( "Visible"  );
                if ( xPropSet->getPropertyValue( sVisible ) >>= bVisible )
                {
                    if (!bVisible )
                        nDeletedSlides++;
                }
                ++aIter;
            }
        }
        else
        {
            Reference< XDrawPagesSupplier > xDrawPagesSupplier( mxController->getModel(), UNO_QUERY_THROW );
            Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
            for( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
            {
                Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
                Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY_THROW );

                sal_Bool bVisible = sal_True;
                const OUString sVisible( "Visible"  );
                if ( xPropSet->getPropertyValue( sVisible ) >>= bVisible )
                {
                    if (!bVisible )
                        nDeletedSlides++;
                }
            }
        }
    }
    if ( GetConfigProperty( TK_DeleteUnusedMasterPages, sal_False ) )
    {
        std::vector< PageCollector::MasterPageEntity > aMasterPageList;
        PageCollector::CollectMasterPages( mxController->getModel(), aMasterPageList );
        Reference< XMasterPagesSupplier > xMasterPagesSupplier( mxController->getModel(), UNO_QUERY_THROW );
        Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
        std::vector< PageCollector::MasterPageEntity >::iterator aIter( aMasterPageList.begin() );
        while( aIter != aMasterPageList.end() )
        {
            if ( !aIter->bUsed )
                nDeletedSlides++;
            ++aIter;
        }
    }
    if ( nDeletedSlides > 1 )
    {
        OUString aStr( getString( STR_DELETE_SLIDES ) );
        OUString aPlaceholder( "%SLIDES"  );
        sal_Int32 i = aStr.indexOf( aPlaceholder, 0 );
        if ( i >= 0 )
            aStr = aStr.replaceAt( i, aPlaceholder.getLength(), OUString::number( nDeletedSlides ) );
        aSummaryStrings.push_back( aStr );
    }

// generating graphic compression info
    sal_Int32 nGraphics = 0;
    sal_Bool bJPEGCompression( GetConfigProperty( TK_JPEGCompression, sal_False ) );
    sal_Int32 nJPEGQuality( GetConfigProperty( TK_JPEGQuality, (sal_Int32)90 ) );
    sal_Int32 nImageResolution( GetConfigProperty( TK_ImageResolution, (sal_Int32)0 ) );
    GraphicSettings aGraphicSettings( bJPEGCompression, nJPEGQuality, GetConfigProperty( TK_RemoveCropArea, sal_False ),
                                        nImageResolution, GetConfigProperty( TK_EmbedLinkedGraphics, sal_True ) );
    GraphicCollector::CountGraphics( mxContext, mxController->getModel(), aGraphicSettings, nGraphics );
    if ( nGraphics > 1 )
    {
        OUString aStr( getString( STR_OPTIMIZE_IMAGES ) );
        OUString aImagePlaceholder( "%IMAGES"  );
        OUString aQualityPlaceholder( "%QUALITY"  );
        OUString aResolutionPlaceholder( "%RESOLUTION"  );
        sal_Int32 i = aStr.indexOf( aImagePlaceholder, 0 );
        if ( i >= 0 )
            aStr = aStr.replaceAt( i, aImagePlaceholder.getLength(), OUString::number( nGraphics ) );

        sal_Int32 j = aStr.indexOf( aQualityPlaceholder, 0 );
        if ( j >= 0 )
            aStr = aStr.replaceAt( j, aQualityPlaceholder.getLength(), OUString::number( nJPEGQuality ) );

        sal_Int32 k = aStr.indexOf( aResolutionPlaceholder, 0 );
        if ( k >= 0 )
            aStr = aStr.replaceAt( k, aResolutionPlaceholder.getLength(), OUString::number( nImageResolution ) );

        aSummaryStrings.push_back( aStr );
    }

    if ( GetConfigProperty( TK_OLEOptimization, sal_False ) )
    {
        sal_Int32 nOLEReplacements = 0;
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( mxController->getModel(), UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        for ( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
        {
            Reference< XShapes > xShapes( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            for ( sal_Int32 j = 0; j < xShapes->getCount(); j++ )
            {
                const OUString sOLE2Shape( "com.sun.star.drawing.OLE2Shape"  );
                Reference< XShape > xShape( xShapes->getByIndex( j ), UNO_QUERY_THROW );
                if ( xShape->getShapeType() == sOLE2Shape )
                    nOLEReplacements++;
            }
        }
        if ( nOLEReplacements > 1 )
        {
            OUString aStr( getString( STR_CREATE_REPLACEMENT ) );
            OUString aPlaceholder( "%OLE"  );
            sal_Int32 i = aStr.indexOf( aPlaceholder, 0 );
            if ( i >= 0 )
                aStr = aStr.replaceAt( i, aPlaceholder.getLength(), OUString::number( nOLEReplacements ) );
            aSummaryStrings.push_back( aStr );
        }
    }
    while( aSummaryStrings.size() < 3 )
        aSummaryStrings.push_back( OUString() );
    setControlProperty( TKGet( TK_FixedText4Pg4 ), TKGet( TK_Label ), Any( aSummaryStrings[ 0 ] ) );
    setControlProperty( TKGet( TK_FixedText5Pg4 ), TKGet( TK_Label ), Any( aSummaryStrings[ 1 ] ) );
    setControlProperty( TKGet( TK_FixedText6Pg4 ), TKGet( TK_Label ), Any( aSummaryStrings[ 2 ] ) );

    sal_Int64 nCurrentFileSize = 0;
    sal_Int64 nEstimatedFileSize = 0;
    Reference< XStorable > xStorable( mxController->getModel(), UNO_QUERY );
    if ( xStorable.is() && xStorable->hasLocation() )
        nCurrentFileSize = PPPOptimizer::GetFileSize( xStorable->getLocation() );

    if ( nCurrentFileSize )
    {
        double fE = static_cast< double >( nCurrentFileSize );
        if ( nImageResolution )
        {
            double v = ( static_cast< double >( nImageResolution ) + 75.0 ) / 300.0;
            if ( v < 1.0 )
                fE *= v;
        }
        if ( bJPEGCompression )
        {
            double v = 0.75 - ( ( 100.0 - static_cast< double >( nJPEGQuality ) ) / 400.0 ) ;
            fE *= v;
        }
        nEstimatedFileSize = static_cast< sal_Int64 >( fE );
    }
    sal_Unicode nSeparator = '.';
    OUString aStr( getString( STR_FILESIZESEPARATOR ) );
    if ( !aStr.isEmpty() )
        nSeparator = aStr[ 0 ];
    setControlProperty( TKGet( TK_FixedText7Pg4 ), TKGet( TK_Label ), Any( ImpValueOfInMB( nCurrentFileSize, nSeparator ) ) );
    setControlProperty( TKGet( TK_FixedText8Pg4 ), TKGet( TK_Label ), Any( ImpValueOfInMB( nEstimatedFileSize, nSeparator ) ) );
    SetConfigProperty( TK_EstimatedFileSize, Any( nEstimatedFileSize ) );
}

void OptimizerDialog::InitPage4()
{
    {   // creating progress bar:
        OUString pNames[] = {
            TKGet( TK_Height ),
            TKGet( TK_Name ),
            TKGet( TK_PositionX ),
            TKGet( TK_PositionY ),
            TKGet( TK_ProgressValue ),
            TKGet( TK_ProgressValueMax ),
            TKGet( TK_ProgressValueMin ),
            TKGet( TK_Width ) };

        Any pValues[] = {
            Any( (sal_Int32)12 ),
            Any( TKGet( STR_SAVE_AS ) ),
            Any( (sal_Int32)( PAGE_POS_X + 6 ) ),
            Any( (sal_Int32)( DIALOG_HEIGHT - 75 ) ),
            Any( (sal_Int32)( 0 ) ),
            Any( (sal_Int32)( 100 ) ),
            Any( (sal_Int32)( 0 ) ),
            Any( (sal_Int32)( PAGE_WIDTH - 12 ) ) };

        sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

        Sequence< OUString >   aNames( pNames, nCount );
        Sequence< Any >             aValues( pValues, nCount );

        Reference< XMultiPropertySet > xMultiPropertySet( insertControlModel( OUString( "com.sun.star.awt.UnoControlProgressBarModel"  ),
            TKGet( TK_Progress ), aNames, aValues ), UNO_QUERY );
    }
    Reference< XTextListener > xTextListener;
    Sequence< OUString > aItemList;
    std::vector< OUString > aControlList;
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText0Pg4 ), getString( STR_SUMMARY_TITLE ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
//  aControlList.push_back( InsertSeparator( *this, TKGet( TK_Separator0Pg4 ), 0, PAGE_POS_X + 6, PAGE_POS_Y + 90, PAGE_WIDTH - 12, 1 ) );

    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText4Pg4 ), OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText5Pg4 ), OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 22, PAGE_WIDTH - 12, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText6Pg4 ), OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 30, PAGE_WIDTH - 12, 8, sal_False, sal_False, mnTabIndex++ ) );

    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText2Pg4 ), getString( STR_CURRENT_FILESIZE ), PAGE_POS_X + 6, PAGE_POS_Y + 50, 88, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText7Pg4 ), OUString(), PAGE_POS_X + 100, PAGE_POS_Y + 50, 30, 8, sal_False, sal_False, mnTabIndex++ ) );
    setControlProperty( TKGet( TK_FixedText7Pg4 ), TKGet( TK_Align ), Any( static_cast< short >( 2 ) ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText3Pg4 ), getString( STR_ESTIMATED_FILESIZE ), PAGE_POS_X + 6, PAGE_POS_Y + 58, 88, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText8Pg4 ), OUString(), PAGE_POS_X + 100, PAGE_POS_Y + 58, 30, 8, sal_False, sal_False, mnTabIndex++ ) );
    setControlProperty( TKGet( TK_FixedText8Pg4 ), TKGet( TK_Align ), Any( static_cast< short >( 2 ) ) );

    aControlList.push_back( InsertRadioButton( *this, TKGet( TK_RadioButton0Pg4 ), mxItemListener, getString(  STR_APPLY_TO_CURRENT ), PAGE_POS_X + 6, PAGE_POS_Y + 78, PAGE_WIDTH - 12, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, TKGet( TK_RadioButton1Pg4 ), mxItemListener, getString( STR_SAVE_AS ), PAGE_POS_X + 6, PAGE_POS_Y + 90, PAGE_WIDTH - 12, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, TKGet( TK_FixedText1Pg4 ), OUString(), PAGE_POS_X + 6, DIALOG_HEIGHT - 87, PAGE_WIDTH - 12, 8, sal_True, sal_False, mnTabIndex++ ) );
    aControlList.push_back( TKGet( TK_Progress ) );
    aControlList.push_back( InsertSeparator( *this, TKGet( TK_Separator1Pg4 ), 0, PAGE_POS_X + 6, DIALOG_HEIGHT - 58, PAGE_WIDTH - 12, 1 ) );
    aControlList.push_back( InsertCheckBox(  *this, TKGet( TK_CheckBox1Pg4 ), mxItemListener, getString( STR_SAVE_SETTINGS ), PAGE_POS_X + 6, DIALOG_HEIGHT - 47, 100, 8, mnTabIndex++ ) );
    aControlList.push_back( InsertComboBox(  *this, TKGet( TK_ComboBox0Pg4 ), xTextListener, sal_True, aItemList, PAGE_POS_X + 106, DIALOG_HEIGHT - 48, 100, 12, mnTabIndex++ ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 4 );

    // creating a default session name that hasn't been used yet
    OUString aSettingsName;
    OUString aDefault( getString( STR_MY_SETTINGS ) );
    sal_Int32 nSession = 1;
    sal_uInt32 i;
    const std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
    do
    {
        OUString aTemp( aDefault.concat( OUString::number( nSession++ ) ) );
        for ( i = 1; i < rList.size(); i++ )
        {
            if ( rList[ i ].maName == aTemp )
                break;
        }
        if ( i == rList.size() )
            aSettingsName = aTemp;
    }
    while( aSettingsName.isEmpty() );

    setControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_Text ), Any( aSettingsName ) );
    setControlProperty( TKGet( TK_RadioButton0Pg4 ), TKGet( TK_Enabled ), Any( !mbIsReadonly ) );
    setControlProperty( TKGet( TK_RadioButton1Pg4 ), TKGet( TK_Enabled ), Any( !mbIsReadonly ) );

    UpdateControlStatesPage4();
}

// -----------------------------------------------------------------------------
void OptimizerDialog::EnablePage( sal_Int16 nStep )
{
    std::vector< OUString >::iterator aBeg( maControlPages[ nStep ].begin() );
    std::vector< OUString >::iterator aEnd( maControlPages[ nStep ].end() );
    while( aBeg != aEnd )
        setControlProperty( *aBeg++, TKGet( TK_Enabled ), Any( sal_True ) );
}
void OptimizerDialog::DisablePage( sal_Int16 nStep )
{
    std::vector< OUString >::iterator aBeg( maControlPages[ nStep ].begin() );
    std::vector< OUString >::iterator aEnd( maControlPages[ nStep ].end() );
    while( aBeg != aEnd )
        setControlProperty( *aBeg++, TKGet( TK_Enabled ), Any( sal_False ) );
}
void OptimizerDialog::ActivatePage( sal_Int16 nStep )
{
    std::vector< OUString >::iterator aBeg( maControlPages[ nStep ].begin() );
    std::vector< OUString >::iterator aEnd( maControlPages[ nStep ].end() );
    while( aBeg != aEnd )
        setVisible( *aBeg++, sal_True );
}
void OptimizerDialog::DeactivatePage( sal_Int16 nStep )
{
    std::vector< OUString >::iterator aBeg( maControlPages[ nStep ].begin() );
    std::vector< OUString >::iterator aEnd( maControlPages[ nStep ].end() );
    while( aBeg != aEnd )
        setVisible( *aBeg++, sal_False );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
