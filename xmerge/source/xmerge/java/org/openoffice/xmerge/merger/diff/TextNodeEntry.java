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

