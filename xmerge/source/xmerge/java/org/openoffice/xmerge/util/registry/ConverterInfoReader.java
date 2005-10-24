/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.util.registry;

import java.io.*;
import java.util.*;
import java.util.jar.*;
import org.xml.sax.*;
import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.net.URL;
import java.net.JarURLConnection;

/**
 *  The <code>ConverterInfoReader</code> pulls a META-INF/converter.xml
 *  file out of a jar file and parses it, providing access to this
 *  information in a <code>Vector</code> of <code>ConverterInfo</code>
 *  objects.
 *
 *  @author  Brian Cameron
 */
public class ConverterInfoReader {

    private final static String TAG_CONVERTER      = "converter";
    private final static String ATTRIB_OFFICE_TYPE = "type";
    private final static String ATTRIB_VERSION     = "version";
    private final static String TAG_NAME           = "converter-display-name";
    private final static String TAG_DESC           = "converter-description";
    private final static String TAG_VENDOR         = "converter-vendor";
    private final static String TAG_CLASS_IMPL     = "converter-class-impl";
    private final static String TAG_TARGET         = "converter-target";
    private final static String ATTRIB_DEVICE_TYPE = "type";
    private final static String TAG_XSLT_DESERIAL     = "converter-xslt-deserialize";
    private final static String TAG_XSLT_SERIAL     = "converter-xslt-serialize";
    private String   jarfilename;
    private Document document;
    private Vector   converterInfoList;


    /**
     *  Constructor.  A jar file is passed in.  The jar file is
     *  parsed and the <code>Vector</code> of <code>ConverterInfo</code>
     *  objects is built.
     *
     *  @param  jar  The URL of the jar file to process.
     *  @param  shouldvalidate Boolean to enable or disable xml validation.
     *
     *  @throws  IOException                   If the jar file cannot
     *                                         be read or if the
     *                                         META-INF/converter.xml
     *                                         can not be read in the
     *                                         jar file.
     *  @throws  ParserConfigurationException  If the DocumentBuilder
     *                                         can not be built.
     *  @throws  org.xml.sax.SAXException      If the converter.xml
     *                                         file can not be parsed.
     *  @throws  RegistryException             If the ConverterFactory
     *                                         implementation of a
     *                                         plug-in cannot be loaded.
     */
    public ConverterInfoReader(String jar,boolean shouldvalidate) throws IOException,
        ParserConfigurationException, org.xml.sax.SAXException,
        RegistryException  {

        InputStream            istream;
        InputSource            isource;
        DocumentBuilderFactory builderFactory;
        DocumentBuilder        builder;
        JarURLConnection       jarConnection;
        JarEntry               jarentry;
        JarFile                jarfile;
        URL                    url;

        converterInfoList = new Vector();
        jarfilename       = jar;

        // Get Jar via URL
        //
        url               = new URL("jar:" + jar + "!/META-INF/converter.xml");
        jarConnection     = (JarURLConnection)url.openConnection();
        jarentry          = jarConnection.getJarEntry();
        jarfile           = jarConnection.getJarFile();

        // Build the InputSource
        //
        istream           = jarfile.getInputStream(jarentry);
        isource           = new InputSource(istream);

        // Get the DOM builder and build the document.
        //
        builderFactory    = DocumentBuilderFactory.newInstance();

    //DTD validation
    if (shouldvalidate){
        System.out.println("Validating xml...");
        builderFactory.setValidating(true);
        }
    //
        builder           = builderFactory.newDocumentBuilder();
    document          = builder.parse(isource);

        // Parse the document.
        //
        parseDocument();
    }


    /**
     *  Loops over the <i>converter</i> <code>Node</code> in the converter.xml
     *  file and processes them.
     *
     *  @throws  RegistryException  If the plug-in associated with a
     *                              specific <i>converter</i> <code>Node</code>
     *                              cannot be loaded.
     */
    private void parseDocument() throws RegistryException {

        Node     converterNode;
        NodeList converterNodes = document.getElementsByTagName(TAG_CONVERTER);

        for (int i=0; i < converterNodes.getLength(); i++) {
            converterNode = converterNodes.item(i);
            if (converterNode.getNodeType() == Node.ELEMENT_NODE) {
                parseConverterNode((Element)converterNode);
            }
        }
    }


