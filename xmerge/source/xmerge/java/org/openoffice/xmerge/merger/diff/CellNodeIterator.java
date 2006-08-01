/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CellNodeIterator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:35:07 $
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

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;
import org.w3c.dom.Element;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.Resources;


/**
 *  <p>This is an implementations of the <code>Iterator</code> interface.
 *  It will traverse the tree and find cell <code>Node</code> sequences.</p>
 *
 *  <p>Note: Once the XML Tree is parsed, then the <code>Iterator</code>
 *  will be a snap shot of that tree. That means even the tree is
 *  modified later, than the cached paragraph <code>Node</code> list will
 *  not be updated accordingly.  For this reason and for performance reasons
 *  this <code>Iterator</code> does not support any operation methods such
 *  as insert, remove or replace.  The main purpose of this
 *  <code>Iterator</code> is to be used with difference, not with merge.</p>
 *
 *  @author smak
 */
public final class CellNodeIterator extends NodeIterator  {

    private Resources res = Resources.getInstance();

    // can be expanded to an array in the future, not necessary right now
    private static final String SUPPORTED_TAG1 = OfficeConstants.TAG_TABLE_CELL;

    /**
     *  The standard constructor.
     *
     *  @param  cc    The <code>ConverterCapabilities</code>.
     *  @param  node  The initial root <code>Node</code>.
     */
    public CellNodeIterator(ConverterCapabilities cc, Node node) {
        super(cc, node);
    }


    /**
     *  Overwrite the parent <code>nodeSupported</code> method.  Only cell
     *  <code>Node</code> objects are supported.
     *
     *  @param  node  The <code>Node</code> to check.
     *
     *  @return  true if the <code>Node</code> is supported, false otherwise.
     */
    protected boolean nodeSupported(Node node) {

        // can use an array later to check all possible tags for
        // future expansion
        if (node.getNodeType() == Node.ELEMENT_NODE &&
            node.getNodeName().equals(SUPPORTED_TAG1)) {
            return true;
        } else {
            return false;
        }
    }


    protected boolean childrenEqual(Node node1, Node node2) {

        boolean equal = false;

        if (node1.hasChildNodes() && node2.hasChildNodes()) {
            Element cell1 = (Element)node1;
            Element cell2 = (Element)node2;

            // only need compare the first <text:p> children node, don't want
            // to compare any non-supported features
            // TODO: need to confirm whether all the text string is the
            // first <text:p>, though I checked with the openoffice 619 build
            Node paraNode1 = cell1.getElementsByTagName(
                            OfficeConstants.TAG_PARAGRAPH).item(0);
            Node paraNode2 = cell2.getElementsByTagName(
                            OfficeConstants.TAG_PARAGRAPH).item(0);

            equal = super.compareNode(paraNode1, paraNode2);
        }

        return equal;
    }
}

