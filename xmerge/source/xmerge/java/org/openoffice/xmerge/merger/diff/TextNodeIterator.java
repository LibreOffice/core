/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextNodeIterator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:37:27 $
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
import org.w3c.dom.Document;

import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

import java.util.Vector;
import java.util.List;


/**
 *  <p>This is an implementation of the <code>Iterator</code> interface.
 *  It will traverse the tree and find text/space/tab <code>Node</code>
 *  sequences.</p>
 *
 *  <p>Note: Once the XML Tree is parsed, then the <code>Iterator</code>
 *  will be a snap shot of that tree. That means even the tree is modified
 *  later, than the cached paragraph <code>Node</code> list will not be
 *  updated accordingly.  For this reason  and for performance reasons
 *  this <code>Iterator</code> does not support any operation methods
 *  such as insert, remove or replace.  The main purpose of this
 *  <code>Iterator</code> is to be used with difference, not with merge.</p>
 *
 *  @author smak
 */
public final class TextNodeIterator extends NodeIterator {

    /**
     *  Standard constructor.
     *
     *  @param  initial  The initial root <code>Node</code>.
     */
    public TextNodeIterator(Node node) {
        super(null, node);
    }

    /**
     *  Overwrite the parent <code>nodeSupported</code> method.  Only text
     *  <code>Node</code> objects are supported.
     *
     *  @param  node  <code>Node</code> to check.
     *
     *  @return  true if the <code>Node</code> is supported, false
     *           otherwise.
     */
    protected boolean nodeSupported(Node node) {

        // can use an array later to check all possible tags for
        // future expansion
        if (node.getNodeType() == Node.TEXT_NODE ||
            node.getNodeName().equals(OfficeConstants.TAG_SPACE) ||
            node.getNodeName().equals(OfficeConstants.TAG_TAB_STOP) ||
            node.getNodeName().equals(OfficeConstants.TAG_LINE_BREAK)) {
            return true;
        } else {
            return false;
        }
    }
}

