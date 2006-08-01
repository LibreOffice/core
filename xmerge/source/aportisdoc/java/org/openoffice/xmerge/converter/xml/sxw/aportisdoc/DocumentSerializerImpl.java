/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentSerializerImpl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 12:41:01 $
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
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import java.io.IOException;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.converter.palm.PdbEncoder;
import org.openoffice.xmerge.converter.palm.PdbDecoder;
import org.openoffice.xmerge.converter.palm.PalmDB;
import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.converter.palm.PalmDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.XmlUtil;

/**
 *  <p>AportisDoc implementation of
 *  org.openoffice.xmerge.DocumentSerializer
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxw.aportisdoc.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>The <code>serialize</code> method traverses the DOM
 *  document from the given <code>Document</code> object.  It uses a
 *  <code>DocEncoder</code> object for the actual conversion of
 *  contents to the AportisDoc format.</p>
 *
 *  @author      Herbie Ong
 */


public final class DocumentSerializerImpl
    implements OfficeConstants, DocConstants, DocumentSerializer {

    /** A <code>DocEncoder</code> object for encoding to AportisDoc. */
    private DocEncoder encoder = null;

    /** SXW <code>Document</code> object that this converter processes. */
    private SxwDocument sxwDoc = null;


    /**
     *  Constructor.
     *
     *  @param  doc  A SXW <code>Document</code> to be converted.
     */
    public DocumentSerializerImpl(Document doc) {
        sxwDoc = (SxwDocument) doc;
    }


    /**
     *  <p>Method to convert a <code>Document</code> into a PDB.
     *  It passes back the converted data as a <code>ConvertData</code>
     *  object.</p>
     *
     *  <p>This method is not thread safe for performance reasons.
     *  This method should not be called from within two threads.
     *  It would be best to call this method only once per object
     *  instance.</p>
     *
     *  @return  The <code>ConvertData</code> object containing the output.
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData serialize() throws ConvertException, IOException {


        // get the server document name

        String docName = sxwDoc.getName();

        // get DOM document

        org.w3c.dom.Document domDoc = sxwDoc.getContentDOM();

        encoder = new DocEncoder();

        // Traverse to the office:body element.
        // There should only be one.

        NodeList list = domDoc.getElementsByTagName(TAG_OFFICE_BODY);
        int len = list.getLength();

        if (len > 0) {
            Node node = list.item(0);
            traverseBody(node);
        }

        // create a ConvertData object.
        //
        Record records[] = encoder.getRecords();
        ConvertData cd = new ConvertData();

        PalmDocument palmDoc = new PalmDocument(docName,
            DocConstants.CREATOR_ID, DocConstants.TYPE_ID,
            0, PalmDB.PDB_HEADER_ATTR_BACKUP, records);

        cd.addDocument(palmDoc);
        return cd;
    }


    /**
     *  This method traverses <i>office:body</i> element.
     *
     *  @param  node  <i>office:body</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseBody(Node node) throws IOException {

        log("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        log("<AportisDOC>");

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {
                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_PARAGRAPH) ||
                        nodeName.equals(TAG_HEADING)) {

                        traverseParagraph(child);

                    } else if (nodeName.equals(TAG_UNORDERED_LIST)) {

                        traverseList(child);

                    } else if (nodeName.equals(TAG_ORDERED_LIST)) {

                        traverseList(child);

                    } else {

                        log("<OTHERS " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }
        }

        log("</AportisDOC>");
    }


    /**
     *  This method traverses the <i>text:p</i> and <i>text:h</i>
     *  element <code>Node</code> objects.
     *
     *  @param  node  A <i>text:p</i> or <i>text:h</i>
     *                <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseParagraph(Node node) throws IOException {

        log("<PARA>");
        traverseParaContents(node);
        encoder.addText(EOL_CHAR);
        log("</PARA>");
    }


    /**
     *  This method traverses a paragraph content.
     *  It uses the <code>traverseParaElem</code> method to
     *  traverse into Element <code>Node</code> objects.
     *
     *  @param  node  A paragraph or content <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseParaContents(Node node) throws IOException {

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {

                Node child = nodeList.item(i);
                short nodeType = child.getNodeType();

                switch (nodeType) {

                    case Node.TEXT_NODE:
                        // this is for grabbing text nodes.
                        String s = child.getNodeValue();

                        if (s.length() > 0) {
                            encoder.addText(s);
                        }

                        log("<TEXT>");
                        log(s);
                        log("</TEXT>");

                        break;

                    case Node.ELEMENT_NODE:

                        traverseParaElem(child);
                        break;

                    case Node.ENTITY_REFERENCE_NODE:

                        log("<ENTITY_REFERENCE>");
                        traverseParaContents(child);
                        log("<ENTITY_REFERENCE/>");
                        break;

                    default:
                        log("<OTHERS " + XmlUtil.getNodeInfo(node) + " />");
                }
            }
        }
    }


    /**
     *  This method traverses an <code>Element</code> <code>Node</code>
     *  within a paragraph.
     *
     *  @param  node  <code>Element</code> <code>Node</code> within a
     *                paragraph.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseParaElem(Node node) throws IOException {

        String nodeName = node.getNodeName();

        if (nodeName.equals(TAG_SPACE)) {

            // this is for text:s tags.
            NamedNodeMap map = node.getAttributes();
            Node attr = map.getNamedItem(ATTRIBUTE_SPACE_COUNT);
            StringBuffer space = new StringBuffer(SPACE_CHAR);
            int count = 1;

            if (attr != null) {

                try {

                    String countStr = attr.getNodeValue();
                    count = Integer.parseInt(countStr.trim());

                } catch (NumberFormatException e) {

                    // TODO: for now, throw IOException.
                    // later, perhaps will have to throw
                    // some other conversion exception instead.
                    throw new IOException(e.getMessage());
                }
            }

            for (int j = 0; j < count; j++) {

                space.append(SPACE_CHAR);
            }

            encoder.addText(space.toString());

            log("<SPACE count=\"" + count + "\" />");

        } else if (nodeName.equals(TAG_TAB_STOP)) {

            // this is for text:tab-stop
            encoder.addText(TAB_CHAR);

            log("<TAB/>");

        } else if (nodeName.equals(TAG_LINE_BREAK)) {

            // commented out by Csaba: There is no point to convert a linebreak
            // into a EOL, because it messes up the number of XML nodes and the
            // merge won't work properly. Other solution would be to implement such
            // nodemerger, which would be able to merge embedded tags in a paragraph

            // this is for text:line-break
            // encoder.addText(EOL_CHAR);

            log("skipped <LINE-BREAK/>");

        } else if (nodeName.equals(TAG_SPAN)) {

            // this is for text:span
            log("<SPAN>");
            traverseParaContents(node);
            log("</SPAN>");

        } else if (nodeName.equals(TAG_HYPERLINK)) {

            // this is for text:a
            log("<HYPERLINK>");
            traverseParaContents(node);
            log("<HYPERLINK/>");

        } else if (nodeName.equals(TAG_BOOKMARK) ||
                   nodeName.equals(TAG_BOOKMARK_START)) {

            log("<BOOKMARK/>");

        } else if (nodeName.equals(TAG_TEXT_VARIABLE_SET)
                   || nodeName.equals(TAG_TEXT_VARIABLE_GET)
                   || nodeName.equals(TAG_TEXT_EXPRESSION)
                   || nodeName.equals(TAG_TEXT_USER_FIELD_GET)
                   || nodeName.equals(TAG_TEXT_PAGE_VARIABLE_GET)
                   || nodeName.equals(TAG_TEXT_SEQUENCE)
                   || nodeName.equals( TAG_TEXT_VARIABLE_INPUT)
                   || nodeName.equals(TAG_TEXT_TIME)
                   || nodeName.equals( TAG_TEXT_PAGE_COUNT)
                   || nodeName.equals(TAG_TEXT_PAGE_NUMBER )
                   || nodeName.equals(TAG_TEXT_SUBJECT)
                   || nodeName.equals(TAG_TEXT_TITLE)
                   || nodeName.equals(TAG_TEXT_CREATION_TIME)
                   || nodeName.equals(TAG_TEXT_DATE)
                   || nodeName.equals(TAG_TEXT_TEXT_INPUT)
                   || nodeName.equals(TAG_TEXT_AUTHOR_INITIALS)) {
            log("<FIELD>");
            traverseParaContents(node);
            log("</FIELD>");

        }else if (nodeName.startsWith(TAG_TEXT)) {
            log("<Unknown text Field>");
            traverseParaContents(node);
            log("</Unknown text Field>");

        }else {

            log("<OTHERS " + XmlUtil.getNodeInfo(node) + " />");
        }
    }


    /**
     *  This method traverses list tags <i>text:unordered-list</i> and
     *  <i>text:ordered-list</i>.  A list can only contain one optional
     *  <i>text:list-header</i> and one or more <i>text:list-item</i>
     *  elements.
     *
     *  @param  node  A list <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseList(Node node) throws IOException {

        log("<LIST>");

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {

                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {

                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_LIST_ITEM)) {

                        traverseListItem(child);

                    } else if (nodeName.equals(TAG_LIST_HEADER)) {

                        traverseListHeader(child);

                    } else {

                        log("<INVALID-XML-BUG " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }
        }

        log("</LIST>");
    }


    /**
     *  This method traverses a <i>text:list-header</i> element.
     *  It contains one or more <i>text:p</i> elements.
     *
     *  @param  node  A list header <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseListHeader(Node node) throws IOException {

        log("<LIST-HEADER>");

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {

                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {

                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_PARAGRAPH)) {

                        traverseParagraph(child);

                    } else {

                        log("<INVALID-XML-BUG " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }
        }

        log("</LIST-HEADER>");
    }


    /**
     *  <p>This method will traverse a <i>text:list-item</i>.
     *  A list item may contain one or more of <i>text:p</i>,
     *  <i>text:h</i>, <i>text:section</i>, <i>text:ordered-list</i>
     *  and <i>text:unordered-list</i>.</p>
     *
     *  <p>This method currently only implements grabbing <i>text:p</i>,
     *  <i>text:h</i>, <i>text:unordered-list</i> and
     *  <i>text:ordered-list</i>.</p>
     *
     *  @param  node  The <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseListItem(Node node) throws IOException {

        log("<LIST-ITEM>");

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {

                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {

                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_PARAGRAPH)) {

                        traverseParagraph(child);

                    } else if (nodeName.equals(TAG_UNORDERED_LIST)) {

                        traverseList(child);

                    } else if (nodeName.equals(TAG_ORDERED_LIST)) {

                        traverseList(child);

                    } else {

                        log("<INVALID-XML-BUG " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }
        }

        log("</LIST-ITEM>");
    }


    /**
     *  Logs debug messages.
     *
     *  @param  str  The debug message.
     */
    private void log(String str) {

        Debug.log(Debug.TRACE, str);
    }
}

