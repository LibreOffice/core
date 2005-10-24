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

package org.openoffice.xmerge.merger.merge;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.merger.NodeMergeAlgorithm;
import org.openoffice.xmerge.util.Debug;

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

        Node child;

        // clear all the cells under the row node but maintain any unsupported
        // features
        // TODO: we can actually check anything left after the clear up.
        // if there is nothing left, then we can even delete the cell nodes
        for (int i = 0; i < numOfChildren; i++) {
            SheetUtil.emptyCell(cc_, children.item(i));
        }
    }
}

