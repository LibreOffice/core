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

package org.openoffice.xmerge.converter.xml.xslt;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.*;

import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.util.Enumeration;
import java.util.Properties;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.converter.xml.xslt.GenericOfficeDocument;
import org.openoffice.xmerge.converter.dom.DOMDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

// Imported TraX classes
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.stream.StreamSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.dom.DOMResult;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerConfigurationException;

//
import org.apache.xalan.serialize.Serializer;
import org.apache.xalan.serialize.SerializerFactory;
import org.apache.xalan.templates.OutputProperties;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

// Imported java classes
import java.io.FileNotFoundException;

/**
 *  <p>Xslt implementation of
 *  org.openoffice.xmerge.DocumentSerializer
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.xslt.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>The <code>serialize</code> method transforms the DOM
 *  document from the given <code>Document</code> object by
 *  means of a supplied Xsl Stylesheet.</p>
 *
 *  @author      Aidan Butler
 */


public final class DocumentSerializerImpl
    implements DocumentSerializer,OfficeConstants {


    /** SXW <code>Document</code> object that this converter processes. */
    private GenericOfficeDocument sxwDoc = null;

    private PluginFactoryImpl pluginFactory = null;

    /**
     *  Constructor.
     *
     *  @param  pf   A <code>PluginFactoryImpl</code>
     *  @param  doc  A SXW <code>Document</code> to be converted.
     */
    public DocumentSerializerImpl(PluginFactoryImpl pf,Document doc) {
    pluginFactory=pf;
        sxwDoc = (GenericOfficeDocument) doc;
    }


    /**
     *  Method to convert a <code>Document</code> with an xsl stylesheet.
     *  It creates a <code>Document</code> object, which is then transformed
     *  with the Xslt processer. A <code>ConvertData </code> object is
     *  constructed and returned.
     *
     *  @returns cd     A <code>ConvertData</code> object.
     *  @throws  ConvertException  If any I/O error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData serialize() throws ConvertException, IOException {
    String docName = sxwDoc.getName();
    org.w3c.dom.Document domDoc = sxwDoc.getContentDOM();
    org.w3c.dom.Document metaDoc = sxwDoc.getMetaDOM();
    org.w3c.dom.Document styleDoc = sxwDoc.getStyleDOM();
    ByteArrayOutputStream baos= new ByteArrayOutputStream();
           ConvertData cd = new ConvertData();
    Node offnode = (Node)domDoc.getDocumentElement();
    if (!(offnode.getNodeName()).equals("office:document")){
        try{
        DocumentBuilderFactory builderFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder= builderFactory.newDocumentBuilder();
        DOMImplementation domImpl = builder.getDOMImplementation();
        DocumentType docType =domImpl.createDocumentType("office:document","-//OpenOffice.org//DTD OfficeDocument 1.0//EN",null);
        org.w3c.dom.Document newDoc = domImpl.createDocument("http://openoffice.org/2000/office","office:document",docType);


        Element rootElement=newDoc.getDocumentElement();
        rootElement.setAttribute("xmlns:office","http://openoffice.org/2000/office");
        rootElement.setAttribute("xmlns:style","http://openoffice.org/2000/style" );
        rootElement.setAttribute("xmlns:text","http://openoffice.org/2000/text");
         rootElement.setAttribute("xmlns:table","http://openoffice.org/2000/table");

        rootElement.setAttribute("xmlns:draw","http://openoffice.org/2000/drawing");
        rootElement.setAttribute("xmlns:fo","http://www.w3.org/1999/XSL/Format" );
        rootElement.setAttribute("xmlns:xlink","http://www.w3.org/1999/xlink" );
        rootElement.setAttribute("xmlns:dc","http://purl.org/dc/elements/1.1/" );
        rootElement.setAttribute("xmlns:meta","http://openoffice.org/2000/meta" );
        rootElement.setAttribute("xmlns:number","http://openoffice.org/2000/datastyle" );
        rootElement.setAttribute("xmlns:svg","http://www.w3.org/2000/svg" );
        rootElement.setAttribute("xmlns:chart","http://openoffice.org/2000/chart" );
        rootElement.setAttribute("xmlns:dr3d","http://openoffice.org/2000/dr3d" );
        rootElement.setAttribute("xmlns:math","http://www.w3.org/1998/Math/MathML" );
        rootElement.setAttribute("xmlns:form","http://openoffice.org/2000/form" );
        rootElement.setAttribute("xmlns:script","http://openoffice.org/2000/script" );
        rootElement.setAttribute("xmlns:config","http://openoffice.org/2001/config" );
        rootElement.setAttribute("office:class","text" );
        rootElement.setAttribute("office:version","1.0");


        NodeList nodeList;
        Node tmpNode;
        Node rootNode = (Node)rootElement;
        if (metaDoc !=null){
        nodeList= metaDoc.getElementsByTagName(TAG_OFFICE_META);
        if (nodeList.getLength()>0){
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
        }
        } if (styleDoc !=null){
        nodeList= styleDoc.getElementsByTagName(TAG_OFFICE_STYLES);
        if (nodeList.getLength()>0){
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
        }
        }if (domDoc !=null){
        nodeList= domDoc.getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
        if (nodeList.getLength()>0){
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
        }
        nodeList= domDoc.getElementsByTagName(TAG_OFFICE_BODY);
        if (nodeList.getLength()>0){
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
        }
        }
        domDoc=newDoc;
        }catch(Exception e){
        System.out.println("\nAn Exception occurred with Xslt Serializer"+e);
        }

    }

    try{
         baos=transform(domDoc);
    }
    catch (Exception e){
        System.out.println("\n Error with Xslt\n");
    }

    String ext = pluginFactory.getDeviceFileExtension();
        DOMDocument resultDomDoc=(DOMDocument)pluginFactory.createDeviceDocument(docName,new ByteArrayInputStream(baos.toByteArray()));
    cd.addDocument (resultDomDoc);
    return cd;
    }



    /*
     * This method performs the sxl transformation on the supplied <code>
     * Document</code> and returns a <code>DOMResult</code> object.
     *
     *  Xslt transformation code
     *
     * @returns baos A <code>ByteArrayOutputStream</code> object containing
     *               the result of the Xslt transformation.
     * @throws TransformerException,TransformerConfigurationException
     *        , FileNotFoundException,IOException
     *
     */


    private ByteArrayOutputStream transform(org.w3c.dom.Document domDoc)
       throws TransformerException,TransformerConfigurationException
          , FileNotFoundException,IOException{
       //System.out.println("\nTransforming...");
       ConverterInfo ci = pluginFactory.getConverterInfo();
       //DOMResult xmlDomResult = new DOMResult();
       ByteArrayOutputStream baos= new ByteArrayOutputStream();
       try{

          DocumentBuilderFactory dFactory = DocumentBuilderFactory.newInstance();
          dFactory.setNamespaceAware(true);

          DocumentBuilder dBuilder = dFactory.newDocumentBuilder();
          String teststr = ci.getXsltSerial();

          teststr= teststr.substring(0,6);
          org.w3c.dom.Document xslDoc=null;
              if ((teststr.equals("http:/"))||(teststr.equals("file:/"))
                                        ||(teststr.equals("jar://"))){
              System.out.println(ci.getXsltSerial());
              xslDoc= dBuilder.parse(ci.getXsltSerial());

          }
          else{
              //System.out.println(ci.getJarName()+"!/"+ci.getXsltSerial());
              xslDoc = dBuilder.parse(
                  "jar:"+ci.getJarName()+"!/"+ci.getXsltSerial());
          }

          DOMSource xslDomSource = new DOMSource(xslDoc);
          DOMSource xmlDomSource = new DOMSource(domDoc);

          //call the tranformer using the XSL, Source and Result.
          TransformerFactory tFactory = TransformerFactory.newInstance();
          Transformer transformer = tFactory.newTransformer(xslDomSource);

          transformer.transform(xmlDomSource, new StreamResult(baos));

            /*
          transformer.transform(xmlDomSource, xmlDomResult); //Removed this impl because the DocType was not being written out

         // Serialize for output to standard out
          Serializer serializer = SerializerFactory.getSerializer
                                   (OutputProperties.getDefaultMethodProperties("xml"));
                 //serializer.setOutputStream(System.out);
          serializer.setOutputStream(baos);
          serializer.asDOMSerializer().serialize(xmlDomResult.getNode());
          //serializer.asDOMSerializer().serialize(xmlDomSource.getNode());


          //System.out.println("\n** Transform Complete ***");
          */
       }
       catch(Exception e){
              System.out.println("An error occured in the transformation : "+e);
       }
       return baos;
     }


}

