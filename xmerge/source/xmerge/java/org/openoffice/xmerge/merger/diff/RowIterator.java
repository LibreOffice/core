/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

