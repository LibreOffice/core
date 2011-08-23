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

package org.openoffice.xmerge.merger;

/**
 *  This is an interface used by the {@link
 *  org.openoffice.xmerge.merger.DiffAlgorithm
 *  DiffAlgorithm} and {@link
 *  org.openoffice.xmerge.merger.MergeAlgorithm
 *  MergeAlgorithm} to access a <code>Document</code>.
 *
 *  @author smak
 */
public interface Iterator {


    /**
     *  Move to next element in the sequence.
     *
     *  @return  The <code>Object</code> of the next element in the sequence.
     *           If there is no next element, then return null.
     */
    public Object next();


    /**
     *  Move to previous element in the sequence.
     *
     *  @return  The <code>Object</code> of the previous element in the sequence.
     *           If there is no previous element, then return null.
     */
    public Object previous();


    /**
     * Move to the beginning of the sequence.
     *
     * @return  The <code>Object</code> of the first element in the sequence.
     *          If it is empty, then return null.
     */
    public Object start();


    /**
     * Move to the end of the sequence.
     *
     * @return  The <code>Object</code> of the last element in the sequence.
     *          If it is empty, then return null.
     */
    public Object end();


    /**
     * Return the current element <code>Object</code> content.
     *
     * @return  The <code>Object</code> at current position.
     */
    public Object currentElement();


    /**
     * Return the total element count in the sequence.
     *
     * @return  The total element count.
     */
    public int elementCount();


    /**
     *  A method to allow the difference algorithm to test whether the
     *  <code>obj1</code> and <code>obj2</code> in the
     *  <code>Iterator</code> are considered equal.  As not every
     *  <code>Object</code> in the <code>Iterator</code> can implement its
     *  own equal method, with this equivalent method, we can allow
     *  flexibility for the <code>Iterator</code> to choose a custom way
     *  to compare two objects.  Two objects can even be compared based on
     *  the position in the <code>Iterator</code> rather than by 
     *  the content via this option.
     *
     *  @param  obj1  The first <code>Object</code>.
     *  @param  obj2  The second <code>Object</code>.
     *
     *  @return  true if equal, false otherwise.
     */
    public boolean equivalent(Object obj1, Object obj2);


    /**
     *  <p>A method to force the <code>Iterator</code> to transverse the tree
     *  again to refresh the content.</p>
     *
     *  <p>It is used mainly for <code>Iterator</code> objects which take a snap
     *  shot instead of dynamically transversing the tree.  The current
     *  position will be set to the beginning.</p>
     */
    public void refresh();
}

