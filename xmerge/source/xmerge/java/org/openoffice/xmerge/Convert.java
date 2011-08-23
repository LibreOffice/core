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

package org.openoffice.xmerge;

import java.io.InputStream;
import java.io.IOException;
import java.util.Enumeration;

import org.openoffice.xmerge.util.registry.ConverterInfo;

/**
 *  The <code>Convert</code> class manages a conversion from one
 *  mime-type to another.  The <code>ConvertFactory</code> is
 *  responsible for returning the appropriate <code>Convert</code>
 *  class for a specified conversion.  This class is responsible
 *  for all interactions with the <code>PluginFactory</code>
 *  implementation.
 *
 *  @see  ConverterFactory
 *  @see  PluginFactory
 *  @see  org.openoffice.xmerge.util.registry.ConverterInfo
 *
 *  @author  Martin Maher
 */
public class Convert implements Cloneable {

    /**
     *  ConvertInfo that corresponds to the from-mime/to-mime
     *  conversion.
     */
    private ConverterInfo ci;

    /**
     *  true if converting to the Office format, false if converting
     *  to the device format.
     */
    private boolean toOffice;

    /**
     *  Holds the convert input data.
     */
    private ConvertData inputCD = new ConvertData();


    /**
     *  Construct a Convert class with specified <code>ConvertInfo</code>
     *  registry information.
     *
     *  @param  ci        A <code>ConvertInfo</code> object containing
     *                    registry information corresponding to a
     *                    specific plug-in.
     *  @param  toOffice  true if converting to the Office format,
     *                    false if converting to the device format.
     */
    public Convert(ConverterInfo ci, boolean toOffice) {
        this.ci = ci;
        this.toOffice = toOffice;
    }


    /** 
     *  Adds an <code>InputStream</code> to be used as input by the
     *  <code>Convert</code> class.  It is possible that many files
     *  need to be converted into a single output <code>Documetn</code>,
     *  so this function may be called more than one time.  It is the
     *  plug-in's responsibility to know how to handle the input.
     *
     *  @param  name  The name corresponding to the <code>InputStream</code>.
     *  @param  is    <code>InputStream</code> to be used as input.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void addInputStream(String name, InputStream is)
        throws IOException {
            
        Document inputDoc;

        if (toOffice == true) {
            inputDoc = ci.getPluginFactory().createDeviceDocument(name, is);
        } else {
            inputDoc = ci.getPluginFactory().createOfficeDocument(name, is);
        }
        inputCD.addDocument(inputDoc);
    }

     /** 
     *  Adds an <code>InputStream</code> to be used as input by the
     *  <code>Convert</code> class.  It is possible that many files
     *  need to be converted into a single output <code>Documetn</code>,
     *  so this function may be called more than one time.  It is the
     *  plug-in's responsibility to know how to handle the input.
     *
     *  @param  name  The name corresponding to the <code>InputStream</code>.
     *  @param  is    <code>InputStream</code> to be used as input.
     *  @param  isZip <code>boolean</code> to identify that incoming stream is      *                zipped
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void addInputStream(String name, InputStream is,boolean isZip)
        throws IOException {
            
        Document inputDoc;

        if (toOffice == true) {
            inputDoc = ci.getPluginFactory().createDeviceDocument(name, is);
        } else {
            inputDoc = ci.getPluginFactory().createOfficeDocument(name, is, isZip);
        }
        inputCD.addDocument(inputDoc);
    }


    /** 
     *  Returns a <code>DocumentMerger</code> for the given <code>Document</code>.
     *
     *  @param  origDoc The <code>Document</code> were later changes will be merged to
     *
     *  @return  The <code>DocumentMerger</code> object for the given document.
     *     
     *  @throws  IOException  If any I/O error occurs.
     */
    public DocumentMerger getDocumentMerger(Document origDoc)
        throws IOException {
            
     DocumentMergerFactory myDocMergerFactory = ci.getDocMergerFactory();
     DocumentMerger merger = myDocMergerFactory.createDocumentMerger(origDoc);
     return merger;
    }    

