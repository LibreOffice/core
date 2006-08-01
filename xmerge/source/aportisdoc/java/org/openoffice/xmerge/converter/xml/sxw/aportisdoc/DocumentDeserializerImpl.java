/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentDeserializerImpl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 12:40:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.ArrayList;

import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.util.Resources;
import org.openoffice.xmerge.util.Debug;

/**
 *  This class is used by {@link
 *  org.openoffice.xmerge.converter.xml.sxw.DocumentDeserializerImpl}
 *  to decode the AportisDoc format.  It currently decodes
 *  the text content into a single <code>String</code> object.
 *
 *  @author   Herbie Ong
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
     *  @param  Record  <code>Record</code> array holding AportisDoc
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
     *  @param  bytes           Compressed <code>byte</code> array.
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
        header.textRecordCount = ((int) dis.readShort()) & 0x0000ffff;

        // read the record size - unsigned 2 bytes
        header.textRecordSize = ((int) dis.readShort()) & 0x0000ffff;

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

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.ArrayList;

import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.util.Resources;
import org.openoffice.xmerge.util.Debug;

/**
 *  This class is used by {@link
 *  org.openoffice.xmerge.converter.xml.sxw.DocumentDeserializerImpl}
 *  to decode the AportisDoc format.  It currently decodes
 *  the text content into a single <code>String</code> object.
 *
 *  @author   Herbie Ong
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
     *  @param  Record  <code>Record</code> array holding AportisDoc
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
     *  @param  bytes           Compressed <code>byte</code> array.
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
        header.textRecordCount = ((int) dis.readShort()) & 0x0000ffff;

        // read the record size - unsigned 2 bytes
        header.textRecordSize = ((int) dis.readShort()) & 0x0000ffff;

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

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.Text;

import java.io.IOException;
import java.util.Enumeration;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.converter.palm.PalmDB;
import org.openoffice.xmerge.converter.palm.PdbDecoder;
import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.converter.palm.PalmDocument;
import org.openoffice.xmerge.util.Debug;

/**
 *  <p>AportisDoc implementation of <code>DocumentDeserializer</code>
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxw.aportisdoc.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>This converts an file in AportisDoc PDB format to StarOffice
 *  XML format.</p>
 *
 *  <p>The <code>deserialize</code> method uses a <code>DocDecoder</code>
 *  to read the AportisDoc format into a <code>String</code> object, then
 *  it calls <code>buildDocument</code> to create a <code>SxwDocument</code>
 *  object from it.</p>
 *
 *  @author      Herbie Ong
 */
