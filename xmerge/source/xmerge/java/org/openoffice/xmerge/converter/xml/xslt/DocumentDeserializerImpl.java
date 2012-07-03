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
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.URIResolver;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.converter.dom.DOMDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.registry.ConverterInfo;


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
public final class DocumentDeserializerImpl
    implements  DocumentDeserializer,URIResolver {

    /**  A <code>ConvertData</code> object assigned to this object. */
    private ConvertData cd = null;
    private PluginFactoryImpl pluginFactory = null;

    /**
     *  Constructor that assigns the given <code>ConvertData</code>
     *  to this object.
     *
     *  @param  pf  A <code>PluginFactoryImpl</code> object.
     *
     *  @param  cd  A <code>ConvertData</code> object to read data for
     *              the conversion process by the <code>deserialize</code>
     *              method.
     */
    public DocumentDeserializerImpl(PluginFactoryImpl pf,ConvertData cd) {
        this.cd = cd;
    pluginFactory = pf;
    }



     /*
     * This method performs the xslt transformation on the supplied <code>
     * Document</code> and returns a <code>ByteArrayOutputStream</code> object.
     *
     *  Xslt transformation code
     *
     * @return baos A <code>ByteArrayOutputStream</code> object containing
     *               the result of the Xslt transformation.
     * @throws TransformerException,TransformerConfigurationException
     *        , FileNotFoundException,IOException
     *
     */
    public Document deserialize() throws ConvertException, IOException {
    log("\nFound the XSLT deserializer");
    Iterator<Object> enumerate = cd.getDocumentEnumeration();
    org.w3c.dom.Document domDoc=null;
    DOMDocument docOut=null;
    ByteArrayOutputStream baos =null;
    GenericOfficeDocument sxwDoc = new GenericOfficeDocument("output");
    while (enumerate.hasNext()) {
         docOut = (DOMDocument) enumerate.next();
    }
    domDoc = docOut.getContentDOM();
    try{
         baos = transform(domDoc);
         sxwDoc.initContentDOM();
         DocumentBuilderFactory dFactory = DocumentBuilderFactory.newInstance();
         dFactory.setNamespaceAware(true);
         DocumentBuilder dBuilder = dFactory.newDocumentBuilder();
         sxwDoc.setContentDOM(dBuilder.parse(new ByteArrayInputStream(baos.toByteArray())));

    }
    catch(Exception e){
        System.out.println("The following error occurred:"+e);
    }
    return sxwDoc;
    }

 public Source resolve(String href,String base)
    throws TransformerException{
        if (href !=null){
        if(href.equals("javax.xml.transform.dom.DOMSource")|| href.equals(""))
            return null;
        try{
            ConverterInfo ci = pluginFactory.getConverterInfo();
            String newhRef ="jar:"+ci.getJarName()+"!/"+href;
            StreamSource sheetFile= new StreamSource(newhRef);
            return sheetFile;
        }
        catch (Exception e){
            System.out.println("\nException in Xslt Resolver " +e);
            return null;
        }
        }
        else
        return null;
    }

     /*
     * This method performs the xslt transformation on the supplied Dom Tree.
     *
     *  Xslt transformation code
     *
     * @throws TransformerException,TransformerConfigurationException
     *        , FileNotFoundException,IOException
     *
     */
    private ByteArrayOutputStream  transform(org.w3c.dom.Document xmlDoc)
       throws TransformerException,TransformerConfigurationException
          , FileNotFoundException,IOException{

       log("\nTransforming...");
       ConverterInfo ci = pluginFactory.getConverterInfo();
       ByteArrayOutputStream baos= new ByteArrayOutputStream();
       try{
          DocumentBuilderFactory dFactory = DocumentBuilderFactory.newInstance();
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

           //call the tranformer using the XSL, Source and Result dom.
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

