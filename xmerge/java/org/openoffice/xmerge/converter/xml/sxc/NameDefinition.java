/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxc;

/**
 * This is a class to define a Name Definition structure. This can then be
 * used by plugins to write or read their own definition types.
 *
 * @author Martin Maher
 */
public class NameDefinition {

    private String name;
    private String definition;
    private String baseCellAddress;
    private boolean rangeType = false;
    private boolean expressionType = false;

    /**
     * Constructor for a <code>NameDefinition</code>
     *
     * @param name name which identifies the definition
     * @param definition the definition itself
     * @param baseCellAddress the basecelladdress
     * @param rangeType true if definition of type range
     * @param expressionType true if definition of type expression
     */
    public NameDefinition(String name, String definition, String baseCellAddress, boolean rangeType, boolean expressionType) {

        this.name = name;
        this.definition = definition;
        this.expressionType = expressionType;
        this.rangeType = rangeType;
        this.baseCellAddress = baseCellAddress;
    }

    /**
     * returns Name of the definition
     *
     * @return the name which identifies the definition
     */
    public String getName() {

        return name;
    }
    /**
     * sets the definition
     *
     * @param newDefinition sets the definition
     */
    public void setDefinition(String newDefinition) {

        definition = newDefinition;
    }
    /**
     * Returns the definition itself
     *
     * @return the definition
     */
    public String getDefinition() {

        return definition;
    }

    /**
     * Returns the base Cell address
     *
     * @return the base cell address
     */
    public String getBaseCellAddress() {

        return baseCellAddress;
    }

    /**
     * Tests if definition is of type expression
     *
     * @return whether or not this name definition is of type expression
     */
    public boolean isExpressionType() {
        return expressionType;
    }

    /**
     * Tests if definition is of type range
     *
     * @return whether or not this name definition is of type range
     */
    public boolean isRangeType() {
        return rangeType;
    }

}
