/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

