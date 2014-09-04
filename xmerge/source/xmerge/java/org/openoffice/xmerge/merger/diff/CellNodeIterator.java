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

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;
import org.w3c.dom.Element;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 * This is an implementations of the {@code Iterator} interface.
 *
 * <p>It will traverse the tree and find cell {@code Node} sequences.</p>
 *
 * <p>Note: Once the XML Tree is parsed, then the {@code Iterator} will be a
 * snap shot of that tree. That means even the tree is modified later, than the
 * cached paragraph {@code Node} list will not be updated accordingly.  For this
 * reason and for performance reasons this {@code Iterator} does not support any
 * operation methods such as insert, remove or replace. The main purpose of this
 * {@code Iterator} is to be used with difference, not with merge.</p>
 */
public final class CellNodeIterator extends NodeIterator  {

    // can be expanded to an array in the future, not necessary right now
    private static final String SUPPORTED_TAG1 = OfficeConstants.TAG_TABLE_CELL;

    /**
     * The standard constructor.
     *
     * @param  cc    The {@code ConverterCapabilities}.
     * @param  node  The initial root {@code Node}.
     */
    public CellNodeIterator(ConverterCapabilities cc, Node node) {
        super(cc, node);
    }

    /**
     * Overwrite the parent {@code nodeSupported} method.
     *
     * <p>Only cell {@code Node} objects are supported.</p>
     *
     * @param   node  The {@code Node} to check.
     *
     * @return  {@code true} if the {@code Node} is supported, {@code false}
     *          otherwise.
     */
    @Override
    protected boolean nodeSupported(Node node) {

        // can use an array later to check all possible tags for
        // future expansion
        return node.getNodeType() == Node.ELEMENT_NODE &&
                node.getNodeName().equals(SUPPORTED_TAG1);
    }

    @Override
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