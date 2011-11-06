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

/**
 *  A small class to hold the start/end character position and the
 *  <code>Node</code> pointer in a text <code>Node</code>.  It is
 *  mainly used for character parser to make a list of text
 *  <code>Node</code> cache entries.
 *
 *  @author smak
 */
public class TextNodeEntry {

    private int startChar_;
    private int endChar_;
    private Node node_;

    /**
     *  Constructor
     *
     *  @param  startChar  The start character position.
     *  @param  endChar    The end character position.
     *  @param  node       The text <code>Node</code>.
     */
    public TextNodeEntry(int startChar, int endChar, Node node) {
        startChar_ = startChar;
        endChar_   = endChar;
        node_      = node;
    }

    /**
     *  Returns the start character.
     *
     *  @return  The start character.
     */
    public int startChar() {
        return startChar_;
    }


    /**
     *  Returns the end character.
     *
     *  @return  The end character.
     */
    public int endChar() {
        return endChar_;
    }


    /**
     *  Returns the <code>Node</code>.
     *
     *  @return  The <code>Node</code>.
     */
    public Node node() {
        return node_;
    }
}

