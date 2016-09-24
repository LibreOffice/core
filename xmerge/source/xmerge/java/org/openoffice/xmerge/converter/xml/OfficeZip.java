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

package org.openoffice.xmerge.converter.xml;

import java.util.List;
import java.util.ListIterator;
import java.util.LinkedList;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.CRC32;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.ByteArrayOutputStream;

import org.openoffice.xmerge.util.Debug;

/**
 * Class used by {@link org.openoffice.xmerge.converter.xml.OfficeDocument
 * OfficeDocument} to handle reading and writing from a ZIP file, as well as
 * storing ZIP entries.
 */
class OfficeZip {

    /** File name of the XML file in a zipped document. */
    private static final String CONTENTXML = "content.xml";

    private static final String STYLEXML = "styles.xml";
    private static final String METAXML = "meta.xml";
    private static final String SETTINGSXML = "settings.xml";
    private static final String MANIFESTXML = "META-INF/manifest.xml";

    private static final int BUFFERSIZE = 1024;

    private final List<Entry> entryList;

    private int contentIndex = -1;
    private int styleIndex = -1;
    private int metaIndex = -1;
    private int settingsIndex = -1;
    private int manifestIndex = -1;

    /** Default constructor. */
    OfficeZip() {

        entryList = new LinkedList<Entry>();
    }

    /**
     * Read each zip entry in the {@code InputStream} object and store in
     * entryList both the {@code ZipEntry} object as well as the bits of each
     * entry.
     *
     * <p>Call this method before calling the {@code getContentXMLBytes} method
     * or the {@code getStyleXMLBytes} method.</p>
     *
     * <p>Keep track of the {@code CONTENTXML} and {@code STYLEXML} using
     * {@code contentIndex} and {@code styleIndex}, respectively.</p>
     *
     * @param   is  {@code InputStream} object to read.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    void read(InputStream is) throws IOException {

        ZipInputStream zis = new ZipInputStream(is);
        ZipEntry ze;
        int i = -1;

        while ((ze = zis.getNextEntry()) != null) {

            String name = ze.getName();

            Debug.log(Debug.TRACE, "reading entry: " + name);

            ByteArrayOutputStream baos = new ByteArrayOutputStream();

            int len;
            byte buffer[] = new byte[BUFFERSIZE];

            while ((len = zis.read(buffer)) > 0) {
                baos.write(buffer, 0, len);
            }

            byte bytes[] = baos.toByteArray();
            Entry entry = new Entry(ze,bytes);

            entryList.add(entry);

            i++;

            if (name.equalsIgnoreCase(CONTENTXML)) {
                contentIndex = i;
            }
            else if (name.equalsIgnoreCase(STYLEXML)) {
                styleIndex = i;
            }
            else if (name.equalsIgnoreCase(METAXML)) {
                metaIndex = i;
            }
            else if (name.equalsIgnoreCase(SETTINGSXML)) {
                settingsIndex = i;
            }
            else if (name.equalsIgnoreCase(MANIFESTXML)) {
                manifestIndex = i;
            }

        }

        zis.close();
    }

    /**
     * This method returns the CONTENTXML file in a {@code byte} array.
     *
     * <p>It returns null if there is no {@code CONTENTXML} in this zip file.</p>
     *
     * @return  CONTENTXML in a {@code byte} array.
     */
    byte[] getContentXMLBytes() {

        return getEntryBytes(contentIndex);
    }

    /**
     * This method returns the {@code STYLEXML} file in a {@code byte} array.
     *
     * <p>It returns {@code null} if there is no {@code STYLEXML} in this zip
     * file.</p>
     *
     * @return  STYLEXML in a {@code byte} array.
     */
    byte[] getStyleXMLBytes() {

        return getEntryBytes(styleIndex);
    }

    /**
     * This method returns the METAXML file in a {@code byte} array.
     *
     * <p>It returns {@code null} if there is no {@code METAXML} in this zip
     * file.</p>
     *
     * @return  METAXML in a {@code byte} array.
     */
    byte[] getMetaXMLBytes() {
        return getEntryBytes(metaIndex);
    }

