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



package org.openoffice.xmerge.converter.xml.sxc;

/**
 * This is a class to define a table-column structure. This can then be
 * used by plugins to write or read their own column types.
 *
 * @author Martin Maher
 */
public class ColumnRowInfo {

    final public static int COLUMN  = 0x01;
    final public static int ROW     = 0x02;

    final private static int DEFAULTROWSIZE_MIN = 250;
    final private static int DEFAULTROWSIZE_MAX = 260;

    private int type;
    private int dimension   = 0;
    private int repeated    = 1;
    private boolean userDefined = true;
    private Format fmt = new Format();

    /**
     * Constructor for a <code>ColumnRowInfo</code>
     *
     * @param type whether Row or column record
     */
    public ColumnRowInfo(int type) {

        this.type = type;
    }

    /**
     * Constructor for a <code>ColumnRowInfo</code>
     *
     * @param dimension if it's a row the height, a column the width
     * @param repeated how many times it is repeated
     * @param type whether Row or column record
     */
    public ColumnRowInfo(int dimension, int repeated, int type) {

        this.dimension = dimension;
        this.repeated = repeated;
        this.type = type;
    }

    /**
     * Constructor that includes userDefined field
     *
     * @param userDefined whether the record is manually set
     */
    public ColumnRowInfo(int dimension, int repeated, int type, boolean userDefined) {

        this(dimension, repeated, type);
        this.userDefined = userDefined;
    }

    /**
     * sets the definition
     *
     * @param fmt sets the definition
     */
    public void setFormat(Format fmt) {

        this.fmt = fmt;
    }

    /**
     * returns Name of the definition
     *
     * @return the name which identifies the definition
     */
    public Format getFormat() {

        return fmt;
    }

    /**
     * returns Name of the definition
     *
     * @return the name which identifies the definition
     */
    public int getSize() {

        return dimension;
    }

    /**
     * Sets the definition
     *
     * @param dimension
     */
    public void setSize(int dimension) {

        this.dimension = dimension;
    }
    /**
     * Returns the definition itself
     *
     * @return the definition
     */
    public int getRepeated() {

        return repeated;
    }

    public void setRepeated(int repeated) {

        this.repeated = repeated;
    }

    /**
     * Returns the definition itself
     *
     * @return the definition
     */
    public boolean isRow() {

        if(type==ROW)
            return true;
        else
            return false;
    }

    /**
     * Returns the base Cell address
     *
     * @return the base cell address
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

        if( type==ROW &&
            dimension>DEFAULTROWSIZE_MIN &&
            dimension<DEFAULTROWSIZE_MAX)
            return true;
        else
            return false;
    }
}
