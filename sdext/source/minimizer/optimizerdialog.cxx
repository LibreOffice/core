/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "optimizerdialog.hxx"
#include "fileopendialog.hxx"
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <sal/macros.h>
#include <osl/time.h>

// -------------------
// - OPTIMIZERDIALOG -
// -------------------

using namespace ::rtl;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;


// -----------------------------------------------------------------------------

void OptimizerDialog::InitDialog()
{
   // setting the dialog properties
    OUString pNames[] = {
        TKGet( TK_Closeable ),
        TKGet( TK_Height ),
        TKGet( TK_Moveable ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Title ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( sal_Int32( DIALOG_HEIGHT ) ),
        Any( sal_True ),
        Any( sal_Int32( 200 ) ),
        Any( sal_Int32( 52 ) ),
        Any( getString( STR_SUN_OPTIMIZATION_WIZARD2 ) ),
        Any( sal_Int32( OD_DIALOG_WIDTH ) ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    mxDialogModelMultiPropertySet->setPropertyValues( aNames, aValues );
}

// -----------------------------------------------------------------------------

void OptimizerDialog::InitRoadmap()
{
    try
    {
        OUString pNames[] = {
            TKGet( TK_Height ),
            TKGet( TK_PositionX ),
            TKGet( TK_PositionY ),
            TKGet( TK_Step ),
            TKGet( TK_TabIndex ),
            TKGet( TK_Width ) };

        Any pValues[] = {
            Any( sal_Int32( DIALOG_HEIGHT - 26 ) ),
            Any( sal_Int32( 0 ) ),
            Any( sal_Int32( 0 ) ),
            Any( sal_Int32( 0 ) ),
            Any( mnTabIndex++ ),
            Any( sal_Int32( 85 ) ) };

        sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

        Sequence< rtl::OUString >   aNames( pNames, nCount );
        Sequence< Any >             aValues( pValues, nCount );

        mxRoadmapControlModel = insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlRoadmapModel" ) ),
                                                              TKGet( TK_rdmNavi ), aNames, aValues  );

        Reference< XPropertySet > xPropertySet( mxRoadmapControlModel, UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( TKGet( TK_Name ), Any( TKGet( TK_rdmNavi ) ) );
        mxRoadmapControl = mxDialogControlContainer->getControl( TKGet( TK_rdmNavi ) );
        InsertRoadmapItem( 0, sal_True, getString( STR_INTRODUCTION ), ITEM_ID_INTRODUCTION );
        InsertRoadmapItem( 1, sal_True, getString( STR_SLIDES ), ITEM_ID_SLIDES );
        InsertRoadmapItem( 2, sal_True, getString( STR_IMAGE_OPTIMIZATION ), ITEM_ID_GRAPHIC_OPTIMIZATION );
        InsertRoadmapItem( 3, sal_True, getString( STR_OLE_OBJECTS ), ITEM_ID_OLE_OPTIMIZATION );
        InsertRoadmapItem( 4, sal_True, getString( STR_SUMMARY ), ITEM_ID_SUMMARY );

        rtl::OUString sBitmapPath( getPath( TK_BitmapPath ) );
        rtl::OUString sBitmap( isHighContrast() ? RTL_CONSTASCII_USTRINGPARAM("/minimizepresi_80_h.png")
                                                : RTL_CONSTASCII_USTRINGPARAM("/minimizepresi_80.png") );
        rtl::OUString sURL( sBitmapPath += sBitmap );

        xPropertySet->setPropertyValue( TKGet( TK_ImageURL ), Any( sURL ) );
        xPropertySet->setPropertyValue( TKGet( TK_Activated ), Any( (sal_Bool)sal_True ) );
        xPropertySet->setPropertyValue( TKGet( TK_Complete ), Any( (sal_Bool)sal_True ) );
        xPropertySet->setPropertyValue( TKGet( TK_CurrentItemID ), Any( (sal_Int16)ITEM_ID_INTRODUCTION ) );
        xPropertySet->setPropertyValue( TKGet( TK_Text ), Any( getString( STR_STEPS ) ) );
    }
    catch( Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void OptimizerDialog::InsertRoadmapItem( const sal_Int32 nIndex, const sal_Bool bEnabled, const rtl::OUString& rLabel, const sal_Int32 nItemID )
{
    try
    {
        Reference< XSingleServiceFactory > xSFRoadmap( mxRoadmapControlModel, UNO_QUERY_THROW );
        Reference< XIndexContainer > aIndexContainerRoadmap( mxRoadmapControlModel, UNO_QUERY_THROW );
        Reference< XInterface > xRoadmapItem( xSFRoadmap->createInstance(), UNO_QUERY_THROW );
        Reference< XPropertySet > xPropertySet( xRoadmapItem, UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( TKGet( TK_Label ), Any( rLabel ) );
        xPropertySet->setPropertyValue( TKGet( TK_Enabled ), Any( bEnabled ) );
        xPropertySet->setPropertyValue( TKGet( TK_ID ), Any( nItemID ) );
        aIndexContainerRoadmap->insertByIndex( nIndex, Any( xRoadmapItem ) );
    }
    catch( Exception& )
    {

    }
}

// -----------------------------------------------------------------------------

void OptimizerDialog::UpdateConfiguration()
{
    sal_Int16   nInt16 = 0;
    OUString    aString;
    Any         aAny;

    Sequence< sal_Int16 > aSelectedItems;
    Sequence< OUString > aStringItemList;

    // page0
    aAny = getControlProperty( TKGet( TK_ListBox0Pg0 ), TKGet( TK_SelectedItems ) );
    if ( aAny >>= aSelectedItems )
    {
        if ( aSelectedItems.getLength() )
        {
            sal_Int16 nSelectedItem = aSelectedItems[ 0 ];
            aAny = getControlProperty( TKGet( TK_ListBox0Pg0 ), TKGet( TK_StringItemList ) );
            if ( aAny >>= aStringItemList )
            {
                if ( aStringItemList.getLength() > nSelectedItem )
                    SetConfigProperty( TK_Name, Any( aStringItemList[ nSelectedItem ] ) );
            }
        }
    }

    aAny = getControlProperty( TKGet( TK_CheckBox3Pg3 ), TKGet( TK_State ) );
    if ( aAny >>= nInt16 )
    {
        if ( nInt16 )
        {
            aAny = getControlProperty( TKGet( TK_ListBox0Pg3 ), TKGet( TK_SelectedItems ) );
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
}

// -----------------------------------------------------------------------------

OptimizerDialog::OptimizerDialog( const Reference< XComponentContext > &rxMSF, Reference< XFrame >& rxFrame, Reference< XDispatch > rxStatusDispatcher ) :
    UnoDialog( rxMSF, rxFrame ),
    ConfigurationAccess( rxMSF, NULL ),
    mnCurrentStep( 0 ),
    mnTabIndex( 0 ),
    mxMSF( rxMSF ),
    mxFrame( rxFrame ),
    mxItemListener( new ItemListener( *this ) ),
    mxActionListener( new ActionListener( *this ) ),
    mxActionListenerListBox0Pg0( new ActionListenerListBox0Pg0( *this ) ),
    mxTextListenerFormattedField0Pg1( new TextListenerFormattedField0Pg1( *this ) ),
    mxTextListenerComboBox0Pg1( new TextListenerComboBox0Pg1( *this ) ),
    mxSpinListenerFormattedField0Pg1( new SpinListenerFormattedField0Pg1( *this ) ),
    mxStatusDispatcher( rxStatusDispatcher )
{
    Reference< XStorable > xStorable( mxController->getModel(), UNO_QUERY_THROW );
    mbIsReadonly = xStorable->isReadonly();

    InitDialog();
    InitRoadmap();
    InitNavigationBar();
    InitPage0();
    InitPage1();
    InitPage2();
    InitPage3();
    InitPage4();
    ActivatePage( 0 );

    OptimizationStats aStats;
    aStats.InitializeStatusValuesFromDocument( mxController->getModel() );
    Sequence< PropertyValue > aStatusSequence( aStats.GetStatusSequence() );
    UpdateStatus( aStatusSequence );
}

// -----------------------------------------------------------------------------

OptimizerDialog::~OptimizerDialog()
{
    // not saving configuration if the dialog has been finished via cancel or close window
    if ( mbStatus )
        SaveConfiguration();
}

// -----------------------------------------------------------------------------

sal_Bool OptimizerDialog::execute()
{
    Reference< XItemEventBroadcaster > maRoadmapBroadcaster( mxRoadmapControl, UNO_QUERY_THROW );
    maRoadmapBroadcaster->addItemListener( mxItemListener );
    UnoDialog::execute();
    UpdateConfiguration();          // taking actual control settings for the configuration
    maRoadmapBroadcaster->removeItemListener( mxItemListener );
    return mbStatus;
}

// -----------------------------------------------------------------------------

void OptimizerDialog::SwitchPage( sal_Int16 nNewStep )
{
    if ( ( nNewStep != mnCurrentStep ) && ( ( nNewStep <= MAX_STEP ) || ( nNewStep >= 0 ) ) )
    {
        sal_Int16 nOldStep = mnCurrentStep;
        if ( nNewStep == 0 )
            disableControl( TKGet( TK_btnNavBack ) );
        else if ( nOldStep == 0 )
            enableControl( TKGet( TK_btnNavBack ) );

        if ( nNewStep == MAX_STEP )
            disableControl( TKGet( TK_btnNavNext ) );
        else if ( nOldStep == MAX_STEP )
            enableControl( TKGet( TK_btnNavNext ) );

        setControlProperty( TKGet( TK_rdmNavi ), TKGet( TK_CurrentItemID ), Any( nNewStep ) );

        DeactivatePage( nOldStep );
        UpdateControlStates( nNewStep );

        ActivatePage( nNewStep );
        mnCurrentStep = nNewStep;
    }
}

void OptimizerDialog::UpdateControlStates( sal_Int16 nPage )
{
    switch( nPage )
    {
        case 0 : UpdateControlStatesPage0(); break;
        case 1 : UpdateControlStatesPage1(); break;
        case 2 : UpdateControlStatesPage2(); break;
        case 3 : UpdateControlStatesPage3(); break;
        case 4 : UpdateControlStatesPage4(); break;
        default:
        {
            UpdateControlStatesPage0();
            UpdateControlStatesPage1();
            UpdateControlStatesPage2();
            UpdateControlStatesPage3();
            UpdateControlStatesPage4();
        }
    }
}

// -----------------------------------------------------------------------------

rtl::OUString OptimizerDialog::GetSelectedString( const PPPOptimizerTokenEnum eToken )
{
    OUString aSelectedItem;
    Sequence< sal_Int16 > sSelectedItems;
    Sequence< OUString >  sItemList;

    if ( ( getControlProperty( TKGet( eToken ), TKGet( TK_SelectedItems ) ) >>= sSelectedItems ) &&
            ( getControlProperty( TKGet( eToken ), TKGet( TK_StringItemList ) ) >>= sItemList ) )
    {
        if ( sSelectedItems.getLength() == 1 )
        {
            sal_Int16 nSelectedItem = sSelectedItems[ 0 ];
            if ( nSelectedItem < sItemList.getLength() )
                aSelectedItem = sItemList[ nSelectedItem ];
        }
    }
    return aSelectedItem;
}

// -----------------------------------------------------------------------------

void OptimizerDialog::UpdateStatus( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rStatus )
{
    if ( mxReschedule.is() )
    {
        maStats.InitializeStatusValues( rStatus );
        const Any* pVal( maStats.GetStatusValue( TK_Status ) );
        if ( pVal )
        {
            rtl::OUString sStatus;
            if ( *pVal >>= sStatus )
            {
                setControlProperty( TKGet( TK_FixedText1Pg4 ), TKGet( TK_Enabled ), Any( sal_True ) );
                setControlProperty( TKGet( TK_FixedText1Pg4 ), TKGet( TK_Label ), Any( getString( TKGet( sStatus ) ) ) );
            }
        }
        pVal = maStats.GetStatusValue( TK_Progress );
        if ( pVal )
        {
            sal_Int32 nProgress = 0;
            if ( *pVal >>= nProgress )
                setControlProperty( TKGet( TK_Progress ), TKGet( TK_ProgressValue ), Any( nProgress ) );
        }
        pVal = maStats.GetStatusValue( TK_OpenNewDocument );
        if ( pVal )
            SetConfigProperty( TK_OpenNewDocument, *pVal );

        mxReschedule->reschedule();
    }
}

// -----------------------------------------------------------------------------

void ItemListener::itemStateChanged( const ItemEvent& Event )
    throw ( RuntimeException )
{
    try
    {
        sal_Int16 nState;
        OUString aControlName;
        Reference< XControl > xControl;
        Any aSource( Event.Source );
        if ( aSource >>= xControl )
        {
            Reference< XPropertySet > xPropertySet( xControl->getModel(), UNO_QUERY_THROW );
            xPropertySet->getPropertyValue( TKGet( TK_Name ) ) >>= aControlName;
            PPPOptimizerTokenEnum eControl( TKGet( aControlName ) );
            switch( eControl )
            {
                case TK_rdmNavi :
                {
                    mrOptimizerDialog.SwitchPage( static_cast< sal_Int16 >( Event.ItemId ) );
                }
                break;
                case TK_CheckBox1Pg1 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_RemoveCropArea, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox2Pg1 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_EmbedLinkedGraphics, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox0Pg2 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                    {
                        mrOptimizerDialog.SetConfigProperty( TK_OLEOptimization, Any( nState != 0 ) );
                        mrOptimizerDialog.setControlProperty( TKGet( TK_RadioButton0Pg2 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                        mrOptimizerDialog.setControlProperty( TKGet( TK_RadioButton1Pg2 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                    }
                }
                break;
                case TK_RadioButton0Pg1 :
                {
                    sal_Int16 nInt16 = 0;
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nInt16 )
                    {
                        nInt16 ^= 1;
                        mrOptimizerDialog.SetConfigProperty( TK_JPEGCompression, Any( nInt16 != 0 ) );
                        mrOptimizerDialog.setControlProperty( TKGet( TK_FixedText1Pg1 ), TKGet( TK_Enabled ), Any( nInt16 != 0 ) );
                        mrOptimizerDialog.setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_Enabled ), Any( nInt16 != 0 ) );
                    }
                }
                break;
                case TK_RadioButton1Pg1 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                    {
                        mrOptimizerDialog.SetConfigProperty( TK_JPEGCompression, Any( nState != 0 ) );
                        mrOptimizerDialog.setControlProperty( TKGet( TK_FixedText1Pg1 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                        mrOptimizerDialog.setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                    }
                }
                break;
                case TK_RadioButton0Pg2 :
                {
                    sal_Int16 nInt16;
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nInt16 )
                    {
                        nInt16 ^= 1;
                        mrOptimizerDialog.SetConfigProperty( TK_OLEOptimizationType, Any( nInt16 ) );
                    }
                }
                break;
                case TK_RadioButton1Pg2 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_OLEOptimizationType, Any( nState ) );
                }
                break;
                case TK_CheckBox0Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_DeleteUnusedMasterPages, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox1Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_DeleteNotesPages, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox2Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_DeleteHiddenSlides, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox3Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.setControlProperty( TKGet( TK_ListBox0Pg3 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox1Pg4 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.setControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                }
                break;
                case TK_RadioButton0Pg4 :
                case TK_RadioButton1Pg4 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_SaveAs, Any( eControl == TK_RadioButton1Pg4 ? nState != 0 : nState == 0 ) );
                }
                break;
                default:
                break;
            }
        }
    }
    catch ( Exception& )
    {

    }
}
void ItemListener::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw ( com::sun::star::uno::RuntimeException )
{
}

// -----------------------------------------------------------------------------

void ActionListener::actionPerformed( const ActionEvent& rEvent )
    throw ( com::sun::star::uno::RuntimeException )
{
    switch( TKGet( rEvent.ActionCommand ) )
    {
        case TK_btnNavHelp :
        {
            try
            {
                static Reference< XFrame > xHelpFrame;
                if ( !xHelpFrame.is() )
                {
                    rtl::OUString sHelpFile( mrOptimizerDialog.getPath( TK_HelpFile ) );
                    Reference< XDesktop > desktop( mrOptimizerDialog.GetComponentContext()->getServiceManager()->createInstanceWithContext(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")), mrOptimizerDialog.GetComponentContext() ), UNO_QUERY_THROW );
                    Reference< XSimpleFileAccess > xSimpleFileAccess( mrOptimizerDialog.GetComponentContext()->getServiceManager()->createInstanceWithContext(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess")), mrOptimizerDialog.GetComponentContext() ), UNO_QUERY_THROW );
                    Reference< XInputStream > xInputStream( xSimpleFileAccess->openFileRead( sHelpFile ) );
                    Reference< XDesktop > xDesktop( mrOptimizerDialog.GetComponentContext()->getServiceManager()->createInstanceWithContext(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")), mrOptimizerDialog.GetComponentContext() ), UNO_QUERY_THROW );
                    Reference< XFrame > xDesktopFrame( xDesktop, UNO_QUERY_THROW );
                    xHelpFrame = Reference< XFrame >( xDesktopFrame->findFrame( TKGet( TK__blank ), 0 ) );
                    Reference< XCloseBroadcaster > xCloseBroadcaster( xHelpFrame, UNO_QUERY_THROW );
                    xCloseBroadcaster->addCloseListener( new HelpCloseListener( xHelpFrame ) );
                    Reference< XComponentLoader > xLoader( xHelpFrame, UNO_QUERY_THROW );

                    Sequence< PropertyValue > aLoadProps( 2 );
                    aLoadProps[ 0 ].Name = TKGet( TK_ReadOnly );
                    aLoadProps[ 0 ].Value <<= (sal_Bool)( sal_True );
                    aLoadProps[ 1 ].Name = TKGet( TK_InputStream );
                    aLoadProps[ 1 ].Value <<= xInputStream;

                    Reference< XComponent >( xLoader->loadComponentFromURL( OUString(RTL_CONSTASCII_USTRINGPARAM("private:stream")),
                        TKGet( TK__self ), 0, aLoadProps ) );

                    Reference< XPropertySet > xPropSet( xHelpFrame, UNO_QUERY_THROW );
                    Reference< XLayoutManager > xLayoutManager;
                    if ( xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("LayoutManager")) ) >>= xLayoutManager )
                    {
                        xLayoutManager->setVisible( sal_False );
                        xLayoutManager->hideElement( OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/menubar/menubar")) );
                        xLayoutManager->destroyElement( OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/statusbar/statusbar")) );
                    }
                }
            }
            catch( Exception& )
            {

            }
        }
        break;
        case TK_btnNavBack :    mrOptimizerDialog.SwitchPage( mrOptimizerDialog.mnCurrentStep - 1 ); break;
        case TK_btnNavNext :    mrOptimizerDialog.SwitchPage( mrOptimizerDialog.mnCurrentStep + 1 ); break;
        case TK_btnNavFinish :
        {
            mrOptimizerDialog.UpdateConfiguration();

            mrOptimizerDialog.SwitchPage( ITEM_ID_SUMMARY );
            mrOptimizerDialog.DisablePage( ITEM_ID_SUMMARY );
            mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavHelp ), TKGet( TK_Enabled ), Any( sal_False ) );
            mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavBack ), TKGet( TK_Enabled ), Any( sal_False ) );
            mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavNext ), TKGet( TK_Enabled ), Any( sal_False ) );
            mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavFinish ), TKGet( TK_Enabled ), Any( sal_False ) );
            mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavCancel ), TKGet( TK_Enabled ), Any( sal_False ) );
            mrOptimizerDialog.setControlProperty( TKGet( TK_FixedText0Pg4 ), TKGet( TK_Enabled ), Any( sal_True ) );

            // check if we have to open the FileDialog
            sal_Bool    bSuccessfullyExecuted = sal_True;
            sal_Int16   nInt16 = 0;
            mrOptimizerDialog.getControlProperty( TKGet( TK_RadioButton1Pg4 ), TKGet( TK_State ) ) >>= nInt16;
            if ( nInt16 )
            {
                rtl::OUString aSaveAsURL;
                FileOpenDialog aFileOpenDialog( ((UnoDialog&)mrOptimizerDialog).mxMSF );

                // generating default file name
                Reference< XStorable > xStorable( mrOptimizerDialog.mxController->getModel(), UNO_QUERY );
                if ( xStorable.is() && xStorable->hasLocation() )
                {
                    rtl::OUString aLocation( xStorable->getLocation() );
                    if ( aLocation.getLength() )
                    {
                        sal_Int32 nIndex = aLocation.lastIndexOf( '/', aLocation.getLength() - 1 );
                        if ( nIndex >= 0 )
                        {
                            if ( nIndex < aLocation.getLength() - 1 )
                                aLocation = aLocation.copy( nIndex + 1 );

                            // remove extension
                            nIndex = aLocation.lastIndexOf( '.', aLocation.getLength() - 1 );
                            if ( nIndex >= 0 )
                                aLocation = aLocation.copy( 0, nIndex );

                            // adding .mini
                            aLocation = aLocation.concat( OUString(RTL_CONSTASCII_USTRINGPARAM(".mini")) );
                            aFileOpenDialog.setDefaultName( aLocation );
                        }
                    }
                }
                 sal_Bool bDialogExecuted = aFileOpenDialog.execute() == dialogs::ExecutableDialogResults::OK;
                if ( bDialogExecuted )
                {
                    aSaveAsURL = aFileOpenDialog.getURL();
                    mrOptimizerDialog.SetConfigProperty( TK_SaveAsURL, Any( aSaveAsURL ) );
                    mrOptimizerDialog.SetConfigProperty( TK_FilterName, Any( aFileOpenDialog.getFilterName() ) );
                }
                if ( !aSaveAsURL.getLength() )
                {
                    // something goes wrong...
                    bSuccessfullyExecuted = sal_False;
                }

                // waiting for 500ms
                if ( mrOptimizerDialog.mxReschedule.is() )
                {
                    mrOptimizerDialog.mxReschedule->reschedule();
                    for ( sal_uInt32 i = osl_getGlobalTimer(); ( i + 500 ) > ( osl_getGlobalTimer() ); )
                    mrOptimizerDialog.mxReschedule->reschedule();
                }
            }
            if ( bSuccessfullyExecuted )
            {   // now check if we have to store a session template
                nInt16 = 0;
                OUString aSettingsName;
                mrOptimizerDialog.getControlProperty( TKGet( TK_CheckBox1Pg4 ), TKGet( TK_State ) ) >>= nInt16;
                mrOptimizerDialog.getControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_Text ) ) >>= aSettingsName;
                if ( nInt16 && aSettingsName.getLength() )
                {
                    std::vector< OptimizerSettings >::iterator aIter( mrOptimizerDialog.GetOptimizerSettingsByName( aSettingsName ) );
                    std::vector< OptimizerSettings >& rSettings( mrOptimizerDialog.GetOptimizerSettings() );
                    OptimizerSettings aNewSettings( rSettings[ 0 ] );
                    aNewSettings.maName = aSettingsName;
                    if ( aIter == rSettings.end() )
                        rSettings.push_back( aNewSettings );
                    else
                        *aIter = aNewSettings;
                }
            }
            if ( bSuccessfullyExecuted )
            {
                Sequence< Any > aArgs( 1 );
                aArgs[ 0 ] <<= mrOptimizerDialog.GetFrame();

                Reference < XDispatch > xDispatch( mrOptimizerDialog.GetComponentContext()->getServiceManager()->createInstanceWithArgumentsAndContext(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.PPPOptimizer")), aArgs, mrOptimizerDialog.GetComponentContext() ), UNO_QUERY );

                URL aURL;
                aURL.Protocol = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.com.sun.star.comp.PPPOptimizer:" ) );
                aURL.Path = OUString( RTL_CONSTASCII_USTRINGPARAM( "optimize" ) );

                Sequence< PropertyValue > lArguments( 3 );
                lArguments[ 0 ].Name = TKGet( TK_Settings );
                lArguments[ 0 ].Value <<= mrOptimizerDialog.GetConfigurationSequence();
                lArguments[ 1 ].Name = TKGet( TK_StatusDispatcher );
                lArguments[ 1 ].Value <<= mrOptimizerDialog.GetStatusDispatcher();
                lArguments[ 2 ].Name = TKGet( TK_InformationDialog );
                lArguments[ 2 ].Value <<= mrOptimizerDialog.GetFrame();

                if( xDispatch.is() )
                    xDispatch->dispatch( aURL, lArguments );

                mrOptimizerDialog.endExecute( bSuccessfullyExecuted );
            }
            else
            {
                mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavHelp ), TKGet( TK_Enabled ), Any( sal_True ) );
                mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavBack ), TKGet( TK_Enabled ), Any( sal_True ) );
                mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavNext ), TKGet( TK_Enabled ), Any( sal_False ) );
                mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavFinish ), TKGet( TK_Enabled ), Any( sal_True ) );
                mrOptimizerDialog.setControlProperty( TKGet( TK_btnNavCancel ), TKGet( TK_Enabled ), Any( sal_True ) );
                mrOptimizerDialog.EnablePage( ITEM_ID_SUMMARY );
            }
        }
        break;
        case TK_btnNavCancel :  mrOptimizerDialog.endExecute( sal_False ); break;
        case TK_Button0Pg0 :    // delete configuration
        {
            OUString aSelectedItem( mrOptimizerDialog.GetSelectedString( TK_ListBox0Pg0 ) );
            if ( aSelectedItem.getLength() )
            {
                std::vector< OptimizerSettings >::iterator aIter( mrOptimizerDialog.GetOptimizerSettingsByName( aSelectedItem ) );
                std::vector< OptimizerSettings >& rList( mrOptimizerDialog.GetOptimizerSettings() );
                if ( aIter != rList.end() )
                {
                    rList.erase( aIter );
                    mrOptimizerDialog.UpdateControlStates();
                }
            }
        }
        break;
        default: break;
    }
}
void ActionListener::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw ( com::sun::star::uno::RuntimeException )
{
}

