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

import org.openoffice.xmerge.converter.xml.OfficeConstants;


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
     *  @param  node  The initial root <code>Node</code>.
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

