/*************************************************************************
 *
 *  $RCSfile: Inspector.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-30 08:10:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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

import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;
import org.openoffice.XInstanceInspector;


/** The purpose of this class is to provide a factory for creating the service
 * (<CODE>__getServiceFactory</CODE>) and writes the information into the given
 * registry key (<CODE>__writeRegistryServiceInfo</CODE>).
 */
public class Inspector{


    /** This class implements the method of the interface XInstanceInspector.
     * Also the class implements the interfaces XServiceInfo, and XTypeProvider.
     */
    static public class _Inspector extends WeakBase implements XInstanceInspector, XServiceInfo{

        static private final String __serviceName = "org.openoffice.InstanceInspector";
        private HashMap aApplicationHashMap = new HashMap();
        private String sTitle = "Object Inspector";
        private Vector aHiddenDocuments = new Vector();
//        private String[] sApplicationDocUrls = new String[]{"private:factory/swriter", "private:factory/scalc", "private:factory/simpress", "private:factory/sdraw", "private:factory/sbase"};
//        private String[] sApplicationDocNames = new String[]{"Text Document", "Spreadsheet", "Presentation", "Drawing", "Database"};
        private XComponentContext m_xComponentContext;
        private HashMap aInspectorPanes = new HashMap();
        private XDialogProvider m_oSwingDialogProvider;
        private TDocSupplier oTDocSupplier;

        /** Creates a new instance of Dialog */
        public _Inspector(XComponentContext _xComponentContext) {
            m_xComponentContext = _xComponentContext;
            aApplicationHashMap.put("private:factory/swriter", "Text Document");
            aApplicationHashMap.put("private:factory/scalc", "Spreadsheet");
            aApplicationHashMap.put("private:factory/simpress", "Presentation");
            aApplicationHashMap.put("private:factory/sdraw", "Drawing");
            aApplicationHashMap.put("private:factory/smath", "Formula");
            m_oSwingDialogProvider = new SwingDialogProvider(this, sTitle);
//            aApplicationHashMap.put("private:factory/sbase", "Database");
        }


        public XComponentContext getXComponentContext(){
            return m_xComponentContext;
        }


        public HashMap getInspectorPages(){
            return aInspectorPanes;
        }


        public void openIdlFileforSelectedNode(){
            InspectorPane oInspectorPane = m_oSwingDialogProvider.getSelectedInspectorPage();
            if (oInspectorPane != null){
                XUnoNode oUnoNode = oInspectorPane.getSelectedNode();
                if (oUnoNode != null){
                    oUnoNode.openIdlDescription();
                }
            }
        }


        public void inspect(java.lang.Object _oUserDefinedObject, String _sTitle) throws com.sun.star.uno.RuntimeException {
        try {
            int nPageIndex = m_oSwingDialogProvider.getInspectorPageCount();
            SwingTreeControlProvider oSwingTreeControlProvider = new SwingTreeControlProvider(m_oSwingDialogProvider);
            InspectorPane oInspectorPane = new InspectorPane(getXComponentContext(), m_oSwingDialogProvider, oSwingTreeControlProvider);
            oInspectorPane.inspect(_oUserDefinedObject, _sTitle);
            getInspectorPages().put(_sTitle, oInspectorPane);
            m_oSwingDialogProvider.show(nPageIndex);
        }catch( Exception exception ) {
            exception.printStackTrace(System.out);
        }}


        public void inspectOpenEmptyDocument(String _sApplicationDocUrl){
            XComponent xComponent = getTDocSupplier().openEmptyDocument(_sApplicationDocUrl);
            String sRootTitle = (String) aApplicationHashMap.get(_sApplicationDocUrl);
            inspect(xComponent, sRootTitle);
            aHiddenDocuments.add(xComponent);
        }

        public void inspectOpenDocument(String _sTDocUrl){
            String sTreeNodeName = getTDocSupplier().getTitleByTDocUrl(_sTDocUrl);
            XModel xTDocModel = getTDocSupplier().getXModelByTDocUrl(_sTDocUrl);
            inspect(xTDocModel, sTreeNodeName);
        }


        public void inspectSelectedNode(){
            InspectorPane oInspectorPane = m_oSwingDialogProvider.getSelectedInspectorPage();
            if (oInspectorPane != null){
                XUnoNode oUnoNode = oInspectorPane.getSelectedNode();
                Object oUnoObject = oUnoNode.getUnoObject();
                if (oUnoObject != null){
                    String sNodeDescription = UnoNode.getNodeDescription(oUnoObject);
                    inspect(oUnoObject, sNodeDescription);
                }
            }
        }


        public void addSourceCodeOfSelectedNode(){
            InspectorPane oInspectorPane = m_oSwingDialogProvider.getSelectedInspectorPage();
            if (oInspectorPane != null){
                oInspectorPane.addSourceCodeOfSelectedNode();
            }
        }


        public void invokeSelectedMethod(){
            InspectorPane oInspectorPane = m_oSwingDialogProvider.getSelectedInspectorPage();
            if (oInspectorPane != null){
                oInspectorPane.invokeSelectedMethodNode();
            }
        }



        private TDocSupplier getTDocSupplier(){
            if (oTDocSupplier == null){
                oTDocSupplier = new TDocSupplier(m_xComponentContext);
            }
            return oTDocSupplier;
        }

        public String[] getTDocUrls(){
            return getTDocSupplier().getTDocUrls();
        }


        public String[] getTDocTitles(String[] _sTDocUrls){
            return getTDocSupplier().getTDocTitles(_sTDocUrls);
        }


        public String[][] getApplicationUrls(){
            Set aSet = aApplicationHashMap.keySet();
            String[][] sReturnList = new String[aSet.size()][];
            int n= 0;
            for ( Iterator i = aSet.iterator(); i.hasNext(); ){
                String[] sSingleApplication = new String[2];
                sSingleApplication[0] = (String) i.next();
                // assign the title in the second index
                sSingleApplication[1] = (String) aApplicationHashMap.get(sSingleApplication[0]);
                sReturnList[n++] = sSingleApplication;
            }
            return sReturnList;
        }


        public void disposeHiddenDocuments(){
            int nHiddenCount = aHiddenDocuments.size();
            if (nHiddenCount > 0){
                for (int i = nHiddenCount - 1; i >= 0; i--){
                    XComponent xComponent = (XComponent) aHiddenDocuments.get(i);
                    if (xComponent != null){
                        try {
                            XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, xComponent);
                            xCloseable.close(true);
                            aHiddenDocuments.remove(i);
                        } catch (CloseVetoException ex) {
                            ex.printStackTrace();
                        }
                    }
                }
            }
        }


        public static String[] getServiceNames() {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
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
            return sServiceName.equals( __serviceName );
        }

        // Implement the interface XServiceInfo
        /** Get the implementation name of the component.
         * @return Implementation name of the component.
         */
        public String getImplementationName() {
            return _Inspector.class.getName();
        }
    }
// end of inner class


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
        if ( sImplName.equals( _Inspector.class.getName() ) )
            xFactory = Factory.createComponentFactory(_Inspector.class, _Inspector.getServiceNames());
        if ( xFactory == null )
            xFactory = InspectorAddon.__getComponentFactory(sImplName);
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
        return (Factory.writeRegistryServiceInfo(_Inspector.class.getName(), _Inspector.getServiceNames(), regKey)
                && InspectorAddon.__writeRegistryServiceInfo(regKey));
    }
}

