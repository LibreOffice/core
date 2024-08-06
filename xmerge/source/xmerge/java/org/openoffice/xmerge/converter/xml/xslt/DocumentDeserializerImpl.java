/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge.converter.xml.xslt;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Iterator;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.URIResolver;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import javax.xml.XMLConstants;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.converter.dom.DOMDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.registry.ConverterInfo;

/**
 * Xslt implementation of {@code org.openoffice.xmerge.DocumentSerializer}
 * for the {@link org.openoffice.xmerge.converter.xml.xslt.PluginFactoryImpl
 * PluginFactoryImpl}.
 *
 * <p>The {@code serialize} method transforms the DOM document from the given
 * {@code Document} object by means of a supplied Xsl Stylesheet.</p>
 */
public final class DocumentDeserializerImpl
    implements  DocumentDeserializer,URIResolver {

    /** A {@code ConvertData} object assigned to this object. */
    private final ConvertData cd;
    private final PluginFactoryImpl pluginFactory;

    private static DocumentBuilderFactory makeFactory() {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();

        try {
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
        } catch (ParserConfigurationException e) {
            Debug.log(Debug.ERROR, "Exception when calling setFeature: ", e);
        }

        try {
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
        } catch (ParserConfigurationException e) {
            Debug.log(Debug.ERROR, "Exception when calling setFeature: ", e);
        }

        try {
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
        } catch (ParserConfigurationException e) {
            Debug.log(Debug.ERROR, "Exception when calling setFeature: ", e);
        }

        try {
            factory.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        } catch (ParserConfigurationException e) {
            Debug.log(Debug.ERROR, "Exception when calling setFeature: ", e);
        }

        return factory;
    }

    /**
     * Constructor that assigns the given {@code ConvertData} to this object.
     *
     *  @param  pf  A {@code PluginFactoryImpl}  object.
     *  @param  cd  A {@code ConvertData} object to read data for the conversion
     *              process by the {@code deserialize} method.
     */
    public DocumentDeserializerImpl(PluginFactoryImpl pf,ConvertData cd) {
        this.cd = cd;
        pluginFactory = pf;
    }

    /**
     * This method performs the xslt transformation on the supplied
     * {@code Document} and returns a {@code ByteArrayOutputStream} object.
     *
     * <p>Xslt transformation code.</p>
     *
     * @return A {@code ByteArrayOutputStream} object containing the result
     *         of the Xslt transformation.
     */
    public Document deserialize() throws ConvertException, IOException {
        log("\nFound the XSLT deserializer");
        Iterator<Object> enumerate = cd.getDocumentEnumeration();
        org.w3c.dom.Document domDoc = null;
        DOMDocument docOut = null;
        ByteArrayOutputStream baos = null;
        GenericOfficeDocument sxwDoc = new GenericOfficeDocument("output");
        while (enumerate.hasNext()) {
            docOut = (DOMDocument) enumerate.next();
        }
        if (docOut != null) {
            try {
                domDoc = docOut.getContentDOM();
                baos = transform(domDoc);
                sxwDoc.initContentDOM();
                DocumentBuilderFactory dFactory = makeFactory();
                dFactory.setNamespaceAware(true);
                DocumentBuilder dBuilder = dFactory.newDocumentBuilder();
                sxwDoc.setContentDOM(dBuilder.parse(new ByteArrayInputStream(baos.toByteArray())));

            } catch (Exception e) {
                System.out.println("The following error occurred:" + e);
            }
        }
        return sxwDoc;
    }

    public Source resolve(String href, String base) throws TransformerException {
        if (href != null) {
            if (href.equals("javax.xml.transform.dom.DOMSource") || href.length() == 0) {
                return null;
            }
            try {
                ConverterInfo ci = pluginFactory.getConverterInfo();
                String newhRef = "jar:" + ci.getJarName() + "!/" + href;
                StreamSource sheetFile = new StreamSource(newhRef);
                return sheetFile;
            } catch (Exception e) {
                System.out.println("\nException in Xslt Resolver " + e);
                return null;
            }
        } else {
            return null;
        }
    }

    /**
     * This method performs the xslt transformation on the supplied Dom Tree.
     *
     * <p>Xslt transformation code.</p>
     */
    private ByteArrayOutputStream  transform(org.w3c.dom.Document xmlDoc){

       log("\nTransforming...");
       ConverterInfo ci = pluginFactory.getConverterInfo();
       ByteArrayOutputStream baos= new ByteArrayOutputStream();
       try{
          DocumentBuilderFactory dFactory = makeFactory();
          dFactory.setNamespaceAware(true);
          DocumentBuilder dBuilder = dFactory.newDocumentBuilder();

          String teststr = ci.getXsltDeserial();
          teststr= teststr.substring(0,6);
          org.w3c.dom.Document xslDoc=null;
              if ((teststr.equals("http:/"))||(teststr.equals("file:/"))
                                        ||(teststr.equals("jar://"))){
              log(ci.getXsltDeserial());
              xslDoc= dBuilder.parse(ci.getXsltDeserial());

          }
          else{
              log(ci.getJarName()+"!/"+ci.getXsltDeserial());
              xslDoc = dBuilder.parse(
                  "jar:"+ci.getJarName()+"!/"+ci.getXsltDeserial());
          }


          DOMSource xslDomSource = new DOMSource(xslDoc);
          DOMSource xmlDomSource = new DOMSource(xmlDoc);

           //call the transformer using the XSL, Source and Result dom.
          TransformerFactory tFactory = TransformerFactory.newInstance();
          tFactory.setURIResolver(this);
          Transformer transformer = tFactory.newTransformer(xslDomSource);
          transformer.transform(xmlDomSource,new StreamResult(baos));

          log("\n** Transform Complete ***");

       }
       catch (StackOverflowError sOE){
              System.out.println("\nERROR : Stack Overflow Error During Transformation\n Try increasing the stack size by passing the -Xss1m option to the JRE.");
          throw sOE;
       }
       catch(Exception e){
              System.out.println("An error occurred in the transformation : "+e);
       }
        return baos;
     }

    /**
     *  Sends message to the log object.
     *
     *  @param  str  Debug message.
     */
    private void log(String str) {

        Debug.log(Debug.TRACE, str);
    }
}
