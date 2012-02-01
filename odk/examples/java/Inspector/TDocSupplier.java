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




import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.sdbc.XRow;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import javax.swing.JOptionPane;


public class TDocSupplier {
    private XMultiComponentFactory m_xMultiComponentFactory;
    private XComponentContext m_xComponentContext;


    /** Creates a new instance of TDocSupplier */
    public TDocSupplier(XComponentContext _xComponentContext) {
        m_xComponentContext = _xComponentContext;
        m_xMultiComponentFactory = m_xComponentContext.getServiceManager();
    }


    protected XComponentContext getXComponentContext(){
        return m_xComponentContext;
    }


    protected XMultiComponentFactory getXMultiComponentFactory(){
        return m_xMultiComponentFactory;
    }

        public XModel getXModelByTDocUrl(String _sTDocUrl){
        try{
            XRow xRow = getXRowOfTDocUrl(_sTDocUrl, "DocumentModel");
            if (xRow != null){
                Object oModel = xRow.getObject(1, null);
                XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, oModel);
                return xModel;
            }
        }catch(Exception exception){
            exception.printStackTrace(System.out);
        }
        JOptionPane.showMessageDialog(new javax.swing.JFrame(), "The selected Document could not be opened!", "Object Inspector", JOptionPane.ERROR_MESSAGE);
        return null;
        }


        public String getTitleByTDocUrl(String _sTDocUrl){
        try{
            XRow xRow = this.getXRowOfTDocUrl(_sTDocUrl, "Title");
            if (xRow != null){
                return xRow.getString(1);
            }
        }catch(Exception exception){
            exception.printStackTrace(System.out);
        }
        JOptionPane.showMessageDialog(new javax.swing.JFrame(), "The selected Document could not be opened!", "Object Inspector", JOptionPane.ERROR_MESSAGE);
        return "";
        }


        private XRow getXRowOfTDocUrl(String _sTDocUrl, String _sPropertyName){
        try{
            String[] keys = new String[2];
            keys[ 0 ] = "Local";
            keys[ 1 ] = "Office";
            Object oUCB = getXMultiComponentFactory().createInstanceWithArgumentsAndContext( "com.sun.star.ucb.UniversalContentBroker", keys, getXComponentContext() );
            XContentIdentifierFactory xIdFactory = (XContentIdentifierFactory)UnoRuntime.queryInterface(XContentIdentifierFactory.class, oUCB);
            XContentProvider xProvider = (XContentProvider)UnoRuntime.queryInterface(XContentProvider.class, oUCB);
            XContentIdentifier xId = xIdFactory.createContentIdentifier(_sTDocUrl);
            XContent xContent = xProvider.queryContent(xId);
            XCommandProcessor xCmdProcessor = (XCommandProcessor) UnoRuntime.queryInterface(XCommandProcessor.class, xContent);
            Property aProperty = new Property();
            aProperty.Name = _sPropertyName; // "DocumentModel";                //DocumentModel
            Command aCommand  = new Command();
            aCommand.Name = "getPropertyValues";
            aCommand.Handle = -1; // not available
            aCommand.Argument = new Property[]{aProperty};
            Object oAny = xCmdProcessor.execute(aCommand, 0, null);
            XRow xRow = (XRow) UnoRuntime.queryInterface(XRow.class, oAny);
            return xRow;
        }catch(Exception exception){
            exception.printStackTrace(System.out);
            return null;
        }}


        protected String[] getTDocTitles(String[] _sTDocUrls){
            String[] sTitles = new String[_sTDocUrls.length];
            for (int i = 0; i < _sTDocUrls.length; i++){
                sTitles[i] = getTitleByTDocUrl(_sTDocUrls[i]);
            }
            return sTitles;
        }


        protected String[] getTDocUrls(){
        try{
            Object oSimpleFileAccess = getXMultiComponentFactory().createInstanceWithContext("com.sun.star.ucb.SimpleFileAccess", getXComponentContext());
            XSimpleFileAccess xSimpleFileAccess =  (XSimpleFileAccess) UnoRuntime.queryInterface(XSimpleFileAccess.class, oSimpleFileAccess);
            String[] sContent = xSimpleFileAccess.getFolderContents("vnd.sun.star.tdoc:/", false);
            return sContent;
        } catch( Exception e ) {
            System.err.println( e );
            return new String[]{};
        }}


    public XComponent openEmptyDocument(String _sUrl){
    try{
        PropertyValue[] aPropertyValues = new PropertyValue[1];
        aPropertyValues[0] = new PropertyValue();
        aPropertyValues[0].Name = "Hidden";
        aPropertyValues[0].Value = Boolean.TRUE;
        Object oDesktop = getXMultiComponentFactory().createInstanceWithContext("com.sun.star.frame.Desktop", getXComponentContext());
        XComponentLoader xCL = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, oDesktop);
        return xCL.loadComponentFromURL(_sUrl, "_default", 0, aPropertyValues);
    }
    catch( Exception exception ) {
        System.err.println( exception );
        return null;
    }}

}
