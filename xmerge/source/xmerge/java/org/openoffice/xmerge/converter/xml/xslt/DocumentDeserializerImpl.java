/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.xslt;

import java.io.InputStream;
import java.io.IOException;
import java.util.Enumeration;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;




import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.converter.dom.DOMDocument;
//import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.converter.xml.xslt.GenericOfficeDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.registry.ConverterInfo;

// Imported TraX classes
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.dom.DOMSource;
//import javax.xml.transform.dom.DOMResult;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import javax.xml.transform.URIResolver;
import javax.xml.transform.Source;


//
//import org.apache.xalan.serialize.Serializer;
//import org.apache.xalan.serialize.SerializerFactory;
//import org.apache.xalan.templates.OutputProperties;

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
public final class DocumentDeserializerImpl
    implements  DocumentDeserializer,URIResolver {

    /**  A <code>ConvertData</code> object assigned to this object. */
    private InputStream is = null;
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
     *		  , FileNotFoundException,IOException
     *
     */
    public Document deserialize() throws ConvertException, IOException {
    log("\nFound the XSLT deserializer");
    Enumeration enumerate = cd.getDocumentEnumeration();
    org.w3c.dom.Document domDoc=null;
    DOMDocument docOut=null;
    GenericOfficeDocument doc = null;
    ByteArrayOutputStream baos =null;
    GenericOfficeDocument sxwDoc = new GenericOfficeDocument("output");
    while (enumerate.hasMoreElements()) {       
         docOut = (DOMDocument) enumerate.nextElement();
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
        //System.out.println("\nhref "+href+"\nbase "+base);
        if (href !=null){
        if(href.equals("javax.xml.transform.dom.DOMSource")|| href.equals(""))
            return null;
        try{
            ConverterInfo ci = pluginFactory.getConverterInfo();
            String newhRef ="jar:"+ci.getJarName()+"!/"+href;
            //System.out.println("\n Looking For "+ newhRef);
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
     *		  , FileNotFoundException,IOException
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
          /*
          // Serialize for output to standard out
          Serializer serializer = SerializerFactory.getSerializer
                                   (OutputProperties.getDefaultMethodProperties("xml"));
          serializer.setOutputStream(System.out);
          serializer.asDOMSerializer().serialize(xmlDomResult.getNode());
          */
  
          log("\n** Transform Complete ***");
         
       }
       catch (StackOverflowError sOE){
              System.out.println("\nERROR : Stack Overflow Error During Transformation\n Try increasing the stack size by passing the -Xss1m option to the JRE.");
          throw sOE;
       }
       catch(Exception e){
              System.out.println("An error occured in the transformation : "+e);
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

