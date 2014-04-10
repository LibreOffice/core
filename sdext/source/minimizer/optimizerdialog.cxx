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
#include "fileopendialog.hxx"
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#ifndef _COM_SUN_STAR_UTIL_XCloseBroadcaster_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <osl/time.h>

#include "minimizer.hrc"
#include "helpid.hrc"

#define URL_GRAPHIC_REPO        "private:graphicrepository"
#define IMAGE_ROADMAP           URL_GRAPHIC_REPO "/minimizer/minimizepresi_80.png"
#define IMAGE_ROADMAP_HC        URL_GRAPHIC_REPO "/minimizer/minimizepresi_80_h.png"

// -------------------
// - OPTIMIZERDIALOG -
// -------------------

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::graphic;
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

using ::rtl::OUString;

// -----------------------------------------------------------------------------

void OptimizerDialog::InitDialog()
{
   // setting the dialog properties
    OUString pNames[] = {
        TKGet( TK_Closeable ),
        TKGet( TK_Height ),
        TKGet( TK_HelpURL ),
        TKGet( TK_Moveable ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Title ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( sal_Int32( DIALOG_HEIGHT ) ),
        Any( HID( HID_SDEXT_MINIMIZER_WIZ_DLG ) ),
        Any( sal_True ),
        Any( sal_Int32( 200 ) ),
        Any( sal_Int32( 52 ) ),
        Any( getString( STR_PRESENTATION_MINIMIZER ) ),
        Any( sal_Int32( OD_DIALOG_WIDTH ) ) };

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

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

        sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

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

        rtl::OUString sBitmap(
            isHighContrast() ?
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMAGE_ROADMAP_HC ) ) :
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMAGE_ROADMAP ) ) );

        xPropertySet->setPropertyValue( TKGet( TK_ImageURL ), Any( sBitmap ) );
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

OptimizerDialog::OptimizerDialog(
    const Reference< XComponentContext > &rxContext,
    const Reference< XFrame > &rxFrame,
    const Reference< XWindowPeer >& rxParent )
    : UnoDialog( rxContext, rxParent )
    , ConfigurationAccess( rxContext, NULL )
    , mxFrame( rxFrame )
    , mnCurrentStep( 0 )
    , mnTabIndex( 0 )
{
    OSL_TRACE("OptimizerDialog::OptimizerDialog");
    OSL_ENSURE( mxFrame.is(), "OptimizerDialog: no XFrame!" );
    Reference< XController > xController( mxFrame->getController() );
    mxModel = xController->getModel();
    Reference< XStorable> xStorable( mxModel, UNO_QUERY_THROW );
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
    aStats.InitializeStatusValuesFromDocument( mxModel );
    Sequence< PropertyValue > aStatusSequence( aStats.GetStatusSequence() );
    UpdateStatus( aStatusSequence );

    centerDialog();
}

// -----------------------------------------------------------------------------

OptimizerDialog::~OptimizerDialog()
{
    OSL_TRACE("OptimizerDialog::~OptimizerDialog");
    // not saving configuration if the dialog has been finished via cancel or close window
    if ( mbStatus )
        SaveConfiguration();

    Reference< XComponent > xComponent( mxDialog, UNO_QUERY );
    if ( xComponent.is() )
    {
        OSL_TRACE("OptimizerDialog::~OptimizerDialog - disposing dialog!");
        xComponent->dispose();
    }
}


void SAL_CALL OptimizerDialog::statusChanged(
    const ::com::sun::star::frame::FeatureStateEvent& aState )
throw (::com::sun::star::uno::RuntimeException)
{
    Sequence< PropertyValue > aArguments;
    if ( ( aState.State >>= aArguments ) && aArguments.getLength() )
        UpdateStatus( aArguments );
}

void SAL_CALL OptimizerDialog::disposing(
    const ::com::sun::star::lang::EventObject& /*aSource*/ )
throw (::com::sun::star::uno::RuntimeException)
{}

// -----------------------------------------------------------------------------

