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

package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;

import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.Resources;

/**
 *  This class is used by {@link
 *  org.openoffice.xmerge.converter.xml.sxw.wordsmith.DocumentDeserializerImpl
 *  DocumentDeserializerImpl} to decode a WordSmith format.  It currently
 *  decodes the text content into a single <code>String</code> object.
 *
 *  @author   Herbie Ong, David Proulx
 */
final class WSDecoder implements DOCConstants {

    /** For decoding purposes. */
    private final static int COUNT_BITS = 3;

    /** Resources object for I18N. */
    private Resources res = null;

    /**
     *  Default constructor creates a header and
     *  a text buffer for holding all the text in
     *  the DOC db.
     */
    WSDecoder() {
        res = Resources.getInstance();
    }

    /**
     *  Decode the text records into a single <code>byte</code> array.
     *
     *  @param  recs  <code>Record</code> array holding WordSmith
     *                  contents.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    byte[] parseRecords(Record[] recs) throws IOException {

        // read the header record
        HeaderInfo header = readHeader(recs[0].getBytes());
        dumpHeader(header);
        byte[][] byteArrays = new byte[recs.length - 1][];
        for (int i = 0; i < recs.length - 1; i++) byteArrays[i] = null;

        switch (header.version & ~4) {  // DJP: "4" indicates OOB data is present.
        // Add a constant to handle this, might also need code to handle it.

            case COMPRESSED:
            case 3:   // DJP: determined this empirically.  Are Herbie's constants wrong?
                for (int i = 1; i < recs.length; i++) {
                    byteArrays[i-1] = decompress(recs[i].getBytes(),
                    header.textRecordSize);
                    Debug.log(Debug.INFO, "processing " + byteArrays[i-1].length + " bytes");
                }

                break;

            case UNCOMPRESSED:
                for (int i = 1; i < recs.length; i++) {
                    byteArrays[i-1] = recs[i].getBytes();
                    Debug.log(Debug.INFO, "processing " + byteArrays[i-1].length + " bytes");
                }

                break;

            default:
                throw new IOException(res.getString("UNKNOWN_DOC_VERSION"));

        }

        // Concatenate byteArrays[][] into a single byte array.
        int length = 0;
        for (int i = 0; i < recs.length - 1; i++)
            length += byteArrays[i].length;
        byte bigArray[] = new byte[length];
        int offset = 0;
        for (int i = 0; i < recs.length - 1; i++) {
            System.arraycopy(byteArrays[i], 0, bigArray, offset,
            byteArrays[i].length);
            offset += byteArrays[i].length;
        }
        return bigArray;
    }


    /**
     *  Decode the text records into a <code>Wse</code> array.
     *
     *  @param  recs  <code>Record</code> array holding DOC
     *                    contents.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    Wse[] parseDocument(Record[] recs) throws IOException {

        java.util.ArrayList<Wse> v = new java.util.ArrayList<Wse>(20);
        WseFontTable fontTable = null;
        WseColorTable colorTable = null;

        // rawData is the document data to be parsed.
        byte rawData[] = parseRecords(recs);

        // beginning of document has some header information, including
        // optional font and color tables.
        // DJP: maybe should add a new WSelement (docHeader) to hold
        // header info.
        // DJP: finish code here to parse header
        if (rawData[0] != 2) throw new IOException();
        int nParagraphs = util.intFrom4bytes(rawData, 2);
        int nAtoms      = util.intFrom4bytes(rawData, 6);
        int nChars      = util.intFrom4bytes(rawData, 10);
        int miscSize    = util.intFrom4bytes(rawData, 14);
        int curIndex = 18;

        while (curIndex < rawData.length) {
            if (WsePara.isValid(rawData, curIndex)) {
                v.add(new WsePara(rawData, curIndex));
                curIndex = WsePara.computeNewIndex(rawData, curIndex);
            } else if (WseTextRun.isValid(rawData, curIndex)) {
                v.add(new WseTextRun(rawData, curIndex, fontTable, colorTable));
                curIndex = WseTextRun.computeNewIndex(rawData, curIndex);
            } else if (WseFontTable.isValid(rawData, curIndex)) {
                fontTable = new WseFontTable(rawData, curIndex);
                v.add(fontTable);
                curIndex = WseFontTable.computeNewIndex(rawData, curIndex);
            } else if (WseColorTable.isValid(rawData, curIndex)) {
                colorTable = new WseColorTable(rawData, curIndex);
                v.add(colorTable);
                curIndex = WseColorTable.computeNewIndex(rawData, curIndex);
            } else {
                Debug.log(Debug.ERROR, "Unknown code " + rawData[curIndex]);
                throw new IOException();
            }
        }

        return v.toArray(new Wse[2]);
    }


    /**
     *  <p>Decompress the <code>byte</code> array.</p>
     *
     *  <p>The resulting uncompressed <code>byte</code> array
     *  should be within <code>textRecordSize</code> length,
     *  definitely within twice the size it claims, else treat
     *  it as a problem with the encoding of that PDB and
     *  throw <code>IOException</code>.</p>
     *
     *  @param  cBytes           Compressed <code>byte</code> array
     *  @param  textRecordSize  Size of uncompressed <code>byte</code>
     *                          array
     *
     *  @throws   IOException  If <code>textRecordSize</codeL &lt;
     *                         <code>cBytes.length</code>.
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
        dis.readShort();

        // read the record size - unsigned 2 bytes
        header.textRecordSize = dis.readShort() & 0x0000ffff;

        // read extra 4 unused bytes
        dis.readInt();

        return header;
    }


    /**
     *  Prints out header info into log.
     *  Used for debugging purposes only.
     *
     *  @param  header  <code>HeaderInfo</code> structure.
     */
    private void dumpHeader(HeaderInfo header) {
    /*
        log("<DOC_INFO ");
        log("version=\"" + header.version + "\" ");
        log("text-length=\"" + header.textLen + "\" ");
        log("number-of-records=\"" + header.textRecordCount + "\" ");
        log("record-size=\"" + header.textRecordSize  + "\" />\n");
    */
    }


    /**
     *  Inner class to store DOC header information.
     */
    private class HeaderInfo {

        /** length of text section */
        int textLen = 0;

        /**
         *  size of a text record.  This is normally the same as
         *  TEXT_RECORD_SIZE, but some applications may modify this.
         */
        int textRecordSize = 0;

        /** compression type */
        int version = 0;
    }
}

