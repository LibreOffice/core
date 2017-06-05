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

import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XDialogEventHandler;
import com.sun.star.awt.XDialogProvider2;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;

import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.WindowAttribute;
import com.sun.star.awt.WindowClass;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.Rectangle;

import com.sun.star.test.XTestDialogHandler;

// DialogComponent implements all necessary interfaces self, this is only
// for demonstration. More convenient is to use the implementation WeakBase or
// ComponentBase, see implementation of TestComponentA.
public class DialogComponent {

    public static class _DialogComponent
        implements XTypeProvider, XServiceInfo, XTestDialogHandler, XDialogEventHandler {

        private static final String __serviceName= "com.sun.star.test.TestDialogHandler";


        private final XComponentContext m_xCmpCtx;

        private XFrame m_xFrame;
        private XToolkit m_xToolkit;

        public _DialogComponent(XComponentContext context) {
            m_xCmpCtx= context;

            try {
                // Create the toolkit to have access to it later
                m_xToolkit = UnoRuntime.queryInterface(
                    XToolkit.class,
                    m_xCmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.awt.Toolkit",
                                                                            m_xCmpCtx));
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }

        // XTestDialogHandler
        public String createDialog( String DialogURL, XModel xModel, XFrame xFrame ) {
            m_xFrame = xFrame;

            try {
                XMultiComponentFactory xMCF = m_xCmpCtx.getServiceManager();
                Object obj;

                // If valid we must pass the XModel when creating a DialogProvider object
                if( xModel != null ) {
                    Object[] args = new Object[1];
                    args[0] = xModel;

                    obj = xMCF.createInstanceWithArgumentsAndContext(
                        "com.sun.star.awt.DialogProvider2", args, m_xCmpCtx );
                }
                else {
                    obj = xMCF.createInstanceWithContext(
                        "com.sun.star.awt.DialogProvider2", m_xCmpCtx );
                }

                XDialogProvider2 xDialogProvider = UnoRuntime.queryInterface( XDialogProvider2.class, obj );

                XDialog xDialog = xDialogProvider.createDialogWithHandler( DialogURL, this );
                if( xDialog != null )
                    xDialog.execute();
            }
            catch (Exception e) {
                e.printStackTrace();
            }
            return "Created dialog \"" + DialogURL + "\"";
        }

        public void copyText( XDialog xDialog, Object aEventObject ) {
            XControlContainer xControlContainer = UnoRuntime.queryInterface(
                XControlContainer.class, xDialog );
            String aTextPropertyStr = "Text";
            String aText = "";
            XControl xTextField1Control = xControlContainer.getControl( "TextField1" );
            XControlModel xControlModel1 = xTextField1Control.getModel();
            XPropertySet xPropertySet1 = UnoRuntime.queryInterface(
                XPropertySet.class, xControlModel1 );
            try
            {
                aText = (String)xPropertySet1.getPropertyValue( aTextPropertyStr );
            }
            catch (Exception e) {
                e.printStackTrace();
            }

            XControl xTextField2Control = xControlContainer.getControl( "TextField2" );
            XControlModel xControlModel2 = xTextField2Control.getModel();
            XPropertySet xPropertySet2 = UnoRuntime.queryInterface(
                XPropertySet.class, xControlModel2 );
            try
            {
                xPropertySet2.setPropertyValue( aTextPropertyStr, aText );
            }
            catch (Exception e) {
                e.printStackTrace();
            }

            showMessageBox( "DialogComponent", "copyText() called" );
        }

        public void handleEvent() {
            showMessageBox( "DialogComponent", "handleEvent() called" );
        }

        public void handleEventWithArguments( XDialog xDialog, Object aEventObject ) {
            showMessageBox( "DialogComponent", "handleEventWithArguments() called\n\n" +
                "Event Object = " + aEventObject );
        }

        private static final String aHandlerMethod1 = "doit1";
        private static final String aHandlerMethod2 = "doit2";
        private static final String aHandlerMethod3 = "doit3";

        //XDialogEventHandler
        public boolean callHandlerMethod( /*IN*/XDialog xDialog, /*IN*/Object EventObject, /*IN*/String MethodName ) {
            if ( MethodName.equals( aHandlerMethod1 ) )
            {
                showMessageBox( "DialogComponent", "callHandlerMethod() handled \"" + aHandlerMethod1 + "\"" );
                return true;
            }
            else if ( MethodName.equals( aHandlerMethod2 ) )
            {
                showMessageBox( "DialogComponent", "callHandlerMethod() handled \"" + aHandlerMethod2 + "\"" );
                return true;
            }
            else if ( MethodName.equals( aHandlerMethod3 ) )
            {
                showMessageBox( "DialogComponent", "callHandlerMethod() handled \"" + aHandlerMethod3 + "\"" );
                return true;
            }
            return false;
        }

        public String[] getSupportedMethodNames() {
            String[] retValue= new String[] {
                aHandlerMethod1,
                aHandlerMethod2,
                aHandlerMethod3 };
            return retValue;
        }


        //XTypeProvider
        public com.sun.star.uno.Type[] getTypes(  ) {
            Type[] retValue= new Type[4];
            retValue[0]= new Type( XServiceInfo.class);
            retValue[1]= new Type( XTypeProvider.class);
            retValue[2]= new Type( XTestDialogHandler.class);
            retValue[3]= new Type( XDialogEventHandler.class);
            return retValue;
        }
        //XTypeProvider
        public byte[] getImplementationId(  ) {
            return new byte[0];
        }



        /** This method is a simple helper function to used in the
         * static component initialisation functions as well as in
         * getSupportedServiceNames.
         */
        private static String[] getServiceNames() {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
        }

        //XServiceInfo
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }

        //XServiceInfo
        public boolean supportsService( String sServiceName ) {
            return sServiceName.equals( __serviceName );
        }

        //XServiceInfo
        public String getImplementationName() {
            return  _DialogComponent.class.getName();
        }

        private void showMessageBox(String sTitle, String sMessage) {
            if ( null != m_xFrame && null != m_xToolkit ) {

                // describe window properties.
                WindowDescriptor aDescriptor = new WindowDescriptor();
                aDescriptor.Type              = WindowClass.MODALTOP;
                aDescriptor.WindowServiceName = "infobox";
                aDescriptor.ParentIndex       = -1;
                aDescriptor.Parent            = UnoRuntime.queryInterface(
                    XWindowPeer.class, m_xFrame.getContainerWindow());
                aDescriptor.Bounds            = new Rectangle(0,0,300,200);
                aDescriptor.WindowAttributes  = WindowAttribute.BORDER |
                    WindowAttribute.MOVEABLE |
                    WindowAttribute.CLOSEABLE;

                XWindowPeer xPeer = m_xToolkit.createWindow( aDescriptor );
                if ( null != xPeer ) {
                    XMessageBox xMsgBox = UnoRuntime.queryInterface(
                        XMessageBox.class, xPeer);
                    if ( null != xMsgBox )
                    {
                        xMsgBox.setCaptionText( sTitle );
                        xMsgBox.setMessageText( sMessage );
                        xMsgBox.execute();
                    }
                }
            }
        }
    }

    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(String sImplName)
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( _DialogComponent.class.getName() ) )
            xFactory = Factory.createComponentFactory(_DialogComponent.class,
                                             _DialogComponent.getServiceNames());

        return xFactory;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
