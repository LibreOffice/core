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
     *  Adds the original Document to the Convert Class. This
     *  is to be used for merges. If this Document is added then
     *  it is assumed that a convert with merge is the desired
     *  operation.
     *
     *  @param  name  The <code>Document</code> name.
     *  @param  is    <code>InputStream</code> object corresponding
     *                to the <code>Document</code>.
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

