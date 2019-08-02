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
 * This is an implementation of the {@code Iterator} interface.
 *
 * <p>It will traverse the tree and find the Paragraph/Heading {@code Node}
 * sequences.</p>
 *
 * <p>Note: Once the XML Tree is parsed, then the {@code Iterator} will be a
 * snapshot of that tree. That means even the tree is modified later, then the
 * cached paragraph {@code Node} list will not be updated accordingly. For this
 * reason and for performance reasons this {@code Iterator} does not support any
 * operation methods such as insert, remove or replace. The main purpose of this
 * {@code Iterator} is to be used with difference, not with merge.</p>
 */
public final class ParaNodeIterator extends NodeIterator {

    /**
     * Standard constructor.
     *
     * @param  cc    The {@code ConverterCapabilities}.
     * @param  node  The initial root {@code Node}.
     */
    public ParaNodeIterator(ConverterCapabilities cc, Node node) {
        // not using convertercapabilities unless it's needed in future.
        super(cc, node);
    }

    /**
     * Overwrite the parent <code>nodeSupported</code> method.
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
        String nodeName = node.getNodeName();
        return node.getNodeType() == Node.ELEMENT_NODE &&
            (nodeName.equals(OfficeConstants.TAG_PARAGRAPH) ||
            nodeName.equals(OfficeConstants.TAG_HEADING));
    }
}
