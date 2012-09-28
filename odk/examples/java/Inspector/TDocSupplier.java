
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

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.sdbc.XRow;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.UniversalContentBroker;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.ucb.XUniversalContentBroker;
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
            exception.printStackTrace(System.err);
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
            exception.printStackTrace(System.err);
        }
        JOptionPane.showMessageDialog(new javax.swing.JFrame(), "The selected Document could not be opened!", "Object Inspector", JOptionPane.ERROR_MESSAGE);
        return "";
        }


        private XRow getXRowOfTDocUrl(String _sTDocUrl, String _sPropertyName){
        try{
            XUniversalContentBroker xUCB = UniversalContentBroker.create( getXComponentContext() );
            XContentIdentifier xId = xUCB.createContentIdentifier(_sTDocUrl);
            XContent xContent = xUCB.queryContent(xId);
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
            exception.printStackTrace(System.err);
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
