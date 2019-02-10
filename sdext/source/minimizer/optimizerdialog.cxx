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
#include "impoptimizer.hxx"
#include "fileopendialog.hxx"
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <sal/macros.h>
#include <osl/time.h>
#include <vcl/errinf.hxx>
#include <vcl/weld.hxx>
#include <vcl/layout.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <bitmaps.hlst>

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;


void OptimizerDialog::InitDialog()
{
   // setting the dialog properties
    OUString pNames[] = {
        OUString("Closeable"),
        OUString("Height"),
        OUString("Moveable"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Title"),
        OUString("Width") };

    Any pValues[] = {
        Any( true ),
        Any( sal_Int32( DIALOG_HEIGHT ) ),
        Any( true ),
        Any( sal_Int32( 200 ) ),
        Any( sal_Int32( 52 ) ),
        Any( getString( STR_SUN_OPTIMIZATION_WIZARD2 ) ),
        Any( sal_Int32( OD_DIALOG_WIDTH ) ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    mxDialogModelMultiPropertySet->setPropertyValues( aNames, aValues );
}


void OptimizerDialog::InitRoadmap()
{
    try
    {
        OUString pNames[] = {
            OUString("Height"),
            OUString("PositionX"),
            OUString("PositionY"),
            OUString("Step"),
            OUString("TabIndex"),
            OUString("Width") };

        Any pValues[] = {
            Any( sal_Int32( DIALOG_HEIGHT - 26 ) ),
            Any( sal_Int32( 0 ) ),
            Any( sal_Int32( 0 ) ),
            Any( sal_Int32( 0 ) ),
            Any( mnTabIndex++ ),
            Any( sal_Int32( 85 ) ) };

        sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

        Sequence< OUString >   aNames( pNames, nCount );
        Sequence< Any >        aValues( pValues, nCount );

        mxRoadmapControlModel = insertControlModel( "com.sun.star.awt.UnoControlRoadmapModel",
                                                              "rdmNavi", aNames, aValues  );

        Reference< XPropertySet > xPropertySet( mxRoadmapControlModel, UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( "Name", Any( OUString("rdmNavi") ) );
        mxRoadmapControl = mxDialog->getControl( "rdmNavi" );
        InsertRoadmapItem( 0, getString( STR_INTRODUCTION ), ITEM_ID_INTRODUCTION );
        InsertRoadmapItem( 1, getString( STR_SLIDES ), ITEM_ID_SLIDES );
        InsertRoadmapItem( 2, getString( STR_IMAGE_OPTIMIZATION ), ITEM_ID_GRAPHIC_OPTIMIZATION );
        InsertRoadmapItem( 3, getString( STR_OLE_OBJECTS ), ITEM_ID_OLE_OPTIMIZATION );
        InsertRoadmapItem( 4, getString( STR_SUMMARY ), ITEM_ID_SUMMARY );

        OUString const sURL("private:graphicrepository/" BMP_PRESENTATION_MINIMIZER);

        xPropertySet->setPropertyValue( "ImageURL", Any( sURL ) );
        xPropertySet->setPropertyValue( "Activated", Any( true ) );
        xPropertySet->setPropertyValue( "Complete", Any( true ) );
        xPropertySet->setPropertyValue( "CurrentItemID", Any( sal_Int16(ITEM_ID_INTRODUCTION) ) );
        xPropertySet->setPropertyValue( "Text", Any( getString( STR_STEPS ) ) );
    }
    catch( Exception& )
    {
    }
}


void OptimizerDialog::InsertRoadmapItem( const sal_Int32 nIndex, const OUString& rLabel, const sal_Int32 nItemID )
{
    try
    {
        Reference< XSingleServiceFactory > xSFRoadmap( mxRoadmapControlModel, UNO_QUERY_THROW );
        Reference< XIndexContainer > aIndexContainerRoadmap( mxRoadmapControlModel, UNO_QUERY_THROW );
        Reference< XInterface > xRoadmapItem( xSFRoadmap->createInstance(), UNO_QUERY_THROW );
        Reference< XPropertySet > xPropertySet( xRoadmapItem, UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( "Label", Any( rLabel ) );
        xPropertySet->setPropertyValue( "Enabled", Any( true ) );
        xPropertySet->setPropertyValue( "ID", Any( nItemID ) );
        aIndexContainerRoadmap->insertByIndex( nIndex, Any( xRoadmapItem ) );
    }
    catch( Exception& )
    {

    }
}


void OptimizerDialog::UpdateConfiguration()
{
    sal_Int16   nInt16 = 0;
    Any         aAny;

    Sequence< sal_Int16 > aSelectedItems;
    Sequence< OUString > aStringItemList;

    // page0
    aAny = getControlProperty( "ListBox0Pg0", "SelectedItems" );
    if ( aAny >>= aSelectedItems )
    {
        if ( aSelectedItems.getLength() )
        {
            sal_Int16 nSelectedItem = aSelectedItems[ 0 ];
            aAny = getControlProperty( "ListBox0Pg0", "StringItemList" );
            if ( aAny >>= aStringItemList )
            {
                if ( aStringItemList.getLength() > nSelectedItem )
                    SetConfigProperty( TK_Name, Any( aStringItemList[ nSelectedItem ] ) );
            }
        }
    }

    aAny = getControlProperty( "CheckBox3Pg3", "State" );
    if ( !((aAny >>= nInt16) && nInt16) )
        return;

    aAny = getControlProperty( "ListBox0Pg3", "SelectedItems" );
    if ( !(aAny >>= aSelectedItems) )
        return;

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


OptimizerDialog::OptimizerDialog( const Reference< XComponentContext > &rxContext, Reference< XFrame > const & rxFrame, Reference< XDispatch > const & rxStatusDispatcher ) :
    UnoDialog( rxContext, rxFrame ),
    ConfigurationAccess( rxContext ),
    mnCurrentStep( 0 ),
    mnTabIndex( 0 ),
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


OptimizerDialog::~OptimizerDialog()
{
    // not saving configuration if the dialog has been finished via cancel or close window
    if ( mbStatus )
        SaveConfiguration();
}


void OptimizerDialog::execute()
{
    Reference< XItemEventBroadcaster > xRoadmapBroadcaster( mxRoadmapControl, UNO_QUERY_THROW );
    xRoadmapBroadcaster->addItemListener( mxItemListener );
    UnoDialog::execute();
    UpdateConfiguration();          // taking actual control settings for the configuration
    xRoadmapBroadcaster->removeItemListener( mxItemListener );
}


void OptimizerDialog::SwitchPage( sal_Int16 nNewStep )
{
    if ( !(( nNewStep != mnCurrentStep ) && ( nNewStep <= MAX_STEP ) && ( nNewStep >= 0 )) )
        return;

    sal_Int16 nOldStep = mnCurrentStep;
    if ( nNewStep == 0 )
        disableControl( "btnNavBack" );
    else if ( nOldStep == 0 )
        enableControl( "btnNavBack" );

    if ( nNewStep == MAX_STEP )
        disableControl( "btnNavNext" );
    else if ( nOldStep == MAX_STEP )
        enableControl( "btnNavNext" );

    setControlProperty( "rdmNavi", "CurrentItemID", Any( nNewStep ) );

    DeactivatePage( nOldStep );
    UpdateControlStates( nNewStep );

    ActivatePage( nNewStep );
    mnCurrentStep = nNewStep;
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


OUString OptimizerDialog::GetSelectedString( OUString const & token )
{
    OUString aSelectedItem;
    Sequence< sal_Int16 > sSelectedItems;
    Sequence< OUString >  sItemList;

    if ( ( getControlProperty( token, "SelectedItems" ) >>= sSelectedItems ) &&
            ( getControlProperty( token, "StringItemList" ) >>= sItemList ) )
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


void OptimizerDialog::UpdateStatus( const css::uno::Sequence< css::beans::PropertyValue >& rStatus )
{
    if ( !mxReschedule.is() )
        return;

    maStats.InitializeStatusValues( rStatus );
    const Any* pVal( maStats.GetStatusValue( TK_Status ) );
    if ( pVal )
    {
        OUString sStatus;
        if ( *pVal >>= sStatus )
        {
            setControlProperty( "FixedText1Pg4", "Enabled", Any( true ) );
            setControlProperty( "FixedText1Pg4", "Label", Any( getString( TKGet( sStatus ) ) ) );
        }
    }
    pVal = maStats.GetStatusValue( TK_Progress );
    if ( pVal )
    {
        sal_Int32 nProgress = 0;
        if ( *pVal >>= nProgress )
            setControlProperty( "Progress", "ProgressValue", Any( nProgress ) );
    }
    pVal = maStats.GetStatusValue( TK_OpenNewDocument );
    if ( pVal )
        SetConfigProperty( TK_OpenNewDocument, *pVal );

    mxReschedule->reschedule();
}


void ItemListener::itemStateChanged( const ItemEvent& Event )
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
            xPropertySet->getPropertyValue( "Name" ) >>= aControlName;
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
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_RemoveCropArea, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox2Pg1 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_EmbedLinkedGraphics, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox0Pg2 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                    {
                        mrOptimizerDialog.SetConfigProperty( TK_OLEOptimization, Any( nState != 0 ) );
                        mrOptimizerDialog.setControlProperty( "RadioButton0Pg2", "Enabled", Any( nState != 0 ) );
                        mrOptimizerDialog.setControlProperty( "RadioButton1Pg2", "Enabled", Any( nState != 0 ) );
                    }
                }
                break;
                case TK_RadioButton0Pg1 :
                {
                    sal_Int16 nInt16 = 0;
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nInt16 )
                    {
                        nInt16 ^= 1;
                        mrOptimizerDialog.SetConfigProperty( TK_JPEGCompression, Any( nInt16 != 0 ) );
                        mrOptimizerDialog.setControlProperty( "FixedText1Pg1", "Enabled", Any( nInt16 != 0 ) );
                        mrOptimizerDialog.setControlProperty( "FormattedField0Pg1", "Enabled", Any( nInt16 != 0 ) );
                    }
                }
                break;
                case TK_RadioButton1Pg1 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                    {
                        mrOptimizerDialog.SetConfigProperty( TK_JPEGCompression, Any( nState != 0 ) );
                        mrOptimizerDialog.setControlProperty( "FixedText1Pg1", "Enabled", Any( nState != 0 ) );
                        mrOptimizerDialog.setControlProperty( "FormattedField0Pg1", "Enabled", Any( nState != 0 ) );
                    }
                }
                break;
                case TK_RadioButton0Pg2 :
                {
                    sal_Int16 nInt16;
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nInt16 )
                    {
                        nInt16 ^= 1;
                        mrOptimizerDialog.SetConfigProperty( TK_OLEOptimizationType, Any( nInt16 ) );
                    }
                }
                break;
                case TK_RadioButton1Pg2 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_OLEOptimizationType, Any( nState ) );
                }
                break;
                case TK_CheckBox0Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_DeleteUnusedMasterPages, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox1Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_DeleteNotesPages, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox2Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                        mrOptimizerDialog.SetConfigProperty( TK_DeleteHiddenSlides, Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox3Pg3 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                        mrOptimizerDialog.setControlProperty( "ListBox0Pg3", "Enabled", Any( nState != 0 ) );
                }
                break;
                case TK_CheckBox1Pg4 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
                        mrOptimizerDialog.setControlProperty( "ComboBox0Pg4", "Enabled", Any( nState != 0 ) );
                }
                break;
                case TK_RadioButton0Pg4 :
                case TK_RadioButton1Pg4 :
                {
                    if ( xPropertySet->getPropertyValue( "State" ) >>= nState )
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
void ItemListener::disposing( const css::lang::EventObject& /* Source */ )
{
}

void ActionListener::actionPerformed( const ActionEvent& rEvent )
{
    switch( TKGet( rEvent.ActionCommand ) )
    {
        case TK_btnNavBack :    mrOptimizerDialog.SwitchPage( mrOptimizerDialog.mnCurrentStep - 1 ); break;
        case TK_btnNavNext :    mrOptimizerDialog.SwitchPage( mrOptimizerDialog.mnCurrentStep + 1 ); break;
        case TK_btnNavFinish :
        {
            mrOptimizerDialog.UpdateConfiguration();

            mrOptimizerDialog.SwitchPage( ITEM_ID_SUMMARY );
            mrOptimizerDialog.DisablePage( ITEM_ID_SUMMARY );
            mrOptimizerDialog.setControlProperty( "btnNavBack", "Enabled", Any( false ) );
            mrOptimizerDialog.setControlProperty( "btnNavNext", "Enabled", Any( false ) );
            mrOptimizerDialog.setControlProperty( "btnNavFinish", "Enabled", Any( false ) );
            mrOptimizerDialog.setControlProperty( "btnNavCancel", "Enabled", Any( false ) );
            mrOptimizerDialog.setControlProperty( "FixedText0Pg4", "Enabled", Any( true ) );

            // check if we have to open the FileDialog
            bool    bSuccessfullyExecuted = true;
            sal_Int16   nInt16 = 0;
            mrOptimizerDialog.getControlProperty( "RadioButton1Pg4", "State" ) >>= nInt16;
            if ( nInt16 )
            {
                // Duplicate presentation before applying changes
                OUString aSaveAsURL;
                FileOpenDialog aFileOpenDialog( mrOptimizerDialog.GetComponentContext() );

                // generating default file name
                OUString aName;
                Reference< XStorable > xStorable( mrOptimizerDialog.mxController->getModel(), UNO_QUERY );
                if ( xStorable.is() && xStorable->hasLocation() )
                {
                    INetURLObject aURLObj( xStorable->getLocation() );
                    if ( !aURLObj.hasFinalSlash() )
                    {
                        // tdf#105382 uri-decode file name
                        aURLObj.removeExtension(INetURLObject::LAST_SEGMENT, false);
                        aName = aURLObj.getName(INetURLObject::LAST_SEGMENT, false,
                                                INetURLObject::DecodeMechanism::WithCharset);
                    }
                }
                else
                {
                    // If no filename, try to use model title ("Untitled 1" or something like this)
                    Reference<XTitle> xTitle(
                        mrOptimizerDialog.GetFrame()->getController()->getModel(), UNO_QUERY);
                    aName = xTitle->getTitle();
                }

                if (!aName.isEmpty())
                {
                    aName += " ";
                    aName += mrOptimizerDialog.getString(STR_FILENAME_SUFFIX);
                    aFileOpenDialog.setDefaultName(aName);
                }

                if (aFileOpenDialog.execute() == dialogs::ExecutableDialogResults::OK)
                {
                    aSaveAsURL = aFileOpenDialog.getURL();
                    mrOptimizerDialog.SetConfigProperty( TK_SaveAsURL, Any( aSaveAsURL ) );
                    mrOptimizerDialog.SetConfigProperty( TK_FilterName, Any( aFileOpenDialog.getFilterName() ) );
                }
                if ( aSaveAsURL.isEmpty() )
                {
                    // something goes wrong...
                    bSuccessfullyExecuted = false;
                }

                // waiting for 500ms
                if ( mrOptimizerDialog.mxReschedule.is() )
                {
                    mrOptimizerDialog.mxReschedule->reschedule();
                    for ( sal_uInt32 i = osl_getGlobalTimer(); ( i + 500 ) > ( osl_getGlobalTimer() ); )
                    mrOptimizerDialog.mxReschedule->reschedule();
                }
            }
            else
            {
                // Apply changes to current presentation
                Reference<XModifiable> xModifiable(mrOptimizerDialog.mxController->getModel(),
                                                   UNO_QUERY_THROW );
                if ( xModifiable->isModified() )
                {
                    SolarMutexGuard aSolarGuard;
                    std::unique_ptr<weld::MessageDialog> popupDlg(Application::CreateMessageDialog(
                        nullptr, VclMessageType::Question, VclButtonsType::YesNo,
                        mrOptimizerDialog.getString(STR_WARN_UNSAVED_PRESENTATION)));
                    if (popupDlg->run() != RET_YES)
                    {
                        // Selected not "yes" ("no" or dialog was cancelled) so return to previous step
                        mrOptimizerDialog.setControlProperty("btnNavBack", "Enabled",
                                                                  Any(true));
                        mrOptimizerDialog.setControlProperty("btnNavNext", "Enabled", Any(false));
                        mrOptimizerDialog.setControlProperty("btnNavFinish", "Enabled", Any(true));
                        mrOptimizerDialog.setControlProperty("btnNavCancel", "Enabled", Any(true));
                        mrOptimizerDialog.EnablePage(ITEM_ID_SUMMARY);
                        return;
                    }
                }
            }
            if ( bSuccessfullyExecuted )
            {   // now check if we have to store a session template
                nInt16 = 0;
                OUString aSettingsName;
                mrOptimizerDialog.getControlProperty( "CheckBox1Pg4", "State" ) >>= nInt16;
                mrOptimizerDialog.getControlProperty( "ComboBox0Pg4", "Text" ) >>= aSettingsName;
                if ( nInt16 && !aSettingsName.isEmpty() )
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
                URL aURL;
                aURL.Protocol = "vnd.com.sun.star.comp.PPPOptimizer:";
                aURL.Path = "optimize";

                Sequence< PropertyValue > lArguments( 3 );
                lArguments[ 0 ].Name = "Settings";
                lArguments[ 0 ].Value <<= mrOptimizerDialog.GetConfigurationSequence();
                lArguments[ 1 ].Name = "StatusDispatcher";
                lArguments[ 1 ].Value <<= mrOptimizerDialog.GetStatusDispatcher();
                lArguments[ 2 ].Name = "InformationDialog";
                lArguments[ 2 ].Value <<= mrOptimizerDialog.GetFrame();


                ErrCode errorCode;
                try
                {
                    ImpOptimizer aOptimizer(
                        mrOptimizerDialog.GetComponentContext(),
                        mrOptimizerDialog.GetFrame()->getController()->getModel());
                    aOptimizer.Optimize(lArguments);
                }
                catch (css::io::IOException&)
                {
                    // We always receive just ERRCODE_IO_CANTWRITE in case of problems, so no need to bother
                    // about extracting error code from exception text
                    errorCode = ERRCODE_IO_CANTWRITE;
                }
                catch (css::uno::Exception&)
                {
                    // Other general exception
                    errorCode = ERRCODE_IO_GENERAL;
                }

                if (errorCode != ERRCODE_NONE)
                {
                    // Restore wizard controls
                    mrOptimizerDialog.maStats.SetStatusValue(TK_Progress,
                                                             Any(static_cast<sal_Int32>(0)));
                    mrOptimizerDialog.setControlProperty("btnNavBack", "Enabled", Any(true));
                    mrOptimizerDialog.setControlProperty("btnNavNext", "Enabled", Any(false));
                    mrOptimizerDialog.setControlProperty("btnNavFinish", "Enabled", Any(true));
                    mrOptimizerDialog.setControlProperty("btnNavCancel", "Enabled", Any(true));

                    OUString aFileName;
                    mrOptimizerDialog.GetConfigProperty(TK_SaveAsURL) >>= aFileName;
                    SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC, aFileName);
                    ErrorHandler::HandleError(errorCode);
                    break;
                }

                mrOptimizerDialog.endExecute( bSuccessfullyExecuted );
            }
            else
            {
                mrOptimizerDialog.setControlProperty( "btnNavBack", "Enabled", Any( true ) );
                mrOptimizerDialog.setControlProperty( "btnNavNext", "Enabled", Any( false ) );
                mrOptimizerDialog.setControlProperty( "btnNavFinish", "Enabled", Any( true ) );
                mrOptimizerDialog.setControlProperty( "btnNavCancel", "Enabled", Any( true ) );
                mrOptimizerDialog.EnablePage( ITEM_ID_SUMMARY );
            }
        }
        break;
        case TK_btnNavCancel :  mrOptimizerDialog.endExecute( false ); break;
        case TK_Button0Pg0 :    // delete configuration
        {
            OUString aSelectedItem( mrOptimizerDialog.GetSelectedString( "ListBox0Pg0" ) );
            if ( !aSelectedItem.isEmpty() )
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
void ActionListener::disposing( const css::lang::EventObject& /* Source */ )
{
}


void ActionListenerListBox0Pg0::actionPerformed( const ActionEvent& rEvent )
{
    if ( !rEvent.ActionCommand.isEmpty() )
    {
        std::vector< OptimizerSettings >::iterator aIter( mrOptimizerDialog.GetOptimizerSettingsByName( rEvent.ActionCommand ) );
        std::vector< OptimizerSettings >& rList( mrOptimizerDialog.GetOptimizerSettings() );
        if ( aIter != rList.end() )
            rList[ 0 ] = *aIter;
    }
    mrOptimizerDialog.UpdateControlStates();
}
void ActionListenerListBox0Pg0::disposing( const css::lang::EventObject& /* Source */ )
{
}


void TextListenerFormattedField0Pg1::textChanged( const TextEvent& /* rEvent */ )
{
    double fDouble = 0;
    Any aAny = mrOptimizerDialog.getControlProperty( "FormattedField0Pg1", "EffectiveValue" );
    if ( aAny >>= fDouble )
        mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( static_cast<sal_Int32>(fDouble) ) );
}
void TextListenerFormattedField0Pg1::disposing( const css::lang::EventObject& /* Source */ )
{
}

namespace
{

bool lcl_mapResolution(OUString& rResolution, const OUString& rImageResolution)
{
    if (rImageResolution.getToken(1, ';')!=rResolution)
        return false;
    rResolution = rImageResolution.getToken(0, ';');
    return true;
}

}

void TextListenerComboBox0Pg1::textChanged( const TextEvent& /* rEvent */ )
{
    OUString aString;
    Any aAny = mrOptimizerDialog.getControlProperty( "ComboBox0Pg1", "Text" );
    if ( !(aAny >>= aString) )
        return;

    for (int nIR{ STR_IMAGE_RESOLUTION_0 }; nIR <= STR_IMAGE_RESOLUTION_3; ++nIR)
    {
        if (lcl_mapResolution(aString, mrOptimizerDialog.getString(static_cast<PPPOptimizerTokenEnum>(nIR))))
            break;
    }

    mrOptimizerDialog.SetConfigProperty( TK_ImageResolution, Any( aString.toInt32() ) );
}
void TextListenerComboBox0Pg1::disposing( const css::lang::EventObject& /* Source */ )
{
}


void SpinListenerFormattedField0Pg1::up( const SpinEvent& /* rEvent */ )
{
    double fDouble;
    Any aAny = mrOptimizerDialog.getControlProperty( "FormattedField0Pg1", "EffectiveValue" );
    if ( aAny >>= fDouble )
    {
        fDouble += 9;
        if ( fDouble > 100 )
            fDouble = 100;
        mrOptimizerDialog.setControlProperty( "FormattedField0Pg1", "EffectiveValue", Any( fDouble ) );
        mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( static_cast<sal_Int32>(fDouble) ) );
    }
}
void SpinListenerFormattedField0Pg1::down( const SpinEvent& /* rEvent */ )
{
    double fDouble;
    Any aAny = mrOptimizerDialog.getControlProperty( "FormattedField0Pg1", "EffectiveValue" );
    if ( aAny >>= fDouble )
    {
        fDouble -= 9;
        if ( fDouble < 0 )
            fDouble = 0;
        mrOptimizerDialog.setControlProperty( "FormattedField0Pg1", "EffectiveValue", Any( fDouble ) );
        mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( static_cast<sal_Int32>(fDouble) ) );
    }
}
void SpinListenerFormattedField0Pg1::first( const SpinEvent& /* rEvent */ )
{
    mrOptimizerDialog.setControlProperty( "FormattedField0Pg1", "EffectiveValue", Any( static_cast< double >( 0 ) ) );
    mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( sal_Int32(0) ) );
}
void SpinListenerFormattedField0Pg1::last( const SpinEvent& /* rEvent */ )
{
    mrOptimizerDialog.setControlProperty( "FormattedField0Pg1", "EffectiveValue", Any( static_cast< double >( 100 ) ) );
    mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( sal_Int32(100) ) );
}
void SpinListenerFormattedField0Pg1::disposing( const css::lang::EventObject& /* Source */ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
