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

import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import  com.sun.star.frame.XStatusListener;
import  com.sun.star.frame.XDispatchProvider;
import  com.sun.star.frame.XDispatch;
import  com.sun.star.frame.XFrame;
import  com.sun.star.frame.DispatchDescriptor;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.WindowAttribute;
import com.sun.star.awt.WindowClass;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.Rectangle;

public class ProtocolHandlerAddon {
    /** This class implements the component. At least the interfaces XServiceInfo,
     * XTypeProvider, and XInitialization should be provided by the service.
     */
    public static class ProtocolHandlerAddonImpl extends WeakBase implements
                                                 XDispatchProvider,
                                                 XDispatch,
                                                 XInitialization,
                                                 XServiceInfo {

        /** The service name, that must be used to get an instance of this service.
         */
        static private final String[] m_serviceNames = { "com.sun.star.frame.ProtocolHandler" };

        /** The component context, that gives access to the service manager and all registered services.
         */
        private XComponentContext m_xCmpCtx;

        /** The toolkit, that we can create UNO dialogs.
         */
        private XToolkit m_xToolkit;

        /** The frame where the addon depends on.
         */
        private XFrame m_xFrame;
        private XStatusListener m_xStatusListener;


        /** The constructor of the inner class has a XMultiServiceFactory parameter.
         * @param xmultiservicefactoryInitialization A special service factory
         * could be introduced while initializing.
         */
        public ProtocolHandlerAddonImpl( XComponentContext xComponentContext ) {
            m_xCmpCtx = xComponentContext;
        }

        /** This method is a member of the interface for initializing an object
         * directly after its creation.
         * @param object This array of arbitrary objects will be passed to the
         * component after its creation.
         * @throws Exception Every exception will not be handled, but will be
         * passed to the caller.
         */
        public void initialize( Object[] object )
            throws com.sun.star.uno.Exception {

            if ( object.length > 0 )
            {
                m_xFrame = ( XFrame ) UnoRuntime.queryInterface(
                    XFrame.class, object[ 0 ] );
            }

            // Create the toolkit to have access to it later
            m_xToolkit = (XToolkit) UnoRuntime.queryInterface(
                XToolkit.class,
                m_xCmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.awt.Toolkit",
                                                                        m_xCmpCtx));
        }