    /**
     * This method returns the {@code SETTINGSXML} file in a {@code byte} array.
     *
     * <p>It returns {@code null} if there is no {@code SETTINGSXML} in this zip
     * file.</p>
     *
     *  @return  SETTINGSXML in a <code>byte</code> array.
     */
    byte[] getSettingsXMLBytes() {
        return getEntryBytes(settingsIndex);
    }

    /**
     * This method returns the {@code MANIFESTXML} file in a {@code byte} array.
     *
     * <p>It returns {@code null} if there is no {@code MANIFESTXML} in this zip
     * file.</p>
     *
     * @return  MANIFESTXML in a {@code byte} array.
     */
    byte[] getManifestXMLBytes() {
        return getEntryBytes(manifestIndex);
    }

    /**
     * This method returns the bytes corresponding to the entry named in the
     * parameter.
     *
     * @param   name    The name of the entry in the Zip file to retrieve.
     *
     * @return  The data for the named entry in a {@code byte} array or
     *          {@code null} if no entry is found.
     */
    byte[] getNamedBytes(String name) {

        // The list is not sorted, and sorting it for a binary search would
        // invalidate the indices stored for the main files.

        // Could improve performance by caching the name and index when
        // iterating through the ZipFile in read().
        for (int i = 0; i < entryList.size(); i++) {
            Entry e = entryList.get(i);

            if (e.zipEntry.getName().equals(name)) {
                return getEntryBytes(i);
            }
        }

        return null;
    }

    /**
     * This method sets the bytes for the named entry.
     *
     * <p>It searches for a matching entry in the LinkedList.  If no entry is
     * found, a new one is created.</p>
     *
     * <p>Writing of data is deferred to {@code setEntryBytes()}.</p>
     *
     * @param   name    The name of the entry to search for.
     * @param   bytes   The new data to write.
     */
    void setNamedBytes(String name, byte[] bytes) {
        for (int i = 0; i < entryList.size(); i++) {
            Entry e = entryList.get(i);

            if (e.zipEntry.getName().equals(name)) {
                setEntryBytes(i, bytes, name);
                return;
            }
        }

        // If we're here, no entry was found.  Call setEntryBytes with an index
        // of -1 to insert a new entry.
        setEntryBytes(-1, bytes, name);
    }

    /**
     * Used by the {@code getContentXMLBytes} method and the
     * {@code getStyleXMLBytes} method to return the {@code byte} array from the
     * corresponding {@code Entry} in the {@code entryList}.
     *
     *  @param   index  Index of {@code Entry} object in {@code entryList}.
     *
     *  @return  {@code byte} array associated in that {@code Entry} object or
     *           {@code null}, if there is not such {@code Entry}.
     */
    private byte[] getEntryBytes(int index) {

        byte[] bytes = null;

        if (index > -1) {
            Entry entry = entryList.get(index);
            bytes = entry.bytes;
        }
        return bytes;
    }

    /**
     * Set or replace the <code>byte</code> array for the {@code CONTENTXML} file.
     *
     * @param  bytes  {@code byte} array for the {@code CONTENTXML} file.
     */
    void setContentXMLBytes(byte bytes[]) {

        contentIndex = setEntryBytes(contentIndex, bytes, CONTENTXML);
    }

    /**
     * Set or replace the {@code byte} array for the {@code STYLEXML} file.
     *
     * @param  bytes  {@code byte} array for the {@code STYLEXML} file.
     */
    void setStyleXMLBytes(byte bytes[]) {

        styleIndex = setEntryBytes(styleIndex, bytes, STYLEXML);
    }

    /**
     * Set or replace the {@code byte} array for the {@code METAXML} file.
     *
     * @param  bytes  {@code byte} array for the {@code METAXML} file.
     */
    void setMetaXMLBytes(byte bytes[]) {

        metaIndex = setEntryBytes(metaIndex, bytes, METAXML);
    }

