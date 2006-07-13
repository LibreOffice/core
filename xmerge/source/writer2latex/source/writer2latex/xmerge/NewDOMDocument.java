/************************************************************************
 *
 *  NewDOMDocument.java
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
 *  Copyright: 2002-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3g (2004-10-30)
 *
 */

package writer2latex.xmerge;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;

import org.openoffice.xmerge.converter.dom.DOMDocument;

import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.IOException;

/**
 *  An extension of <code>DOMDocument</code>
 *  that overrides the write method.
 *  (This method fails with the version of xerces shipped with jre 1.5)
 */
public class NewDOMDocument extends DOMDocument {

    /** Constructor
     */
    public NewDOMDocument(String sFileName, String sExtension) {
        super(sFileName,sExtension);
    }

    /**
     *  Write out content to the supplied <code>OutputStream</code>.
     *  (with pretty printing)
     *  @param  os  XML <code>OutputStream</code>.
     *  @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException {
        OutputStreamWriter osw = new OutputStreamWriter(os,"UTF-8");
        osw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
        write(getContentDOM().getDocumentElement(),0,osw);
        osw.flush();
        osw.close();
    }

    // Write nodes; we only need element, text and comment nodes
    private void write(Node node, int nLevel, OutputStreamWriter osw) throws IOException {
        short nType = node.getNodeType();
        switch (nType) {
            case Node.ELEMENT_NODE:
                if (node.hasChildNodes()) {
                    // Block pretty print from this node?
                    NodeList list = node.getChildNodes();
                    int nLen = list.getLength();
                    boolean bBlockPrettyPrint = false;
                    if (nLevel>=0) {
                        for (int i = 0; i < nLen; i++) {
                            bBlockPrettyPrint |= list.item(i).getNodeType()==Node.TEXT_NODE;
                        }
                    }
                    // Print start tag
                    if (nLevel>=0) { writeSpaces(nLevel,osw); }
                    osw.write("<"+node.getNodeName());
                    writeAttributes(node,osw);
                    osw.write(">");
                    if (nLevel>=0 && !bBlockPrettyPrint) { osw.write("\n"); }
                    // Print children
                    for (int i = 0; i < nLen; i++) {
                        int nNextLevel;
                        if (bBlockPrettyPrint || nLevel<0) { nNextLevel=-1; }
                        else { nNextLevel=nLevel+1; }
                        write(list.item(i),nNextLevel,osw);
                    }
                    // Print end tag
                    if (nLevel>=0 && !bBlockPrettyPrint) { writeSpaces(nLevel,osw); }
                    osw.write("</"+node.getNodeName()+">");
                    if (nLevel>=0) { osw.write("\n"); }
                }
                else { // empty element
                    if (nLevel>=0) { writeSpaces(nLevel,osw); }
                    osw.write("<"+node.getNodeName());
                    writeAttributes(node,osw);
                    osw.write(" />");
                    if (nLevel>=0) { osw.write("\n"); }
                }
                break;
            case Node.TEXT_NODE:
                write(node.getNodeValue(),osw);
                break;
            case Node.COMMENT_NODE:
                if (nLevel>=0) { writeSpaces(nLevel,osw); }
                osw.write("<!-- ");
                write(node.getNodeValue(),osw);
                osw.write(" -->");
                if (nLevel>=0) { osw.write("\n"); }
        }
    }

    private void writeAttributes(Node node, OutputStreamWriter osw) throws IOException {
        NamedNodeMap attr = node.getAttributes();
        int nLen = attr.getLength();
        for (int i=0; i<nLen; i++) {
            Node item = attr.item(i);
            osw.write(" ");
            write(item.getNodeName(),osw);
            osw.write("=\"");
            write(item.getNodeValue(),osw);
            osw.write("\"");
        }
    }

    private void writeSpaces(int nCount, OutputStreamWriter osw) throws IOException {
        for (int i=0; i<nCount; i++) { osw.write("  "); }
    }

    private void write(String s, OutputStreamWriter osw) throws IOException {
        int nLen = s.length();
        char c;
        for (int i=0; i<nLen; i++) {
            c = s.charAt(i);
            switch (c) {
                case ('<'): osw.write("&lt;"); break;
                case ('>'): osw.write("&gt;"); break;
                case ('&'): osw.write("&amp;"); break;
                case ('"'): osw.write("&quot;"); break;
                case ('\''): osw.write( "&apos;"); break;
                default: osw.write(c);
            }
        }
    }

}








