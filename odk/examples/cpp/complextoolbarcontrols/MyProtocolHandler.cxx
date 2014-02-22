/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "ListenerHelper.h"
#include "MyProtocolHandler.h"

#include <com/sun/star/awt/MessageBoxButtons.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XMessageBoxFactory.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star::awt;
using namespace com::sun::star::frame;
using namespace com::sun::star::system;
using namespace com::sun::star::uno;

using com::sun::star::beans::NamedValue;
using com::sun::star::beans::PropertyValue;
using com::sun::star::sheet::XSpreadsheetView;
using com::sun::star::text::XTextViewCursorSupplier;
using com::sun::star::util::URL;

ListenerHelper aListenerHelper;

void BaseDispatch::ShowMessageBox( const Reference< XFrame >& rFrame, const ::rtl::OUString& aTitle, const ::rtl::OUString& aMsgText )
{
    if ( !mxToolkit.is() )
        mxToolkit = Toolkit::create(mxContext);
    Reference< XMessageBoxFactory > xMsgBoxFactory( mxToolkit, UNO_QUERY );
    if ( rFrame.is() && xMsgBoxFactory.is() )
    {
        Reference< XMessageBox > xMsgBox = xMsgBoxFactory->createMessageBox(
            Reference< XWindowPeer >( rFrame->getContainerWindow(), UNO_QUERY ),
            com::sun::star::awt::MessageBoxType_INFOBOX,
            MessageBoxButtons::BUTTONS_OK,
            aTitle,
            aMsgText );

        if ( xMsgBox.is() )
            xMsgBox->execute();
    }
}

void BaseDispatch::SendCommand( const com::sun::star::util::URL& aURL, const ::rtl::OUString& rCommand, const Sequence< NamedValue >& rArgs, sal_Bool bEnabled )
{
    Reference < XDispatch > xDispatch =
            aListenerHelper.GetDispatch( mxFrame, aURL.Path );

    FeatureStateEvent aEvent;

    aEvent.FeatureURL = aURL;
    aEvent.Source     = xDispatch;
    aEvent.IsEnabled  = bEnabled;
    aEvent.Requery    = sal_False;

    ControlCommand aCtrlCmd;
    aCtrlCmd.Command   = rCommand;
    aCtrlCmd.Arguments = rArgs;

    aEvent.State <<= aCtrlCmd;
    aListenerHelper.Notify( mxFrame, aEvent.FeatureURL.Path, aEvent );
}

void BaseDispatch::SendCommandTo( const Reference< XStatusListener >& xControl, const URL& aURL, const ::rtl::OUString& rCommand, const Sequence< NamedValue >& rArgs, sal_Bool bEnabled )
{
    FeatureStateEvent aEvent;

    aEvent.FeatureURL = aURL;
    aEvent.Source     = (::com::sun::star::frame::XDispatch*) this;
    aEvent.IsEnabled  = bEnabled;
    aEvent.Requery    = sal_False;

    ControlCommand aCtrlCmd;
    aCtrlCmd.Command   = rCommand;
    aCtrlCmd.Arguments = rArgs;

    aEvent.State <<= aCtrlCmd;
    xControl->statusChanged( aEvent );
}

void SAL_CALL MyProtocolHandler::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException)
{
    Reference < XFrame > xFrame;
    if ( aArguments.getLength() )
    {
        
        
        aArguments[0] >>= xFrame;
        mxFrame = xFrame;
    }
}

