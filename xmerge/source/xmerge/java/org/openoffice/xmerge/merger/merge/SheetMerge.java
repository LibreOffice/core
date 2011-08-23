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

package org.openoffice.xmerge.merger.merge;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.merger.NodeMergeAlgorithm;

/**
 *  This class extends the <code>DocumentMerge</code> class.
 *  This class will merge two spreadsheet documents. 
 *  The main difference between this implementation and
 *  <code>DocumentMerge</code>
 *  is that this merge will try to maintain unsupported features by
 *  examing the cell <code>node</code> objects one by one when it
 *  removes a node from the original <code>Iterator</code>.
 *
 * @author smak
 */
public final class SheetMerge extends DocumentMerge {

    /**
     *  Constructor.
     *
     *  @param  cc     The <code>ConverterCapabilities</code>.
     *  @param  merge  The <code>NodeMergeAlgorithm</code>.
     */
    public SheetMerge(ConverterCapabilities cc, NodeMergeAlgorithm merge) {
        super(cc, merge);
    }


    /**
     *  Remove specified <code>Node</code>.
     *
     *  @param  node  <code>Node</code> to remove.
     */
    protected void removeNode(Node node) {

        clearRow(node);
    }


    /**
     *  Clear the row corresponding to the <code>Node</code>
     *
     *  @param  node  <code>Node</code> containing the row to clear.
     */
    private void clearRow(Node node) {
        NodeList children = node.getChildNodes();
        int numOfChildren = children.getLength();
 
        // clear all the cells under the row node but maintain any unsupported
        // features
        // TODO: we can actually check anything left after the clear up.
        // if there is nothing left, then we can even delete the cell nodes
        for (int i = 0; i < numOfChildren; i++) {
            SheetUtil.emptyCell(cc_, children.item(i));
        }
    }
}

