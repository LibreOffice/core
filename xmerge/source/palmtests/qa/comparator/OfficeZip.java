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
 *  Class used by OfficeDocument to handle zip reading and writing,
 *  as well as storing zip entries.
 *
 *  @author   Herbie Ong
 */

class OfficeZip {

    /** file name of the xml file in a zipped document. */
    private final static String XMLFILE = "content.xml";

    private final static int BUFFERSIZE = 1024;

    private List entryList = null;

    private int contentIndex = -1;

    private String filename = null;

    private class Entry {

        ZipEntry zipEntry = null;
        byte bytes[] = null;
    }

    /**
     * Constructor 
     *
     * @param filename Full Path to Zip file to process
     *
     */
    public OfficeZip(String filename) {
        this.filename = filename;
    }


    /**
     *  Read each zip entry in the given InputStream object
     *  and store in entryList both the ZipEntry object as well
     *  as the bits of each entry.  Return the bytes for the
     *  entry of XMLFILE.
     *
     *  @param   is   InputStream object to read from
     *  @return   byte[]   byte array of XML file
     *  @throws   IOException   if any I/O error occurs
     */

    byte[] read(InputStream is) throws IOException {

        ZipInputStream zis = new ZipInputStream(is);
        ZipEntry ze = null;
        int i = -1;

        entryList = new LinkedList();

        while ((ze = zis.getNextEntry()) != null) {

            String name = ze.getName();

            Entry entry = new Entry();
            entry.zipEntry = ze;

            Debug.log(Debug.TRACE, "reading entry: " + name);

            ByteArrayOutputStream baos = new ByteArrayOutputStream();

            int len = 0;
            byte bytes[] = new byte[BUFFERSIZE];

            while ((len = zis.read(bytes)) > 0) {
                baos.write(bytes, 0, len);
            }

            entry.bytes = baos.toByteArray();

            entryList.add(entry);

            i++;

            if (isContentXML(name)) {
                contentIndex = i;
            }
        }

        if (contentIndex == -1) {
            throw new IOException(XMLFILE + " not found.");
        }

        Entry contentEntry = (Entry) entryList.get(contentIndex);

        return contentEntry.bytes;
    }

    /**
     *  Write out the XMLFILE as a zip into the OutputStream object.
     *
     *  If a zip inputstream was previously read, then use
     *  those zip contents to recreate the zip, except for XMLFILE,
     *  update it using the new content from xmlBytes.
     *
     *  If there was no zip inputstream previously read, write
     *  XMLFILE out into the zip outputstream.
     *
     *  @param   os   OutputStream object to write zip
     *  @param   xmlBytes   bytes of XMLFILE
     *  @throws   IOException   if any I/O errors occur.
     */

    void write(OutputStream os, byte xmlBytes[]) throws IOException {

        ZipOutputStream zos = new ZipOutputStream(os);

        // if read was not invoked previously, store the bytes directly.
        if (contentIndex == -1) {

            Debug.log(Debug.TRACE, "Writing out " + XMLFILE + " into zip.");

            ZipEntry ze = new ZipEntry(XMLFILE);
            ze.setSize(xmlBytes.length);

            CRC32 crc = new CRC32();
            crc.reset();
            crc.update(xmlBytes);
            ze.setCrc(crc.getValue());

            ze.setTime(System.currentTimeMillis());
            ze.setMethod(ZipEntry.DEFLATED);

            zos.putNextEntry(ze);
            zos.write(xmlBytes);

        } else {

            saveEntries(zos, xmlBytes);
        }

        zos.close();
    }

    /**
     *  Used by write method if there was a zip inputstream
     *  previously read.  It would write out each ZipEntry of
     *  the previously read zip, except for XMLFILE, it would
     *  update it with new values and with the content from
     *  xmlBytes.
     *
     *  @param   os   OutputStream object to write zip
     *  @param   xmlBytes   bytes of XMLFILE
     *  @throws   ZipException   if any zip I/O errors occur.
     */

    private void saveEntries(ZipOutputStream zos, byte xmlBytes[])
        throws IOException {

        Debug.log(Debug.TRACE, "Writing out the following entries into zip.");

        ListIterator iterator = entryList.listIterator();

        while (iterator.hasNext()) {

            Entry entry = (Entry) iterator.next();
            ZipEntry ze = entry.zipEntry;

            String name = ze.getName();

            Debug.log(Debug.TRACE, "... " + name);

            if (isContentXML(name)) {

                // set new values for this ZipEntry

                ZipEntry zipEntry = new ZipEntry(name);

                zipEntry.setMethod(ze.getMethod());
                zipEntry.setSize(xmlBytes.length);

                CRC32 crc = new CRC32();
                crc.reset();
                crc.update(xmlBytes);
                zipEntry.setCrc(crc.getValue());

                zipEntry.setTime(System.currentTimeMillis());

                zos.putNextEntry(zipEntry);
                zos.write(xmlBytes);

            } else {

                zos.putNextEntry(ze);
                zos.write(entry.bytes);
            }
        }
    }

    private boolean isContentXML(String name) {

        String lname = name.toLowerCase();
        return lname.equals(XMLFILE);
    }
}
