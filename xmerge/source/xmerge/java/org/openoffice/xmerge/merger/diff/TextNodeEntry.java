/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextNodeEntry.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:37:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

