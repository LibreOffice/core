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
 *  <p>This is an implementation of the <code>Iterator</code> interface.
 *  It will traverse the tree and find the Paragraph/Heading <code>Node</code>
 *  sequences.</p>
 *
 *  <p>Note: Once the XML Tree is parsed, then the <code>Iterator</code> will
 *  be a snap shot of that tree. That means even the tree is modified later,
 *  than the cached paragraph <code>Node</code> list will not be updated
 *  accordingly.  For this reason and for performance reasons this
 *  <code>Iterator</code> does not support any operation methods such as
 *  insert, remove or replace.  The main purpose of this
 *  <code>Iterator</code> is to be used with difference, not with merge.</p>
 */
public final class ParaNodeIterator extends NodeIterator {

    // can be expanded to an array in the future, not necessary right now
    private static final String SUPPORTED_TAG1 = OfficeConstants.TAG_PARAGRAPH;
    private static final String SUPPORTED_TAG2 = OfficeConstants.TAG_HEADING;

    /**
     *  Standard constructor.
     *
     *  @param  cc    The <code>ConverterCapabilities</code>.
     *  @param  node  The initial root <code>Node</code>.
     */
    public ParaNodeIterator(ConverterCapabilities cc, Node node) {
        // not using convertercapabilities unless it's needed in future.
        super(cc, node);
    }


    /**
     *  Overwrite the parent <code>nodeSupported</code> method.
     *
     *  @param  node  <code>Node</code> to check.
     *
     *  @return  true if the <code>Node</code> is supported, false
     *           otherwise.
     */
    protected boolean nodeSupported(Node node) {

        // can use an array later to check all possible tags for
        // future expansion
        if (node.getNodeType() == Node.ELEMENT_NODE &&
            (node.getNodeName().equals(SUPPORTED_TAG1) ||
             node.getNodeName().equals(SUPPORTED_TAG2))) {
            return true;
        } else {
            return false;
        }
    }
}

