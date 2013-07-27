/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "optimizerdialog.hxx"
#include "minimizer.hrc"
#include "helpid.hrc"

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
#ifndef _COM_SUN_STAR_AWT_XFONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#include <rtl/ustrbuf.hxx>

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

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
// -----------------------------------------------------------------------------

void OptimizerDialog::ImplSetBold( const rtl::OUString& rControl )
{
    FontDescriptor aFontDescriptor;
    if ( getControlProperty( rControl, TKGet( TK_FontDescriptor ) ) >>= aFontDescriptor )
    {
        aFontDescriptor.Weight = FontWeight::BOLD;
        setControlProperty( rControl, TKGet( TK_FontDescriptor ), Any( aFontDescriptor ) );
    }
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::ImplInsertSeparator(
    const OUString& rControlName,
    sal_Int32 nOrientation,
    sal_Int32 nPosX,
    sal_Int32 nPosY,
    sal_Int32 nWidth,
    sal_Int32 nHeight )
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

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedLineModel" ) ),
        rControlName, aNames, aValues );
    return rControlName;
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::ImplInsertButton(
    const OUString& rControlName,
    const rtl::OUString& rHelpURL,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nTabIndex,
    sal_Bool bEnabled,
    sal_Int32 nResID,
    sal_Int16 nPushButtonType )
{
    OUString pNames[] = {
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_HelpURL ),
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
        Any( rHelpURL ),
        Any( getString( nResID ) ),
        Any( nXPos ),
        Any( nYPos ),
        Any( nPushButtonType ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };


    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    insertButton( rControlName, this, aNames, aValues );
    return rControlName;
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::ImplInsertFixedText(
    const rtl::OUString& rControlName,
    const OUString& rLabel,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Bool bMultiLine,
    sal_Bool bBold,
    sal_Int16 nTabIndex )
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

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    insertFixedText( rControlName, aNames, aValues );
    if ( bBold )
        ImplSetBold( rControlName );
    return rControlName;
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::ImplInsertCheckBox(
    const OUString& rControlName,
    const OUString& rLabel,
    const rtl::OUString& rHelpURL,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_HelpURL ),
        TKGet( TK_Label ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( nHeight ),
        Any( rHelpURL ),
        Any( rLabel ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XCheckBox > xCheckBox( insertCheckBox( rControlName, aNames, aValues ) );
    xCheckBox->addItemListener( this );
    return rControlName;
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::ImplInsertFormattedField(
    const OUString& rControlName,
    const rtl::OUString& rHelpURL,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    double fEffectiveMin,
    double fEffectiveMax,
    sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_EffectiveMax ),
        TKGet( TK_EffectiveMin ),
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_HelpURL ),
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
        Any( rHelpURL ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Bool)sal_True ),
        Any( (sal_Bool)sal_True ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XTextComponent > xTextComponent( insertFormattedField( rControlName, aNames, aValues ), UNO_QUERY_THROW );
    xTextComponent->addTextListener( this );
    Reference< XSpinField > xSpinField( xTextComponent, UNO_QUERY_THROW );
    xSpinField->addSpinListener( this );

    return rControlName;
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::ImplInsertComboBox(
    const OUString& rControlName,
    const rtl::OUString& rHelpURL,
    const sal_Bool bEnabled,
    const Sequence< OUString >& rItemList,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nTabIndex,
    bool bListen )
{
    OUString pNames[] = {
        TKGet( TK_Dropdown ),
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_HelpURL ),
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
        Any( rHelpURL ),
        Any( (sal_Int16)8),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( rItemList ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XTextComponent > xTextComponent( insertComboBox( rControlName, aNames, aValues ), UNO_QUERY_THROW );
    if ( bListen )
        xTextComponent->addTextListener( this );
    return rControlName;
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::ImplInsertRadioButton(
    const rtl::OUString& rControlName,
    const OUString& rLabel,
    const rtl::OUString& rHelpURL,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Bool bMultiLine,
    sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Height ),
        TKGet( TK_HelpURL ),
        TKGet( TK_Label ),
        TKGet( TK_MultiLine ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( nHeight ),
        Any( rHelpURL ),
        Any( rLabel ),
        Any( bMultiLine ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XRadioButton > xRadioButton( insertRadioButton( rControlName, aNames, aValues ) );
    xRadioButton->addItemListener( this );
    return rControlName;
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::ImplInsertListBox(
    const OUString& rControlName,
    const rtl::OUString& rHelpURL,
    const sal_Bool bEnabled,
    const Sequence< OUString >& rItemList,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Dropdown ),
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_HelpURL ),
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
        Any( rHelpURL ),
        Any( (sal_Int16)8),
        Any( sal_False ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( rItemList ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XListBox > xListBox( insertListBox( rControlName, aNames, aValues ) );
    xListBox->addActionListener( this );
    return rControlName;
}

// -----------------------------------------------------------------------------

void OptimizerDialog::InitNavigationBar()
{
    sal_Int32   nCancelPosX = OD_DIALOG_WIDTH - BUTTON_WIDTH - 6;
    sal_Int32   nFinishPosX = nCancelPosX - 6 - BUTTON_WIDTH;
    sal_Int32   nNextPosX = nFinishPosX - 6 - BUTTON_WIDTH;
    sal_Int32   nBackPosX = nNextPosX - 3 - BUTTON_WIDTH;

    ImplInsertSeparator( TKGet( TK_lnNavSep1 ), 0, 0, DIALOG_HEIGHT - 26, OD_DIALOG_WIDTH, 1 );
    ImplInsertSeparator( TKGet( TK_lnNavSep2 ), 1, 85, 0, 1, BUTTON_POS_Y - 6 );

    ImplInsertButton( TKGet( TK_btnNavHelp ), HID( HID_SDEXT_MINIMIZER_WIZ_PB_HELP ), 8, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_True, STR_HELP, PushButtonType_HELP );
    ImplInsertButton( TKGet( TK_btnNavBack ), HID( HID_SDEXT_MINIMIZER_WIZ_PB_BACK ), nBackPosX, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_False, STR_BACK, PushButtonType_STANDARD );
    ImplInsertButton( TKGet( TK_btnNavNext ), HID( HID_SDEXT_MINIMIZER_WIZ_PB_NEXT ), nNextPosX, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_True, STR_NEXT, PushButtonType_STANDARD );
    ImplInsertButton( TKGet( TK_btnNavFinish ), HID( HID_SDEXT_MINIMIZER_WIZ_PB_FINISH ), nFinishPosX, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_True, STR_FINISH, PushButtonType_STANDARD );
    ImplInsertButton( TKGet( TK_btnNavCancel ), HID( HID_SDEXT_MINIMIZER_WIZ_PB_CANCEL ), nCancelPosX, BUTTON_POS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, mnTabIndex++, sal_True, STR_CANCEL, PushButtonType_STANDARD );

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
    std::vector< rtl::OUString > aControlList;
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText0Pg0 ), getString( STR_INTRODUCTION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText1Pg0 ), getString( STR_INTRODUCTION_T ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 100, sal_True, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertSeparator( TKGet( TK_Separator1Pg0 ), 0, PAGE_POS_X + 6, DIALOG_HEIGHT - 66, PAGE_WIDTH - 12, 1 ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText2Pg0 ), getString( STR_CHOSE_SETTINGS ), PAGE_POS_X + 6, DIALOG_HEIGHT - 60, PAGE_WIDTH - 12, 8, sal_True, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertListBox(  TKGet( TK_ListBox0Pg0 ), HID(HID_SDEXT_MINIMIZER_WIZ_STEP1_LB_SETTINGS),sal_True, aItemList, PAGE_POS_X + 6, DIALOG_HEIGHT - 48, ( OD_DIALOG_WIDTH - 50 ) - ( PAGE_POS_X + 6 ), 12, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertButton( TKGet( TK_Button0Pg0 ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP1_PB_DELSETTS ),OD_DIALOG_WIDTH - 46, DIALOG_HEIGHT - 49, 40, 14, mnTabIndex++, sal_True, STR_REMOVE, PushButtonType_STANDARD ) );
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
    Reference< XModel > xModel( mxModel );
    if ( xModel.is() )
    {
        Reference< XCustomPresentationSupplier > aXCPSup( xModel, UNO_QUERY_THROW );
        Reference< XNameContainer > aXCont( aXCPSup->getCustomPresentations() );
        if ( aXCont.is() )
            aCustomShowList = aXCont->getElementNames();
    }
    std::vector< rtl::OUString > aControlList;
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText0Pg3 ), getString( STR_CHOOSE_SLIDES ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertCheckBox(  TKGet( TK_CheckBox0Pg3 ), getString( STR_DELETE_MASTER_PAGES ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP2_CB_MASTERPAGES ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertCheckBox(  TKGet( TK_CheckBox2Pg3 ), getString( STR_DELETE_HIDDEN_SLIDES ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP2_CB_HIDDENSLIDES ), PAGE_POS_X + 6, PAGE_POS_Y + 28, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertCheckBox(  TKGet( TK_CheckBox3Pg3 ), getString( STR_CUSTOM_SHOW ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP2_CB_CUSTOMSHOW ), PAGE_POS_X + 6, PAGE_POS_Y + 42, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertListBox(  TKGet( TK_ListBox0Pg3 ), HID(HID_SDEXT_MINIMIZER_WIZ_STEP2_LB_CUSTOMSHOW),sal_True, aCustomShowList, PAGE_POS_X + 14, PAGE_POS_Y + 54, 150, 12, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertCheckBox(  TKGet( TK_CheckBox1Pg3 ), getString( STR_DELETE_NOTES_PAGES ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP2_CB_NOTES ), PAGE_POS_X + 6, PAGE_POS_Y + 70, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
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
    if ( !aResolutionText.getLength() )
        aResolutionText = OUString::valueOf( nImageResolution );

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

    std::vector< rtl::OUString > aControlList;
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText0Pg1 ), getString( STR_GRAPHIC_OPTIMIZATION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertRadioButton( TKGet( TK_RadioButton0Pg1 ), getString( STR_LOSSLESS_COMPRESSION ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP3_RB_LOSSLESS ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertRadioButton( TKGet( TK_RadioButton1Pg1 ), getString( STR_JPEG_COMPRESSION ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP3_RB_JPEG ), PAGE_POS_X + 6, PAGE_POS_Y + 28, PAGE_WIDTH - 12, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText1Pg1 ), getString( STR_QUALITY ), PAGE_POS_X + 20, PAGE_POS_Y + 40, 72, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFormattedField( TKGet( TK_FormattedField0Pg1 ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP3_MF_QUALITY ), PAGE_POS_X + 106, PAGE_POS_Y + 38, 50, 0, 100, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText2Pg1 ), getString( STR_IMAGE_RESOLUTION ), PAGE_POS_X + 6, PAGE_POS_Y + 54, 94, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertComboBox(  TKGet( TK_ComboBox0Pg1 ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP3_LB_DPI ), sal_True, aResolutionItemList, PAGE_POS_X + 106, PAGE_POS_Y + 52, 100, 12, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertCheckBox(  TKGet( TK_CheckBox1Pg1 ), getString( STR_REMOVE_CROP_AREA ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP3_CB_CROP ), PAGE_POS_X + 6, PAGE_POS_Y + 68, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertCheckBox(  TKGet( TK_CheckBox2Pg1 ), getString( STR_EMBED_LINKED_GRAPHICS ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP3_CB_LINKS ), PAGE_POS_X + 6, PAGE_POS_Y + 82, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
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
    Reference< XModel > xModel( mxModel );
    Reference< XDrawPagesSupplier > xDrawPagesSupplier( xModel, UNO_QUERY_THROW );
    Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
    for ( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
    {
        Reference< XShapes > xShapes( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
        for ( sal_Int32 j = 0; j < xShapes->getCount(); j++ )
        {
            const OUString sOLE2Shape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.OLE2Shape" ) );
            Reference< XShape > xShape( xShapes->getByIndex( j ), UNO_QUERY_THROW );
            if ( xShape->getShapeType() == sOLE2Shape )
                nOLECount++;
        }
    }

    std::vector< rtl::OUString > aControlList;
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText0Pg2 ), getString( STR_OLE_OPTIMIZATION ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertCheckBox(  TKGet( TK_CheckBox0Pg2 ), getString( STR_OLE_REPLACE ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP4_CB_OLE ), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertRadioButton( TKGet( TK_RadioButton0Pg2 ), getString( STR_ALL_OLE_OBJECTS ), HID(HID_SDEXT_MINIMIZER_WIZ_STEP4_RB_ALLOLE), PAGE_POS_X + 14, PAGE_POS_Y + 28, PAGE_WIDTH - 22, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertRadioButton( TKGet( TK_RadioButton1Pg2 ), getString( STR_ALIEN_OLE_OBJECTS_ONLY ), HID(HID_SDEXT_MINIMIZER_WIZ_STEP4_RB_NOTODF),PAGE_POS_X + 14, PAGE_POS_Y + 40, PAGE_WIDTH - 22, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText1Pg2 ), nOLECount ? getString( STR_OLE_OBJECTS_DESC ) : getString( STR_NO_OLE_OBJECTS_DESC ), PAGE_POS_X + 6, PAGE_POS_Y + 64, PAGE_WIDTH - 22, 50, sal_True, sal_False, mnTabIndex++ ) );
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
    rtl::OUStringBuffer aVal( OUString::valueOf( fVal ) );
    sal_Int32 nX( OUString( aVal.getStr() ).indexOf( '.', 0 ) );
    if ( nX >= 0 )
    {
        aVal.setLength( nX + 2 );
        aVal.setCharAt( nX, nSeparator );
    }
    aVal.append( OUString::createFromAscii( " MB" ) );
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
    rtl::OUString aCustomShowName;
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
    if ( aCustomShowName.getLength() )
    {
        std::vector< Reference< XDrawPage > > vNonUsedPageList;
        PageCollector::CollectNonCustomShowPages( mxModel, aCustomShowName, vNonUsedPageList );
        nDeletedSlides += vNonUsedPageList.size();
    }
    if ( GetConfigProperty( TK_DeleteHiddenSlides, sal_False ) )
    {
        if ( aCustomShowName.getLength() )
        {
            std::vector< Reference< XDrawPage > > vUsedPageList;
            PageCollector::CollectCustomShowPages( mxModel, aCustomShowName, vUsedPageList );
            std::vector< Reference< XDrawPage > >::iterator aIter( vUsedPageList.begin() );
            while( aIter != vUsedPageList.end() )
            {
                Reference< XPropertySet > xPropSet( *aIter, UNO_QUERY_THROW );
                sal_Bool bVisible = sal_True;
                const OUString sVisible( RTL_CONSTASCII_USTRINGPARAM( "Visible" ) );
                if ( xPropSet->getPropertyValue( sVisible ) >>= bVisible )
                {
                    if (!bVisible )
                        nDeletedSlides++;
                }
                aIter++;
            }
        }
        else
        {
            Reference< XDrawPagesSupplier > xDrawPagesSupplier( mxModel, UNO_QUERY_THROW );
            Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
            for( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
            {
                Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
                Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY_THROW );

                sal_Bool bVisible = sal_True;
                const OUString sVisible( RTL_CONSTASCII_USTRINGPARAM( "Visible" ) );
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
        PageCollector::CollectMasterPages( mxModel, aMasterPageList );
        Reference< XMasterPagesSupplier > xMasterPagesSupplier( mxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
        std::vector< PageCollector::MasterPageEntity >::iterator aIter( aMasterPageList.begin() );
        while( aIter != aMasterPageList.end() )
        {
            if ( !aIter->bUsed )
                nDeletedSlides++;
            aIter++;
        }
    }
    if ( nDeletedSlides > 1 )
    {
        OUString aStr( getString( STR_DELETE_SLIDES ) );
        OUString aPlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%SLIDES" ) );
        sal_Int32 i = aStr.indexOf( aPlaceholder, 0 );
        if ( i >= 0 )
            aStr = aStr.replaceAt( i, aPlaceholder.getLength(), OUString::valueOf( nDeletedSlides ) );
        aSummaryStrings.push_back( aStr );
    }

// generating graphic compression info
    sal_Int32 nGraphics = 0;
    sal_Bool bJPEGCompression( GetConfigProperty( TK_JPEGCompression, sal_False ) );
    sal_Int32 nJPEGQuality( GetConfigProperty( TK_JPEGQuality, (sal_Int32)90 ) );
    sal_Int32 nImageResolution( GetConfigProperty( TK_ImageResolution, (sal_Int32)0 ) );
    GraphicSettings aGraphicSettings( bJPEGCompression, nJPEGQuality, GetConfigProperty( TK_RemoveCropArea, sal_False ),
                                        nImageResolution, GetConfigProperty( TK_EmbedLinkedGraphics, sal_True ) );
    GraphicCollector::CountGraphics( mxContext, mxModel, aGraphicSettings, nGraphics );
    if ( nGraphics > 1 )
    {
        OUString aStr( getString( STR_OPTIMIZE_IMAGES ) );
        OUString aImagePlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%IMAGES" ) );
        OUString aQualityPlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%QUALITY" ) );
        OUString aResolutionPlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%RESOLUTION" ) );
        sal_Int32 i = aStr.indexOf( aImagePlaceholder, 0 );
        if ( i >= 0 )
            aStr = aStr.replaceAt( i, aImagePlaceholder.getLength(), OUString::valueOf( nGraphics ) );

        sal_Int32 j = aStr.indexOf( aQualityPlaceholder, 0 );
        if ( j >= 0 )
            aStr = aStr.replaceAt( j, aQualityPlaceholder.getLength(), OUString::valueOf( nJPEGQuality ) );

        sal_Int32 k = aStr.indexOf( aResolutionPlaceholder, 0 );
        if ( k >= 0 )
            aStr = aStr.replaceAt( k, aResolutionPlaceholder.getLength(), OUString::valueOf( nImageResolution ) );

        aSummaryStrings.push_back( aStr );
    }

    if ( GetConfigProperty( TK_OLEOptimization, sal_False ) )
    {
        sal_Int32 nOLEReplacements = 0;
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( mxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        for ( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
        {
            Reference< XShapes > xShapes( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            for ( sal_Int32 j = 0; j < xShapes->getCount(); j++ )
            {
                const OUString sOLE2Shape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.OLE2Shape" ) );
                Reference< XShape > xShape( xShapes->getByIndex( j ), UNO_QUERY_THROW );
                if ( xShape->getShapeType() == sOLE2Shape )
                    nOLEReplacements++;
            }
        }
        if ( nOLEReplacements > 1 )
        {
            OUString aStr( getString( STR_CREATE_REPLACEMENT ) );
            OUString aPlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%OLE" ) );
            sal_Int32 i = aStr.indexOf( aPlaceholder, 0 );
            if ( i >= 0 )
                aStr = aStr.replaceAt( i, aPlaceholder.getLength(), OUString::valueOf( nOLEReplacements ) );
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
    Reference< XStorable > xStorable( mxModel, UNO_QUERY );
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
    if ( aStr.getLength() )
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
            Any( OUString( RTL_CONSTASCII_USTRINGPARAM("STR_SAVE_AS") ) ),//TODO
            Any( (sal_Int32)( PAGE_POS_X + 6 ) ),
            Any( (sal_Int32)( DIALOG_HEIGHT - 75 ) ),
            Any( (sal_Int32)( 0 ) ),
            Any( (sal_Int32)( 100 ) ),
            Any( (sal_Int32)( 0 ) ),
            Any( (sal_Int32)( PAGE_WIDTH - 12 ) ) };

        sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

        Sequence< rtl::OUString >   aNames( pNames, nCount );
        Sequence< Any >             aValues( pValues, nCount );

        Reference< XMultiPropertySet > xMultiPropertySet( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlProgressBarModel" ) ),
            TKGet( TK_Progress ), aNames, aValues ), UNO_QUERY );
    }

    Sequence< OUString > aItemList;
    std::vector< rtl::OUString > aControlList;
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText0Pg4 ), getString( STR_SUMMARY_TITLE ), PAGE_POS_X, PAGE_POS_Y, PAGE_WIDTH, 8, sal_False, sal_True, mnTabIndex++ ) );
//  aControlList.push_back( ImplInsertSeparator( TKGet( TK_Separator0Pg4 ), 0, PAGE_POS_X + 6, PAGE_POS_Y + 90, PAGE_WIDTH - 12, 1 ) );

    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText4Pg4 ), OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 14, PAGE_WIDTH - 12, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText5Pg4 ), OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 22, PAGE_WIDTH - 12, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText6Pg4 ), OUString(), PAGE_POS_X + 6, PAGE_POS_Y + 30, PAGE_WIDTH - 12, 8, sal_False, sal_False, mnTabIndex++ ) );

    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText2Pg4 ), getString( STR_CURRENT_FILESIZE ), PAGE_POS_X + 6, PAGE_POS_Y + 50, 88, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText7Pg4 ), OUString(), PAGE_POS_X + 100, PAGE_POS_Y + 50, 30, 8, sal_False, sal_False, mnTabIndex++ ) );
    setControlProperty( TKGet( TK_FixedText7Pg4 ), TKGet( TK_Align ), Any( static_cast< short >( 2 ) ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText3Pg4 ), getString( STR_ESTIMATED_FILESIZE ), PAGE_POS_X + 6, PAGE_POS_Y + 58, 88, 8, sal_False, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText8Pg4 ), OUString(), PAGE_POS_X + 100, PAGE_POS_Y + 58, 30, 8, sal_False, sal_False, mnTabIndex++ ) );
    setControlProperty( TKGet( TK_FixedText8Pg4 ), TKGet( TK_Align ), Any( static_cast< short >( 2 ) ) );

    aControlList.push_back( ImplInsertRadioButton( TKGet( TK_RadioButton0Pg4 ), getString(  STR_APPLY_TO_CURRENT ), HID(HID_SDEXT_MINIMIZER_WIZ_STEP5_RB_CURDOC),PAGE_POS_X + 6, PAGE_POS_Y + 78, PAGE_WIDTH - 12, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertRadioButton( TKGet( TK_RadioButton1Pg4 ), getString( STR_SAVE_AS ), HID(HID_SDEXT_MINIMIZER_WIZ_STEP5_RB_NEWDOC),PAGE_POS_X + 6, PAGE_POS_Y + 90, PAGE_WIDTH - 12, 8, sal_False, mnTabIndex++ ) );
    aControlList.push_back( ImplInsertFixedText( TKGet( TK_FixedText1Pg4 ), OUString(), PAGE_POS_X + 6, DIALOG_HEIGHT - 87, PAGE_WIDTH - 12, 8, sal_True, sal_False, mnTabIndex++ ) );
    aControlList.push_back( TKGet( TK_Progress ) );
    aControlList.push_back( ImplInsertSeparator( TKGet( TK_Separator1Pg4 ), 0, PAGE_POS_X + 6, DIALOG_HEIGHT - 58, PAGE_WIDTH - 12, 1 ) );
    aControlList.push_back( ImplInsertCheckBox(  TKGet( TK_CheckBox1Pg4 ), getString( STR_SAVE_SETTINGS ), HID( HID_SDEXT_MINIMIZER_WIZ_STEP5_CB_SAVESETTINGS ), PAGE_POS_X + 6, DIALOG_HEIGHT - 47, 100, 8, mnTabIndex++ ) );
    // don't listen to this
    aControlList.push_back(
        ImplInsertComboBox(
            TKGet( TK_ComboBox0Pg4 ),  HID( HID_SDEXT_MINIMIZER_WIZ_STEP5_LB_SETTINGSNAME ),sal_True, aItemList,
            PAGE_POS_X + 106, DIALOG_HEIGHT - 48, 100, 12, mnTabIndex++ ,
            false) );
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
        OUString aTemp( aDefault.concat( OUString::valueOf( nSession++ ) ) );
        for ( i = 1; i < rList.size(); i++ )
        {
            if ( rList[ i ].maName == aTemp )
                break;
        }
        if ( i == rList.size() )
            aSettingsName = aTemp;
    }
    while( !aSettingsName.getLength() );

    setControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_Text ), Any( aSettingsName ) );
    setControlProperty( TKGet( TK_RadioButton0Pg4 ), TKGet( TK_Enabled ), Any( !mbIsReadonly ) );
    setControlProperty( TKGet( TK_RadioButton1Pg4 ), TKGet( TK_Enabled ), Any( !mbIsReadonly ) );

    UpdateControlStatesPage4();
}

// -----------------------------------------------------------------------------
void OptimizerDialog::EnablePage( sal_Int16 nStep )
{
    std::vector< rtl::OUString >::iterator aBeg( maControlPages[ nStep ].begin() );
    std::vector< rtl::OUString >::iterator aEnd( maControlPages[ nStep ].end() );
    while( aBeg != aEnd )
        setControlProperty( *aBeg++, TKGet( TK_Enabled ), Any( sal_True ) );
}
void OptimizerDialog::DisablePage( sal_Int16 nStep )
{
    std::vector< rtl::OUString >::iterator aBeg( maControlPages[ nStep ].begin() );
    std::vector< rtl::OUString >::iterator aEnd( maControlPages[ nStep ].end() );
    while( aBeg != aEnd )
        setControlProperty( *aBeg++, TKGet( TK_Enabled ), Any( sal_False ) );
}
void OptimizerDialog::ActivatePage( sal_Int16 nStep )
{
    std::vector< rtl::OUString >::iterator aBeg( maControlPages[ nStep ].begin() );
    std::vector< rtl::OUString >::iterator aEnd( maControlPages[ nStep ].end() );
    while( aBeg != aEnd )
        setVisible( *aBeg++, sal_True );
}
void OptimizerDialog::DeactivatePage( sal_Int16 nStep )
{
    std::vector< rtl::OUString >::iterator aBeg( maControlPages[ nStep ].begin() );
    std::vector< rtl::OUString >::iterator aEnd( maControlPages[ nStep ].end() );
    while( aBeg != aEnd )
        setVisible( *aBeg++, sal_False );
}
