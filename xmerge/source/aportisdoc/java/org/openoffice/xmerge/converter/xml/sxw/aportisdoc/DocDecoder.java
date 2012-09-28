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

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;

import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.util.Resources;
import org.openoffice.xmerge.util.Debug;

/**
 *  This class is used by {@link DocumentDeserializerImpl}
 *  to decode the AportisDoc format.  It currently decodes
 *  the text content into a single <code>String</code> object.
 *
 */
final class DocDecoder implements DocConstants {

    /** For decoding purposes. */
    private final static int COUNT_BITS = 3;

    /** Resources object for I18N. */
    private Resources res = null;


    /**
     *  Default constructor creates a header and a text buffer
     *  for holding all the text in the AportisDoc database.
     */
    DocDecoder() {
        res = Resources.getInstance();
    }


    /**
     *  Decode the text records into a single <code>String</code>
     *  of text content.
     *
     *  @param  recs  <code>Record</code> array holding AportisDoc
     *                  contents.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    String parseRecords(Record[] recs) throws IOException {

        // read the header record
        HeaderInfo header = readHeader(recs[0].getBytes());

        dumpHeader(header);

        // store all the characters in textBuffer
        StringBuffer textBuffer = new StringBuffer(header.textLen);

        switch (header.version) {

            case COMPRESSED:
                for (int i = 1; i <= header.textRecordCount; i++) {

                    byte[] bytes = decompress(recs[i].getBytes(),
                                              header.textRecordSize);
                    log("processing " + bytes.length + " bytes");
                    String str = new String(bytes, ENCODING);
                    textBuffer.append(str);
                }

                break;

            case UNCOMPRESSED:
                for (int i = 1; i <= header.textRecordCount; i++) {

                    byte[] bytes = recs[i].getBytes();
                    log("processing " + bytes.length + " bytes");
                    String str = new String(bytes, ENCODING);
                    textBuffer.append(str);
                }

                break;

            default:
                throw new IOException(res.getString("UNKNOWN_DOC_VERSION"));

        }

        return textBuffer.toString();
    }


    /**
     *  <p>Decompress the <code>byte</code> array.</p>
     *
     *  <p>The resulting uncompressed <code>byte</code> array should
     *  be within <code>textRecordSize</code> length, definitely
     *  within twice the size it claims, else treat it as a problem
     *  with the encoding of that PDB and throw
     *  <code>IOException</code>.</p>
     *
     *  @param  cBytes           Compressed <code>byte</code> array.
     *  @param  textRecordSize  Size of uncompressed
     *                          <code>byte</code> array.
     *
     *  @throws  IOException  If <code>textRecordSize</code> &lt;
     *                        <code>cBytes.length</code>.
     */
    private byte[] decompress(byte[] cBytes, int textRecordSize)
        throws IOException {

        // create byte array for storing uncompressed bytes
        // it should be within textRecordSize range, definitely
        // within twice of textRecordSize!  if not, then
        // an ArrayIndexOutOfBoundsException will get thrown,
        // and it should be converted into an IOException, and
        // treat it as a conversion error.
        byte[] uBytes = new byte[textRecordSize*2];

        int up = 0;
        int cp = 0;

        try {

            while (cp < cBytes.length) {

                int c = cBytes[cp++] & 0xff;

                // codes 1...8 mean copy that many bytes
                if (c > 0 && c < 9) {

                    while (c-- > 0)
                        uBytes[up++] = cBytes[cp++];
                }

                // codes 0, 9...0x7F represent themselves
                else if (c < 0x80) {
                    uBytes[up++] = (byte) c;
                }

                // codes 0xC0...0xFF represent "space + ascii char"
                else if (c >= 0xC0) {
                    uBytes[up++] = (byte) ' ';
                    uBytes[up++] = (byte) (c ^ 0x80);
                }

                // codes 0x80...0xBf represent sequences
                else {
                    c <<= 8;
                    c += cBytes[cp++] & 0xff;
                    int m = (c & 0x3fff) >> COUNT_BITS;
                    int n = c & ((1 << COUNT_BITS) - 1);
                    n += COUNT_BITS;
                    while (n-- > 0) {
                        uBytes[up] = uBytes[up - m];
                        up++;
                    }
                }
            }

        } catch (ArrayIndexOutOfBoundsException e) {

            throw new IOException(
                res.getString("DOC_TEXT_RECORD_SIZE_EXCEEDED"));
        }

        // note that ubytes may be larger that the amount of
        // uncompressed bytes, so trim it to another byte array
        // with the exact size.
        byte[] textBytes = new byte[up];
        System.arraycopy(uBytes, 0, textBytes, 0, up);

        return textBytes;
    }


    /**
     *  Read the header <code>byte</code> array.
     *
     *  @param  bytes  <code>byte</code> array containing header
     *                 record data.
     *
     *  @return  <code>HeaderInfo</code> object.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private HeaderInfo readHeader(byte[] bytes) throws IOException {

        HeaderInfo header = new HeaderInfo();

        ByteArrayInputStream bis = new ByteArrayInputStream(bytes);
        DataInputStream dis = new DataInputStream(bis);

        // Normally the first 2 bytes comprised of the version
        // which should either be COMPRESSED or UNCOMPRESSED
        // SmartDoc/Quickword would add a 0x01 to the first
        // byte, thus their version would be 0x0101 for UNCOMPRESSED
        // instead of 0x0001 and 0x0102 for UNCOMPRESSED instead of
        // 0x0002.

        dis.readByte();
        header.version = dis.readByte();

        // read extra 2 unused bytes
        dis.readShort();

        // Read the text length, this should be unsigned 4 bytes.
        // We could store the read value into a long, but then
        // our current buffer limit is the max positive of an int.
        // That is a large enough limit, thus we shall stay with
        // storing the value in an int.  If it exceeds, then
        // an IOException should be thrown.
        header.textLen = dis.readInt();
        if (header.textLen < 0) {
            throw new IOException(res.getString("DOC_TEXT_LENGTH_EXCEEDED"));
        }

        // read the number of records - unsigned 2 bytes
        header.textRecordCount = dis.readShort() & 0x0000ffff;

        // read the record size - unsigned 2 bytes
        header.textRecordSize = dis.readShort() & 0x0000ffff;

        // read extra 4 unused bytes
        dis.readInt();

        return header;
    }


    /**
     *  Prints out header info into log. Used for debugging purposes only.
     *
     *  @param  header  <code>HeaderInfo</code> structure.
     */
    private void dumpHeader(HeaderInfo header) {

        log("<DOC_INFO ");
        log("version=\"" + header.version + "\" ");
        log("text-length=\"" + header.textLen + "\" ");
        log("number-of-records=\"" + header.textRecordCount + "\" ");
        log("record-size=\"" + header.textRecordSize  + "\" />");
    }


    /**
     *  Sends message to the log object.
     *
     *  @param  str  Debug string message.
     */
    private void log(String str) {
        Debug.log(Debug.TRACE, str);
    }


    /**
     *  Inner class to store AportisDoc header information.
     */
    private class HeaderInfo {

        /** length of text section */
        int textLen = 0;

        /** number of text records */
        int textRecordCount = 0;

        /**
         *  size of a text record.  This is normally the same as
         *  TEXT_RECORD_SIZE, but some applications may modify this.
         */
        int textRecordSize = 0;

        /** compression type */
        int version = 0;
    }
}

