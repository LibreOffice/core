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
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.converter.palm.PalmDB;
import org.openoffice.xmerge.converter.palm.PalmDocument;
import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.ParaStyle;
import org.openoffice.xmerge.converter.xml.Style;
import org.openoffice.xmerge.converter.xml.StyleCatalog;
import org.openoffice.xmerge.converter.xml.TextStyle;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.XmlUtil;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;

/**
 *  <p>WordSmith implementation of
 *  org.openoffice.xmerge.DocumentDeserializer
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxw.wordsmith.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  The <code>deserialize</code> method uses a
 *  <code>DocDecoder</code> to read the WordSmith format into a
 *  <code>String</code> object, then it calls <code>buildDocument</code>
 *  to create a <code>SxwDocument</code> object from it.
 *
 */
public final class DocumentDeserializerImpl
implements DOCConstants, OfficeConstants, DocumentDeserializer {

    /** A Decoder object for decoding WordSmith format. */
    private WSDecoder decoder = null;

    WseFontTable fontTable = null;
    WseColorTable colorTable = null;
    StyleCatalog styleCat = null;
    StyleCatalog oldStyleCat = null;

    /** A <code>ConvertData</code> object assigned to this object. */
    private ConvertData cd = null;


    /**
     *  Constructor that assigns the given <code>ConvertData</code>
     *  to the object.
     *
     *  @param  cd  A <code>ConvertData</code> object to read data for
     *              the conversion process by the deserialize method.
     */
    public DocumentDeserializerImpl(ConvertData cd) {
        this.cd = cd;
    }


    /**
     *  Convert the given <code>ConvertData</code> into a
     *  <code>SxwDocument</code> object.
     *
     *  @return  Resulting <code>Document</code> object.
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public Document deserialize() throws ConvertException,
        IOException {
        return deserialize(null, cd);
    }


    public Document deserialize(Document origDoc, ConvertData cd)
    throws IOException {

        Document doc         = null;
        PalmDocument palmDoc = null;
        Iterator<Object> e        = cd.getDocumentEnumeration();

        while(e.hasNext()) {
            palmDoc        = (PalmDocument) e.next();
            PalmDB pdb     = palmDoc.getPdb();
            Record[] recs  = pdb.getRecords();
            decoder        = new WSDecoder();
            Wse[] b        = decoder.parseDocument(recs);
            String docName = palmDoc.getName();
            doc            = buildDocument(docName, b, origDoc);
        }
        return doc;
    }


    /**
     *  Temporary method to read existing <code>StyleCatalog</code>
     *  as a starting point.
     *
     *  @param  parentDoc  The parent <code>Document</code>.
     */
    private void readStyleCatalog(Document parentDoc) {
        try {
            java.io.ByteArrayOutputStream bos = new java.io.ByteArrayOutputStream();
            parentDoc.write(bos);
            SxwDocument sxwDoc = new SxwDocument("old");
            sxwDoc.read(new ByteArrayInputStream(bos.toByteArray()));
            org.w3c.dom.Document domDoc = sxwDoc.getContentDOM();

            String families[] = new String[3];
            families[0] = "text";
            families[1] = "paragraph";
            families[2] = "paragraph";
            Class<?> classes[] = new Class<?>[3];
            classes[0] = TextStyle.class;
            classes[1] = ParaStyle.class;
            classes[2] = TextStyle.class;

            NodeList nl = domDoc.getElementsByTagName(TAG_OFFICE_STYLES);
            oldStyleCat.add(nl.item(0), families, classes, null, false);
            nl = domDoc.getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
            oldStyleCat.add(nl.item(0), families, classes, null, false);
            nl = domDoc.getElementsByTagName(TAG_OFFICE_MASTER_STYLES);
            oldStyleCat.add(nl.item(0), families, classes, null, false);

        } catch (Exception e) {
            Debug.log(Debug.ERROR, "", e);
        }

    }


    /**
     *  Given an array of paragraph <code>Style</code> objects, see if
     *  there is exactly one which matches the text formatting
     *  <code>Style</code> of <code>tStyle</code>.
     *
     *  @param  paraStyles  An array of paragraph <code>Style</code>
     *                      objects.
     *  @param  tStyle      Text <code>Style</code> to match.
     *
     *  @return  The paragraph <code>Style</code> that matches.
     */
    private ParaStyle matchParaByText(Style paraStyles[], TextStyle tStyle) {
        int matchIndex = -1;
    int matchCount = 0;
    Style txtMatches[] = oldStyleCat.getMatching(tStyle);
    if (txtMatches.length >= 1) {
        for (int j = 0; j < txtMatches.length; j++) {
            TextStyle t = (TextStyle)txtMatches[j];

            if (!t.getFamily().equals("paragraph"))
                continue;

            for (int k = 0; k < paraStyles.length; k++) {
                if (t.getName().equals(paraStyles[k].getName())) {
                    matchCount++;
                matchIndex = k;
                }
            }
        }
    }
    if (matchCount == 1)
            return (ParaStyle)paraStyles[matchIndex];
        else return null;
    }


    /**
     *  Take a <code>String</code> of text and turn it into a sequence
     *  of <code>Node</code> objects.
     *
     *  @param  text       <code>String</code> of text.
     *  @param  parentDoc  Parent <code>Document</code>.
     *
     *  @return  Array of <code>Node</code> objects.
     */
    private Node[] parseText(String text, org.w3c.dom.Document parentDoc) {
        ArrayList<Node> nodeVec = new ArrayList<Node>();

        // Break up the text from the WordSmith text run into Open
        // Office text runs.  There may be more runs in OO because
        // runs of 2 or more spaces map to nodes.
        while ((text.indexOf("  ") != -1) || (text.indexOf("\t") != 1)) {

            // Find the indices of tabs and multiple spaces, and
            // figure out which of them occurs first in the string.
            int spaceIndex = text.indexOf("  ");
            int tabIndex = text.indexOf("\t");
            if ((spaceIndex == -1) && (tabIndex == -1))
                break;  // DJP This should not be necessary.  What is wrong
            // with the while() stmt up above?
            int closerIndex;  // Index of the first of these
            if (spaceIndex == -1)
                closerIndex = tabIndex;
            else if (tabIndex == -1)
                closerIndex = spaceIndex;
            else
                closerIndex = (spaceIndex > tabIndex) ? tabIndex : spaceIndex;

            // If there is any text prior to the first occurrence of a
            // tab or spaces, create a text node from it, then chop it
            // off the string we're working with.
            if (closerIndex > 0) {
                String beginningText = text.substring(0, closerIndex);
                Text textNode = parentDoc.createTextNode(beginningText);
                nodeVec.add(textNode);
                log("<TEXT>");
                log(beginningText);
                log("</TEXT>");
            }
            text = text.substring(closerIndex);

            // Handle either tab character or space sequence by creating
            // an element for it, and then chopping out the text that
            // represented it in "text".
            if (closerIndex == tabIndex) {
                Element tabNode = parentDoc.createElement(TAG_TAB_STOP);
                nodeVec.add(tabNode);
                text = text.substring(1);  // tab is always a single character
                log("<TAB/>");
            } else {
                // Compute length of space sequence.
                int nrSpaces = 2;
                while ((nrSpaces < text.length())
                && text.substring(nrSpaces, nrSpaces + 1).equals(" "))
                    nrSpaces++;

                Element spaceNode = parentDoc.createElement(TAG_SPACE);
                spaceNode.setAttribute(ATTRIBUTE_SPACE_COUNT, new Integer(nrSpaces).toString());
                nodeVec.add(spaceNode);
                text = text.substring(nrSpaces);
                log("<SPACE count=\"" + nrSpaces + "\" />");
            }
        }

        // No more tabs or space sequences.  If there's any remaining
        // text create a text node for it.
        if (text.length() > 0) {
            Text textNode = parentDoc.createTextNode(text);
            nodeVec.add(textNode);
            log("<TEXT>");
            log(text);
            log("</TEXT>");
        }

        // Now create and populate an array to return the nodes in.
        Node nodes[] = new Node[nodeVec.size()];
        for (int i = 0; i < nodeVec.size(); i++)
            nodes[i] = nodeVec.get(i);
        return nodes;
    }


    /**
     *  Parses the text content of a WordSmith format and builds a
     *  <code>SXWDocument</code>.
     *
     *  @param  docName  <code>Document</code> name
     *  @param  data      Text content of WordSmith format
     *
     *  @return  Resulting <code>SXWDocument</code> object.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private SxwDocument buildDocument(String docName, Wse[] data, Document origDoc)
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

        styleCat = new StyleCatalog(50);
        oldStyleCat = new StyleCatalog(50);
           if (origDoc != null)
             readStyleCatalog(origDoc);

        Element currPara = null;
        ParaStyle currParaStyle = null;
        int newTextStyleNr = 0;
        int newParaStyleNr = 0;

        // Now write out the document body by running through
        // the list of WordSmith elements and processing each one
        // in turn.
        for (int i = 0; i < data.length; i++) {

            if (data[i].getClass() == WsePara.class) {

                currPara = doc.createElement(TAG_PARAGRAPH);
                log("</PARA>");
                log("<PARA>");

                WsePara p = (WsePara)data[i];

                // Save info about the first text run, if there is one.
                WseTextRun firstTextRun = null;

                if ((data.length >= i + 2)
                && (data[i+1].getClass() == WseTextRun.class))
                    firstTextRun = (WseTextRun)data[i+1];

                Style matches[] = oldStyleCat.getMatching(p.makeStyle());

                // See if we can find a unique match in the catalog
                // of existing styles from the original document.
                ParaStyle pStyle = null;
                if (matches.length == 1) {
                    pStyle = (ParaStyle)matches[0];
                    log("using an existing style");
                } else if ((matches.length > 1) && (firstTextRun != null)) {
                    pStyle = matchParaByText(matches, firstTextRun.makeStyle());
                    log("resolved a para by looking @ text");
                }

                // If nothing found so far, try looking in the catalog
                // of newly-created styles.
                // DJP FIXME: if we need to add two para styles with the
                // same para formatting info but different default text
                // styles, this won't work!
                if (pStyle == null) {
                    log("had " + matches.length + " matches in old catalog");
                    matches = styleCat.getMatching(p.makeStyle());
                    if (matches.length == 0) {
                        pStyle = p.makeStyle();
                        String newName = new String("PPP" + ++newParaStyleNr);
                        pStyle.setName(newName);
                        styleCat.add(pStyle);
                        // DJP: write in the text format info here
                        log("created a new style");
                    } else if (matches.length == 1) {
                        pStyle = (ParaStyle)matches[0];
                        log("re-using a new style");
                    } else if (firstTextRun != null) {
                        pStyle = matchParaByText(matches, firstTextRun.makeStyle());
                        if (pStyle != null) {
                            log("resolved a (new) para by looking @ text");
                    } else
                            log("Hey this shouldn't happen! - nr of matches is "
                            + matches.length);
                    }
                }

                if (pStyle == null)
                    log("Unable to figure out a para style");

                // Figured out a style to use.  Specify the style in this
                // paragraph's attributes.
                currPara.setAttribute(ATTRIBUTE_TEXT_STYLE_NAME, pStyle.getName());

                bodyNode.appendChild(currPara);
                currParaStyle = pStyle;
            } else if (data[i].getClass() == WseTextRun.class) {
                WseTextRun tr = (WseTextRun)data[i];
                TextStyle trStyle = null;
                Node trNodes[] = parseText(tr.getText(), doc);

                // First see if the formatting of this text run matches
                // the default text formatting for this paragraph.  If
                // it does, then just make the text node(s) children of
                // the current paragraph.
                Style[] cps = new Style[1];
                cps[0] = currParaStyle;
                if (matchParaByText(cps, tr.makeStyle()) != null) {
                    for (int ii  = 0; ii < trNodes.length; ii++) {
                        currPara.appendChild(trNodes[ii]);
                    }
                    continue;
             }

                // Check for existing, matching styles in the old style
                // catalog.  If exactly one is found, use it.  Otherwise,
                // check the new style catalog, and either use the style
                // found or add this new one to it.
                Style matches[] = oldStyleCat.getMatching(tr.makeStyle());
                if (matches.length == 1)
                    trStyle = (TextStyle)matches[0];
                else {
                    matches = styleCat.getMatching(tr.makeStyle());
                    if (matches.length == 0) {
                        trStyle = tr.makeStyle();
                        String newName = new String("TTT" + ++newTextStyleNr);
                        trStyle.setName(newName);
                        styleCat.add(trStyle);
                    } else if (matches.length == 1)
                        trStyle = (TextStyle)matches[0];
                    else
                        log("multiple text style matches from new catalog");
                }

                // Create a text span node, set the style attribute, make the
                // text node(s) its children, and append it to current paragraph's
                // list of children.
                Element textSpanNode = doc.createElement(TAG_SPAN);
                textSpanNode.setAttribute(ATTRIBUTE_TEXT_STYLE_NAME, trStyle.getName());
                for (int ii  = 0; ii < trNodes.length; ii++) {
                    textSpanNode.appendChild(trNodes[ii]);
                }
                currPara.appendChild(textSpanNode);
                log("</SPAN>");
            }

            else if (data[i].getClass() == WseFontTable.class) {
                fontTable = (WseFontTable)data[i];
            }

            else if (data[i].getClass() == WseColorTable.class) {
                colorTable = (WseColorTable)data[i];
            }
        }


        //NodeList r = doc.getElementsByTagName(TAG_OFFICE_DOCUMENT);
        NodeList r = doc.getElementsByTagName(TAG_OFFICE_DOCUMENT_CONTENT);
        Node rootNode = r.item(0);

        // read the original document
        org.w3c.dom.NodeList nl;
        if (origDoc != null) {
            java.io.ByteArrayOutputStream bos = new java.io.ByteArrayOutputStream();
            origDoc.write(bos);
            SxwDocument origSxwDoc = new SxwDocument("old");
            origSxwDoc.read(new ByteArrayInputStream(bos.toByteArray()));
            org.w3c.dom.Document origDomDoc = origSxwDoc.getContentDOM();

            org.w3c.dom.DocumentFragment df;
            org.w3c.dom.Node newNode;

            // copy font declarations from original document to the new document
            nl = origDomDoc.getElementsByTagName(TAG_OFFICE_FONT_DECLS);
            df = doc.createDocumentFragment();
            newNode = XmlUtil.deepClone(df, nl.item(0));
            rootNode.insertBefore(newNode, bodyNode);

            // copy style catalog from original document to the new document
            nl = origDomDoc.getElementsByTagName(TAG_OFFICE_STYLES);
            df = doc.createDocumentFragment();
            newNode = XmlUtil.deepClone(df, nl.item(0));
            rootNode.insertBefore(newNode, bodyNode);

            nl = origDomDoc.getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
            df = doc.createDocumentFragment();
            newNode = XmlUtil.deepClone(df, nl.item(0));
            rootNode.insertBefore(newNode, bodyNode);

            nl = origDomDoc.getElementsByTagName(TAG_OFFICE_MASTER_STYLES);
            df = doc.createDocumentFragment();
            newNode = XmlUtil.deepClone(df, nl.item(0));
            rootNode.insertBefore(newNode, bodyNode);
        }

        // Original document not specified.  We need to add font declarations.
        // DJP: this might just be for debugging.  Merger will probably put
        // the "real" ones in.
        // DJP: if really doing it this way, do it right: gather font names
        // from style catalog(s).
        else {
            org.w3c.dom.Node declNode;

            log("<FONT-DECLS/>");

            declNode = doc.createElement(TAG_OFFICE_FONT_DECLS);
            rootNode.insertBefore(declNode, bodyNode);
            org.w3c.dom.Element fontNode;

            fontNode = doc.createElement(TAG_STYLE_FONT_DECL);
            fontNode.setAttribute(ATTRIBUTE_STYLE_NAME, "Arial");
            fontNode.setAttribute(ATTRIBUTE_FO_FONT_FAMILY, "Arial");
            fontNode.setAttribute(ATTRIBUTE_STYLE_FONT_PITCH, "variable");
            declNode.appendChild(fontNode);

            fontNode = doc.createElement(TAG_STYLE_FONT_DECL);
            fontNode.setAttribute(ATTRIBUTE_STYLE_NAME, "Arioso");
            fontNode.setAttribute(ATTRIBUTE_FO_FONT_FAMILY, "Arioso");
            fontNode.setAttribute(ATTRIBUTE_STYLE_FONT_PITCH, "variable");
            declNode.appendChild(fontNode);
        }


        // Now add any new styles we have created in this document.
        nl = doc.getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
        Node autoStylesNode = nl.item(0);
        if (autoStylesNode == null) {
            autoStylesNode = doc.createElement(TAG_OFFICE_AUTOMATIC_STYLES);
            log("<OFFICE-AUTOMATIC-STYLES/>");
            rootNode.insertBefore(autoStylesNode, bodyNode);
        }

        Node newStyleCatNode = styleCat.writeNode(doc, "dummy");
        nl = newStyleCatNode.getChildNodes();
        int nNodes = nl.getLength();
        for (int i = 0; i < nNodes; i++) {
            autoStylesNode.appendChild(nl.item(0));
        }

        oldStyleCat.dumpCSV(true);
        styleCat.dumpCSV(true);
        return sxwDoc;
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

