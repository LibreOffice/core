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

package org.openoffice.xmerge.converter.xml.sxw.pocketword;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.NamedNodeMap;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentSerializer;

import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;

import org.openoffice.xmerge.converter.xml.ParaStyle;
import org.openoffice.xmerge.converter.xml.TextStyle;
import org.openoffice.xmerge.converter.xml.StyleCatalog;

import java.io.IOException;

/**
 * <p>Pocket Word implementation of <code>DocumentDeserializer</code>
 * for use by {@link
 * org.openoffice.xmerge.converter.xml.sxw.pocketword.PluginFactoryImpl
 * PluginFactoryImpl}.</p>
 *
 * <p>This converts an OpenOffice Writer XML files to a Pocket Word file<.</p>
 *
 * @version 1.1
 */
public final class DocumentSerializerImpl
            implements DocumentSerializer, OfficeConstants {

    private PocketWordDocument pswDoc;
    private SxwDocument        sxwDoc;

    private StyleCatalog styleCat = null;

    private boolean inList = false;

    /**
     *  <p>Initialises a new <code>DocumentSerializerImpl</code> using the.<br>
     *     supplied <code>Document</code></p>
     *
     * <p>The supplied document should be an {@link
     *    org.openoffice.xmerge.converter.xml.sxw.SxwDocument SxwDocument}
     *    object.</p>
     *
     *  @param  doc  The <code>Document</code> to convert.
     */
    public DocumentSerializerImpl(Document doc) {
        sxwDoc = (SxwDocument)doc;
        pswDoc = new PocketWordDocument(sxwDoc.getName());
    }

    /**
     *  <p>Convert the data passed into the <code>DocumentSerializerImpl</code>
     *  constructor into Pocket Word format.</p>
     *
     *  <p>This method may or may not be thread-safe.  It is expected
     *  that the user code does not call this method in more than one
     *  thread.  And for most cases, this method is only done once.</p>
     *
     *  @return  <code>ConvertData</code> object to pass back the
     *           converted data.
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData serialize() throws IOException, ConvertException {
        ConvertData cd = new ConvertData();

        org.w3c.dom.Document doc = sxwDoc.getContentDOM();

        // Load any style info before traversing the document content tree
        loadStyles();

        NodeList list = doc.getElementsByTagName(TAG_OFFICE_BODY);

        int len = list.getLength();
        if (len > 0) {
            Node node = list.item(0);
            traverseBody(node);
        }

        cd.addDocument(pswDoc);

        return cd;
    }

    /*
     * Handles the loading of defined styles from the style.xml file as well
     * as automatic styles from the content.xml file.
     *
     * Any change to a defined style, such as a short bold section, falls into
     * the latter category.
     */
    private void loadStyles() {
        org.w3c.dom.Document contentDom = sxwDoc.getContentDOM();
        org.w3c.dom.Document styleDom   = sxwDoc.getStyleDOM();

        styleCat = new StyleCatalog(25);

        NodeList nl = null;
        String families[] = new String[] { PocketWordConstants.TEXT_STYLE_FAMILY,
                                           PocketWordConstants.PARAGRAPH_STYLE_FAMILY,
                                           PocketWordConstants.PARAGRAPH_STYLE_FAMILY };
        Class<?> classes[]   = new Class[] { TextStyle.class,
                                          ParaStyle.class,
                                          TextStyle.class };

        String[] styleTypes = new String[] { TAG_OFFICE_STYLES,
                                             TAG_OFFICE_AUTOMATIC_STYLES,
                                             TAG_OFFICE_MASTER_STYLES };

        /*
         * Documents converted from PSW -> SXW will not have a style.xml when
         * being converted back to PSW.  This would occur if a document was
         * not modified within Writer between conversions.
         *
         * Any Writer modifications and saves create the style.xml and other
         * portions of a complete Writer SXW file.
         */
        if (styleDom != null) {
           // Process the Style XML tree
           for (int i = 0; i < styleTypes.length; i++ ) {
               nl = styleDom.getElementsByTagName(styleTypes[i]);
               if (nl.getLength() != 0) {
                   styleCat.add(nl.item(0), families, classes, null, false);
               }
           }
        }

        /*
         * Process the content XML for any other style info.
         * Should only be automatic types here.
         */
        for (int i = 0; i < styleTypes.length; i++ ) {
            nl = contentDom.getElementsByTagName(styleTypes[i]);
            if (nl.getLength() != 0) {
                styleCat.add(nl.item(0), families, classes, null, false);
            }
        }
    }

    /*
     * Process the office:body tag.
     */
    private void traverseBody(Node node) throws IOException, ConvertException {

        if (node.hasChildNodes()) {
            NodeList nList = node.getChildNodes();
            int len = nList.getLength();

            for (int i = 0; i < len; i++) {
                Node child = nList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_PARAGRAPH)
                            || nodeName.equals(TAG_HEADING)) {
                        traverseParagraph(child);
                    }

                    if (nodeName.equals(TAG_UNORDERED_LIST) ||
                        nodeName.equals(TAG_ORDERED_LIST)) {
                        traverseList(child);
                    }
                }
            }
        }
    }

    /*
     * Process a text:p tag
     */
    private void traverseParagraph(Node node) throws IOException, ConvertException {
        String styleName = getAttribute(node, ATTRIBUTE_TEXT_STYLE_NAME);

        ParaStyle pstyle = (ParaStyle)styleCat.lookup(styleName,
                                PocketWordConstants.PARAGRAPH_STYLE_FAMILY, null,
                                ParaStyle.class);
        if (pstyle != null) {
            pstyle = (ParaStyle)pstyle.getResolved();
        }

        TextStyle tstyle = (TextStyle)styleCat.lookup(styleName,
                                PocketWordConstants.PARAGRAPH_STYLE_FAMILY, null,
                                TextStyle.class);
        if (pstyle != null) {
            tstyle = (TextStyle)tstyle.getResolved();
        }

        try {
            pswDoc.addParagraph(pstyle, inList);
        }
        catch (Exception e) {
            throw new ConvertException(
                                "Error adding paragraph to PocketWordDocument.\n"
                                + e.toString());
        }

        traverseParagraphContents(node, tstyle);
    }

    /*
     * Process the contents of a paragraph.  This method handles situations
     * where the paragraph contains multiple children, each representing a
     * differently formatted piece of text.
     */
    private void traverseParagraphContents (Node node, TextStyle defTextStyle)
        throws IOException, ConvertException {
        // First up, get the style of this little bit
        String styleName = getAttribute(node, ATTRIBUTE_TEXT_STYLE_NAME);
        TextStyle tStyle = (TextStyle)styleCat.lookup(styleName,
                                PocketWordConstants.TEXT_STYLE_FAMILY, null,
                                TextStyle.class);

        if (tStyle == null) {
            tStyle = defTextStyle;
        }

        if (node.hasChildNodes()) {
            NodeList nList = node.getChildNodes();
            int len = nList.getLength();

            for (int i = 0; i < len; i++) {

                Node child = nList.item(i);
                short nodeType = child.getNodeType();

                switch (nodeType) {
                    case Node.TEXT_NODE:
                        String s = child.getNodeValue();
                        if (s.length() > 0) {
                            try {
                                pswDoc.addParagraphData(s, tStyle);
                            }
                            catch (Exception e) {
                                throw new ConvertException(
                                    "Error adding data to paragraph in " +
                                    "PocketWordDocument.\n" + e.toString());

                            }
                        }
                        break;

                    case Node.ELEMENT_NODE:
                        if (child.getNodeName().equals(TAG_SPACE)) {
                            StringBuffer sb = new StringBuffer("");
                            int count = 1;

                            NamedNodeMap map = child.getAttributes();

                            if (map.getLength() > 0) {
                                Node attr = map.getNamedItem(ATTRIBUTE_SPACE_COUNT);
                                count = Integer.parseInt(attr.getNodeValue().trim());
                            }

                            for ( ; count > 0; count--) {
                                sb.append(" ");
                            }

                            /*
                             * May want to look at style info for spaces.  Could
                             * be important when calculating font metrics.
                             */
                            try {
                                pswDoc.addParagraphData(sb.toString(), tStyle);
                            }
                            catch (Exception e) {
                                throw new ConvertException(
                                    "Error adding data to paragraph in " +
                                    "PocketWordDocument.\n" + e.toString());

                            }
                        }
                        else if (child.getNodeName().equals(TAG_TAB_STOP)) {
                            try {
                                pswDoc.addParagraphData("\t", tStyle);
                            }
                            catch (Exception e) {
                                throw new ConvertException(
                                    "Error adding data to paragraph in " +
                                    "PocketWordDocument.\n" + e.toString());

                            }
                        }
                        else if (child.getNodeName().equals(TAG_LINE_BREAK)) {
                            /*
                             * Pocket Word does not support soft line breaks.
                             * They are just new paragraphs.
                             */
                        }
                        else if (child.getNodeName().equals(TAG_SPAN)) {
                            /*
                             * This is where the interesting ones, i.e. format
                             * changes occur.
                             */
                            traverseParagraphContents (child, defTextStyle);
                        }
                        else if (child.getNodeName().equals(TAG_HYPERLINK)) {
                            traverseParagraphContents (child, defTextStyle);
                        }
                        else {
                            // Should maybe have a default in here.
                        }
                        break;
                    default:
                        // Do nothing
                }
            }
        }
        else {
            /*
             * If the node has no children, then it is a blank paragraph, but
             * they still require an entry in the Paragraph class to make sense.
             */
            pswDoc.addParagraphData("", tStyle);
        }
    }

    /*
     * Process a text:ordered-list or text:unordered-list tag.  Pocket Word has
     * no concept of a list so there is no need to differentiate between the
     * two.
     *
     * Each item on the list contains a text:p node.
     */
    private void traverseList (Node node) throws IOException, ConvertException {
        inList = true;

        if (node.hasChildNodes()) {
            NodeList nList = node.getChildNodes();
            int len = nList.getLength();

            for (int i = 0; i < len; i++) {
                Node child = nList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_LIST_ITEM)) {
                        traverseListItem(child);
                    }
                }
            }
        }

        inList = false;
    }

    /*
     * Process a text:list-item node.  They usually contain have a single
     * text:p child but can also have sections or other lists.
     *
     * For this case, only paragraphs are supported.
     */
    private void traverseListItem (Node node) throws IOException, ConvertException {
        if (node.hasChildNodes()) {
            NodeList nList = node.getChildNodes();
            int len = nList.getLength();

            for (int i = 0; i < len; i++) {
                Node child = nList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_PARAGRAPH)) {
                        traverseParagraph(child);
                    }
                }
            }
        }

    }

    /*
     * Utility method to retrieve a Node attribute.
     */
    private String getAttribute (Node node, String attribute) {
        NamedNodeMap attrNodes = node.getAttributes();

        if (attrNodes != null) {
            Node attr = attrNodes.getNamedItem(attribute);
            if (attr != null) {
                return attr.getNodeValue();
            }
        }

        return null;
    }
}
