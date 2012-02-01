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



import java.io.*;
import java.util.*;
import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.registry.XRegistryKey;

import com.sun.star.xml.*;
import com.sun.star.xml.sax.*;

import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XActiveDataSource;



public class FlatXml implements XImportFilter, XExportFilter, XServiceName,
        XServiceInfo, XDocumentHandler, XTypeProvider
{

    /*
     * private data members
     */
    private XMultiServiceFactory m_xServiceFactory;
    private XExtendedDocumentHandler m_xHandler;
    private boolean m_bPrettyPrint = true;

    static private final String __serviceName = "devguide.officedev.samples.filter.FlatXmlJava";
    static private final String __implName = "FlatXml";
    static private final String[] __supportedServiceNames = {
        "devguide.officedev.samples.filter.FlatXmlJava"
    };

    public FlatXml(XMultiServiceFactory f) {
        m_xServiceFactory = f;
    }

    // --- XTypeProvider ---
    public byte[] getImplementationId() {
        return Integer.toString(this.hashCode()).getBytes();
    }

    // --- XServiceName ---
    public String getServiceName() {
        return( __serviceName );
    }

    // --- XServiceInfo ---
    public boolean supportsService(String sName) {
        for (int i = 0; i < __supportedServiceNames.length; i++) {
            if (__supportedServiceNames[i].equals(sName)) return true;
        }
        return false;
    }
    public String getImplementationName() {
        return( this.getClass().getName() );
    }
    public String[] getSupportedServiceNames() {
        return( __supportedServiceNames );
    }

    public com.sun.star.uno.Type[] getTypes() {
        Type[] typeReturn = {};
        try {
            typeReturn = new Type[] {
                new Type( XTypeProvider.class ),
                new Type( XExportFilter.class ),
                new Type( XImportFilter.class ),
                new Type( XServiceName.class ),
                new Type( XServiceInfo.class )
            };
        } catch( java.lang.Exception exception ) {
            return null;
        }
        return( typeReturn );
    }

    public boolean importer(PropertyValue[] aSourceData, XDocumentHandler xDocHandler, String[] msUserData)
        throws com.sun.star.uno.RuntimeException, com.sun.star.lang.IllegalArgumentException
    {
        String sName = null;
        String sFileName = null;
        String sURL = null;
        com.sun.star.io.XInputStream xin = null;

        try {

            for  (int  i = 0 ; i < aSourceData.length; i++)
            {
                sName = aSourceData[i].Name;
                if (sName.equals("InputStream"))
                    xin = (XInputStream)AnyConverter.toObject(XInputStream.class, aSourceData[i].Value);
                if (sName.equals("URL"))
                    sURL=(String)AnyConverter.toObject(String.class, aSourceData[i].Value);
                if (sName.equals("FileName"))
                    sFileName=(String)AnyConverter.toObject(String.class, aSourceData[i].Value);
            }

            Object tmpObj=m_xServiceFactory.createInstance("com.sun.star.xml.sax.Parser");
            if (tmpObj == null) return false;

            XParser xParser = (XParser)UnoRuntime.queryInterface(XParser.class , tmpObj);
            if (xParser == null) return false;

            InputSource aInput = new InputSource();
            aInput.sSystemId = sURL;
            aInput.aInputStream =xin;
            xParser.setDocumentHandler ( xDocHandler );
            xParser.parseStream ( aInput );
        } catch (com.sun.star.uno.Exception e){
            e.printStackTrace();
            return false;
        }

        // done...
        return true;
    }

    public boolean exporter(PropertyValue[] aSourceData, String[] msUserData)
        throws com.sun.star.uno.RuntimeException, com.sun.star.lang.IllegalArgumentException
    {
        try {
            String sURL = null;
            String sName = null;
            XOutputStream xos = null;

            // get interesting values from sourceData
            for  (int  i = 0 ; i < aSourceData.length; i++)
            {
                sName = aSourceData[i].Name;
                if (sName.equals("OutputStream"))
                    xos = (XOutputStream)AnyConverter.toObject(XOutputStream.class, aSourceData[i].Value);
                if (sName.equals("URL"))
                    sURL=(String)AnyConverter.toObject(String.class, aSourceData[i].Value);
            }

            // prepare the XML writer
            Object tmpObj = null;
            if (m_xHandler == null)
            {
                tmpObj = m_xServiceFactory.createInstance("com.sun.star.xml.sax.Writer");
                if (tmpObj != null)
                    m_xHandler = (XExtendedDocumentHandler)UnoRuntime.queryInterface(XExtendedDocumentHandler.class, tmpObj);
            }
            if (m_xHandler == null)
                return false;

            // Connect the provided output stream to the writer
            XActiveDataSource xADSource = (XActiveDataSource)UnoRuntime.queryInterface(
                    XActiveDataSource.class, m_xHandler);

            if (xADSource != null && xos != null)
                xADSource.setOutputStream(xos);
            else
                return false;
        } catch (com.sun.star.uno.Exception e){
            return false;
        }

        // done ...
        return true;
    }

    public void  startDocument ()
        throws com.sun.star.xml.sax.SAXException
    {
        m_xHandler.startDocument();
    }

    public void endDocument()
        throws com.sun.star.xml.sax.SAXException
    {
        m_xHandler.endDocument();
    }

    public void startElement (String str, com.sun.star.xml.sax.XAttributeList xattribs)
        throws com.sun.star.xml.sax.SAXException
    {
        m_xHandler.startElement(str, xattribs);
    }

    public void endElement(String str)
        throws com.sun.star.xml.sax.SAXException
    {
        m_xHandler.endElement(str);
    }

    public void characters(String str)
        throws com.sun.star.xml.sax.SAXException
    {
        m_xHandler.characters(str);
    }

    public void ignorableWhitespace(String str)
        throws com.sun.star.xml.sax.SAXException
    {
        if (!m_bPrettyPrint) return;
        else m_xHandler.ignorableWhitespace(str);
    }

    public void processingInstruction(String aTarget, String aData)
        throws com.sun.star.xml.sax.SAXException
    {
        m_xHandler.processingInstruction(aTarget, aData);
    }

    public void setDocumentLocator(XLocator xLocator)
        throws com.sun.star.xml.sax.SAXException
    {
        m_xHandler.setDocumentLocator(xLocator);
    }

    // ------------------------------------------------------------
    // component management

    public static XSingleServiceFactory __getServiceFactory(String implName,
            XMultiServiceFactory multiFactory, XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;
        if (implName.equals(__implName) ) {
            try {
                xSingleServiceFactory = FactoryHelper.getServiceFactory(
                    Class.forName(implName), __serviceName, multiFactory, regKey);
            } catch (java.lang.ClassNotFoundException e) {
                return null;
            }
        }
        return xSingleServiceFactory;
    }

    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.services.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(XRegistryKey regKey)
//     {
//         return FactoryHelper.writeRegistryServiceInfo(__implName,
//             __serviceName, regKey);
//     }

}
