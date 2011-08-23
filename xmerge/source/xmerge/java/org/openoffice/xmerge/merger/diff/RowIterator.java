/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.util.Resources;


/**
 *  This is an implementation of the <code>Iterator</code> interface and extends
 *  <code>NodeIterator</code>.  It will traverse the tree and find row sequences.
 *
 * @author smak
 */
public final class RowIterator extends NodeIterator {

    private Resources res = Resources.getInstance();

    // TODO: should compare the ConverterCapabilities supported feature only!
    // otherwise even though one with a chart, one without, will still be
    // considered to be not equivalent.

    /**
     *  Standard constructor.
     *
     *  @param  cc    The <code>ConverterCapabilities</code>.
     *  @param  node  The initial root <code>Node</code>.
     */
    public RowIterator(ConverterCapabilities cc, Node node) {
        super(cc, node);
    }

    /**
     *  Overwrite the parent <code>nodeSupported</code> method.  Only
     *  row <code>Node</code> objects are supported.
     *
     *  @param  node  <code>Node</code> to check.
     *
     *  @return  true if the <code>Node</code> is supported, false otherwise.
     */
    protected boolean nodeSupported(Node node) {

        // can use an array later to check all possible tags for
        // future expansion
        if (node.getNodeType() == Node.ELEMENT_NODE &&
            node.getNodeName().equals(OfficeConstants.TAG_TABLE_ROW)) {
            return true;
        } else {
            return false;
        }
    }
}

