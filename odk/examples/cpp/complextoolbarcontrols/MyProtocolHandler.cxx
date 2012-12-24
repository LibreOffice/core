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



#include "ListenerHelper.h"
#include "MyProtocolHandler.h"

#include <com/sun/star/awt/MessageBoxButtons.hpp>
#include <com/sun/star/awt/XMessageBoxFactory.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>


using namespace com::sun::star::awt;
using namespace com::sun::star::frame;
using namespace com::sun::star::system;
using namespace com::sun::star::uno;

using com::sun::star::beans::NamedValue;
using com::sun::star::beans::PropertyValue;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::sheet::XSpreadsheetView;
using com::sun::star::text::XTextViewCursorSupplier;
using com::sun::star::util::URL;

ListenerHelper aListenerHelper;

void BaseDispatch::ShowMessageBox( const Reference< XFrame >& rFrame, const ::rtl::OUString& aTitle, const ::rtl::OUString& aMsgText )
{
    if ( !mxToolkit.is() )
        mxToolkit = Reference< XToolkit > ( mxMSF->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit" ))), UNO_QUERY );

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
        // das erste Argument ist immer der Frame, da ein ProtocolHandler den braucht um Zugriff
        // auf den Context zu haben, in dem er aufgerufen wird
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
    if ( xCtrl.is() && !aURL.Protocol.compareToAscii(
        RTL_CONSTASCII_STRINGPARAM( "vnd.demo.complextoolbarcontrols.demoaddon:" ) ) )
    {
        Reference < XTextViewCursorSupplier > xCursor( xCtrl, UNO_QUERY );
        Reference < XSpreadsheetView > xView( xCtrl, UNO_QUERY );
        if ( !xCursor.is() && !xView.is() )
            // ohne ein entsprechendes Dokument funktioniert der Handler nicht
            return xRet;

        if ( aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ImageButtonCmd" ) ) ||
             aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ComboboxCmd" ) ) ||
             aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ToggleDropdownButtonCmd" ) ) ||
             aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DropdownButtonCmd" ) ) ||
             aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SpinfieldCmd" ) ) ||
             aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "EditfieldCmd" ) ) ||
             aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DropdownboxCmd" ) ) )
        {
            xRet = aListenerHelper.GetDispatch( mxFrame, aURL.Path );
            if ( !xRet.is() )
            {
                xRet = xCursor.is() ? (BaseDispatch*) new WriterDispatch( mxMSF, mxFrame ) :
                    (BaseDispatch*) new CalcDispatch( mxMSF, mxFrame );
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
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MYPROTOCOLHANDLER_IMPLEMENTATIONNAME ) );
}

sal_Bool SAL_CALL MyProtocolHandler_supportsService( const ::rtl::OUString& ServiceName )
    throw (RuntimeException)
{
    return (
            ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MYPROTOCOLHANDLER_SERVICENAME ) ) ||
            ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.frame.ProtocolHandler" ) )
           );
}

Sequence< ::rtl::OUString > SAL_CALL MyProtocolHandler_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MYPROTOCOLHANDLER_SERVICENAME ) );
    return aRet;
}

#undef SERVICE_NAME

Reference< XInterface > SAL_CALL MyProtocolHandler_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new MyProtocolHandler( rSMgr );
}

// XServiceInfo
::rtl::OUString SAL_CALL MyProtocolHandler::getImplementationName(  )
    throw (RuntimeException)
{
    return MyProtocolHandler_getImplementationName();
}

sal_Bool SAL_CALL MyProtocolHandler::supportsService( const ::rtl::OUString& rServiceName )
    throw (RuntimeException)
{
    return MyProtocolHandler_supportsService( rServiceName );
}

Sequence< ::rtl::OUString > SAL_CALL MyProtocolHandler::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return MyProtocolHandler_getSupportedServiceNames();
}

