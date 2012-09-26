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

package org.openoffice.xmerge.merger;

/**
 *  This is an interface used by the {@link
 *  org.openoffice.xmerge.merger.DiffAlgorithm
 *  DiffAlgorithm} and {@link
 *  org.openoffice.xmerge.merger.MergeAlgorithm
 *  MergeAlgorithm} to access a <code>Document</code>.
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

