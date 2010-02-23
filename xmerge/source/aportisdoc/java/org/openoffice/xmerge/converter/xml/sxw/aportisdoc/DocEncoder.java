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

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;

import org.openoffice.xmerge.converter.palm.Record;

/**
 *  This class is used by {@link
 *  org.openoffice.xmerge.converter.xml.sxw.DocumentSerializerImpl
 *  DocumentSerializerImpl} to encode the AportisDoc format.
 *  It does not deal with any XML tags.  It only knows how to encode
 *  from <code>String</code>.
 *
 *  @author   Herbie Ong
 */
final class DocEncoder implements DocConstants {

    /** Text buffer to contain text section. */
    private StringBuffer textBuffer = null;

    /** Length of text section. */
    private int textLen = 0;

    /** Number of text records. */
    private int textRecCount = 0;


    /**
     *  Default constructor creates a header and
     *  a text buffer for holding all the text in
     *  the AportisDoc database.
     */
    DocEncoder() {

        textBuffer = new StringBuffer(TEXT_RECORD_SIZE);
    }


    /**
     *  This method appends text into the text section of
     *  the AportisDoc database.
     *
     *  @param  text  <code>String</code> to append.
     */
    void addText(String text) {

        textBuffer.append(text);
    }


    /**
     *  This method appends text into the text section of
     *  the AportisDoc database.
     *
     *  @param  text  <code>char</code> array to append.
     */
    void addText(char[] text) {

        textBuffer.append(text);
    }


    /**
     *  This method appends text character into the text
     *  section of the AportisDoc database.
     *
     *  @param  text  <code>char</code> to append.
     */
    void addText(char text) {

        textBuffer.append(text);
    }


    /**
     *  This method encodes the information given to a
     *  palm <code>Record</code> array in the AportisDoc
     *  database format.
     *
     *  @return  <code>Record</code> array holding AportisDoc
     *           contents.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    Record[] getRecords() throws IOException {

        byte textBytes[] = processTextBuffer();
        textLen = textBytes.length;
        textRecCount = (short) (textBytes.length / TEXT_RECORD_SIZE);

        // recBytes to hold a record of bytes at a time
        byte recBytes[] = new byte[TEXT_RECORD_SIZE];
        int pos = 0;

        List textRecords = new ArrayList(textRecCount + 1);

        // split textBytes into chunks of Record objects
        // and store in textRecords object.
        for (int i = 0; i < textRecCount; i++) {

            System.arraycopy(textBytes, pos, recBytes, 0, recBytes.length);
            pos += recBytes.length;
            Record zRec = new Record(recBytes);
            textRecords.add(zRec);
        }

        // there's more if ...

        if (pos < textLen) {

            textRecCount++;

            recBytes = new byte[textLen - pos];
            System.arraycopy(textBytes, pos, recBytes, 0, recBytes.length);
            Record rec = new Record(recBytes);
            textRecords.add(rec);
        }

        // construct the Record array and copy
        // references from textRecords.

        Record[] allRecords = new Record[textRecords.size() + 1];

        allRecords[0] = new Record(getHeaderBytes());

        for (int i = 1; i < allRecords.length; i++) {

            allRecords[i] = (Record) textRecords.get(i-1);
        }

        return allRecords;
    }


    /**
     *  This method converts the text buffer into a <code>byte</code>
     *  array with the proper encoding of the text section of the
     *  AportisDoc format.
     *
     *  TODO: do compression.
     *
     *  @return  byte[]  Converted <code>byte</code> array of text
     *                   section.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private byte[] processTextBuffer() throws IOException
    {
        String str = textBuffer.toString();
        byte bytes[] = str.getBytes(ENCODING);

        return bytes;
    }


    /**
     *  This method produces the <code>byte</code> array for the header.
     *
     *  @return  <code>byte</code> array containing header record data.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private byte[] getHeaderBytes() throws IOException
    {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(bos);

        // TODO:  for now, we shall use UNCOMPRESSED.
        // later, we need to use COMPRESSED or a setting.
        dos.writeShort(UNCOMPRESSED);
        dos.writeShort(SPARE);
        dos.writeInt(textLen);
        dos.writeShort(textRecCount);
        dos.writeShort(TEXT_RECORD_SIZE);
        dos.writeInt(SPARE);

        byte[] bytes = bos.toByteArray();

        return bytes;
    }
}