    /**
     *  Parses a <i>converter</i> node, pulling the information out of
     *  the <code>Node</code> and placing it in a <code>ConverterInfo</code>
     *  object, and adds that object to a <code>Vector</code> of
     *  <code>ConverterInfo</code> objects.
     *
     *  @param  e  The <code>Element</code> corresponding to the
     *             <i>converter</i> XML tag.
     *
     *
     *  @throws  RegistryException  If the plug-in cannot be loaded.
     */
    private void parseConverterNode(Element e) throws RegistryException {

        Element detailElement;
        Node    detailNode;
        String  elementTagName;
        String  officeMime  = null;
        Vector  deviceMime  = new Vector();
        String  name        = null;
        String  desc        = null;
        String  version     = null;
        String  vendor      = null;
        String  classImpl   = null;
    String  xsltSerial  = null;
    String  xsltDeserial= null;
        String  temp;

        temp = e.getAttribute(ATTRIB_OFFICE_TYPE);
        if (temp.length() != 0) {
           officeMime = temp;
        }

        temp = e.getAttribute(ATTRIB_VERSION);
        if (temp.length() != 0) {
           version = temp;
        }

        NodeList detailNodes = e.getChildNodes();
        for (int i=0; i < detailNodes.getLength(); i++) {

            detailNode = detailNodes.item(i);
            if (detailNode.getNodeType() == Node.ELEMENT_NODE) {

                detailElement  = (Element)detailNode;
                elementTagName = detailElement.getTagName();

                if (TAG_NAME.equalsIgnoreCase(elementTagName)) {
                    name = getTextValue(detailElement);
                } else if (TAG_DESC.equalsIgnoreCase(elementTagName)) {
                    desc = getTextValue(detailElement);
                } else if (TAG_VENDOR.equalsIgnoreCase(elementTagName)) {
                    vendor = getTextValue(detailElement);
        } else if (TAG_XSLT_SERIAL.equalsIgnoreCase(elementTagName)) {
                    xsltSerial = getTextValue(detailElement);
                } else if (TAG_XSLT_DESERIAL.equalsIgnoreCase(elementTagName)) {
                    xsltDeserial = getTextValue(detailElement);
                } else if (TAG_CLASS_IMPL.equalsIgnoreCase(elementTagName)) {
                    classImpl = getTextValue(detailElement);
                } else if (TAG_TARGET.equalsIgnoreCase(elementTagName)) {

                    temp = detailElement.getAttribute(ATTRIB_DEVICE_TYPE);
                    if (temp.length() != 0) {
                        deviceMime.add(temp);
                    }
                }
            }
        }
    ConverterInfo converterInfo;
    if ((xsltSerial==null) || (xsltDeserial==null)){
        converterInfo = new ConverterInfo(jarfilename,
                officeMime, deviceMime, name,
                desc, version, vendor,classImpl);
    }
    else{
        converterInfo = new ConverterInfo(jarfilename,
                officeMime, deviceMime, name,
                desc, version, vendor,classImpl,
                xsltSerial,xsltDeserial);
        }
        /*ConverterInfo converterInfo = new ConverterInfo(jarfilename,
            officeMime, deviceMime, name, desc, version, vendor,
            classImpl);*/
        converterInfoList.add(converterInfo);
    }


    /**
     *  Helper function to get the text value of an
     *  <code>Element</code>.
     *
     *  @param  e  The <code>Element</code> to process.
     *
     *  @return  The text value of the <code>Element</code>.
     */
    private String getTextValue(Element e) {

        NodeList tempNodes = e.getChildNodes();
        String   text      = null;
        Node     tempNode;

        for (int j=0; j < tempNodes.getLength(); j++) {
            tempNode = tempNodes.item(j);
            if (tempNode.getNodeType() == Node.TEXT_NODE) {
               text = tempNode.getNodeValue().trim();
               break;
            }
        }

        return text;
    }


    /**
     *  Returns an <code>Enumeration</code> of <code>ConverterInfo</code>
     *  objects.
     *
     *  @return  An <code>Enumeration</code> of <code>ConverterInfo</code>
     *           objects.
     */
    public Enumeration getConverterInfoEnumeration() {
       return (converterInfoList.elements());
    }
}