        /** This method returns an array of all supported service names.
         * @return Array of supported service names.
         */
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }

        public static String[] getServiceNames() {
            return m_serviceNames;
        }

        /** This method returns true, if the given service will be
         * supported by the component.
         * @param stringService Service name.
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService( String sService ) {
            int len = m_serviceNames.length;

            for( int i=0; i < len; i++) {
                if ( sService.equals( m_serviceNames[i] ) )
                    return true;
            }

            return false;
        }

        /** Return the class name of the component.
         * @return Class name of the component.
         */
        public String getImplementationName() {
            return ProtocolHandlerAddonImpl.class.getName();
        }

        // XDispatchProvider
        public XDispatch queryDispatch( /*IN*/com.sun.star.util.URL aURL,
                                        /*IN*/String sTargetFrameName,
                                        /*IN*/int iSearchFlags ) {
            XDispatch xRet = null;
            if ( aURL.Protocol.compareTo("org.openoffice.Office.addon.example:") == 0 ) {
                if ( aURL.Path.compareTo( "Function1" ) == 0 )
                    xRet = this;
                if ( aURL.Path.compareTo( "Function2" ) == 0 )
                    xRet = this;
                if ( aURL.Path.compareTo( "Help" ) == 0 )
                    xRet = this;
            }
            return xRet;
        }

        public XDispatch[] queryDispatches( /*IN*/DispatchDescriptor[] seqDescripts ) {
            int nCount = seqDescripts.length;
            XDispatch[] lDispatcher = new XDispatch[nCount];

            for( int i=0; i<nCount; ++i )
                lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL,
                                                seqDescripts[i].FrameName,
                                                seqDescripts[i].SearchFlags );

            return lDispatcher;
        }

        // XDispatch
        public void dispatch( /*IN*/com.sun.star.util.URL aURL,
                              /*IN*/com.sun.star.beans.PropertyValue[] aArguments ) {

            if ( aURL.Protocol.compareTo("org.openoffice.Office.addon.example:") == 0 )
            {
                if ( aURL.Path.compareTo( "Function1" ) == 0 )
                {
                    showMessageBox("SDK DevGuide Add-On example", "Function 1 activated");
                }
                if ( aURL.Path.compareTo( "Function2" ) == 0 )
                {
                    showMessageBox("SDK DevGuide Add-On example", "Function 2 activated");
                }
                if ( aURL.Path.compareTo( "Help" ) == 0 )
                {
                    showMessageBox("About SDK DevGuide Add-On example", "This is the SDK Add-On example");
                }
            }
        }

        public void addStatusListener( /*IN*/XStatusListener xControl,
                                       /*IN*/com.sun.star.util.URL aURL ) {
        }

        public void removeStatusListener( /*IN*/XStatusListener xControl,
                                          /*IN*/com.sun.star.util.URL aURL ) {
        }

        public void showMessageBox(String sTitle, String sMessage) {
            try {
                if ( null != m_xFrame && null != m_xToolkit ) {

                    // describe window properties.
                    WindowDescriptor aDescriptor = new WindowDescriptor();
                    aDescriptor.Type              = WindowClass.MODALTOP;
                    aDescriptor.WindowServiceName = new String( "infobox" );
                    aDescriptor.ParentIndex       = -1;
                    aDescriptor.Parent            = (XWindowPeer)UnoRuntime.queryInterface(
                        XWindowPeer.class, m_xFrame.getContainerWindow());
                    aDescriptor.Bounds            = new Rectangle(0,0,300,200);
                    aDescriptor.WindowAttributes  = WindowAttribute.BORDER |
                        WindowAttribute.MOVEABLE |
                        WindowAttribute.CLOSEABLE;

                    XWindowPeer xPeer = m_xToolkit.createWindow( aDescriptor );
                    if ( null != xPeer ) {
                        XMessageBox xMsgBox = (XMessageBox)UnoRuntime.queryInterface(
                            XMessageBox.class, xPeer);
                        if ( null != xMsgBox )
                        {
                            xMsgBox.setCaptionText( sTitle );
                            xMsgBox.setMessageText( sMessage );
                            xMsgBox.execute();
                        }
                    }
                }
            } catch ( com.sun.star.uno.Exception e) {
                // do your error handling
            }
        }
    }


    /** Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return Returns a <code>XSingleServiceFactory</code> for creating the
     * component.
     * @see com.sun.star.comp.loader.JavaLoader#
     * @param stringImplementationName The implementation name of the component.
     * @param xmultiservicefactory The service manager, who gives access to every
     * known service.
     * @param xregistrykey Makes structural information (except regarding tree
     * structures) of a single
     * registry key accessible.
     */
    public static XSingleComponentFactory __getComponentFactory( String sImplementationName ) {
        XSingleComponentFactory xFactory = null;

        if ( sImplementationName.equals( ProtocolHandlerAddonImpl.class.getName() ) )
            xFactory = Factory.createComponentFactory(ProtocolHandlerAddonImpl.class,
                                                      ProtocolHandlerAddonImpl.getServiceNames());

        return xFactory;
    }

    /** Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>.
     * @return returns true if the operation succeeded
     * @see com.sun.star.comp.loader.JavaLoader#
     * @see com.sun.star.lib.uno.helper.Factory#
     * @param xregistrykey Makes structural information (except regarding tree
     * structures) of a single
     * registry key accessible.
     */
    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.services.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(
//         XRegistryKey xRegistryKey ) {
//         return Factory.writeRegistryServiceInfo(
//             ProtocolHandlerAddonImpl.class.getName(),
//             ProtocolHandlerAddonImpl.getServiceNames(),
//             xRegistryKey );
//   }
}
