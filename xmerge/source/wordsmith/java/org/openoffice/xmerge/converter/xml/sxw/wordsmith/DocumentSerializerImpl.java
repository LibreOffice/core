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

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import java.io.IOException;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.converter.palm.PalmDB;
import org.openoffice.xmerge.converter.palm.PdbEncoder;
import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.converter.palm.PdbUtil;
import org.openoffice.xmerge.converter.palm.PalmDocument;
import org.openoffice.xmerge.converter.xml.OfficeDocument;
import org.openoffice.xmerge.util.*;
import org.openoffice.xmerge.converter.xml.*;

/**
 *  <p>WordSmith implementation of
 *  org.openoffice.xmerge.DocumentSerializer
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxw.wordsmith.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>The <code>serialize</code> method traverses the DOM
 *  document from the given <code>Document</code> object.  It uses a
 *  <code>DocEncoder</code> object for the actual conversion of
 *  contents to the WordSmith format.</p>
 *
 *  @author      Herbie Ong, David Proulx
 */

// DJP: take out "implements OfficeConstants"
public final class DocumentSerializerImpl
implements OfficeConstants, DocumentSerializer {

    /**  A WSEncoder object for encoding to WordSmith. */
    private WSEncoder encoder = null;

    /**  The <code>StyleCatalog</code>. */
    private StyleCatalog styleCat = null;

    private WseFontTable fontTable = new WseFontTable();
    private WseColorTable colorTable = new WseColorTable();

    /**
     *  The <code>SxwDocument</code> object that this converter
     *  processes.
     */
    private SxwDocument sxwDoc = null;

    /**
     *  Constructor.
     *
     *  @param  doc  The <code>Document</code> to convert.
     */
    public DocumentSerializerImpl(Document doc) {
        sxwDoc = (SxwDocument) doc;
    }


    /**
     *  <p>Method to convert a <code>Document</code> into a
     *  <code>PalmDocument</code>.</p>
     *
     *  <p>This method is not thread safe for performance reasons.
     *  This method should not be called from within two threads.
     *  It would be best to call this method only once per object
     *  instance.</p>
     *
     *  <p>Note that the doc parameter needs to be an XML
     *  <code>Document</code>, else this method will throw a
     *  <code>ClassCastException</code>.  I think this is a hack,
     *  but this is the only way to not modify most of the existing
     *  code right now.</p>
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData serialize()
    throws IOException {


        // get the server document name
        String docName = sxwDoc.getName();

        // get DOM document
        org.w3c.dom.Document domDoc = sxwDoc.getContentDOM();

        // Create WordSmith encoder object.  Add WordSmith header,
        // empty font table to it.
        encoder = new WSEncoder();
        encoder.addElement(fontTable);
        encoder.addElement(colorTable);

        // Read the styles into the style catalog
        String families[] = new String[3];
        families[0] = "text";
        families[1] = "paragraph";
        families[2] = "paragraph";
        Class<?> classes[] = new Class[3];
        classes[0] = TextStyle.class;
        classes[1] = ParaStyle.class;
        classes[2] = TextStyle.class;
        styleCat = new StyleCatalog(25);

        // Parse the input document
        // DJP todo: eliminate multiple calls to add() when it can
        // recurse properly.
        NodeList nl = domDoc.getElementsByTagName(TAG_OFFICE_STYLES);
        styleCat.add(nl.item(0), families, classes, null, false);
        nl = domDoc.getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
        styleCat.add(nl.item(0), families, classes, null, false);
        nl = domDoc.getElementsByTagName(TAG_OFFICE_MASTER_STYLES);
        styleCat.add(nl.item(0), families, classes, null, false);

        // Traverse to the office:body element.
        // There should only be one.
        NodeList list = domDoc.getElementsByTagName(TAG_OFFICE_BODY);
        int len = list.getLength();
        if (len > 0) {
            Node node = list.item(0);
            traverseBody(node);
        }

        // create a PalmDB object and ConvertData object.
        //
        Record records[] = encoder.getRecords();

        ConvertData cd = new ConvertData();
        PalmDocument palmDoc = new PalmDocument(docName,
            PdbUtil.intID("WrdS"), PdbUtil.intID("BDOC"), 0,
            PalmDB.PDB_HEADER_ATTR_BACKUP, records);
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

                        Debug.log(Debug.INFO, "<OTHERS " /* + XmlDebug.nodeInfo(child) */ + " />");
                    }
                }
            }
        }

    }


    /**
     *  This method traverses the <i>text:p</i> and <i>text:h</i>
     *  element <code>Node</code> objects.
     *
     *  @param  node  A <i>text:p</i> or <i>text:h</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseParagraph(Node node) throws IOException {

        String styleName = findAttribute(node, "text:style-name");
        ParaStyle pstyle = (ParaStyle)styleCat.lookup(styleName, "paragraph",
        null, ParaStyle.class);

        // If the style does not exist in the style catalog for some reason,
        // make up a default style and use it.  We'll have to add this default
        // style to the style catalog the first time it is used.
        if (pstyle == null) {
            styleName = "CONVERTER-DEFAULT";
            pstyle = (ParaStyle)styleCat.lookup(styleName, "paragraph", null,
                                                ParaStyle.class);
            if (pstyle == null) {
                pstyle = new ParaStyle(styleName, "paragraph", null,
                   (String [])null, null, styleCat);
                styleCat.add(pstyle);
                styleCat.add(new TextStyle(styleName, "paragraph", null,
                         0, 0, 12, "Times-Roman", styleCat));
            }
        }

        pstyle = (ParaStyle)pstyle.getResolved();
        encoder.addElement(new WsePara(pstyle, styleCat));
        TextStyle defParaTextStyle = (TextStyle)
           styleCat.lookup(styleName, "paragraph", null, TextStyle.class);

        traverseParaContents(node, defParaTextStyle);
    }


    /**
     *  This method traverses a paragraph content.  Note that this
     *  method may recurse to call itself.
     *
     *  @param  node  A paragraph or content <code>Node</code>
     */
    private void traverseParaContents(Node node, TextStyle defTextStyle) {

        String styleName = findAttribute(node, "text:style-name");
        TextStyle style = (TextStyle)
           styleCat.lookup(styleName, "text", null, TextStyle.class);

        if (node.hasChildNodes()) {
            NodeList nodeList = node.getChildNodes();
            int nChildren = nodeList.getLength();

            for (int i = 0; i < nChildren; i++) {
                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.TEXT_NODE) {

                    // this is for grabbing text nodes.
                    String s = child.getNodeValue();

                    if (s.length() > 0) {
                        if (style != null)
                            encoder.addElement(new WseTextRun(s, style, styleCat,
                            fontTable, colorTable));
                        else
                            encoder.addElement(new WseTextRun(s, defTextStyle,
                            styleCat, fontTable, colorTable));
                    }

                } else if (child.getNodeType() == Node.ELEMENT_NODE) {

                    String childNodeName = child.getNodeName();

                    if (childNodeName.equals(TAG_SPACE)) {

                        // this is for text:s tags.
                        NamedNodeMap map = child.getAttributes();
                        Node attr = map.getNamedItem(ATTRIBUTE_SPACE_COUNT);
                        StringBuffer space = new StringBuffer(" ");
                        int count = 1;

                        if (attr != null) {
                            try {
                                String countStr = attr.getNodeValue();
                                count = Integer.parseInt(countStr.trim());
                            } catch (NumberFormatException e) {
                                Debug.log(Debug.ERROR, "Problem parsing space tag", e);
                            }
                        }

                        for (int j = 1; j < count; j++)
                            space.append(" ");

                        encoder.addElement(new WseTextRun(space.toString(),
                                                       defTextStyle,
                                                      styleCat, fontTable, colorTable));
                        Debug.log(Debug.INFO, "<SPACE count=\"" + count + "\" />");

                    } else if (childNodeName.equals(TAG_TAB_STOP)) {

                        // this is for text:tab-stop
                        encoder.addElement(new WseTextRun("\t", defTextStyle, styleCat,
                            fontTable, colorTable));

                        Debug.log(Debug.INFO, "<TAB/>");

                    } else if (childNodeName.equals(TAG_LINE_BREAK)) {

                        // this is for text:line-break
                        encoder.addElement(new WseTextRun("\n", defTextStyle,
                                            styleCat, fontTable, colorTable));

                        Debug.log(Debug.INFO, "<LINE-BREAK/>");

                    } else if (childNodeName.equals(TAG_SPAN)) {

                        // this is for text:span
                        Debug.log(Debug.INFO, "<SPAN>");
                        traverseParaContents(child, defTextStyle);
                        Debug.log(Debug.INFO, "</SPAN>");

                    } else if (childNodeName.equals(TAG_HYPERLINK)) {

                        // this is for text:a
                        Debug.log(Debug.INFO, "<HYPERLINK>");
                        traverseParaContents(child, defTextStyle);
                        Debug.log(Debug.INFO, "<HYPERLINK/>");

                    } else if (childNodeName.equals(TAG_BOOKMARK) ||
                    childNodeName.equals(TAG_BOOKMARK_START)) {

                        Debug.log(Debug.INFO, "<BOOKMARK/>");

                    } else {

                        Debug.log(Debug.INFO, "<OTHERS " /* + XmlDebug.nodeInfo(child) */ + " />");
                    }

                }

            }
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

        Debug.log(Debug.TRACE, "<LIST>");

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

                        Debug.log(Debug.ERROR, "<INVALID-XML-BUG " + " />");
                    }
                }
            }
        }

        Debug.log(Debug.TRACE, "</LIST>");
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

        Debug.log(Debug.TRACE, "<LIST-HEADER>");

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

                        Debug.log(Debug.TRACE, "<INVALID-XML-BUG " + " />");
                    }
                }
            }
        }

        Debug.log(Debug.TRACE, "</LIST-HEADER>");
    }


    /**
     *  This method will traverse a <i>text:list-item</i>.
     *  A list item may contain one or more of <i>text:p</i>,
     *  <i>text:h</i>, <i>text:section</i>,
     *  <i>text:ordered-list</i> and <i>text:unordered-list</i>.
     *
     *  This method currently only implements grabbing <i>text:p</i>,
     *  <i>text:h</i>, <i>text:unordered-list</i> and
     *  <i>text:ordered-list</i>.
     *
     *  @param  node  <code>Node</code> to traverse.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private void traverseListItem(Node node) throws IOException {

        Debug.log(Debug.TRACE, "<LIST-ITEM>");

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

                        Debug.log(Debug.ERROR, "<INVALID-XML-BUG " + " />");
                    }
                }
            }
        }

        Debug.log(Debug.TRACE, "</LIST-ITEM>");
    }


    /**
     *  Look up a <code>Node</code> object's named attribute and return
     *  its value
     *
     *  @param node  The <code>Node</code>.
     *  @param name  The attribute name.
     *
     *  @return  The value of the named attribute
     */
    private String findAttribute(Node node, String name) {
        NamedNodeMap attrNodes = node.getAttributes();
        if (attrNodes != null) {
            int len = attrNodes.getLength();
            for (int i = 0; i < len; i++) {
                Node attr = attrNodes.item(i);
                if (attr.getNodeName().equals(name))
                    return attr.getNodeValue();
            }
        }
        return null;
    }
}