Reference< XDispatch > SAL_CALL MyProtocolHandler::queryDispatch(   const URL& aURL, const ::rtl::OUString& sTargetFrameName, sal_Int32 nSearchFlags )
                throw( RuntimeException )
{
    Reference < XDispatch > xRet;
    if ( !mxFrame.is() )
        return 0;

    Reference < XController > xCtrl = mxFrame->getController();
	if ( xCtrl.is() && aURL.Protocol == "vnd.demo.complextoolbarcontrols.demoaddon:" )
    {
        Reference < XTextViewCursorSupplier > xCursor( xCtrl, UNO_QUERY );
        Reference < XSpreadsheetView > xView( xCtrl, UNO_QUERY );
        if ( !xCursor.is() && !xView.is() )
            
            return xRet;

		if ( aURL.Path == "ImageButtonCmd" ||
			 aURL.Path == "ComboboxCmd" ||
			 aURL.Path == "ToggleDropdownButtonCmd" ||
			 aURL.Path == "DropdownButtonCmd" ||
			 aURL.Path == "SpinfieldCmd" ||
			 aURL.Path == "EditfieldCmd" ||
             aURL.Path == "DropdownboxCmd" )
        {
            xRet = aListenerHelper.GetDispatch( mxFrame, aURL.Path );
            if ( !xRet.is() )
            {
                xRet = xCursor.is() ? (BaseDispatch*) new WriterDispatch( mxContext, mxFrame ) :
                    (BaseDispatch*) new CalcDispatch( mxContext, mxFrame );
                aListenerHelper.AddDispatch( xRet, mxFrame, aURL.Path );
            }
        }
    }

    return xRet;
}

Sequence < Reference< XDispatch > > SAL_CALL MyProtocolHandler::queryDispatches( const Sequence < DispatchDescriptor >& seqDescripts )
            throw( RuntimeException )
{
    sal_Int32 nCount = seqDescripts.getLength();
    Sequence < Reference < XDispatch > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );

    return lDispatcher;
}

::rtl::OUString MyProtocolHandler_getImplementationName ()
    throw (RuntimeException)
{
    return ::rtl::OUString( MYPROTOCOLHANDLER_IMPLEMENTATIONNAME );
}

Sequence< ::rtl::OUString > SAL_CALL MyProtocolHandler_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( MYPROTOCOLHANDLER_SERVICENAME );
    return aRet;
}

#undef SERVICE_NAME

Reference< XInterface > SAL_CALL MyProtocolHandler_createInstance( const Reference< XComponentContext > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new MyProtocolHandler( rSMgr );
}


::rtl::OUString SAL_CALL MyProtocolHandler::getImplementationName(  )
    throw (RuntimeException)
{
    return MyProtocolHandler_getImplementationName();
}

sal_Bool SAL_CALL MyProtocolHandler::supportsService( const ::rtl::OUString& rServiceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< ::rtl::OUString > SAL_CALL MyProtocolHandler::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return MyProtocolHandler_getSupportedServiceNames();
}

