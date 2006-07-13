/************************************************************************
 *
 *  IndexMark.java
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
 *  Version 0.3.1 (2004-01-02)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;

import writer2latex.util.*;

/**
 *  <p>This class contains static methods to read an index-mark.</p>
 */
public final class IndexMark {

    // Node must be text:alphabetical-index-mark or text:alphabetical-index-mark-start
    public final static String getIndexValue(Node node) {
        if (XMLString.TEXT_ALPHABETICAL_INDEX_MARK.equals(node.getNodeName())) {
            return Misc.getAttribute(node,XMLString.TEXT_STRING_VALUE);
        }
        else {
            return collectMark(node);
        }
    }

    // Node must be text:alphabetical-index-mark or text:alphabetical-index-mark-start
    public final static String getKey1(Node node) {
        return Misc.getAttribute(node,XMLString.TEXT_KEY1);
    }

    // Node must be text:alphabetical-index-mark or text:alphabetical-index-mark-start
    public final static String getKey2(Node node) {
        return Misc.getAttribute(node,XMLString.TEXT_KEY2);
    }

    // Collect a mark
    private final static Node getRightNode(Node node) {
        Node nextNode;
        do {nextNode = node.getNextSibling();
            if (nextNode!=null) { return nextNode; }
            node = node.getParentNode();
        } while (node!=null);
        return null;
    }

    private final static String collectMark(Node node) {
        StringBuffer buf = new StringBuffer();
        String sId = Misc.getAttribute(node,XMLString.TEXT_ID);
        node = getRightNode(node);
        while (node!=null) {
            if (node.getNodeType()==Node.TEXT_NODE) {
                buf.append(node.getNodeValue());
                node = getRightNode(node);
            }
            else if (node.getNodeType()==Node.ELEMENT_NODE) {
               boolean bReady = false;
               String sNodeName = node.getNodeName();
               if (sId.equals(Misc.getAttribute(node,XMLString.TEXT_ID))) {
                   node = null; // found the end mark
                   bReady = true;
               }
               else if (sNodeName.startsWith("text:")
                        && !sNodeName.equals(XMLString.TEXT_FOOTNOTE)
                        && !sNodeName.equals(XMLString.TEXT_ENDNOTE)) {
                   if (node.hasChildNodes()) {
                       node = node.getFirstChild(); bReady=true;
                   }
               }
               if (!bReady) { node=getRightNode(node); };
            }
        }
        return buf.toString();
    }


}