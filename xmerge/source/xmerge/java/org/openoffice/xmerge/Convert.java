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

package org.openoffice.xmerge;

import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;

import org.openoffice.xmerge.util.registry.ConverterInfo;

/**
 * The {@code Convert} class manages a conversion from one mime-type to another.
 *
 * <p>The {@code ConvertFactory} is responsible for returning the appropriate
 * {@code Convert} class for a specified conversion. This class is responsible
 * for all interactions with the {@code PluginFactory} implementation.</p>
 *
 * @see ConverterFactory
 * @see PluginFactory
 * @see org.openoffice.xmerge.util.registry.ConverterInfo
 */
public class Convert implements Cloneable {

    /** ConvertInfo that corresponds to the from-mime/to-mime conversion. */
    private final ConverterInfo ci;

    /**
     * {@code true} if converting to the Office format, {@code false} if
     * converting to the device format.
     */
    private final boolean toOffice;

    /** Holds the convert input data. */
    private final ConvertData inputCD = new ConvertData();

    /**
     * Construct a Convert class with specified {@code ConvertInfo} registry
     * information.
     *
     * @param  ci        A {@code ConvertInfo} object containing registry
     *                   information corresponding to a specific plug-in.
     * @param  toOffice  {@code true} if converting to the Office format,
     *                   {@code false} if converting to the device format.
     */
    public Convert(ConverterInfo ci, boolean toOffice) {
        this.ci = ci;
        this.toOffice = toOffice;
    }

    /**
     * Adds an {@code InputStream} to be used as input by the {@code Convert}
     * class.
     *
     * <p>It is possible that many files need to be converted into a single
     * output {@code Document}, so this function may be called more than one
     * time. It is the plug-in's responsibility to know how to handle the input.
     * </p>
     *
     * @param   name  The name corresponding to the {@code InputStream}.
     * @param   is    {@code InputStream} to be used as input.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void addInputStream(String name, InputStream is)
        throws IOException {

        Document inputDoc;

        if (toOffice) {
            inputDoc = ci.getPluginFactory().createDeviceDocument(name, is);
        } else {
            inputDoc = ci.getPluginFactory().createOfficeDocument(name, is);
        }
        inputCD.addDocument(inputDoc);
    }

    /**
     * Adds an {@code InputStream} to be used as input by the {@code Convert}
     * class.
     *
     * <p>It is possible that many files need to be converted into a single
     * output {@code Document}, so this function may be called more than one
     * time. It is the plug-in's responsibility to know how to handle the input.
     * </p>
     *
     * @param   name  The name corresponding to the {@code InputStream}.
     * @param   is    {@code InputStream} to be used as input.
     * @param   isZip {@code boolean} to identify that incoming stream is * zipped.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void addInputStream(String name, InputStream is,boolean isZip)
        throws IOException {

        Document inputDoc;

        if (toOffice) {
            inputDoc = ci.getPluginFactory().createDeviceDocument(name, is);
        } else {
            inputDoc = ci.getPluginFactory().createOfficeDocument(name, is, isZip);
        }
        inputCD.addDocument(inputDoc);
    }


    /**
     * Returns a {@code DocumentMerger} for the given {@code Document}.
     *
     * @param   origDoc The {@code Document} were later changes will be merged to.
     *
     * @return  The {@code DocumentMerger} object for the given document.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public DocumentMerger getDocumentMerger(Document origDoc) throws IOException {
        DocumentMergerFactory myDocMergerFactory = ci.getDocMergerFactory();
        DocumentMerger merger = myDocMergerFactory.createDocumentMerger(origDoc);
        return merger;
    }

    /**
     * Resets the input queue, so that the user can use this class to perform
     * another conversion.
     *
     * <p>This causes the {@code addInputStream} method to accept input for the
     * next conversion.</p>
     */
    private void reset() {
        inputCD.reset();
    }

    /**
     * Clones a {@code Convert} object so another Convert object can do the same
     * conversion.
     *
     * <p>{@code InputStream} objects passed in via calls to the
     * {@code addInputStream} method are not copied.</p>
     *
     * @return  The cloned {@code Convert} object.
     */
    @Override
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
     * Convert the input specified in calls to the {@code addInputStream}
     * method to the output format specified by this {@code Convert} class.
     *
     * @return  The output data.
     *
     * @throws  ConvertException  If any conversion error occurs.
     * @throws  IOException       If any I/O error occurs.
     */
    public ConvertData convert() throws ConvertException, IOException {

        ConvertData dataOut = new ConvertData();

        if (toOffice) {

            // From device format to Office format

            DocumentDeserializerFactory myDocDeserializerFactory =
                ci.getDocDeserializerFactory();
            DocumentDeserializer deser =
                myDocDeserializerFactory.createDocumentDeserializer(inputCD);
            Document deviceDoc = deser.deserialize();


            dataOut.addDocument(deviceDoc);
            return dataOut;

        } else {

            // From Office format to device format

            DocumentSerializerFactory myDocSerializerFactory =
                ci.getDocSerializerFactory();

            Iterator<Object> e = inputCD.getDocumentEnumeration();

            Document doc = (Document) e.next();
            DocumentSerializer ser = myDocSerializerFactory.createDocumentSerializer(doc);
            dataOut = ser.serialize();

            return dataOut;
        }
    }

    /**
     * Returns the appropriate &quot;Office&quot; {@code Document} object for
     * this plug-in.
     *
     * @param   name  The name of the {@code Document} to create.
     * @param   is    The {@code InputStream} corresponding to the
     *                {@code Document} to create.
     *
     * @return  The appropriate &quot;Office&quot; {@code Document} object for
     *          this plug-in.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public Document getOfficeDocument(String name, InputStream is)
        throws IOException {
        return ci.getPluginFactory().createOfficeDocument(name, is);
    }
}