void SAL_CALL BaseDispatch::dispatch( const URL& aURL, const Sequence < PropertyValue >& lArgs ) throw (RuntimeException)
{
    /* Its neccessary to hold this object alive, till this method finish.
       May the outside dispatch cache (implemented by the menu/toolbar!)
       forget this instance during de-/activation of frames (focus!).

        E.g. An open db beamer in combination with the My-Dialog
        can force such strange situation :-(
     */
    Reference< XInterface > xSelfHold(static_cast< XDispatch* >(this), UNO_QUERY);

    if ( !aURL.Protocol.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "vnd.demo.complextoolbarcontrols.demoaddon:" ) ) )
    {
        if ( !aURL.Path.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "ImageButtonCmd" ) ) )
        {
            // open the OpenOffice.org web page
            ::rtl::OUString sURL( RTL_CONSTASCII_USTRINGPARAM( "http://www.openoffice.org" ) );
            Reference< XSystemShellExecute > xSystemShellExecute( mxMSF->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SystemShellExecute" ) ) ), UNO_QUERY );
            if ( xSystemShellExecute.is() )
            {
                try
                {
                    xSystemShellExecute->execute( sURL, ::rtl::OUString(), SystemShellExecuteFlags::DEFAULTS );
                }
                catch( Exception& rEx )
                {
                    (void)rEx;
                }
            }
        }
        else if ( !aURL.Path.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "ComboboxCmd" ) ) )
        {
            // remove the text if it's in our list
            Sequence< NamedValue > aRemoveArgs( 1 );
            aRemoveArgs[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ));
            aRemoveArgs[0].Value <<= maComboBoxText;
            SendCommand( aURL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RemoveEntryText" ) ), aRemoveArgs, sal_True );

            // add the new text to the start of the list
            Sequence< NamedValue > aInsertArgs( 2 );
            aInsertArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Pos" ));
            aInsertArgs[0].Value <<= sal_Int32( 0 );
            aInsertArgs[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ));
            aInsertArgs[1].Value <<= maComboBoxText;
            SendCommand( aURL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InsertEntry" ) ), aInsertArgs, sal_True );
        }
        else if ( !aURL.Path.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "ToggleDropdownButtonCmd" ) ) )
        {
            // Retrieve the text argument from the sequence property value
            rtl::OUString aText;
            for ( sal_Int32 i = 0; i < lArgs.getLength(); i++ )
            {
                if ( lArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Text" ) ) )
                {
                    lArgs[i].Value >>= aText;
                    break;
                }
            }

            // create new URL to address the combox box
            URL aCmdURL;
            aCmdURL.Path = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ComboboxCmd" ) );
            aCmdURL.Protocol = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.demo.complextoolbarcontrols.demoaddon:" ) );
            aCmdURL.Complete = aCmdURL.Path + aCmdURL.Protocol;

            // set the selected item as text into the combobox
            Sequence< NamedValue > aArgs( 1 );
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ) );
            aArgs[0].Value <<= aText;
            SendCommand( aCmdURL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SetText" ) ), aArgs, sal_True );
        }
        else if ( !aURL.Path.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "DropdownButtonCmd" ) ) )
        {
            // Retrieve the text argument from the sequence property value
            rtl::OUString aText;
            for ( sal_Int32 i = 0; i < lArgs.getLength(); i++ )
            {
                if ( lArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Text" ) ) )
                {
                    lArgs[i].Value >>= aText;
                    break;
                }
            }

            // just enable this command

            // set enable flag according to selection
            if ( aText.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Button Disabled" ) ))
                mbButtonEnabled = sal_False;
            else
                mbButtonEnabled = sal_True;

            // create new URL to address the image button
            URL aCmdURL;
            aCmdURL.Path = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImageButtonCmd" ) );
            aCmdURL.Protocol = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.demo.complextoolbarcontrols.demoaddon:" ) );
            aCmdURL.Complete = aCmdURL.Path + aCmdURL.Protocol;

            // create and initialize FeatureStateEvent with IsEnabled
            ::com::sun::star::frame::FeatureStateEvent aEvent;
            aEvent.FeatureURL = aCmdURL;
            aEvent.Source = (::com::sun::star::frame::XDispatch*) this;
            aEvent.IsEnabled = mbButtonEnabled;
            aEvent.Requery = sal_False;
            aEvent.State <<= Any();

            // Notify listener about new state
            Reference < XDispatch > xDispatch = aListenerHelper.GetDispatch( mxFrame, aURL.Path );
            aListenerHelper.Notify( mxFrame, aEvent.FeatureURL.Path, aEvent );
        }
        else if ( !aURL.Path.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "SpinfieldCmd" ) ) )
        {
        }
        else if ( !aURL.Path.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "DropdownboxCmd" ) ) )
        {
            // Retrieve the text argument from the sequence property value
            rtl::OUString aText;
            for ( sal_Int32 i = 0; i < lArgs.getLength(); i++ )
            {
                if ( lArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Text" ) ) )
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
    if ( aURL.Protocol.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "vnd.demo.complextoolbarcontrols.demoaddon:" ) ) )
    {
        if ( aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ImageButtonCmd" ) ) )
        {
            // just enable this command
            ::com::sun::star::frame::FeatureStateEvent aEvent;
            aEvent.FeatureURL = aURL;
            aEvent.Source = (::com::sun::star::frame::XDispatch*) this;
            aEvent.IsEnabled = mbButtonEnabled;
            aEvent.Requery = sal_False;
            aEvent.State <<= Any();
            xControl->statusChanged( aEvent );
        }
        else if ( aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ComboboxCmd" ) ) )
        {
            // just enable this command
            ::com::sun::star::frame::FeatureStateEvent aEvent;
            aEvent.FeatureURL = aURL;
            aEvent.Source = (::com::sun::star::frame::XDispatch*) this;
            aEvent.IsEnabled = sal_True;
            aEvent.Requery = sal_False;
            aEvent.State <<= Any();
            xControl->statusChanged( aEvent );
        }
        else if ( aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ToggleDropdownButtonCmd" ) ) )
        {
            // A toggle dropdown box is normally used for a group of commands
            // where the user can select the last issued command easily.
            // E.g. a typical command group would be "Insert shape"
            Sequence< NamedValue > aArgs( 1 );

            // send command to set context menu content
            Sequence< rtl::OUString > aContextMenu( 3 );
            aContextMenu[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Command 1" ) );
            aContextMenu[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Command 2" ) );
            aContextMenu[2] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Command 3" ) );

            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) );
            aArgs[0].Value <<= aContextMenu;
            SendCommandTo( xControl, aURL, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SetList" ) ), aArgs, sal_True );

            // send command to check item on pos=0
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Pos" ));
            aArgs[0].Value <<= sal_Int32( 0 );
            SendCommandTo( xControl, aURL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CheckItemPos" ) ), aArgs, sal_True );
        }
        else if ( aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DropdownButtonCmd" ) ) )
        {
            // A dropdown box is normally used for a group of dependent modes, where
            // the user can only select one. The modes cannot be combined.
            // E.g. a typical group would be left,right,center,block.
            Sequence< NamedValue > aArgs( 1 );

            // send command to set context menu content
            Sequence< rtl::OUString > aContextMenu( 2 );
            aContextMenu[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Button Enabled" ) );
            aContextMenu[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Button Disabled" ) );

            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) );
            aArgs[0].Value <<= aContextMenu;
            SendCommandTo( xControl, aURL, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SetList" ) ), aArgs, sal_True );

            // set position according to enable/disable state of button
            sal_Int32 nPos( mbButtonEnabled ? 0 : 1 );

            // send command to check item on pos=0
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Pos" ) );
            aArgs[0].Value <<= nPos;
            SendCommandTo( xControl, aURL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CheckItemPos" ) ), aArgs, sal_True );
        }
        else if ( aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SpinfieldCmd" ) ) )
        {
            // A spin button
            Sequence< NamedValue > aArgs( 5 );

            // send command to initialize spin button
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Value" ) );
            aArgs[0].Value <<= double( 0.0 );
            aArgs[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UpperLimit" ) );
            aArgs[1].Value <<= double( 10.0 );
            aArgs[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LowerLimit" ) );
            aArgs[2].Value <<= double( 0.0 );
            aArgs[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Step" ) );
            aArgs[3].Value <<= double( 0.1 );
            aArgs[4].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OutputFormat" ) );
            aArgs[4].Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "%.2f cm" ) );

            SendCommandTo( xControl, aURL, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SetValues" ) ), aArgs, sal_True );
        }
        else if ( aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DropdownboxCmd" ) ) )
        {
            // A dropdown box is normally used for a group of commands
            // where the user can select one of a defined set.
            Sequence< NamedValue > aArgs( 1 );

            // send command to set context menu content
            Sequence< rtl::OUString > aList( 10 );
            aList[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "White" ) );
            aList[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Black" ) );
            aList[2] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Red" ) );
            aList[3] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Blue" ) );
            aList[4] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Green" ) );
            aList[5] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Grey" ) );
            aList[6] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Yellow" ) );
            aList[7] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Orange" ) );
            aList[8] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Brown" ) );
            aList[9] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Pink" ) );

            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) );
            aArgs[0].Value <<= aList;
            SendCommandTo( xControl, aURL, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SetList" ) ), aArgs, sal_True );
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
    if ( Event.aURL.Protocol.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "vnd.demo.complextoolbarcontrols.demoaddon:" ) ) )
    {
        if ( Event.aURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ComboboxCmd" ) ) )
        {
            // We get notifications whenever the text inside the combobox has been changed.
            // We store the new text into a member.
            if ( Event.Event.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "TextChanged" ) ) )
            {
                rtl::OUString aNewText;
                sal_Bool      bHasText( sal_False );
                for ( sal_Int32 i = 0; i < Event.aInformation.getLength(); i++ )
                {
                    if ( Event.aInformation[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Text" ) ) )
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

BaseDispatch::BaseDispatch( const Reference< XMultiServiceFactory > &rxMSF,
        const Reference< XFrame >& xFrame, const rtl::OUString& rServiceName )
        : mxMSF( rxMSF )
        , mxFrame( xFrame )
        , msDocService( rServiceName )
        , mbButtonEnabled( sal_True )
{
}


BaseDispatch::~BaseDispatch()
{
    mxFrame.clear();
    mxMSF.clear();
}
