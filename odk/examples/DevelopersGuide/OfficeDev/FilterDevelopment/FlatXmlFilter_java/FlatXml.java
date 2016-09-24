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
    private final XMultiServiceFactory m_xServiceFactory;
    private XExtendedDocumentHandler m_xHandler;
    private static final boolean m_bPrettyPrint = true;

    private static final String __serviceName = "devguide.officedev.samples.filter.FlatXmlJava";
    private static final String __implName = "FlatXml";
    private static final String[] __supportedServiceNames = {
        "devguide.officedev.samples.filter.FlatXmlJava"
    };

    public FlatXml(XMultiServiceFactory f) {
        m_xServiceFactory = f;
    }

    // --- XTypeProvider ---
    public byte[] getImplementationId() {
        return new byte[0];
    }

    // --- XServiceName ---
    public String getServiceName() {
        return __serviceName;
    }

    // --- XServiceInfo ---
    public boolean supportsService(String sName) {
        for (int i = 0; i < __supportedServiceNames.length; i++) {
            if (__supportedServiceNames[i].equals(sName)) return true;
        }
        return false;
    }
    public String getImplementationName() {
        return this.getClass().getName();
    }
    public String[] getSupportedServiceNames() {
        return __supportedServiceNames;
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
        return typeReturn;
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

            XParser xParser = UnoRuntime.queryInterface(XParser.class , tmpObj);
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
                    m_xHandler = UnoRuntime.queryInterface(XExtendedDocumentHandler.class, tmpObj);
            }
            if (m_xHandler == null)
                return false;

            // Connect the provided output stream to the writer
            XActiveDataSource xADSource = UnoRuntime.queryInterface(
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
