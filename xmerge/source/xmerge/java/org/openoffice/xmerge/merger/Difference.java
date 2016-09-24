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
 * This is the {@code Difference} basic unit.
 *
 * Used by the {@code DiffAlgorithm} as a set of difference between two
 * {@code Iterators} (the original and modified {@code Iterators}).
 */
public final class Difference {

    /** Add operation. */
    public static final int ADD = 1;

    /** Delete operation. */
    public static final int DELETE = 2;

    /** Change operation. */
    public static final int CHANGE = 3;

    /** Unchange operation (i.e. no change). */
    public static final int UNCHANGE = 4;

    /** The action of the diff - either {@link #ADD} or {@link #DELETE} */
    private final int operation;

    /**
     * The position of the content that should be operated on (original
     * {@code iterator}).
     *
     * <p>For {@code ADD}, the {@code orgPosition} is the position of the
     * original sequence where the diff will insert (the element count is
     * starting from 0, and always insert before the element). The
     * {@code modPosition} is the position of the diff in the modified sequence
     * (also starting from 0).</p>
     *
     * <blockquote><pre>{@literal example:
     *
     * diff - <B D> and <A B C D E F>
     * note: <B D> is original sequence and <A B C D E F>
     * is the modified one.
     *
     *      and here is the position of those sequence:
     *      <B D> <A B C D E F>
     *       0 1   0 1 2 3 4 5
     *
     * result:
     * <diff orgPos=0 modPos=0 operation=ADD>  <-- element A
     * <diff orgPos=1 modPos=2 operation=ADD>  <-- element C
     * <diff orgPos=2 modPos=4 operation=ADD>  <-- element E
     * <diff orgPos=2 modPos=5 operation=ADD>  <-- element F}</pre></blockquote>
     *
     * <p>One can notice the add operation is inserted before the position.
     * Hence, in order to append an element, we will have a position of original
     * sequence length + 1 to denote an append.</p>
     *
     * <p>For {@code DELETE}, {@code orgPosition} is the position that the
     * element will be deleted (starting from 0) and {@code modPosition} is the
     * position where the deleted element should be (consider as an {@code ADD}).
     * </p>
     *
     * <p>The {@code modPosition} is less useful and it is difficult to
     * understand how the position is calculated.  One can just skip this piece
     * of information.  It is useful if one wants to reverse the role of original
     * sequence and modified sequence and find out the diff easily (just change
     * add to delete and delete to add for operation and swap the
     * {@code orgPosition} and {@code modPosition}).</p>
     *
     * <blockquote><pre>{@literal example:
     *
     * diff - <A B C D E F> and <B D>
     * note: <A B C D E F> is original sequence and <B D>
     * is the modified one.
     *
     *       and here is the position of those sequence:
     *       <A B C D E F> <B D>
     *        0 1 2 3 4 5   0 1
     *
     * result:
     * <diff orgPos=0 modPos=0 operation=DELETE>  <--  element A
     * <diff orgPos=2 modPos=1 operation=DELETE>  <--  element C
     * <diff orgPos=4 modPos=2 operation=DELETE>  <--  element E
     * <diff orgPos=5 modPos=2 operation=DELETE>  <--  element F}</pre></blockquote>
     */
    private final int orgPosition;

    /**
     * The position of the content that should be operated (modified iterator).
     *
     * <p>For explanation and examples, see {@link #orgPosition}</p>.
     */
    private final int modPosition;

    /**
     * Constructor.
     *
     * This is the standard way to create a {@code Difference} object.
     *
     * @param  operation    Either {@link #ADD} or {@link #DELETE}.
     * @param  orgPosition  The position in the original (first) {@code Iterator}.
     * @param  modPosition  The position in the modified (second) {@code Iterator}.
     */
    public Difference(int operation, int orgPosition,
                      int modPosition) {
        this.operation = operation;
        this.orgPosition = orgPosition;
        this.modPosition = modPosition;
    }

    /**
     * Get the operation of the {@code Difference}.
     *
     * @return the operation of the {@code Difference}, either {@link #ADD} or
     *         {@link #DELETE}.
     */
    public int getOperation() {
        return operation;
    }

    /**
     * Get the original {@code Iterator} position.
     *
     * @return The position in the original (first) {@code Iterator}.
     */
    public int getOrgPosition() {
        return orgPosition;
    }

    /**
     * Get the modified {@code Iterator} position.
     *
     * @return The position in the modified (second) {@code Iterator}.
     */
    public int getModPosition() {
        return modPosition;
    }

    /**
     * Two {@code Difference} objects will equal if and only if all
     * {@code operation}, {@code orgPosition}, {@code modPosition} and
     * {@code content} are equal.
     *
     * @param   obj  {@code Object} to compare.
     *
     * @return  {@code true} if equal, {@code false} otherwise.
     */
    @Override
    public boolean equals(Object obj) {
        if (obj instanceof Difference) {
            Difference diff = (Difference) obj;
            if ((operation == diff.operation) &&
                (orgPosition == diff.orgPosition) &&
                (modPosition == diff.modPosition)) {
                return true;
            }
        }

        return false;
    }

    @Override
    public int hashCode() {
        return 0;
    }

    /**
     * Display debug information.
     *
     * @return  Debug string.
     */
    public String debug() {

        String opStr = "";

        switch (operation) {
            case ADD:
                opStr = "add";
                break;
            case DELETE:
                opStr = "del";
                break;
            case CHANGE:
                opStr = "chg";
                break;
            case UNCHANGE:
                opStr = "uch";
                break;
            default:
                break;
        }
        return "<diff orgPos=" + orgPosition + " modPos=" + modPosition +
               " op=" + opStr + ">";
    }

    /**
     * Returns position and operation values as a single string.
     *
     * @return  {@code orgPosition}, {@code modPosition} and {@code operation}
     *          as a single {@code String}.
     */
    @Override
    public String toString() {

        return orgPosition + " " + modPosition + " " + operation;
    }
}