void SAL_CALL BaseDispatch::dispatch( const URL& aURL, const Sequence < PropertyValue >& lArgs ) throw (RuntimeException)
{
    /* Its necessary to hold this object alive, till this method finish.
       May the outside dispatch cache (implemented by the menu/toolbar!)
       forget this instance during de-/activation of frames (focus!).

        E.g. An open db beamer in combination with the My-Dialog
        can force such strange situation :-(
     */
    Reference< XInterface > xSelfHold(static_cast< XDispatch* >(this), UNO_QUERY);

    if ( aURL.Protocol == "vnd.demo.complextoolbarcontrols.demoaddon:" )
    {
		if ( aURL.Path == "ImageButtonCmd" )
        {
            
            ::rtl::OUString sURL("http:
            Reference< XSystemShellExecute > xSystemShellExecute(
                SystemShellExecute::create(mxContext) );
            try
            {
                xSystemShellExecute->execute( sURL, ::rtl::OUString(), SystemShellExecuteFlags::URIS_ONLY );
            }
            catch( Exception& rEx )
            {
                (void)rEx;
            }
        }
        else if ( aURL.Path == "ComboboxCmd" )
        {
            
            Sequence< NamedValue > aRemoveArgs( 1 );
            aRemoveArgs[0].Name  = rtl::OUString( "Text" );
            aRemoveArgs[0].Value <<= maComboBoxText;
            SendCommand( aURL, ::rtl::OUString( "RemoveEntryText" ), aRemoveArgs, sal_True );

            
            Sequence< NamedValue > aInsertArgs( 2 );
            aInsertArgs[0].Name = rtl::OUString( "Pos" );
            aInsertArgs[0].Value <<= sal_Int32( 0 );
            aInsertArgs[1].Name = rtl::OUString( "Text" );
            aInsertArgs[1].Value <<= maComboBoxText;
            SendCommand( aURL, ::rtl::OUString("InsertEntry"), aInsertArgs, sal_True );
        }
        else if ( aURL.Path == "InsertEntry" )
        {
            
            rtl::OUString aText;
            for ( sal_Int32 i = 0; i < lArgs.getLength(); i++ )
            {
                if ( lArgs[i].Name == "Text" )
                {
                    lArgs[i].Value >>= aText;
                    break;
                }
            }

            
            URL aCmdURL;
            aCmdURL.Path = "ComboboxCmd";
            aCmdURL.Protocol = "vnd.demo.complextoolbarcontrols.demoaddon:";
            aCmdURL.Complete = aCmdURL.Path + aCmdURL.Protocol;

            
            Sequence< NamedValue > aArgs( 1 );
            aArgs[0].Name = "Text";
            aArgs[0].Value <<= aText;
            SendCommand( aCmdURL, ::rtl::OUString( "SetText" ), aArgs, sal_True );
        }
        else if ( aURL.Path == "DropdownButtonCmd" )
        {
            
            rtl::OUString aText;
            for ( sal_Int32 i = 0; i < lArgs.getLength(); i++ )
            {
                if ( lArgs[i].Name == "Text" )
                {
                    lArgs[i].Value >>= aText;
                    break;
                }
            }

            

            
            if ( aText == "Button Disabled" )
                mbButtonEnabled = sal_False;
            else
                mbButtonEnabled = sal_True;

            
            URL aCmdURL;
            aCmdURL.Path = "Command1";
            aCmdURL.Protocol = "vnd.demo.complextoolbarcontrols.demoaddon:";
            aCmdURL.Complete = aCmdURL.Path + aCmdURL.Protocol;

            
            ::com::sun::star::frame::FeatureStateEvent aEvent;
            aEvent.FeatureURL = aCmdURL;
            aEvent.Source = (::com::sun::star::frame::XDispatch*) this;
            aEvent.IsEnabled = mbButtonEnabled;
            aEvent.Requery = sal_False;
            aEvent.State <<= Any();

            
            Reference < XDispatch > xDispatch = aListenerHelper.GetDispatch( mxFrame, aURL.Path );
            aListenerHelper.Notify( mxFrame, aEvent.FeatureURL.Path, aEvent );
        }
        else if ( aURL.Path == "SpinfieldCmd" )
        {
        }
        else if ( aURL.Path == "DropdownboxCmd" )
        {
            
            rtl::OUString aText;
            for ( sal_Int32 i = 0; i < lArgs.getLength(); i++ )
            {
                if ( lArgs[i].Name == "Text" )
                {
                    lArgs[i].Value >>= aText;
                    break;
                }
            }
            OSL_TRACE( "Dropdownbox control - selected entry text : %s",
                       rtl::OUStringToOString( aText, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
}

void SAL_CALL BaseDispatch::addStatusListener( const Reference< XStatusListener >& xControl, const URL& aURL ) throw (RuntimeException)
{
    if ( aURL.Protocol == "vnd.demo.complextoolbarcontrols.demoaddon:" )
    {
        if ( aURL.Path == "ImageButtonCmd" )
        {
            
            ::com::sun::star::frame::FeatureStateEvent aEvent;
            aEvent.FeatureURL = aURL;
            aEvent.Source = (::com::sun::star::frame::XDispatch*) this;
            aEvent.IsEnabled = mbButtonEnabled;
            aEvent.Requery = sal_False;
            aEvent.State <<= Any();
            xControl->statusChanged( aEvent );
        }
        else if ( aURL.Path == "ComboboxCmd" )
        {
            
            ::com::sun::star::frame::FeatureStateEvent aEvent;
            aEvent.FeatureURL = aURL;
            aEvent.Source = (::com::sun::star::frame::XDispatch*) this;
            aEvent.IsEnabled = sal_True;
            aEvent.Requery = sal_False;
            aEvent.State <<= Any();
            xControl->statusChanged( aEvent );
        }
        else if ( aURL.Path == "ToggleDropdownButtonCmd" )
        {
            
            
            
            Sequence< NamedValue > aArgs( 1 );

            
            Sequence< rtl::OUString > aContextMenu( 3 );
            aContextMenu[0] = "Command 1";
            aContextMenu[1] = "Command 2";
            aContextMenu[2] = "Command 3";

            aArgs[0].Name = "List";
            aArgs[0].Value <<= aContextMenu;
            SendCommandTo( xControl, aURL, rtl::OUString( "SetList" ), aArgs, sal_True );

            
            aArgs[0].Name = rtl::OUString( "Pos" );
            aArgs[0].Value <<= sal_Int32( 0 );
            SendCommandTo( xControl, aURL, ::rtl::OUString( "CheckItemPos" ), aArgs, sal_True );
        }
        else if ( aURL.Path == "DropdownButtonCmd" )
        {
            
            
            
            Sequence< NamedValue > aArgs( 1 );

            
            Sequence< rtl::OUString > aContextMenu( 2 );
            aContextMenu[0] = "Button Enabled";
            aContextMenu[1] = "Button Disabled";

            aArgs[0].Name = "List";
            aArgs[0].Value <<= aContextMenu;
            SendCommandTo( xControl, aURL, rtl::OUString( "SetList" ), aArgs, sal_True );

            
            sal_Int32 nPos( mbButtonEnabled ? 0 : 1 );

            
            aArgs[0].Name = "Pos";
            aArgs[0].Value <<= nPos;
            SendCommandTo( xControl, aURL, ::rtl::OUString( "CheckItemPos" ), aArgs, sal_True );
        }
        else if ( aURL.Path == "SpinfieldCmd" )
        {
            
            Sequence< NamedValue > aArgs( 5 );

            
            aArgs[0].Name = "Value";
            aArgs[0].Value <<= double( 0.0 );
            aArgs[1].Name = "UpperLimit";
            aArgs[1].Value <<= double( 10.0 );
            aArgs[2].Name = "LowerLimit";
            aArgs[2].Value <<= double( 0.0 );
            aArgs[3].Name = "Step";
            aArgs[3].Value <<= double( 0.1 );
            aArgs[4].Name = "OutputFormat";
            aArgs[4].Value <<= rtl::OUString("%.2f cm");

            SendCommandTo( xControl, aURL, rtl::OUString( "SetValues" ), aArgs, sal_True );
        }
        else if ( aURL.Path == "DropdownboxCmd" )
        {
            
            
            Sequence< NamedValue > aArgs( 1 );

            
            Sequence< rtl::OUString > aList( 10 );
            aList[0] = "White";
            aList[1] = "Black";
            aList[2] = "Red";
            aList[3] = "Blue";
            aList[4] = "Green";
            aList[5] = "Grey";
            aList[6] = "Yellow";
            aList[7] = "Orange";
            aList[8] = "Brown";
            aList[9] = "Pink";

            aArgs[0].Name = "List";
            aArgs[0].Value <<= aList;
            SendCommandTo( xControl, aURL, rtl::OUString( "SetList" ), aArgs, sal_True );
        }

        aListenerHelper.AddListener( mxFrame, xControl, aURL.Path );
    }
}

void SAL_CALL BaseDispatch::removeStatusListener( const Reference< XStatusListener >& xControl, const URL& aURL ) throw (RuntimeException)
{
    aListenerHelper.RemoveListener( mxFrame, xControl, aURL.Path );
}

void SAL_CALL BaseDispatch::controlEvent( const ControlEvent& Event ) throw (RuntimeException)
{
    if ( Event.aURL.Protocol == "vnd.demo.complextoolbarcontrols.demoaddon:" )
    {
        if ( Event.aURL.Path == "ComboboxCmd" )
        {
            
            
            if ( Event.Event == "TextChanged" )
            {
                rtl::OUString aNewText;
                sal_Bool      bHasText( sal_False );
                for ( sal_Int32 i = 0; i < Event.aInformation.getLength(); i++ )
                {
                    if ( Event.aInformation[i].Name == "Text" )
                    {
                        bHasText = Event.aInformation[i].Value >>= aNewText;
                        break;
                    }
                }

                if ( bHasText )
                    maComboBoxText = aNewText;
            }
        }
    }
}

BaseDispatch::BaseDispatch( const Reference< XComponentContext > &rxContext,
                            const Reference< XFrame >& xFrame,
                            const ::rtl::OUString& rServiceName )
        : mxContext( rxContext )
        , mxFrame( xFrame )
        , msDocService( rServiceName )
        , mbButtonEnabled( sal_True )
{
}

BaseDispatch::~BaseDispatch()
{
    mxFrame.clear();
    mxContext.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
