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
 *  examining the cell <code>node</code> objects one by one when it
 *  removes a node from the original <code>Iterator</code>.
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

