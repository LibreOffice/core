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
 * This is an interface used by the {@link
 * org.openoffice.xmerge.merger.DiffAlgorithm DiffAlgorithm} and {@link
 * org.openoffice.xmerge.merger.MergeAlgorithm MergeAlgorithm} to access a
 * {@code Document}.
 */
public interface Iterator {

    /**
     * Move to next element in the sequence.
     *
     * @return  The {@code Object} of the next element in the sequence. If there
     *          is no next element, then return {@code null}.
     */
    Object next();

    /**
     * Move to the beginning of the sequence.
     *
     * @return  The {@code Object} of the first element in the sequence. If it
     *          is empty, then return {@code null}.
     */
    Object start();

    /**
     * Return the current element {@code Object} content.
     *
     * @return  The {@code Object} at current position.
     */
    Object currentElement();

    /**
     * Return the total element count in the sequence.
     *
     * @return  The total element count.
     */
    int elementCount();

    /**
     * A method to allow the difference algorithm to test whether the {@code obj1}
     * and {@code obj2} in the {@code Iterator} are considered equal.
     *
     * <p>As not every {@code Object} in the {@code Iterator} can implement its
     * own equal method, with this equivalent method, we can allow flexibility
     * for the {@code Iterator} to choose a custom way to compare two objects.
     * Two objects can even be compared based on the position in the
     * {@code Iterator} rather than by the content via this option.
     *
     * @param   obj1  The first {@code Object}
     * @param   obj2  The second {@code Object}.
     *
     * @return  {@code true} if equal, {@code false} otherwise.
     */
    boolean equivalent(Object obj1, Object obj2);

    /**
     * A method to force the {@code Iterator} to traverse the tree again to
     * refresh the content.
     *
     * <p>It is used mainly for {@code Iterator} objects which take a snapshot
     * instead of dynamically tranversing the tree.  The current position will
     * be set to the beginning.</p>
     */
    void refresh();
}
