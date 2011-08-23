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

package org.openoffice.xmerge.converter.xml.sxc;

/**
 * This is a class to define a table-column structure. This can then be
 * used by plugins to write or read their own column types.
 *
 * @author Martin Maher
 */
public class ColumnRowInfo {

    final public static int COLUMN	= 0x01;
    final public static int ROW		= 0x02;

    final private static int DEFAULTROWSIZE_MIN	= 250;
    final private static int DEFAULTROWSIZE_MAX	= 260;

    private int type;
    private int dimension	= 0;
    private int repeated	= 1;
    private boolean userDefined = true;
    private Format fmt = new Format();

    /**
     * Constructor for a <code>ColumnRowInfo</code>
     *
     * @param type whether ROW or COLUMN record .
     */
    public ColumnRowInfo(int type) {

        this.type = type;
    }

    /**
     * Constructor for a <code>ColumnRowInfo</code>
     *
     * @param dimension if it's a row the height, a column the width.
     * @param repeated how many times it is repeated.
     * @param type whether ROW or COLUMN record.
     */
    public ColumnRowInfo(int dimension, int repeated, int type) {

        this.dimension = dimension;
        this.repeated = repeated;
        this.type = type;
    }

    /**
     * Constructor for a <code>ColumnRowInfo</code> that includes userDefined
     * field.
     *
     * @param dimension if it's a row the height, a column the width.
     * @param repeated how many times it is repeated.
     * @param type whether ROW or COLUMN record.
     * @param userDefined whether the record is manually set.
     */
    public ColumnRowInfo(int dimension, int repeated, int type, boolean userDefined) {

        this(dimension, repeated, type);
        this.userDefined = userDefined;
    }

    /**
     * Sets the format.
     *
     * @param fmt The new format to use.
     */
    public void setFormat(Format fmt) {

        this.fmt = fmt;
    }

    /**
     * Get the current format.
     *
     * @return The current format.
     */
    public Format getFormat() {

        return fmt;
    }

    /**
     * Get the height (for rows) or width (for columns).
     *
     * @return The height or width.
     */
    public int getSize() {

        return dimension;
    }

    /**
     * Set the height (for rows) or width (for columns).
     *
     * @param dimension The height or width.
     */
    public void setSize(int dimension) {

        this.dimension = dimension;
    }

    /**
     * Get the repeat count for this item.
     *
     * @return The number of times this item is repeated.
     */
    public int getRepeated() {

        return repeated;
    }

    /**
     * Set the repeat count for this item.
     *
     * @param repeated The number of times this item is repeated.
     */
    public void setRepeated(int repeated) {

        this.repeated = repeated;
    }

    /**
     * Does this <code>ColumnRowInfo</code> represent a row?
     *
     * @return True if a row, false if not.
     */
    public boolean isRow() {

        if(type==ROW)
            return true;
        else
            return false;
    }

    /**
     * Does this <code>ColumnRowInfo</code> represent a column?
     *
     * @return True if a column, false if not.
     */
    public boolean isColumn() {

        if(type==COLUMN)
            return true;
        else
            return false;
    }

    /**
     * Test if the row height as been set manually
     *
     * @return true if user defined otherwise false
     */
    public boolean isUserDefined() {

        return userDefined;
    }

    /**
     * Test if the row height is default
     *
     * @return true if default otherwise false
     */
    public boolean isDefaultSize() {

        if(	type==ROW &&
            dimension>DEFAULTROWSIZE_MIN &&
            dimension<DEFAULTROWSIZE_MAX)
            return true;
        else
            return false;
    }
}
