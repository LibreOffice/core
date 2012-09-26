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

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;

import java.io.IOException;
import java.net.URLDecoder;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.converter.xml.sxw.aportisdoc.DocConstants;
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

        String docName = URLDecoder.decode(sxwDoc.getName(), DocConstants.ENCODING);

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

