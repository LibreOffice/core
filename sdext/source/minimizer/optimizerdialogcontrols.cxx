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


#include "pppoptimizer.hxx"
#include "graphiccollector.hxx"
#include "pagecollector.hxx"
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;


static void SetBold( OptimizerDialog& rOptimizerDialog, const OUString& rControl )
{
    FontDescriptor aFontDescriptor;
    if ( rOptimizerDialog.getControlProperty( rControl, "FontDescriptor" ) >>= aFontDescriptor )
    {
        aFontDescriptor.Weight = FontWeight::BOLD;
        rOptimizerDialog.setControlProperty( rControl, "FontDescriptor", Any( aFontDescriptor ) );
    }
}


static OUString InsertSeparator( OptimizerDialog& rOptimizerDialog, const OUString& rControlName, sal_Int32 nOrientation,
                        sal_Int32 nPosX, sal_Int32 nPosY, sal_Int32 nWidth, sal_Int32 nHeight )
{
    OUString pNames[] = {
        OUString("Height"),
        OUString("Orientation"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Step"),
        OUString("Width") };

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

    rOptimizerDialog.insertControlModel( "com.sun.star.awt.UnoControlFixedLineModel",
        rControlName, aNames, aValues );
    return rControlName;
}


static OUString InsertButton( OptimizerDialog& rOptimizerDialog, const OUString& rControlName, Reference< XActionListener > const & xActionListener,
    sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int16 nTabIndex, bool bEnabled, PPPOptimizerTokenEnum nResID, css::awt::PushButtonType nPushButtonType )
{
    sal_Int32 nHeight = BUTTON_HEIGHT;
    OUString pNames[] = {
        OUString("Enabled"),
        OUString("Height"),
        OUString("Label"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("PushButtonType"),
        OUString("Step"),
        OUString("TabIndex"),
        OUString("Width") };

    Any pValues[] = {
        Any( bEnabled  ),
        Any( nHeight ),
        Any( rOptimizerDialog.getString( nResID ) ),
        Any( nXPos ),
        Any( nYPos ),
        Any( static_cast< sal_Int16 >(nPushButtonType) ),
        Any( sal_Int16(0) ),
        Any( nTabIndex ),
        Any( nWidth ) };


    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rOptimizerDialog.insertButton( rControlName, xActionListener, aNames, aValues );
    return rControlName;
}


static OUString InsertFixedText( OptimizerDialog& rOptimizerDialog, const OUString& rControlName, const OUString& rLabel,
                                sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, bool bMultiLine, bool bBold, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        OUString("Height"),
        OUString("Label"),
        OUString("MultiLine"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Step"),
        OUString("TabIndex"),
        OUString("Width") };

    Any pValues[] = {
        Any( nHeight ),
        Any( rLabel ),
        Any( bMultiLine ),
        Any( nXPos ),
        Any( nYPos ),
        Any( sal_Int16(0) ),
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


static OUString InsertCheckBox( OptimizerDialog& rOptimizerDialog, const OUString& rControlName,
    const Reference< XItemListener >& xItemListener, const OUString& rLabel,
        sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int16 nTabIndex )
{
    sal_Int32 nHeight = 8;
    OUString pNames[] = {
        OUString("Enabled"),
        OUString("Height"),
        OUString("Label"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Step"),
        OUString("TabIndex"),
        OUString("Width") };

    Any pValues[] = {
        Any( true ),
        Any( nHeight ),
        Any( rLabel ),
        Any( nXPos ),
        Any( nYPos ),
        Any( sal_Int16(0) ),
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


static OUString InsertFormattedField( OptimizerDialog& rOptimizerDialog, const OUString& rControlName,
        const Reference< XTextListener >& xTextListener, const Reference< XSpinListener >& xSpinListener, sal_Int32 nXPos, sal_Int32 nYPos,
        double fEffectiveMin, double fEffectiveMax, sal_Int16 nTabIndex )
{
    sal_Int32 nWidth = 50;
    OUString pNames[] = {
        OUString("EffectiveMax"),
        OUString("EffectiveMin"),
        OUString("Enabled"),
        OUString("Height"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Repeat"),
        OUString("Spin"),
        OUString("Step"),
        OUString("TabIndex"),
        OUString("Width") };

    Any pValues[] = {
        Any( fEffectiveMax ),
        Any( fEffectiveMin ),
        Any( true ),
        Any( sal_Int32(12) ),
        Any( nXPos ),
        Any( nYPos ),
        Any( true ),
        Any( true ),
        Any( sal_Int16(0) ),
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


static OUString InsertComboBox( OptimizerDialog& rOptimizerDialog, const OUString& rControlName,
    const Reference< XTextListener >& rTextListener, const bool bEnabled, const Sequence< OUString >& rItemList,
        sal_Int32 nXPos, sal_Int32 nYPos, sal_Int16 nTabIndex )
{
    sal_Int32 nHeight = 12;
    sal_Int32 nWidth = 100;
    OUString pNames[] = {
        OUString("Dropdown"),
        OUString("Enabled"),
        OUString("Height"),
        OUString("LineCount"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Step"),
        OUString("StringItemList"),
        OUString("TabIndex"),
        OUString("Width") };

    Any pValues[] = {
        Any( true ),
        Any( bEnabled ),
        Any( nHeight ),
        Any( sal_Int16(8)),
        Any( nXPos ),
        Any( nYPos ),
        Any( sal_Int16(0) ),
        Any( rItemList ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XTextComponent > xTextComponent( rOptimizerDialog.insertComboBox( rControlName, aNames, aValues ), UNO_QUERY_THROW );
    if ( rTextListener.is() )
        xTextComponent->addTextListener( rTextListener );
    return rControlName;
}


static OUString InsertRadioButton( OptimizerDialog& rOptimizerDialog, const OUString& rControlName, const Reference< XItemListener >& rItemListener,
    const OUString& rLabel, sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int16 nTabIndex )
{
    sal_Int32 nHeight = 8;
    OUString pNames[] = {
        OUString("Height"),
        OUString("Label"),
        OUString("MultiLine"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Step"),
        OUString("TabIndex"),
        OUString("Width") };

    Any pValues[] = {
        Any( nHeight ),
        Any( rLabel ),
        Any( false ), // bMultiLine
        Any( nXPos ),
        Any( nYPos ),
        Any( sal_Int16(0) ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XRadioButton > xRadioButton( rOptimizerDialog.insertRadioButton( rControlName, aNames, aValues ) );
    if ( rItemListener.is() )
        xRadioButton->addItemListener( rItemListener );
    return rControlName;
}


static OUString InsertListBox( OptimizerDialog& rOptimizerDialog, const OUString& rControlName,
    const Reference< XActionListener >& rActionListener, const bool bEnabled, const Sequence< OUString >& rItemList,
        sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int16 nTabIndex )
{
    sal_Int32 nHeight = 12;
    OUString pNames[] = {
        OUString("Dropdown"),
        OUString("Enabled"),
        OUString("Height"),
        OUString("LineCount"),
        OUString("MultiSelection"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Step"),
        OUString("StringItemList"),
        OUString("TabIndex"),
        OUString("Width") };

    Any pValues[] = {
        Any( true ),
        Any( bEnabled ),
        Any( nHeight ),
        Any( sal_Int16(8)),
        Any( false ),
        Any( nXPos ),
        Any( nYPos ),
        Any( sal_Int16(0) ),
        Any( rItemList ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XListBox > xListBox( rOptimizerDialog.insertListBox( rControlName, aNames, aValues ) );
    if ( xListBox.is() )
        xListBox->addActionListener( rActionListener );
    return rControlName;
}


void OptimizerDialog::InitNavigationBar()
{
    sal_Int32   nCancelPosX = OD_DIALOG_WIDTH - BUTTON_WIDTH - 6;
    sal_Int32   nFinishPosX = nCancelPosX - 6 - BUTTON_WIDTH;
    sal_Int32   nNextPosX = nFinishPosX - 6 - BUTTON_WIDTH;
    sal_Int32   nBackPosX = nNextPosX - 3 - BUTTON_WIDTH;

    InsertSeparator( *this, "lnNavSep1", 0, 0, DIALOG_HEIGHT - 26, OD_DIALOG_WIDTH, 1 );
    InsertSeparator( *this, "lnNavSep2", 1, 85, 0, 1, BUTTON_POS_Y - 6 );

    InsertButton( *this, "btnNavBack", mxActionListener, nBackPosX, BUTTON_POS_Y, BUTTON_WIDTH, mnTabIndex++, false, STR_BACK, PushButtonType_STANDARD );
    InsertButton( *this, "btnNavNext", mxActionListener, nNextPosX, BUTTON_POS_Y, BUTTON_WIDTH, mnTabIndex++, true, STR_NEXT, PushButtonType_STANDARD );
    InsertButton( *this, "btnNavFinish", mxActionListener, nFinishPosX, BUTTON_POS_Y, BUTTON_WIDTH, mnTabIndex++, true, STR_FINISH, PushButtonType_STANDARD );
    InsertButton( *this, "btnNavCancel", mxActionListener, nCancelPosX, BUTTON_POS_Y, BUTTON_WIDTH, mnTabIndex++, true, STR_CANCEL, PushButtonType_STANDARD );

    setControlProperty( "btnNavNext", "DefaultButton", Any( true ) );
}


void OptimizerDialog::UpdateControlStatesPage0()
{
    short nSelectedItem = -1;
    Sequence< OUString > aItemList;
    const std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        aItemList.realloc( rList.size() - 1 );
        for ( std::vector<OptimizerSettings>::size_type i = 1; i < rList.size(); i++ )
        {
            aItemList[ i - 1 ] = rList[ i ].maName;
            if ( nSelectedItem < 0 )
            {
                if ( rList[ i ] == rList[ 0 ] )
                    nSelectedItem = static_cast< short >( i - 1 );
            }
        }
    }
    bool bRemoveButtonEnabled = false;
    Sequence< short > aSelectedItems;
    if ( nSelectedItem >= 0 )
    {
        aSelectedItems.realloc( 1 );
        aSelectedItems[ 0 ] = nSelectedItem;
        if ( nSelectedItem > 2 )    // only allowing to delete custom themes, the first can|t be deleted
            bRemoveButtonEnabled = true;
    }
    setControlProperty( "ListBox0Pg0", "StringItemList", Any( aItemList ) );
    setControlProperty( "ListBox0Pg0", "SelectedItems", Any( aSelectedItems ) );
    setControlProperty( "Button0Pg0", "Enabled", Any( bRemoveButtonEnabled ) );
}
void OptimizerDialog::InitPage0()
{
    Sequence< OUString > aItemList;
    std::vector< OUString > aControlList;
    aControlList.push_back( InsertFixedText( *this, "FixedText0Pg0", getString( STR_INTRODUCTION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, false, true, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText1Pg0", getString( STR_INTRODUCTION_T ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 100, true, false, mnTabIndex++ ) );
    aControlList.push_back( InsertSeparator( *this, "Separator1Pg0", 0, PAGE_POS_X + 6, DIALOG_HEIGHT - 66, PAGE_WIDTH - 12, 1 ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText2Pg0", getString( STR_CHOSE_SETTINGS ), PAGE_POS_X + 6, DIALOG_HEIGHT - 60, PAGE_WIDTH - 12, 8, true, false, mnTabIndex++ ) );
    aControlList.push_back( InsertListBox(  *this, "ListBox0Pg0", mxActionListenerListBox0Pg0, true, aItemList, PAGE_POS_X + 6, DIALOG_HEIGHT - 48, ( OD_DIALOG_WIDTH - 50 ) - ( PAGE_POS_X + 6 ), mnTabIndex++ ) );
    aControlList.push_back( InsertButton( *this, "Button0Pg0", mxActionListener, OD_DIALOG_WIDTH - 46, DIALOG_HEIGHT - 49, 40, mnTabIndex++, true, STR_REMOVE, PushButtonType_STANDARD ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 0 );
    UpdateControlStatesPage0();
}


void OptimizerDialog::UpdateControlStatesPage1()
{
    bool bDeleteUnusedMasterPages( GetConfigProperty( TK_DeleteUnusedMasterPages, false ) );
    bool bDeleteHiddenSlides( GetConfigProperty( TK_DeleteHiddenSlides, false ) );
    bool bDeleteNotesPages( GetConfigProperty( TK_DeleteNotesPages, false ) );

    setControlProperty( "CheckBox0Pg3", "State", Any( static_cast<sal_Int16>(bDeleteUnusedMasterPages) ) );
    setControlProperty( "CheckBox1Pg3", "State", Any( static_cast<sal_Int16>(bDeleteNotesPages) ) );
    setControlProperty( "CheckBox2Pg3", "State", Any( static_cast<sal_Int16>(bDeleteHiddenSlides) ) );
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
    aControlList.push_back( InsertFixedText( *this, "FixedText0Pg3", getString( STR_CHOOSE_SLIDES ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, false, true, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, "CheckBox0Pg3", mxItemListener, getString( STR_DELETE_MASTER_PAGES ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, "CheckBox2Pg3", mxItemListener, getString( STR_DELETE_HIDDEN_SLIDES ), PAGE_POS_X + 6, PAGE_POS_Y + 28, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, "CheckBox3Pg3", mxItemListener, getString( STR_CUSTOM_SHOW ), PAGE_POS_X + 6, PAGE_POS_Y + 42, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertListBox(  *this, "ListBox0Pg3", mxActionListener, true, aCustomShowList, PAGE_POS_X + 14, PAGE_POS_Y + 54, 150, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, "CheckBox1Pg3", mxItemListener, getString( STR_DELETE_NOTES_PAGES ), PAGE_POS_X + 6, PAGE_POS_Y + 70, PAGE_WIDTH - 12, mnTabIndex++ ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 1 );

    setControlProperty( "CheckBox3Pg3", "State", Any( false ) );
    setControlProperty( "CheckBox3Pg3", "Enabled", Any( aCustomShowList.getLength() != 0 ) );
    setControlProperty( "ListBox0Pg3", "Enabled", Any( false ) );

    UpdateControlStatesPage1();
}

namespace
{

bool lcl_getResolutionText(OUString& rResolutionText, const OUString& rImageResolution, sal_Int32 nTargetRes)
{
    sal_Int32 nIdx{ 0 };
    if (rImageResolution.getToken(0, ';', nIdx).toInt32()!=nTargetRes)
        return false;
    rResolutionText = rImageResolution.getToken(0, ';', nIdx);
    return true;
}

}

void OptimizerDialog::UpdateControlStatesPage2()
{
    bool bJPEGCompression( GetConfigProperty( TK_JPEGCompression, false ) );
    bool bRemoveCropArea( GetConfigProperty( TK_RemoveCropArea, false ) );
    bool bEmbedLinkedGraphics( GetConfigProperty( TK_EmbedLinkedGraphics, true ) );
    sal_Int32 nJPEGQuality( GetConfigProperty( TK_JPEGQuality, sal_Int32(90) ) );

    sal_Int32 nImageResolution( GetConfigProperty( TK_ImageResolution, sal_Int32(0) ) );

    OUString aResolutionText;
    for (int nIR{ STR_IMAGE_RESOLUTION_0 }; nIR<=STR_IMAGE_RESOLUTION_3; ++nIR)
    {
        if (lcl_getResolutionText(aResolutionText, getString(static_cast<PPPOptimizerTokenEnum>(nIR)), nImageResolution))
            break;
    }
    if ( aResolutionText.isEmpty() )
        aResolutionText = OUString::number( nImageResolution );

    setControlProperty( "RadioButton0Pg1", "State", Any( static_cast<sal_Int16>( !bJPEGCompression ) ) );
    setControlProperty( "RadioButton1Pg1", "State", Any( static_cast<sal_Int16>(bJPEGCompression) ) );
    setControlProperty( "FixedText1Pg1", "Enabled", Any( bJPEGCompression ) );
    setControlProperty( "FormattedField0Pg1", "Enabled", Any( bJPEGCompression ) );
    setControlProperty( "FormattedField0Pg1", "EffectiveValue", Any( static_cast<double>(nJPEGQuality) ) );
    setControlProperty( "CheckBox1Pg1", "State", Any( static_cast<sal_Int16>(bRemoveCropArea) ) );
    setControlProperty( "ComboBox0Pg1", "Text", Any( aResolutionText ) );
    setControlProperty( "CheckBox2Pg1", "State", Any( static_cast<sal_Int16>(bEmbedLinkedGraphics) ) );
}
void OptimizerDialog::InitPage2()
{
    Sequence< OUString > aResolutionItemList( 4 );
    aResolutionItemList[ 0 ] = getString( STR_IMAGE_RESOLUTION_0 ).getToken( 1, ';' );
    aResolutionItemList[ 1 ] = getString( STR_IMAGE_RESOLUTION_1 ).getToken( 1, ';' );
    aResolutionItemList[ 2 ] = getString( STR_IMAGE_RESOLUTION_2 ).getToken( 1, ';' );
    aResolutionItemList[ 3 ] = getString( STR_IMAGE_RESOLUTION_3 ).getToken( 1, ';' );

    std::vector< OUString > aControlList;
    aControlList.push_back( InsertFixedText( *this, "FixedText0Pg1", getString( STR_GRAPHIC_OPTIMIZATION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, false, true, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, "RadioButton0Pg1", mxItemListener, getString( STR_LOSSLESS_COMPRESSION ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, "RadioButton1Pg1", mxItemListener, getString( STR_JPEG_COMPRESSION ), PAGE_POS_X + 6, PAGE_POS_Y + 28, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText1Pg1", getString( STR_QUALITY ), PAGE_POS_X + 20, PAGE_POS_Y + 40, 72, 8, false, false, mnTabIndex++ ) );
    aControlList.push_back( InsertFormattedField( *this, "FormattedField0Pg1", mxTextListenerFormattedField0Pg1, mxSpinListenerFormattedField0Pg1, PAGE_POS_X + 106, PAGE_POS_Y + 38, 0, 100, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText2Pg1", getString( STR_IMAGE_RESOLUTION ), PAGE_POS_X + 6, PAGE_POS_Y + 54, 94, 8, false, false, mnTabIndex++ ) );
    aControlList.push_back( InsertComboBox(  *this, "ComboBox0Pg1", mxTextListenerComboBox0Pg1, true, aResolutionItemList, PAGE_POS_X + 106, PAGE_POS_Y + 52, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, "CheckBox1Pg1", mxItemListener, getString( STR_REMOVE_CROP_AREA ), PAGE_POS_X + 6, PAGE_POS_Y + 68, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, "CheckBox2Pg1", mxItemListener, getString( STR_EMBED_LINKED_GRAPHICS ), PAGE_POS_X + 6, PAGE_POS_Y + 82, PAGE_WIDTH - 12, mnTabIndex++ ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 2 );
    UpdateControlStatesPage2();
}


void OptimizerDialog::UpdateControlStatesPage3()
{
    bool bConvertOLEObjects( GetConfigProperty( TK_OLEOptimization, false ) );
    sal_Int16 nOLEOptimizationType( GetConfigProperty( TK_OLEOptimizationType, sal_Int16(0) ) );

    setControlProperty( "CheckBox0Pg2", "State", Any( static_cast<sal_Int16>(bConvertOLEObjects) ) );
    setControlProperty( "RadioButton0Pg2", "Enabled", Any( bConvertOLEObjects ) );
    setControlProperty( "RadioButton0Pg2", "State", Any( static_cast<sal_Int16>( nOLEOptimizationType == 0 ) ) );
    setControlProperty( "RadioButton1Pg2", "Enabled", Any( bConvertOLEObjects ) );
    setControlProperty( "RadioButton1Pg2", "State", Any( static_cast<sal_Int16>( nOLEOptimizationType == 1 ) ) );
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
    aControlList.push_back( InsertFixedText( *this, "FixedText0Pg2", getString( STR_OLE_OPTIMIZATION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, false, true, mnTabIndex++ ) );
    aControlList.push_back( InsertCheckBox(  *this, "CheckBox0Pg2", mxItemListener, getString( STR_OLE_REPLACE ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, "RadioButton0Pg2", mxItemListener, getString( STR_ALL_OLE_OBJECTS ), PAGE_POS_X + 14, PAGE_POS_Y + 28, PAGE_WIDTH - 22, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, "RadioButton1Pg2", mxItemListener, getString( STR_ALIEN_OLE_OBJECTS_ONLY ), PAGE_POS_X + 14, PAGE_POS_Y + 40, PAGE_WIDTH - 22, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText1Pg2", nOLECount ? getString( STR_OLE_OBJECTS_DESC ) : getString( STR_NO_OLE_OBJECTS_DESC ), PAGE_POS_X + 6, PAGE_POS_Y + 64, PAGE_WIDTH - 22, 50, true, false, mnTabIndex++ ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 3 );
    UpdateControlStatesPage3();
}


static OUString ImpValueOfInMB( sal_Int64 rVal, sal_Unicode nSeparator )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 20 );
    fVal += 0.05;
    OUStringBuffer aVal( OUString::number( fVal ) );
    sal_Int32 nX( OUString( aVal.getStr() ).indexOf( '.' ) );
    if ( nX >= 0 )
    {
        aVal.setLength( nX + 2 );
        aVal[nX] = nSeparator;
    }
    aVal.append( " MB" );
    return aVal.makeStringAndClear();
}

void OptimizerDialog::UpdateControlStatesPage4()
{
    bool bSaveAs( GetConfigProperty( TK_SaveAs, true ) );
    if ( mbIsReadonly )
    {
        setControlProperty( "RadioButton0Pg4", "State", Any( sal_Int16(false) ) );
        setControlProperty( "RadioButton1Pg4", "State", Any( sal_Int16(true) ) );
    }
    else
    {
        setControlProperty( "RadioButton0Pg4", "State", Any( static_cast<sal_Int16>( !bSaveAs ) ) );
        setControlProperty( "RadioButton1Pg4", "State", Any( static_cast<sal_Int16>(bSaveAs) ) );
    }
    setControlProperty( "ComboBox0Pg4", "Enabled", Any( false ) );

    Sequence< OUString > aItemList;
    const std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        aItemList.realloc( rList.size() - 1 );
        for ( std::vector<OptimizerSettings>::size_type w = 1; w < rList.size(); w++ )
            aItemList[ w - 1 ] = rList[ w ].maName;
    }
    setControlProperty( "ComboBox0Pg4", "StringItemList", Any( aItemList ) );

    // now check if it is sensible to enable the combo box
    bool bSaveSettingsEnabled = true;
    if ( rList.size() > 1 ) // the first session in the list is the actual one -> skipping first one
    {
        for ( std::vector<OptimizerSettings>::size_type w = 1; w < rList.size(); w++ )
        {
            if ( rList[ w ] == rList[ 0 ] )
            {
                bSaveSettingsEnabled = false;
                break;
            }
        }
    }
    sal_Int16 nInt16 = 0;
    getControlProperty( "CheckBox1Pg4", "State" ) >>= nInt16;
    setControlProperty( "CheckBox1Pg4", "Enabled", Any( bSaveSettingsEnabled ) );
    setControlProperty( "ComboBox0Pg4", "Enabled", Any( bSaveSettingsEnabled && nInt16 ) );

    std::vector< OUString > aSummaryStrings;

    // taking care of deleted slides
    sal_Int32 nDeletedSlides = 0;
    OUString aCustomShowName;
    if ( getControlProperty( "CheckBox3Pg3", "State" ) >>= nInt16 )
    {
        if ( nInt16 )
        {
            Sequence< short > aSelectedItems;
            Sequence< OUString > aStringItemList;
            Any aAny = getControlProperty( "ListBox0Pg3", "SelectedItems" );
            if ( aAny >>= aSelectedItems )
            {
                if ( aSelectedItems.getLength() )
                {
                    sal_Int16 nSelectedItem = aSelectedItems[ 0 ];
                    aAny = getControlProperty( "ListBox0Pg3", "StringItemList" );
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
    if ( GetConfigProperty( TK_DeleteHiddenSlides, false ) )
    {
        if ( !aCustomShowName.isEmpty() )
        {
            std::vector< Reference< XDrawPage > > vUsedPageList;
            PageCollector::CollectCustomShowPages( mxController->getModel(), aCustomShowName, vUsedPageList );
            for( const auto& rxPage : vUsedPageList )
            {
                Reference< XPropertySet > xPropSet( rxPage, UNO_QUERY_THROW );
                bool bVisible = true;
                const OUString sVisible( "Visible"  );
                if ( xPropSet->getPropertyValue( sVisible ) >>= bVisible )
                {
                    if (!bVisible )
                        nDeletedSlides++;
                }
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

                bool bVisible = true;
                const OUString sVisible( "Visible"  );
                if ( xPropSet->getPropertyValue( sVisible ) >>= bVisible )
                {
                    if (!bVisible )
                        nDeletedSlides++;
                }
            }
        }
    }
    if ( GetConfigProperty( TK_DeleteUnusedMasterPages, false ) )
    {
        std::vector< PageCollector::MasterPageEntity > aMasterPageList;
        PageCollector::CollectMasterPages( mxController->getModel(), aMasterPageList );
        Reference< XMasterPagesSupplier > xMasterPagesSupplier( mxController->getModel(), UNO_QUERY_THROW );
        Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
        nDeletedSlides += std::count_if(aMasterPageList.begin(), aMasterPageList.end(),
            [](const PageCollector::MasterPageEntity& rEntity) { return !rEntity.bUsed; });
    }
    if ( nDeletedSlides > 1 )
    {
        OUString aStr( getString( STR_DELETE_SLIDES ) );
        OUString aPlaceholder( "%SLIDES"  );
        sal_Int32 i = aStr.indexOf( aPlaceholder );
        if ( i >= 0 )
            aStr = aStr.replaceAt( i, aPlaceholder.getLength(), OUString::number( nDeletedSlides ) );
        aSummaryStrings.push_back( aStr );
    }

// generating graphic compression info
    sal_Int32 nGraphics = 0;
    bool bJPEGCompression( GetConfigProperty( TK_JPEGCompression, false ) );
    sal_Int32 nJPEGQuality( GetConfigProperty( TK_JPEGQuality, sal_Int32(90) ) );
    sal_Int32 nImageResolution( GetConfigProperty( TK_ImageResolution, sal_Int32(0) ) );
    GraphicSettings aGraphicSettings( bJPEGCompression, nJPEGQuality, GetConfigProperty( TK_RemoveCropArea, false ),
                                        nImageResolution, GetConfigProperty( TK_EmbedLinkedGraphics, true ) );
    GraphicCollector::CountGraphics( UnoDialog::mxContext, mxController->getModel(), aGraphicSettings, nGraphics );
    if ( nGraphics > 1 )
    {
        OUString aStr( getString( STR_OPTIMIZE_IMAGES ) );
        OUString aImagePlaceholder( "%IMAGES"  );
        OUString aQualityPlaceholder( "%QUALITY"  );
        OUString aResolutionPlaceholder( "%RESOLUTION"  );
        sal_Int32 i = aStr.indexOf( aImagePlaceholder );
        if ( i >= 0 )
            aStr = aStr.replaceAt( i, aImagePlaceholder.getLength(), OUString::number( nGraphics ) );

        sal_Int32 j = aStr.indexOf( aQualityPlaceholder );
        if ( j >= 0 )
            aStr = aStr.replaceAt( j, aQualityPlaceholder.getLength(), OUString::number( nJPEGQuality ) );

        sal_Int32 k = aStr.indexOf( aResolutionPlaceholder );
        if ( k >= 0 )
            aStr = aStr.replaceAt( k, aResolutionPlaceholder.getLength(), OUString::number( nImageResolution ) );

        aSummaryStrings.push_back( aStr );
    }

    if ( GetConfigProperty( TK_OLEOptimization, false ) )
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
            sal_Int32 i = aStr.indexOf( aPlaceholder );
            if ( i >= 0 )
                aStr = aStr.replaceAt( i, aPlaceholder.getLength(), OUString::number( nOLEReplacements ) );
            aSummaryStrings.push_back( aStr );
        }
    }
    while( aSummaryStrings.size() < 3 )
        aSummaryStrings.emplace_back( );
    setControlProperty( "FixedText4Pg4", "Label", Any( aSummaryStrings[ 0 ] ) );
    setControlProperty( "FixedText5Pg4", "Label", Any( aSummaryStrings[ 1 ] ) );
    setControlProperty( "FixedText6Pg4", "Label", Any( aSummaryStrings[ 2 ] ) );

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
    setControlProperty( "FixedText7Pg4", "Label", Any( ImpValueOfInMB( nCurrentFileSize, nSeparator ) ) );
    setControlProperty( "FixedText8Pg4", "Label", Any( ImpValueOfInMB( nEstimatedFileSize, nSeparator ) ) );
    SetConfigProperty( TK_EstimatedFileSize, Any( nEstimatedFileSize ) );
}

void OptimizerDialog::InitPage4()
{
    {   // creating progress bar:
        OUString pNames[] = {
            OUString("Height"),
            OUString("Name"),
            OUString("PositionX"),
            OUString("PositionY"),
            OUString("ProgressValue"),
            OUString("ProgressValueMax"),
            OUString("ProgressValueMin"),
            OUString("Width") };

        Any pValues[] = {
            Any( sal_Int32(12) ),
            Any( OUString("STR_SAVE_AS") ),
            Any( sal_Int32( PAGE_POS_X + 6 ) ),
            Any( sal_Int32( DIALOG_HEIGHT - 75 ) ),
            Any( sal_Int32(0) ),
            Any( sal_Int32(100) ),
            Any( sal_Int32(0) ),
            Any( sal_Int32( PAGE_WIDTH - 12 ) ) };

        sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

        Sequence< OUString >   aNames( pNames, nCount );
        Sequence< Any >             aValues( pValues, nCount );

        Reference< XMultiPropertySet > xMultiPropertySet( insertControlModel( "com.sun.star.awt.UnoControlProgressBarModel",
            "Progress", aNames, aValues ), UNO_QUERY );
    }
    Reference< XTextListener > xTextListener;
    Sequence< OUString > aItemList;
    std::vector< OUString > aControlList;
    aControlList.push_back( InsertFixedText( *this, "FixedText0Pg4", getString( STR_SUMMARY_TITLE ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, false, true, mnTabIndex++ ) );
//  aControlList.push_back( InsertSeparator( *this, "Separator0Pg4", 0, PAGE_POS_X + 6, PAGE_POS_Y + 90, PAGE_WIDTH - 12, 1 ) );

    aControlList.push_back( InsertFixedText( *this, "FixedText4Pg4", OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, false, false, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText5Pg4", OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 22, PAGE_WIDTH - 12, 8, false, false, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText6Pg4", OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 30, PAGE_WIDTH - 12, 8, false, false, mnTabIndex++ ) );

    aControlList.push_back( InsertFixedText( *this, "FixedText2Pg4", getString( STR_CURRENT_FILESIZE ), PAGE_POS_X + 6, PAGE_POS_Y + 50, 88, 8, false, false, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText7Pg4", OUString(), PAGE_POS_X + 100, PAGE_POS_Y + 50, 30, 8, false, false, mnTabIndex++ ) );
    setControlProperty( "FixedText7Pg4", "Align", Any( static_cast< short >( 2 ) ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText3Pg4", getString( STR_ESTIMATED_FILESIZE ), PAGE_POS_X + 6, PAGE_POS_Y + 58, 88, 8, false, false, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText8Pg4", OUString(), PAGE_POS_X + 100, PAGE_POS_Y + 58, 30, 8, false, false, mnTabIndex++ ) );
    setControlProperty( "FixedText8Pg4", "Align", Any( static_cast< short >( 2 ) ) );

    aControlList.push_back( InsertRadioButton( *this, "RadioButton0Pg4", mxItemListener, getString(  STR_APPLY_TO_CURRENT ), PAGE_POS_X + 6, PAGE_POS_Y + 78, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertRadioButton( *this, "RadioButton1Pg4", mxItemListener, getString( STR_SAVE_AS ), PAGE_POS_X + 6, PAGE_POS_Y + 90, PAGE_WIDTH - 12, mnTabIndex++ ) );
    aControlList.push_back( InsertFixedText( *this, "FixedText1Pg4", OUString(), PAGE_POS_X + 6, DIALOG_HEIGHT - 87, PAGE_WIDTH - 12, 8, true, false, mnTabIndex++ ) );
    aControlList.emplace_back("Progress" );
    aControlList.push_back( InsertSeparator( *this, "Separator1Pg4", 0, PAGE_POS_X + 6, DIALOG_HEIGHT - 58, PAGE_WIDTH - 12, 1 ) );
    aControlList.push_back( InsertCheckBox(  *this, "CheckBox1Pg4", mxItemListener, getString( STR_SAVE_SETTINGS ), PAGE_POS_X + 6, DIALOG_HEIGHT - 47, 100, mnTabIndex++ ) );
    aControlList.push_back( InsertComboBox(  *this, "ComboBox0Pg4", xTextListener, true, aItemList, PAGE_POS_X + 106, DIALOG_HEIGHT - 48, mnTabIndex++ ) );
    maControlPages.push_back( aControlList );
    DeactivatePage( 4 );

    // creating a default session name that hasn't been used yet
    OUString aSettingsName;
    OUString aDefault( getString( STR_MY_SETTINGS ) );
    sal_Int32 nSession = 1;
    std::vector<OptimizerSettings>::size_type i;
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

    setControlProperty( "ComboBox0Pg4", "Text", Any( aSettingsName ) );
    setControlProperty( "RadioButton0Pg4", "Enabled", Any( !mbIsReadonly ) );
    setControlProperty( "RadioButton1Pg4", "Enabled", Any( !mbIsReadonly ) );

    UpdateControlStatesPage4();
}


void OptimizerDialog::EnablePage( sal_Int16 nStep )
{
    for( const auto& rItem : maControlPages[ nStep ] )
        setControlProperty( rItem, "Enabled", Any( true ) );
}
void OptimizerDialog::DisablePage( sal_Int16 nStep )
{
    for( const auto& rItem : maControlPages[ nStep ] )
        setControlProperty( rItem, "Enabled", Any( false ) );
}
void OptimizerDialog::ActivatePage( sal_Int16 nStep )
{
    for( const auto& rItem : maControlPages[ nStep ] )
        setVisible( rItem, true );
}
void OptimizerDialog::DeactivatePage( sal_Int16 nStep )
{
    for( const auto& rItem : maControlPages[ nStep ] )
        setVisible( rItem, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
