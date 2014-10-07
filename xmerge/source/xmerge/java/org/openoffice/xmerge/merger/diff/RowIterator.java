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

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 * This is an implementation of the {@code Iterator} interface and extends
 * {@code NodeIterator}.
 *
 * <p>It will traverse the tree and find row sequences.</p>
 */
public final class RowIterator extends NodeIterator {

    // TODO: should compare the ConverterCapabilities supported feature only!
    // otherwise even though one with a chart, one without, will still be
    // considered to be not equivalent.

    /**
     * Standard constructor.
     *
     * @param  cc    The {@code ConverterCapabilities}.
     * @param  node  The initial root {@code Node}.
     */
    public RowIterator(ConverterCapabilities cc, Node node) {
        super(cc, node);
    }

    /**
     * Overwrite the parent {@code nodeSupported} method.
     *
     * <p>Only row {@code Node} objects are supported.</p>
     *
     * @param   node  {@code Node} to check.
     *
     * @return  {@code true} if the {@code Node} is supported, {@code false}
     *          otherwise.
     */
    @Override
    protected boolean nodeSupported(Node node) {

        // can use an array later to check all possible tags for
        // future expansion
        return node.getNodeType() == Node.ELEMENT_NODE &&
            node.getNodeName().equals(OfficeConstants.TAG_TABLE_ROW);
    }
}