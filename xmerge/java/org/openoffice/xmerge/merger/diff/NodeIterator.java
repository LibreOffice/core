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

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.Resources;

import java.util.Vector;
import java.util.List;


/**
 *  <p>This is an implementation of the <code>Iterator</code> interface.
 *  It will traverse the tree and find <code>Node</code> sequences.</p>
 *
 *  <p>Note: Once the XML Tree is parsed, then the <code>Iterator</code> will
 *  be a snap shot of that tree. That means even the tree is modified later,
 *  than the cached paragraph <code>Node</code> list will not be updated
 *  accordingly.  For this reason and for performance reasons this
 *  <code>Iterator</code> does not support any operation methods such as
 *  insert, remove or replace.  The main purpose of this
 *  <code>Iterator</code> is to be used with difference, not with merge.</p>
 *
 *  @author smak
 */
public abstract class NodeIterator implements Iterator {

    private List nodeList = null;
    private int currentPosition = 0;
    private Node root;
    private ConverterCapabilities cc_ = null;


    /**
     *  Standard constructor.
     *
     *  @param  cc    The <code>ConverterCapabilities</code>.
     *  @param  node  The initial root <code>Node</code>.
     */
    public NodeIterator(ConverterCapabilities cc, Node node) {
        cc_ = cc;
        nodeList = new Vector();
        root = node;
        markTree(node);
    }


    public Object next() {
        if (currentPosition < nodeList.size() - 1) {
            currentPosition++;
            return currentElement();
        } else {
            return null;
        }
    }


    public Object previous() {
        if (currentPosition > 0) {
            currentPosition--;
            return currentElement();
        } else {
            return null;
        }
    }


    public Object start() {
        currentPosition = 0;
        return currentElement();
    }


    public Object end() {
        int size = nodeList.size();

        if (size > 0) {
            currentPosition = size - 1;
            return currentElement();
        } else  {
            return null;
        }
    }


    public Object currentElement() {

        if (currentPosition < 0 || currentPosition >= nodeList.size()) {
            return null;
        }

        return nodeList.get(currentPosition);
    }


    public int elementCount() {
        return nodeList.size();
    }


    public boolean equivalent(Object obj1, Object obj2) {
        boolean equal = false;
        String errMsg = null;
        if (!(obj1 instanceof Node && obj2 instanceof Node)) {
            errMsg = Resources.getInstance().getString("NOT_NODE_ERROR");
            Debug.log(Debug.ERROR, errMsg);
        } else {
            Node node1 = (Node)obj1;
            Node node2 = (Node)obj2;

            equal = compareNode(node1, node2);
        }
        return equal;
    }


    public void refresh() {
        nodeList = new Vector();
        markTree(root);
        currentPosition = 0;
    }


    /**
     *  Used to compare two <code>Node</code> objects (type/name/value)
     *  and all their children <code>Node</code> objects.
     *
     *  @param  node1  The first <code>Node</code> to compare.
     *  @param  node2  The second <code>Node</code> to compare.
     *
     *  @return  true if <code>Node</code> is equal, false otherwise.
     */
    protected boolean compareNode(Node node1, Node node2) {
        boolean equal = false;

        nodeCheck: {

            if (node1 == null || node2 == null) {
                break nodeCheck;
            }

            // nodevalue is short
            if (node1.getNodeType() != node2.getNodeType()) {
                break nodeCheck;
            }

            // nodeName will not be null
            if (!node1.getNodeName().equals(node2.getNodeName())) {
                break nodeCheck;
            }

            // nodeValue can be null for a lot of type of cells
            if (node1.getNodeValue() == null && node2.getNodeValue() == null) {
                // empty
            } else if (node1.getNodeValue() == null ||
                       node2.getNodeValue() == null) {
                break nodeCheck;
            } else if (!node1.getNodeValue().equals(node2.getNodeValue())) {
                break nodeCheck;
            }

            // try to compare attributes
            if (!attributesEqual(node1, node2)) {
                break nodeCheck;
            }

            // don't need to compare if both node do not have children
            if (!node1.hasChildNodes() && !node2.hasChildNodes()) {
                equal = true;
                break nodeCheck;
            // don't need to compare if one node has children but not the other
            } else if (!node1.hasChildNodes() || !node2.hasChildNodes()) {
                equal = false;
                break nodeCheck;
            // need to compare if both node has children
            } else if (!childrenEqual(node1, node2)) {
                break nodeCheck;
            }

            equal = true;
        }

        return equal;
    }


