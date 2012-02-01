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


import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.DispatchDescriptor;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStatusListener;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.Factory;


public class InspectorAddon {
    /** This class implements the component. At least the interfaces XServiceInfo,
     * XTypeProvider, and XInitialization should be provided by the service.
     */
    public static class InspectorAddonImpl extends WeakBase implements XDispatchProvider, XInitialization, XServiceInfo {
    private static XModel xModel = null;
    org.openoffice.XInstanceInspector xInstInspector = null;
//    Dispatcher oDispatcher = null;
    XFrame m_xFrame = null;

    private static final String[] m_serviceNames = {
        "org.openoffice.InstanceInspectorAddon",
        "com.sun.star.frame.ProtocolHandler" };
                                                     ;
        private XComponentContext m_xContext = null;

    /** Creates a new instance of InspectorAddon */
    public InspectorAddonImpl(XComponentContext _xContext) {
        m_xContext = _xContext;
    }

        public XDispatch queryDispatch( /*IN*/com.sun.star.util.URL aURL, /*IN*/String sTargetFrameName, /*IN*/int iSearchFlags ) {
            XDispatch xRet = null;
            if ( aURL.Protocol.compareTo("org.openoffice.Office.addon.Inspector:") == 0 ) {
                if ( aURL.Path.compareTo( "inspect" ) == 0 ){
                    // Todo: Check if the frame is already administered (use hashtable)
                    xRet = new Dispatcher(m_xFrame);
                }
            }
            return xRet;
        }


        public XDispatch[] queryDispatches( /*IN*/DispatchDescriptor[] seqDescripts ) {
            int nCount = seqDescripts.length;
            XDispatch[] lDispatcher = new XDispatch[nCount];
            for( int i=0; i<nCount; ++i )
                lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );
            return lDispatcher;
        }


        public void initialize( Object[] object ) throws com.sun.star.uno.Exception {
            if ( object.length > 0 ){
                m_xFrame = ( XFrame ) UnoRuntime.queryInterface(XFrame.class, object[ 0 ] );
            }
        }

        public class Dispatcher implements XDispatch{
            private XFrame m_xFrame = null;
            private XModel xModel = null;

            public Dispatcher(XFrame _xFrame){
                m_xFrame = _xFrame;
                if (m_xFrame != null){
                    XController xController = m_xFrame.getController();
                    if (xController != null){
                        xModel = xController.getModel();
                    }
                }
            }

            // XDispatch
            public void dispatch( /*IN*/com.sun.star.util.URL _aURL, /*IN*/com.sun.star.beans.PropertyValue[] aArguments ) {
            try{
                if ( _aURL.Protocol.compareTo("org.openoffice.Office.addon.Inspector:") == 0 ){
                    if ( _aURL.Path.equals("inspect")){
                        Object oUnoInspectObject = xModel;
                        com.sun.star.lang.XMultiComponentFactory xMCF = m_xContext.getServiceManager();
                        if (xInstInspector == null){
                            Object obj= xMCF.createInstanceWithContext("org.openoffice.InstanceInspector", m_xContext);
                            xInstInspector = (org.openoffice.XInstanceInspector)UnoRuntime.queryInterface(org.openoffice.XInstanceInspector.class, obj);
                        }
                        if ((m_xFrame == null) || (xModel == null)){
                            Object oDesktop = xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", m_xContext);
                            m_xFrame = (XFrame) UnoRuntime.queryInterface(XFrame.class, oDesktop);
                            oUnoInspectObject = m_xFrame;
                        }
                        XPropertySet xFramePropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, m_xFrame);
                        String sTitle = (String) xFramePropertySet.getPropertyValue("Title");
                        String[] sTitleList = sTitle.split(" - ");
                        if (sTitleList.length > 0){
                            sTitle = sTitleList[0];
                        }
                        xInstInspector.inspect(oUnoInspectObject, sTitle);
                    }
                }
            } catch( Exception e ) {
                System.err.println( e + e.getMessage());
                e.printStackTrace(System.out);
            }}

            public void addStatusListener( /*IN*/XStatusListener xControl, /*IN*/com.sun.star.util.URL aURL ) {
            }

            public void removeStatusListener( /*IN*/XStatusListener xControl, /*IN*/com.sun.star.util.URL aURL ) {
            }


        }


        public static String[] getServiceNames() {
            return m_serviceNames;
        }

        // Implement the interface XServiceInfo
        /** Get all supported service names.
         * @return Supported service names.
         */
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }

        // Implement the interface XServiceInfo
        /** Test, if the given service will be supported.
         * @param sService Service name.
         * @return Return true, if the service will be supported.
         */
        public boolean supportsService( String sServiceName ) {
            int len = m_serviceNames.length;

            for( int i=0; i < len; i++) {
                if ( sServiceName.equals( m_serviceNames[i] ) )
                    return true;
            }

            return false;
        }

        // Implement the interface XServiceInfo
        /** Get the implementation name of the component.
         * @return Implementation name of the component.
         */
        public String getImplementationName() {
            return InspectorAddonImpl.class.getName();
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
    public static XSingleComponentFactory __getComponentFactory( String sImplName )
    {
        XSingleComponentFactory xFactory = null;
        if ( sImplName.equals( InspectorAddonImpl.class.getName() ) )
            xFactory = Factory.createComponentFactory(InspectorAddonImpl.class, InspectorAddonImpl.getServiceNames());
        return xFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey the registryKey
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return Factory.writeRegistryServiceInfo(InspectorAddonImpl.class.getName(), InspectorAddonImpl.getServiceNames(), regKey);
    }

//    __create( XComponentContext ){
//
//    }
}
