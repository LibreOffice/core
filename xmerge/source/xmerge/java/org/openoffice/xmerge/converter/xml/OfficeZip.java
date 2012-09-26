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
 *  Class used by {@link
 *  org.openoffice.xmerge.converter.xml.OfficeDocument
 *  OfficeDocument} to handle reading and writing
 *  from a ZIP file, as well as storing ZIP entries.
 *
 */
class OfficeZip {

    /** File name of the XML file in a zipped document. */
    private final static String CONTENTXML = "content.xml";

    private final static String STYLEXML = "styles.xml";
    private final static String METAXML = "meta.xml";
    private final static String SETTINGSXML = "settings.xml";
    private final static String MANIFESTXML = "META-INF/manifest.xml";

    private final static int BUFFERSIZE = 1024;

    private List<Entry> entryList = null;

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
     *  <p>Read each zip entry in the <code>InputStream</code> object
     *  and store in entryList both the <code>ZipEntry</code> object
     *  as well as the bits of each entry.  Call this method before
     *  calling the <code>getContentXMLBytes</code> method or the
     *  <code>getStyleXMLBytes</code> method.</p>
     *
     *  <p>Keep track of the CONTENTXML and STYLEXML using
     *  contentIndex and styleIndex, respectively.</p>
     *
     *  @param  is  <code>InputStream</code> object to read.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    void read(InputStream is) throws IOException {

        ZipInputStream zis = new ZipInputStream(is);
        ZipEntry ze = null;
        int i = -1;

        while ((ze = zis.getNextEntry()) != null) {

            String name = ze.getName();

            Debug.log(Debug.TRACE, "reading entry: " + name);

            ByteArrayOutputStream baos = new ByteArrayOutputStream();

            int len = 0;
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
     *  This method returns the CONTENTXML file in a
     *  <code>byte</code> array.  It returns null if there is no
     *  CONTENTXML in this zip file.
     *
     *  @return  CONTENTXML in a <code>byte</code> array.
     */
    byte[] getContentXMLBytes() {

        return getEntryBytes(contentIndex);
    }


    /**
     *  This method returns the STYLEXML file in a
     *  <code>byte</code> array.  It returns null if there is
     *  no STYLEXML in this zip file.
     *
     *  @return  STYLEXML in a <code>byte</code> array.
     */
    byte[] getStyleXMLBytes() {

        return getEntryBytes(styleIndex);
    }

     /**
     *  This method returns the METAXML file in a
     *  <code>byte</code> array.  It returns null if there is
     *  no METAXML in this zip file.
     *
     *  @return  METAXML in a <code>byte</code> array.
     */
    byte[] getMetaXMLBytes() {
        return getEntryBytes(metaIndex);
    }

      /**
     *  This method returns the SETTINGSXML file in a
     *  <code>byte</code> array.  It returns null if there is
     *  no SETTINGSXML in this zip file.
     *
     *  @return  SETTINGSXML in a <code>byte</code> array.
     */
    byte[] getSettingsXMLBytes() {
        return getEntryBytes(settingsIndex);
    }

    /**
     * This method returns the MANIFESTXML file in a <code>byte</code> array.
     * It returns null if there is no MANIFESTXML in this zip file.
     *
     * @return  MANIFESTXML in a <code>byte</code> array.
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
     * @return  The data for the named entry in a <code>byte</code> array or
     *          <code>null</code> if no entry is found.
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
     * This method sets the bytes for the named entry.  It searches for a
     * matching entry in the LinkedList.  If no entry is found, a new one is
     * created.
     *
     * Writing of data is defferred to setEntryBytes().
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
     *  Used by the <code>getContentXMLBytes</code> method and the
     *  <code>getStyleXMLBytes</code> method to return the
     *  <code>byte</code> array from the corresponding
     *  <code>entry</code> in the <code>entryList</code>.
     *
     *  @param  index  Index of <code>Entry</code> object in
     *                 <code>entryList</code>.
     *
     *  @return  <code>byte</code> array associated in that
     *           <code>Entry</code> object or null, if there is
     *           not such <code>Entry</code>.
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
     *  Set or replace the <code>byte</code> array for the
     *  CONTENTXML file.
     *
     *  @param  bytes  <code>byte</code> array for the
     *                 CONTENTXML file.
     */
    void setContentXMLBytes(byte bytes[]) {

        contentIndex = setEntryBytes(contentIndex, bytes, CONTENTXML);
    }


    /**
     *  Set or replace the <code>byte</code> array for the
     *  STYLEXML file.
     *
     *  @param  bytes  <code>byte</code> array for the
     *                 STYLEXML file.
     */
    void setStyleXMLBytes(byte bytes[]) {

        styleIndex = setEntryBytes(styleIndex, bytes, STYLEXML);
    }


      /**
     *  Set or replace the <code>byte</code> array for the
     *  METAXML file.
     *
     *  @param  bytes  <code>byte</code> array for the
     *                 METAXML file.
     */
    void setMetaXMLBytes(byte bytes[]) {

        metaIndex = setEntryBytes(metaIndex, bytes, METAXML);
    }


      /**
     *  Set or replace the <code>byte</code> array for the
     *  SETTINGSXML file.
     *
     *  @param  bytes  <code>byte</code> array for the
     *                 SETTINGSXML file.
     */
    void setSettingsXMLBytes(byte bytes[]) {

        settingsIndex = setEntryBytes(settingsIndex, bytes, SETTINGSXML);
    }


    /**
     * Set or replace the <code>byte</code> array for the MANIFESTXML file.
     *
     * @param   bytes   <code>byte</code> array for the MANIFESTXML file.
     */
    void setManifestXMLBytes(byte bytes[]) {
        manifestIndex = setEntryBytes(manifestIndex, bytes, MANIFESTXML);
    }

    /**
     *  <p>Used by the <code>setContentXMLBytes</code> method and
     *  the <code>setStyleXMLBytes</code> to either replace an
     *  existing <code>Entry</code>, or create a new entry in
     *  <code>entryList</code>.</p>
     *
     *  <p>If there is an <code>Entry</code> object within
     *  entryList that corresponds to the index, replace the
     *  <code>ZipEntry</code> info.</p>
     *
     *  @param  index  Index of <code>Entry</code> to modify.
     *  @param  bytes  <code>Entry</code> value.
     *  @param  name   Name of <code>Entry</code>.
     *
     *  @return Index of value added or modified in entryList
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
     *  Write out the ZIP entries into the <code>OutputStream</code>
     *  object.
     *
     *  @param  os  <code>OutputStream</code> object to write ZIP.
     *
     *  @throws  IOException  If any ZIP I/O error occurs.
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
     *  Creates a <code>ZipEntry</code> object based on the given params.
     *
     *  @param  name    Name for the <code>ZipEntry</code>.
     *  @param  bytes   <code>byte</code> array for <code>ZipEntry</code>.
     *  @param  method  ZIP method to be used for <code>ZipEntry</code>.
     *
     *  @return  A <code>ZipEntry</code> object.
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
     *  This inner class is used as a data structure for holding
     *  a <code>ZipEntry</code> info and its corresponding bytes.
     *  These are stored in entryList.
     */
    private class Entry {

        ZipEntry zipEntry = null;
        byte bytes[] = null;

        Entry(ZipEntry zipEntry, byte bytes[]) {
            this.zipEntry = zipEntry;
            this.bytes = bytes;
        }
    }
}