    /**
     *  Compare the children of two <code>Node</code> objects.  This
     *  method can be intentionally overridden by any class that
     *  extend from <code>NodeIterator</code> so that it can have
     *  its own children comparison if necessary.
     *
     *  @param  node1  The first <code>Node</code> to compare.
     *  @param  node2  The second <code>Node</code> to compare.
     *
     *  @return  true if children are equal, false otherwise.
     */
    protected boolean childrenEqual(Node node1, Node node2) {

        boolean equal = false;

        childrenCheck: {
            NodeList node1Children = node1.getChildNodes();
            NodeList node2Children = node2.getChildNodes();

            if (node1Children == null || node2Children == null) {
                break childrenCheck;
            }

            if (node1Children.getLength() != node2Children.getLength())  {
                break childrenCheck;
            }

            // compare all the childrens
            equal = true;

            for (int i = 0; i < node1Children.getLength(); i++) {
                if (!compareNode(node1Children.item(i),
                                 node2Children.item(i))) {
                    equal = false;
                    break childrenCheck;
                }
            }
        }

        return equal;
    }


    /**
     *  Compare attributes of two <code>Node</code> objects.  This
     *  method can be intentionally overridden by any class that
     *  extends from <code>NodeIterator</code> so that it can have
     *  its own attribute comparison.
     *
     *  @param  node1  The first <code>Node</code> to compare.
     *  @param  node2  The second <code>Node</code> to compare.
     *
     *  @return  true if attributes are equal, false otherwise.
     */
    protected boolean attributesEqual(Node node1, Node node2) {

        boolean equal = false;
        String nodeName = node1.getNodeName();
        NamedNodeMap attrNode[] = new NamedNodeMap[2];
        attrNode[0] = node1.getAttributes();
        attrNode[1] = node2.getAttributes();

        // attribute node will be null if node is not an element node
        // and attribute nodes are equal if both are not element node
        if (attrNode[0] == null || attrNode[1] == null) {
            if (attrNode[0] == null && attrNode[1] == null) {
                equal = true;
            }
            return equal;
        }

        // compare the attributes from node1 vs node2 and node2 vs node1
        // though it's a little inefficient for the duplication of comparison
        // as the number of attributes is not so many, it should not be
        // a big problem.
        int len [] = new int[2];
        int src, dst;

        attrCheck: {
            for (int i = 0; i < 2; i++) {

                if (i == 0) {
                    src = 0;
                    dst = 1;
                } else {
                    src = 1;
                    dst = 0;
                }

                len[src] = attrNode[src].getLength();

                for (int j = 0; j < len[src]; j++) {
                    Node srcAttr = attrNode[src].item(j);
                    String srcAttrName = srcAttr.getNodeName();

                    // copy the supported attrs
                    if (cc_ == null ||
                        cc_.canConvertAttribute(nodeName, srcAttrName)) {

                        // check whether the attribute exist in dst node
                        Node dstAttr = attrNode[dst].getNamedItem(srcAttrName);

                        if (dstAttr == null)  {
                            Debug.log(Debug.INFO,
                                      "[NodeIterator] Attr not exist in dst - "
                                      + srcAttrName);
                            break attrCheck;
                        }

                        // then compare the attribute values
                        if (!srcAttr.getNodeValue().equals(
                             dstAttr.getNodeValue())) {
                            Debug.log(Debug.INFO,
                                      "[NodeIterator] Attr diff src: " +
                                      srcAttr.getNodeValue() + " dst: "+
                                      dstAttr.getNodeValue());
                            break attrCheck;
                        }
                    } // end if cc_ loop
                } // end for j loop
            } // end for i loop

            // the whole checking is done smoothly and all attributes are equal
            equal = true;
        }

        return equal;
    }


    /**
     *  Check whether a <code>Node</code> is supported.  This method
     *  can be intentionally overridden by any class that extends from
     *  <code>NodeIterator</code> so that it can specify which
     *  <code>Node</code> to support.
     *
     *  @param  node  <code>Node</code> to check.
     *
     *  @return  true if <code>Node</code> is supported, false otherwise.
     */
    protected abstract boolean nodeSupported(Node node);

    // doing a depth first search for the tree and mark all supported nodes
    private void markTree(Node node) {

        // if this is a supported node, then we add it to our cache table
        if (nodeSupported(node)) {
            nodeList.add(node);
        } else {
        // or we go through all children nodes recursively
        // (can be optimized in future)
            String nodeName = node.getNodeName();
            if ( cc_ == null || cc_.canConvertTag(nodeName)) {
                NodeList nodeList = node.getChildNodes();
                int nodeListLength = nodeList.getLength();
                for (int i = 0; i < nodeListLength; i++) {
                    markTree(nodeList.item(i));
                }
            }
            else {
                Debug.log(Debug.INFO, " [NodeIterator::markTree] Skipping node "
                                      + nodeName);
            }
        }
    }
}