    /**
     * Set or replace the {@code byte} array for the {@code SETTINGSXML} file.
     *
     * @param  bytes  {@code byte} array for the {@code SETTINGSXML} file.
     */
    void setSettingsXMLBytes(byte bytes[]) {

        settingsIndex = setEntryBytes(settingsIndex, bytes, SETTINGSXML);
    }

    /**
     * Set or replace the {@code byte} array for the {@code MANIFESTXML} file.
     *
     * @param   bytes   {@code byte} array for the {@code MANIFESTXML} file.
     */
    void setManifestXMLBytes(byte bytes[]) {
        manifestIndex = setEntryBytes(manifestIndex, bytes, MANIFESTXML);
    }

    /**
     * Used by the {@code setContentXMLBytes} method and the
     * {@code setStyleXMLBytes} to either replace an existing {@code Entry}, or
     * create a new entry in {@code entryList}.
     *
     * <p>If there is an {@code Entry} object within {@code entryList} that
     * corresponds to the index, replace the {@code ZipEntry} info.</p>
     *
     * @param  index  Index of <code>Entry</code> to modify.
     * @param  bytes  <code>Entry</code> value.
     * @param  name   Name of <code>Entry</code>.
     *
     * @return Index of value added or modified in entryList
     */
    private int setEntryBytes(int index, byte bytes[], String name) {

        if (index > -1) {
            // replace existing entry in entryList
            Entry entry = entryList.get(index);
            name = entry.zipEntry.getName();
            int method = entry.zipEntry.getMethod();

            ZipEntry ze = createZipEntry(name, bytes, method);

            entry.zipEntry = ze;
            entry.bytes= bytes;

        } else {
            // add a new entry into entryList
            ZipEntry ze = createZipEntry(name, bytes, ZipEntry.DEFLATED);
            Entry entry = new Entry(ze, bytes);
            entryList.add(entry);
            index = entryList.size() - 1;
        }

        return index;
    }

    /**
     * Write out the ZIP entries into the {@code OutputStream} object.
     *
     * @param  os  <code>OutputStream</code> object to write ZIP.
     *
     * @throws  IOException  If any ZIP I/O error occurs.
     */
    void write(OutputStream os) throws IOException {

        Debug.log(Debug.TRACE, "Writing out the following entries into zip.");

        ZipOutputStream zos = new ZipOutputStream(os);

        ListIterator<Entry> iterator = entryList.listIterator();
        while (iterator.hasNext()) {

            Entry entry = iterator.next();
            ZipEntry ze = entry.zipEntry;

            String name = ze.getName();

            Debug.log(Debug.TRACE, "... " + name);

            zos.putNextEntry(ze);
            zos.write(entry.bytes);
        }

        zos.close();
    }

    /**
     * Creates a {@code ZipEntry} object based on the given parameters.
     *
     * @param   name    Name for the {@code ZipEntry}.
     * @param   bytes   {@code byte} array for {@code ZipEntry}.
     * @param   method  ZIP method to be used for {@code ZipEntry}.
     *
     * @return  A {@code ZipEntry} object.
     */
    private ZipEntry createZipEntry(String name, byte bytes[], int method) {

        ZipEntry ze = new ZipEntry(name);

        ze.setMethod(method);
        ze.setSize(bytes.length);

        CRC32 crc = new CRC32();
        crc.reset();
        crc.update(bytes);
        ze.setCrc(crc.getValue());

        ze.setTime(System.currentTimeMillis());

        return ze;
    }

    /**
     * This inner class is used as a data structure for holding a {@code ZipEntry}
     * info and its corresponding bytes.
     *
     * <p>These are stored in {@code entryList}.</p>
     */
    private static class Entry {

        ZipEntry zipEntry = null;
        byte bytes[] = null;

        Entry(ZipEntry zipEntry, byte bytes[]) {
            this.zipEntry = zipEntry;
            this.bytes = bytes;
        }
    }
}