public final class DocumentDeserializerImpl
    implements OfficeConstants, DocConstants, DocumentDeserializer {

    /**  A <code>ConvertData</code> object assigned to this object. */
    private ConvertData cd = null;


    /**
     *  Constructor that assigns the given <code>ConvertData</code>
     *  to this object as input.
     *
     *  @param  cd  A <code>ConvertData</code> object to read data for
     *              the conversion process by the <code>deserialize</code>
     *              method.
     */
    public DocumentDeserializerImpl(ConvertData cd) {
        this.cd = cd;
    }


    /**
     *  Convert the given <code>ConvertData</code> object
     *  into a <code>SxwDocument</code> object.
     *
     *  @return  Resulting <code>SxwDocument</code> object.
     *
     *  @throws  ConvertException   If any conversion error occurs.
     *  @throws  IOException        If any I/O error occurs.
     */
    public Document deserialize() throws IOException, ConvertException {

        int numberOfPDBs = cd.getNumDocuments();
        Document doc = null;
        int i=0;
        ConvertData cdOut;
        Enumeration e = cd.getDocumentEnumeration();
        while (e.hasMoreElements()) {
            PalmDocument palmDoc = (PalmDocument) e.nextElement();
            PalmDB pdb = palmDoc.getPdb();

            log("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            log("<AportisDoc>");

            Record[] recs = pdb.getRecords();
            String docName = palmDoc.getName();
            DocDecoder decoder = new DocDecoder();
            String text = decoder.parseRecords(recs);
            doc = buildDocument(docName, text);

            log("</AportisDoc>");
        }

        return doc;
    }


    /**
     *  Parses the text content of an AportisDoc format and build a
     *  <code>SxwDocument</code>.
     *
     *  @param  docName  Name of <code>Document</code>.
     *  @param  str      Text content of AportisDoc format.
     *
     *  @return  Resulting <code>SxwDocument</code> object.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private SxwDocument buildDocument(String docName, String str)
        throws IOException {

        // create minimum office xml document.
        SxwDocument sxwDoc = new SxwDocument(docName);
        sxwDoc.initContentDOM();

        org.w3c.dom.Document doc = sxwDoc.getContentDOM();

        // Grab hold of the office:body tag,
        // Assume there should be one.
        // This is where top level paragraphs will append to.
        NodeList list = doc.getElementsByTagName(TAG_OFFICE_BODY);
        Node bodyNode = list.item(0);

        // Store all the text in a character array.
        char[] text = str.toCharArray();

        // startIndex has 2 purposes:
        // if value is -1, it means that there are no text characters
        // needed to be processed for a Text node.  if value >= 0, it
        // is the index of the starting position of a text section
        // for a Text node.
        int startIndex = -1;

        // Create a paragraph node to start with.
        Element paraNode = doc.createElement(TAG_PARAGRAPH);

        log("<PARA>");

        for (int i = 0; i < text.length; i++) {

            switch (text[i]) {

                case TAB_CHAR:

                    // Check if there are text to be processed first.
                    if (startIndex >= 0) {
                        addTextNode(doc, paraNode, text, startIndex, i - 1);
                        startIndex = -1;
                    }

                    // Then, add tab element.
                    Element tabNode = doc.createElement(TAG_TAB_STOP);
                    paraNode.appendChild(tabNode);

                    log("<TAB/>");
                    break;

                case EOL_CHAR:

                    // Check if there are text to be processed first.
                    if (startIndex >= 0) {
                        addTextNode(doc, paraNode, text, startIndex, i - 1);
                        startIndex = -1;
                    }

                    // Then, add the current paragraph to body.
                    bodyNode.appendChild(paraNode);

                    // Create another paragraph element.
                    paraNode = doc.createElement(TAG_PARAGRAPH);

                    log("</PARA>");
                    log("<PARA>");
                    break;

                case SPACE_CHAR:

                    // count is the number of space chars from i
                    int count = 0;

                    // Do a look ahead and count the number of space chars
                    while (text[i + 1 + count] == SPACE_CHAR) {
                        count++;
                    }

                    // Need to build a space node ONLY if count is > 1.

                    if (count > 0) {

                        // Check if there are text to be processed first
                        if (startIndex >= 0) {
                            addTextNode(doc, paraNode, text,
                                        startIndex, i);
                            startIndex = -1;
                        }

                        // Then, create a space element
                        // with the proper attribute.
                        Element spaceNode = doc.createElement(TAG_SPACE);
                        spaceNode.setAttribute(ATTRIBUTE_SPACE_COUNT,
                            Integer.toString(count));

                        paraNode.appendChild(spaceNode);

                        // reposition i to the last space character.
                        i += count;

                        log("<SPACE count=\"" + count + "\" />");

                    } else {

                        // If there are no chars for text node yet,
                        // consider this one.
                        if (startIndex < 0) {

                            startIndex = i;
                            log("<TEXT>");
                        }
                    }

                    break;

                default:

                    // If there are no chars for text node yet,
                    // this should be the start.
                    if (startIndex < 0) {

                        startIndex = i;
                        log("<TEXT>");
                    }

                    break;
            }
        }

        int lastIndex = text.length - 1;

        // Check if there are text to be processed first.

        if (startIndex >= 0) {
            addTextNode(doc, paraNode, text, startIndex, lastIndex);
        }

        // Then, add the last paragraph element if it is not added yet.
        if (text[lastIndex] != EOL_CHAR) {
            bodyNode.appendChild(paraNode);
        }

        log("</PARA>");

        return sxwDoc;
    }


    /**
     *  Add a Text <code>Node</code> to the given paragraph node with the
     *  text starting at the given <code>startPos</code> until
     *  <code>endPos</code>.
     *
     *  @param  doc       <code>org.w3c.dom.Document</code> object for creating
     *                    <code>Node</code> objects.
     *  @param  para      The current paragraph <code>Node</code> to append
     *                    text <code>Node</code>.
     *  @param  text      Array of characters containing text.
     *  @param  startPos  Starting index position for text value.
     *  @param  endPos    End index position for text value.
     */
    private void addTextNode(org.w3c.dom.Document doc, Node para, char text[],
        int startPos, int endPos) {

        String str = new String(text, startPos, endPos - startPos + 1);
        Text textNode = doc.createTextNode(str);
        para.appendChild(textNode);
        log(str);
        log("</TEXT>");
    }

    /**
     *  Sends message to the log object.
     *
     *  @param  str  Debug message.
     */
    private void log(String str) {

        Debug.log(Debug.TRACE, str);
    }
}

