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
 *  org.openoffice.xmerge.converter.OfficeDocument
 *  OfficeDocument} to handle reading and writing
 *  from a ZIP file, as well as storing ZIP entries.
 *
 *  @author   Herbie Ong
 */
class OfficeZip {

    /** File name of the XML file in a zipped document. */
    private final static String CONTENTXML = "content.xml";

    private final static String STYLEXML = "styles.xml";
    private final static String METAXML = "meta.xml";

    private final static int BUFFERSIZE = 1024;

    private List entryList = null;

    private int contentIndex = -1;

    private int styleIndex = -1;
      private int metaIndex = -1;

    /** Default constructor. */
    OfficeZip() {

        entryList = new LinkedList();
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

            if (isSameName(name, CONTENTXML)) {
                contentIndex = i;
            } else if (isSameName(name, STYLEXML)) {
                styleIndex = i;
            }else if (isSameName(name, METAXML)) {
                metaIndex = i;
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
            Entry entry = (Entry) entryList.get(index);
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

            Entry entry = (Entry) entryList.get(index);
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

        ListIterator iterator = entryList.listIterator();

        while (iterator.hasNext()) {

            Entry entry = (Entry) iterator.next();
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
     *  Checks if name is the same as rName if case insensitive.
     *  Note that we assume here that rName is always in all lowercase.
     *
     *  @param  name   First name to compare.
     *  @param  rName  Second name to compare.
     *
     *  @return  true if identical, false otherwise.
     */
    private boolean isSameName(String name, String rName) {

        String lname = name.toLowerCase();
        return lname.equals(rName);
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

