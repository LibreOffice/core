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
 *  This is the <code>Difference</code> basic unit. Used by the
 *  <code>DiffAlgorithm</code> as a set of difference between two
 *  <code>Iterators</code> (the original and modified
 *  <code>Iterators</code>).
 */
public final class Difference {

    /**
     *  Add operation.
     */
    public static final int ADD = 1;

    /**
     *  Delete operation.
     */
    public static final int DELETE = 2;

    /**
     *  Change operation.
     */
    public static final int CHANGE = 3;

    /**
     *  Unchange operation (i.e. no change).
     */
    public static final int UNCHANGE = 4;

    /**
     *  The action of the diff - either {@link #ADD} or {@link #DELETE}.
     */
    private int operation;

    /**
     *  <p>The position of the content that should be operated on (original
     *  iterator).</p>
     *
     *  <p>For ADD, the orgPosition is the position of the original sequence
     *  where the diff will insert (the element count is starting from 0, and
     *  always insert before the element).  The modPosition is the position
     *  of the diff in the modified sequence (also starting from 0).</p>
     *
     *  <blockquote><pre>
     *  example:
     *
     *   diff - &lt;B D&gt;and &lt;A B C D E F&gt;
     *   note: &lt;B D&gt;is original sequence and &lt;A B C D E F&gt;
     *   is the modified one.
     *
     *        and here is the position of those sequence:
     *        &lt;B D&gt; &lt;A B C D E F&gt;
     *         0 1   0 1 2 3 4 5
     *
     *   result:
     *   &lt;diff orgPos=0 modPos=0 operation=ADD&gt;  &lt;-- element A
     *   &lt;diff orgPos=1 modPos=2 operation=ADD&gt;  &lt;-- element C
     *   &lt;diff orgPos=2 modPos=4 operation=ADD&gt;  &lt;-- element E
     *   &lt;diff orgPos=2 modPos=5 operation=ADD&gt;  &lt;-- element F
     *
     *  </pre> </blockquote>
     *  <p>One can notice the add operation is inserted before the position.
     *  Hence, in order to append an element, we will have a position of
     *  original sequence length + 1 to denote an append.</p>
     *
     *  <p>For DELETE, orgPosition is the position that the element
     *  will be deleted (starting from 0) and modPosition is the position
     *  where the deleted element should be (consider as an ADD).</p>
     *
     *  <p>The modPosition is less useful and it is difficult to understand
     *  how the position is calculated.  One can just skip this piece of
     *  information.  It is useful if one wants to reverse the role
     *  of original sequence and modified sequence and find out the diff
     *  easily (just change add to delete and delete to add for operation
     *  and swap the orgPosition and modPosition).</p>
     *
     *  <blockquote><pre>
     *  example:
     *
     *  diff - &lt;A B C D E F&gt; and &lt; B D&gt;
     *  note: &lt;A B C D E F&gt; is original sequence and &lt;B D&gt;
     *  is the modified one.
     *
     *        and here is the position of those sequence:
     *        &lt;A B C D E F&gt; &lt;B D&gt;
     *         0 1 2 3 4 5   0 1
     *
     *  result:
     *  &lt;diff orgPos=0 modPos=0 operation=DELETE&gt;  &lt;--  element A
     *  &lt;diff orgPos=2 modPos=1 operation=DELETE&gt;  &lt;--  element C
     *  &lt;diff orgPos=4 modPos=2 operation=DELETE&gt;  &lt;--  element E
     *  &lt;diff orgPos=5 modPos=2 operation=DELETE&gt;  &lt;--  element F
     *  </pre></blockquote>
     */
    private int orgPosition;

    /**
     *  The position of the content that should be operated (modified iterator).
     *  For explanation and examples, see {@link #orgPosition}.
     */
    private int modPosition;


    /**
     *  Constructor.  This is the standard way to create a
     *  <code>Difference</code> object.
     *
     *  @param  operation    Either {@link #ADD} or {@link #DELETE}.
     *  @param  orgPosition  The position in the original (first)
     *                       <code>Iterator</code>.
     *  @param  modPosition  The position in the modified (second)
     *                       <code>Iterator</code>.
     */
    public Difference(int operation, int orgPosition,
                      int modPosition) {
        this.operation = operation;
        this.orgPosition = orgPosition;
        this.modPosition = modPosition;
    }


    /**
     *  Get the operation of the <code>Difference</code>.
     *
     *  @return the operation of the <code>Difference</code>,
     *  either {@link #ADD} or {@link #DELETE}
     */
    public int getOperation() {
        return operation;
    }

    /**
     *  Get the original <code>Iterator</code> position.
     *
     *  @return The position in the original (first) <code>Iterator</code>
     */
    public int getOrgPosition() {
        return orgPosition;
    }

    /**
     *  Get the modified <code>Iterator</code> position.
     *
     *  @return The position in the modified (second) <code>Iterator</code>
     */
    public int getModPosition() {
        return modPosition;
    }


    /**
     *  Two <code>Difference</code> objects will equal if and only if
     *  all operation, orgPosition, modPosition and content are equal.
     *
     *  @param  obj  Object to compare.
     *
     *  @return  true if equal, false otherwise.
     */
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

    /**
     *  Display debug information.
     *
     *  @return  Debug string.
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
     *  Returns position and operation values as a single string.
     *
     *  @return  orgPosition, modPosition and operation as a single string.
     */
    public String toString() {

        return orgPosition + " " + modPosition + " " + operation;
    }
}

