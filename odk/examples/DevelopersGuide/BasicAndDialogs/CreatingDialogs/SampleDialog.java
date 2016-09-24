/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

package com.sun.star.comp.sdk.examples;

import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XButton;
import com.sun.star.lang.XComponent;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.task.XJobExecutor;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


/** example of a Java component which creates a dialog at runtime

    This component can be tested by the following StarOffice Basic code:
        Sub Main
            Dim oJobExecutor
            oJobExecutor = CreateUnoService( "com.sun.star.examples.SampleDialog" )
            oJobExecutor.trigger( "execute" )
        End Sub
*/

public class SampleDialog extends WeakBase implements XServiceInfo, XJobExecutor {

    static final String __serviceName = "com.sun.star.examples.SampleDialog";

    private static final String _buttonName = "Button1";
    private static final String _cancelButtonName = "CancelButton";
    private static final String _labelName = "Label1";
    private static final String _labelPrefix = "Number of button clicks: ";

    private XComponentContext _xComponentContext;

    public SampleDialog( XComponentContext xComponentContext ) {
        _xComponentContext = xComponentContext;
    }

    // static component operations
    public static XSingleServiceFactory __getServiceFactory( String implName,
                                                             XMultiServiceFactory multiFactory,
                                                             XRegistryKey regKey ) {
        XSingleServiceFactory xSingleServiceFactory = null;
        if ( implName.equals( SampleDialog.class.getName() ) ) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                SampleDialog.class, SampleDialog.__serviceName, multiFactory, regKey );
        }
        return xSingleServiceFactory;
    }

    public static boolean __writeRegistryServiceInfo( XRegistryKey regKey ) {
        return FactoryHelper.writeRegistryServiceInfo(
            SampleDialog.class.getName(), SampleDialog.__serviceName, regKey );
    }

    // XServiceInfo
    public String getImplementationName(  ) {
        return getClass().getName();
    }

    // XServiceInfo
    public boolean supportsService( /*IN*/String serviceName ) {
        if ( serviceName.equals( __serviceName))
            return true;
        return false;
    }

    // XServiceInfo
    public String[] getSupportedServiceNames(  ) {
        String[] retValue= new String[0];
        retValue[0] = __serviceName;
        return retValue;
    }

    // XJobExecutor
    public void trigger(String sEvent) {
        if ( sEvent.equals( "execute" ) ) {
            try {
                createDialog();
            }
            catch ( Exception e ) {
                throw new com.sun.star.lang.WrappedTargetRuntimeException( e, e.getMessage(), this, e );
            }
        }
    }

    /** method for creating a dialog at runtime
     */
    private void createDialog() throws com.sun.star.uno.Exception {

        // get the service manager from the component context
        XMultiComponentFactory xMultiComponentFactory = _xComponentContext.getServiceManager();

        // create the dialog model and set the properties
        Object dialogModel = xMultiComponentFactory.createInstanceWithContext(
            "com.sun.star.awt.UnoControlDialogModel", _xComponentContext );
        XPropertySet xPSetDialog = ( XPropertySet )UnoRuntime.queryInterface(
            XPropertySet.class, dialogModel );
        xPSetDialog.setPropertyValue( "PositionX", Integer.valueOf( 100 ) );
        xPSetDialog.setPropertyValue( "PositionY", Integer.valueOf( 100 ) );
        xPSetDialog.setPropertyValue( "Width", Integer.valueOf( 150 ) );
        xPSetDialog.setPropertyValue( "Height", Integer.valueOf( 100 ) );
        xPSetDialog.setPropertyValue( "Title", "Runtime Dialog Demo" );

        // get the service manager from the dialog model
        XMultiServiceFactory xMultiServiceFactory = ( XMultiServiceFactory )UnoRuntime.queryInterface(
            XMultiServiceFactory.class, dialogModel );

        // create the button model and set the properties
        Object buttonModel = xMultiServiceFactory.createInstance(
            "com.sun.star.awt.UnoControlButtonModel" );
        XPropertySet xPSetButton = ( XPropertySet )UnoRuntime.queryInterface(
            XPropertySet.class, buttonModel );
        xPSetButton.setPropertyValue( "PositionX", Integer.valueOf( 20 ) );
        xPSetButton.setPropertyValue( "PositionY", Integer.valueOf( 70 ) );
        xPSetButton.setPropertyValue( "Width", Integer.valueOf( 50 ) );
        xPSetButton.setPropertyValue( "Height", Integer.valueOf( 14 ) );
        xPSetButton.setPropertyValue( "Name", _buttonName );
        xPSetButton.setPropertyValue( "TabIndex", Short.valueOf( (short)0 ) );
        xPSetButton.setPropertyValue( "Label", "Click Me" );

        // create the label model and set the properties
        Object labelModel = xMultiServiceFactory.createInstance(
            "com.sun.star.awt.UnoControlFixedTextModel" );
        XPropertySet xPSetLabel = ( XPropertySet )UnoRuntime.queryInterface(
            XPropertySet.class, labelModel );
        xPSetLabel.setPropertyValue( "PositionX", Integer.valueOf( 40 ) );
        xPSetLabel.setPropertyValue( "PositionY", Integer.valueOf( 30 ) );
        xPSetLabel.setPropertyValue( "Width", Integer.valueOf( 100 ) );
        xPSetLabel.setPropertyValue( "Height", Integer.valueOf( 14 ) );
        xPSetLabel.setPropertyValue( "Name", _labelName );
        xPSetLabel.setPropertyValue( "TabIndex", Short.valueOf( (short)1 ) );
        xPSetLabel.setPropertyValue( "Label", _labelPrefix );

        // create a Cancel button model and set the properties
        Object cancelButtonModel = xMultiServiceFactory.createInstance(
            "com.sun.star.awt.UnoControlButtonModel" );
        XPropertySet xPSetCancelButton = ( XPropertySet )UnoRuntime.queryInterface(
            XPropertySet.class, cancelButtonModel );
        xPSetCancelButton.setPropertyValue( "PositionX", Integer.valueOf( 80 ) );
        xPSetCancelButton.setPropertyValue( "PositionY", Integer.valueOf( 70 ) );
        xPSetCancelButton.setPropertyValue( "Width", Integer.valueOf( 50 ) );
        xPSetCancelButton.setPropertyValue( "Height", Integer.valueOf( 14 ) );
        xPSetCancelButton.setPropertyValue( "Name", _cancelButtonName );
        xPSetCancelButton.setPropertyValue( "TabIndex", Short.valueOf( (short)2 ) );
        xPSetCancelButton.setPropertyValue( "PushButtonType", Short.valueOf( (short)2 ) );
        xPSetCancelButton.setPropertyValue( "Label", "Cancel" );

        // insert the control models into the dialog model
        XNameContainer xNameCont = ( XNameContainer )UnoRuntime.queryInterface(
            XNameContainer.class, dialogModel );
        xNameCont.insertByName( _buttonName, buttonModel );
        xNameCont.insertByName( _labelName, labelModel );
        xNameCont.insertByName( _cancelButtonName, cancelButtonModel );

        // create the dialog control and set the model
        Object dialog = xMultiComponentFactory.createInstanceWithContext(
            "com.sun.star.awt.UnoControlDialog", _xComponentContext );
        XControl xControl = ( XControl )UnoRuntime.queryInterface(
            XControl.class, dialog );
        XControlModel xControlModel = ( XControlModel )UnoRuntime.queryInterface(
            XControlModel.class, dialogModel );
        xControl.setModel( xControlModel );

        // add an action listener to the button control
        XControlContainer xControlCont = ( XControlContainer )UnoRuntime.queryInterface(
            XControlContainer.class, dialog );
        Object objectButton = xControlCont.getControl( "Button1" );
        XButton xButton = ( XButton )UnoRuntime.queryInterface(
            XButton.class, objectButton );
        xButton.addActionListener( new ActionListenerImpl( xControlCont ) );

        // create a peer
        Object toolkit = xMultiComponentFactory.createInstanceWithContext(
            "com.sun.star.awt.ExtToolkit", _xComponentContext );
        XToolkit xToolkit = ( XToolkit )UnoRuntime.queryInterface(
            XToolkit.class, toolkit );
        XWindow xWindow = ( XWindow )UnoRuntime.queryInterface(
            XWindow.class, xControl );
        xWindow.setVisible( false );
        xControl.createPeer( xToolkit, null );

        // execute the dialog
        XDialog xDialog = ( XDialog )UnoRuntime.queryInterface(
            XDialog.class, dialog );
        xDialog.execute();

        // dispose the dialog
        XComponent xComponent = ( XComponent )UnoRuntime.queryInterface(
            XComponent.class, dialog );
        xComponent.dispose();
    }

    /** action listener
     */
    public class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        private int _nCounts = 0;

        private XControlContainer _xControlCont;

        public ActionListenerImpl( XControlContainer xControlCont ) {
            _xControlCont = xControlCont;
        }

        // XEventListener
        public void disposing( EventObject eventObject ) {
            _xControlCont = null;
        }

        // XActionListener
        public void actionPerformed( ActionEvent actionEvent ) {

            // increase click counter
            _nCounts++;

            // set label text
            Object label = _xControlCont.getControl( "Label1" );
            XFixedText xLabel = ( XFixedText )UnoRuntime.queryInterface(
                XFixedText.class, label );
            xLabel.setText( _labelPrefix + _nCounts );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