// -----------------------------------------------------------------------------

void ActionListenerListBox0Pg0::actionPerformed( const ActionEvent& rEvent )
    throw ( com::sun::star::uno::RuntimeException )
{
    if ( rEvent.ActionCommand.getLength() )
    {
        std::vector< OptimizerSettings >::iterator aIter( mrOptimizerDialog.GetOptimizerSettingsByName( rEvent.ActionCommand ) );
        std::vector< OptimizerSettings >& rList( mrOptimizerDialog.GetOptimizerSettings() );
        if ( aIter != rList.end() )
            rList[ 0 ] = *aIter;
    }
    mrOptimizerDialog.UpdateControlStates();
}
void ActionListenerListBox0Pg0::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw ( com::sun::star::uno::RuntimeException )
{
}

// -----------------------------------------------------------------------------

void TextListenerFormattedField0Pg1::textChanged( const TextEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    double fDouble = 0;
    Any aAny = mrOptimizerDialog.getControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ) );
    if ( aAny >>= fDouble )
        mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)fDouble ) );
}
void TextListenerFormattedField0Pg1::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw ( com::sun::star::uno::RuntimeException )
{
}

// -----------------------------------------------------------------------------

void TextListenerComboBox0Pg1::textChanged( const TextEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    rtl::OUString aString;
    Any aAny = mrOptimizerDialog.getControlProperty( TKGet( TK_ComboBox0Pg1 ), TKGet( TK_Text ) );
    if ( aAny >>= aString )
    {
        sal_Int32 nI0, nI1, nI2, nI3, nI4;
        nI0 = nI1 = nI2 = nI3 = nI4 = 0;

        if ( mrOptimizerDialog.getString( STR_IMAGE_RESOLUTION_0 ).getToken( 1, ';', nI0 ) == aString )
            aString = mrOptimizerDialog.getString( STR_IMAGE_RESOLUTION_0 ).getToken( 0, ';', nI4 );
        else if ( mrOptimizerDialog.getString( STR_IMAGE_RESOLUTION_1 ).getToken( 1, ';', nI1 ) == aString )
            aString = mrOptimizerDialog.getString( STR_IMAGE_RESOLUTION_1 ).getToken( 0, ';', nI4 );
        else if ( mrOptimizerDialog.getString( STR_IMAGE_RESOLUTION_2 ).getToken( 1, ';', nI2 ) == aString )
            aString = mrOptimizerDialog.getString( STR_IMAGE_RESOLUTION_2 ).getToken( 0, ';', nI4 );
        else if ( mrOptimizerDialog.getString( STR_IMAGE_RESOLUTION_3 ).getToken( 1, ';', nI3 ) == aString )
            aString = mrOptimizerDialog.getString( STR_IMAGE_RESOLUTION_3 ).getToken( 0, ';', nI4 );

        mrOptimizerDialog.SetConfigProperty( TK_ImageResolution, Any( aString.toInt32() ) );
    }
}
void TextListenerComboBox0Pg1::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw ( com::sun::star::uno::RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SpinListenerFormattedField0Pg1::up( const SpinEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    double fDouble;
    Any aAny = mrOptimizerDialog.getControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ) );
    if ( aAny >>= fDouble )
    {
        fDouble += 9;
        if ( fDouble > 100 )
            fDouble = 100;
        mrOptimizerDialog.setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( fDouble ) );
        mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)fDouble ) );
    }
}
void SpinListenerFormattedField0Pg1::down( const SpinEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    double fDouble;
    Any aAny = mrOptimizerDialog.getControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ) );
    if ( aAny >>= fDouble )
    {
        fDouble -= 9;
        if ( fDouble < 0 )
            fDouble = 0;
        mrOptimizerDialog.setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( fDouble ) );
        mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)fDouble ) );
    }
}
void SpinListenerFormattedField0Pg1::first( const SpinEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    mrOptimizerDialog.setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( static_cast< double >( 0 ) ) );
    mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)0 ) );
}
void SpinListenerFormattedField0Pg1::last( const SpinEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    mrOptimizerDialog.setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( static_cast< double >( 100 ) ) );
    mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)100 ) );
}
void SpinListenerFormattedField0Pg1::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw ( com::sun::star::uno::RuntimeException )
{
}

// -----------------------------------------------------------------------------

void HelpCloseListener::addCloseListener( const Reference < XCloseListener >& ) throw( RuntimeException )
{
}
void HelpCloseListener::removeCloseListener( const Reference < XCloseListener >& ) throw( RuntimeException )
{
}
void HelpCloseListener::queryClosing( const EventObject&, sal_Bool /* bDeliverOwnership */ )
        throw ( RuntimeException, CloseVetoException )
{
}
void HelpCloseListener::notifyClosing( const EventObject& )
        throw ( RuntimeException )
{
}
void HelpCloseListener::disposing( const EventObject& ) throw ( RuntimeException )
{
    mrXFrame = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