    /** 
     *  Resets the input queue, so that the user can use this class to 
     *  perform another conversion.  This causes the
     *  <code>addInputStream</code> method to accept input for the next
     *  conversion.
     */
    public void reset() {
        inputCD.reset();
    }

    
    /** 
     *  Clones a Convert object so another Convert object can 
     *  do the same conversion.  <code>InputStream<code> objects passed
     *  in via calls to the <code>addInputStream</code> method are not
     *  copied.
     *
     *  @return  The cloned <code>Convert</code> object.
     */
    public Object clone() {

        Convert aClone = null;

        try { 
            aClone = (Convert) super.clone();
            aClone.reset();
        }
        catch (CloneNotSupportedException e) {
            System.out.println("Convert clone could not be created");
        }
        return aClone;
    }    
    

    /**
     *  Convert the input specified in calls to the <code>addInputStream</code>
     *  method to the output format specified by this <code>Convert</code>
     *  class.
     *
     *  @return  The output data.
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData convert() throws ConvertException, IOException {
        
        ConvertData dataOut = new ConvertData();

        if (toOffice) {
                    
            //  From device format to Office format
            //
            DocumentDeserializerFactory myDocDeserializerFactory =
                ci.getDocDeserializerFactory();
            DocumentDeserializer deser =
                myDocDeserializerFactory.createDocumentDeserializer(inputCD);
            Document deviceDoc = deser.deserialize();

            
            dataOut.addDocument(deviceDoc);
            return dataOut;                                    

        } else {
                
            //  From Office format to device format
            //
            DocumentSerializerFactory myDocSerializerFactory =
                ci.getDocSerializerFactory();

            Enumeration e = inputCD.getDocumentEnumeration();
            
            Document doc = (Document) e.nextElement(); 
            DocumentSerializer ser = myDocSerializerFactory.createDocumentSerializer(doc);
            dataOut = ser.serialize();
            
            return dataOut;
        }
    }

    /**
     *  NEW (HJ):
     *  Convert the input specified in calls to the <code>addInputStream</code>
     *  method to the output format specified by this <code>Convert</code>
     *  class.
     *  The (de)serializer may use the URLs to resolve links and choose name(s)
     *  for destination document(s).
     *
     *  @return  The output data.
     *
     *  @param   sFromURL          URL of the source document (may be null if unknown)
     *  @param   sToURL            URL of the destination document (may be null if unknown)
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData convert(String sFromURL, String sToURL) throws
        ConvertException, IOException {
        
        ConvertData dataOut = new ConvertData();

        if (toOffice) {
                    
            //  From device format to Office format
            //
            DocumentDeserializerFactory myDocDeserializerFactory =
                ci.getDocDeserializerFactory();
            DocumentDeserializer deser =
                myDocDeserializerFactory.createDocumentDeserializer(inputCD);
            Document officeDoc = deser instanceof DocumentSerializer2 ?
                ((DocumentDeserializer2) deser).deserialize(sFromURL,sToURL) :
                deser.deserialize();

            
            dataOut.addDocument(officeDoc);
            return dataOut;                                    

        } else {
                
            //  From Office format to device format
            //
            DocumentSerializerFactory myDocSerializerFactory =
                ci.getDocSerializerFactory();

            Enumeration e = inputCD.getDocumentEnumeration();
            
            Document doc = (Document) e.nextElement(); 
            DocumentSerializer ser = myDocSerializerFactory.createDocumentSerializer(doc);
            dataOut = ser instanceof DocumentSerializer2 ?
                ((DocumentSerializer2) ser).serialize(sFromURL,sToURL) :
                ser.serialize();
            
            return dataOut;
        }
    }

    /**
     *  Returns the appropriate &quot;Office&quot; <code>Document</code>
     *  object for this plug-in.
     *
     *  @param  name  The name of the <code>Document</code> to create.
     *  @param  is    The <code>InputStream</code> corresponding to the
     *                <code>Document</code> to create.
     *
     *  @return  The appropriate &quot;Office&quot; <code>Document</code>
     *           object for this plug-in.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public Document getOfficeDocument(String name, InputStream is)
        throws IOException {
        return(ci.getPluginFactory().createOfficeDocument(name, is));
    }


    /**
     *  Returns the appropriate &quot;Device&quot; <code>Document</code>
     *  object for this plug-in.
     *
     *  @param  name  The name of the <code>Document</code> to create.
     *  @param  is    The <code>InputStream</code> corresponding to the
     *                <code>Document</code> to create.
     *
     *  @return  The appropriate &quot;Device&quot; <code>Document</code>
     *           object for this plug-in.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public Document getDeviceDocument(String name, InputStream is)
        throws IOException {
        return(ci.getPluginFactory().createDeviceDocument(name, is));
    }
}