sal_Bool OptimizerDialog::execute()
{
    Reference< XItemEventBroadcaster > maRoadmapBroadcaster( mxRoadmapControl, UNO_QUERY_THROW );
    maRoadmapBroadcaster->addItemListener( this );
    UnoDialog::execute();
    UpdateConfiguration();          // taking actual control settings for the configuration
    maRoadmapBroadcaster->removeItemListener( this );
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
                setControlProperty( TKGet( TK_FixedText1Pg4 ), TKGet( TK_Label ), Any( sStatus ) );
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

void OptimizerDialog::itemStateChanged( const ItemEvent& Event )
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
                    SwitchPage( static_cast< sal_Int16 >( Event.ItemId ) );
                }
                break;
                case TK_CheckBox1Pg1 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        SetConfigProperty( TK_RemoveCropArea, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox2Pg1 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        SetConfigProperty( TK_EmbedLinkedGraphics, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox0Pg2 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                    {
                        SetConfigProperty( TK_OLEOptimization, Any( nState != 0 ) );
                        setControlProperty( TKGet( TK_RadioButton0Pg2 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                        setControlProperty( TKGet( TK_RadioButton1Pg2 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                    }
                }
                break;
                case TK_RadioButton0Pg1 :
                {
                    sal_Int16 nInt16 = 0;
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nInt16 )
                    {
                        nInt16 ^= 1;
                        SetConfigProperty( TK_JPEGCompression, Any( nInt16 != 0 ) );
                        setControlProperty( TKGet( TK_FixedText1Pg1 ), TKGet( TK_Enabled ), Any( nInt16 != 0 ) );
                        setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_Enabled ), Any( nInt16 != 0 ) );
                    }
                }
                break;
                case TK_RadioButton1Pg1 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                    {
                        SetConfigProperty( TK_JPEGCompression, Any( nState != 0 ) );
                        setControlProperty( TKGet( TK_FixedText1Pg1 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                        setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                    }
                }
                break;
                case TK_RadioButton0Pg2 :
                {
                    sal_Int16 nInt16;
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nInt16 )
                    {
                        nInt16 ^= 1;
                        SetConfigProperty( TK_OLEOptimizationType, Any( nInt16 ) );
                    }
                }
                break;
                case TK_RadioButton1Pg2 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        SetConfigProperty( TK_OLEOptimizationType, Any( nState ) );
                }
                break;
                case TK_CheckBox0Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        SetConfigProperty( TK_DeleteUnusedMasterPages, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox1Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        SetConfigProperty( TK_DeleteNotesPages, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox2Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        SetConfigProperty( TK_DeleteHiddenSlides, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox3Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        setControlProperty( TKGet( TK_ListBox0Pg3 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox1Pg4 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        setControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_Enabled ), Any( nState != 0 ) );
                }
                break;
                case TK_RadioButton0Pg4 :
                case TK_RadioButton1Pg4 :
                {
                    if ( xPropertySet->getPropertyValue( TKGet( TK_State ) ) >>= nState )
                        SetConfigProperty( TK_SaveAs, Any( eControl == TK_RadioButton1Pg4 ? nState != 0 : nState == 0 ) );
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

// -----------------------------------------------------------------------------

void OptimizerDialog::actionPerformed( const ActionEvent& rEvent )
    throw ( com::sun::star::uno::RuntimeException )
{
    switch( TKGet( rEvent.ActionCommand ) )
    {
        case TK_btnNavBack :    SwitchPage( mnCurrentStep - 1 ); break;
        case TK_btnNavNext :    SwitchPage( mnCurrentStep + 1 ); break;
        case TK_btnNavFinish :
        {
            UpdateConfiguration();

            SwitchPage( ITEM_ID_SUMMARY );
            DisablePage( ITEM_ID_SUMMARY );
            setControlProperty( TKGet( TK_btnNavHelp ), TKGet( TK_Enabled ), Any( sal_False ) );
            setControlProperty( TKGet( TK_btnNavBack ), TKGet( TK_Enabled ), Any( sal_False ) );
            setControlProperty( TKGet( TK_btnNavNext ), TKGet( TK_Enabled ), Any( sal_False ) );
            setControlProperty( TKGet( TK_btnNavFinish ), TKGet( TK_Enabled ), Any( sal_False ) );
            setControlProperty( TKGet( TK_btnNavCancel ), TKGet( TK_Enabled ), Any( sal_False ) );
            setControlProperty( TKGet( TK_FixedText0Pg4 ), TKGet( TK_Enabled ), Any( sal_True ) );

            // check if we have to open the FileDialog
            sal_Bool    bSuccessfullyExecuted = sal_True;
            sal_Int16   nInt16 = 0;
            getControlProperty( TKGet( TK_RadioButton1Pg4 ), TKGet( TK_State ) ) >>= nInt16;
            if ( nInt16 )
            {
                rtl::OUString aSaveAsURL;
                FileOpenDialog aFileOpenDialog( mxContext, Reference< XWindow >( mxParent, UNO_QUERY ) );

                // generating default file name
                Reference< XStorable > xStorable( mxModel, UNO_QUERY );
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
                            aLocation = aLocation.concat( OUString::createFromAscii( ".mini" ) );
                            aFileOpenDialog.setDefaultName( aLocation );
                        }
                    }
                }
                 sal_Bool bDialogExecuted = aFileOpenDialog.execute() == dialogs::ExecutableDialogResults::OK;
                if ( bDialogExecuted )
                {
                    aSaveAsURL = aFileOpenDialog.getURL();
                    SetConfigProperty( TK_SaveAsURL, Any( aSaveAsURL ) );
                    SetConfigProperty( TK_FilterName, Any( aFileOpenDialog.getFilterName() ) );
                }
                if ( !aSaveAsURL.getLength() )
                {
                    // something goes wrong...
                    bSuccessfullyExecuted = sal_False;
                }

                // waiting for 500ms
                if ( mxReschedule.is() )
                {
                    mxReschedule->reschedule();
                    for ( sal_uInt32 i = osl_getGlobalTimer(); ( i + 500 ) > ( osl_getGlobalTimer() ); )
                    mxReschedule->reschedule();
                }
            }
            if ( bSuccessfullyExecuted )
            {   // now check if we have to store a session template
                nInt16 = 0;
                OUString aSettingsName;
                getControlProperty( TKGet( TK_CheckBox1Pg4 ), TKGet( TK_State ) ) >>= nInt16;
                getControlProperty( TKGet( TK_ComboBox0Pg4 ), TKGet( TK_Text ) ) >>= aSettingsName;
                if ( nInt16 && aSettingsName.getLength() )
                {
                    std::vector< OptimizerSettings >::iterator aIter( GetOptimizerSettingsByName( aSettingsName ) );
                    std::vector< OptimizerSettings >& rSettings( GetOptimizerSettings() );
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
                aArgs[ 0 ] <<= mxFrame;

                Reference < XDispatch > xDispatch(
                    mxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.presentation.PresentationOptimizer" ) ),
                        aArgs, mxContext ),
                    UNO_QUERY );

                URL aURL;
                aURL.Protocol = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.com.sun.star.presentation.PresentationOptimizer:" ) );
                aURL.Path = OUString( RTL_CONSTASCII_USTRINGPARAM( "optimize" ) );

                Sequence< PropertyValue > lArguments( 3 );
                lArguments[ 0 ].Name = TKGet( TK_Settings );
                lArguments[ 0 ].Value <<= GetConfigurationSequence();
                lArguments[ 1 ].Name = TKGet( TK_StatusListener );
                lArguments[ 1 ].Value <<= Reference< XStatusListener >( this );
                lArguments[ 2 ].Name = TKGet( TK_ParentWindow );
                lArguments[ 2 ].Value <<= mxDialogWindowPeer;

                if( xDispatch.is() )
                    xDispatch->dispatch( aURL, lArguments );

                endExecute( bSuccessfullyExecuted );
            }
            else
            {
                setControlProperty( TKGet( TK_btnNavHelp ), TKGet( TK_Enabled ), Any( sal_True ) );
                setControlProperty( TKGet( TK_btnNavBack ), TKGet( TK_Enabled ), Any( sal_True ) );
                setControlProperty( TKGet( TK_btnNavNext ), TKGet( TK_Enabled ), Any( sal_False ) );
                setControlProperty( TKGet( TK_btnNavFinish ), TKGet( TK_Enabled ), Any( sal_True ) );
                setControlProperty( TKGet( TK_btnNavCancel ), TKGet( TK_Enabled ), Any( sal_True ) );
                EnablePage( ITEM_ID_SUMMARY );
            }
        }
        break;
        case TK_btnNavCancel :  endExecute( sal_False ); break;
        case TK_Button0Pg0 :    // delete configuration
        {
            OUString aSelectedItem( GetSelectedString( TK_ListBox0Pg0 ) );
            if ( aSelectedItem.getLength() )
            {
                std::vector< OptimizerSettings >::iterator aIter( GetOptimizerSettingsByName( aSelectedItem ) );
                std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
                if ( aIter != rList.end() )
                {
                    rList.erase( aIter );
                    UpdateControlStates();
                }
            }
        }
        break;
        default:
        {
            Reference< XControl > xControl( rEvent.Source, UNO_QUERY );
            if ( xControl.is() )
            {
                OUString aName;
                Reference< XPropertySet > xProps( xControl->getModel(), UNO_QUERY );
                xProps->getPropertyValue( TKGet( TK_Name ) ) >>= aName;
                if ( TKGet( aName ) == TK_ListBox0Pg0 )
                {
                    if ( rEvent.ActionCommand.getLength() )
                    {
                        std::vector< OptimizerSettings >::iterator aIter( GetOptimizerSettingsByName( rEvent.ActionCommand ) );
                        std::vector< OptimizerSettings >& rList( GetOptimizerSettings() );
                        if ( aIter != rList.end() )
                            rList[ 0 ] = *aIter;
                    }
                    UpdateControlStates();
                }
            }
        }
            break;
    }
}

// -----------------------------------------------------------------------------

void OptimizerDialog::textChanged( const TextEvent& rEvent )
    throw ( com::sun::star::uno::RuntimeException )
{
    Reference< XSpinField > xFormattedField( rEvent.Source, UNO_QUERY );
    if ( xFormattedField.is() )
    {
        double fDouble = 0;
        Any aAny = getControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ) );
        if ( aAny >>= fDouble )
            SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)fDouble ) );
        return;
    }

    Reference< XComboBox > xComboBox( rEvent.Source, UNO_QUERY );
    if ( xComboBox.is() )
    {
        rtl::OUString aString;
        Any aAny = getControlProperty( TKGet( TK_ComboBox0Pg1 ), TKGet( TK_Text ) );
        if ( aAny >>= aString )
        {
            sal_Int32 nI0, nI1, nI2, nI3, nI4;
            nI0 = nI1 = nI2 = nI3 = nI4 = 0;

            if ( getString( STR_IMAGE_RESOLUTION_0 ).getToken( 1, ';', nI0 ) == aString )
                aString = getString( STR_IMAGE_RESOLUTION_0 ).getToken( 0, ';', nI4 );
            else if ( getString( STR_IMAGE_RESOLUTION_1 ).getToken( 1, ';', nI1 ) == aString )
                aString = getString( STR_IMAGE_RESOLUTION_1 ).getToken( 0, ';', nI4 );
            else if ( getString( STR_IMAGE_RESOLUTION_2 ).getToken( 1, ';', nI2 ) == aString )
                aString = getString( STR_IMAGE_RESOLUTION_2 ).getToken( 0, ';', nI4 );
            else if ( getString( STR_IMAGE_RESOLUTION_3 ).getToken( 1, ';', nI3 ) == aString )
                aString = getString( STR_IMAGE_RESOLUTION_3 ).getToken( 0, ';', nI4 );

            SetConfigProperty( TK_ImageResolution, Any( aString.toInt32() ) );
        }
    }
}

// -----------------------------------------------------------------------------

void OptimizerDialog::up( const SpinEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    double fDouble;
    Any aAny = getControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ) );
    if ( aAny >>= fDouble )
    {
        fDouble += 9;
        if ( fDouble > 100 )
            fDouble = 100;
        setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( fDouble ) );
        SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)fDouble ) );
    }
}

void OptimizerDialog::down( const SpinEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    double fDouble;
    Any aAny = getControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ) );
    if ( aAny >>= fDouble )
    {
        fDouble -= 9;
        if ( fDouble < 0 )
            fDouble = 0;
        setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( fDouble ) );
        SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)fDouble ) );
    }
}

void OptimizerDialog::first( const SpinEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( static_cast< double >( 0 ) ) );
    SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)0 ) );
}

void OptimizerDialog::last( const SpinEvent& /* rEvent */ )
    throw ( com::sun::star::uno::RuntimeException )
{
    setControlProperty( TKGet( TK_FormattedField0Pg1 ), TKGet( TK_EffectiveValue ), Any( static_cast< double >( 100 ) ) );
    SetConfigProperty( TK_JPEGQuality, Any( (sal_Int32)100 ) );
}

