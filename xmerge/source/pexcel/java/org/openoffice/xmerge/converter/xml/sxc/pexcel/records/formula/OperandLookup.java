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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;
import java.util.HashMap;

/**
  * A lookup table containing information about operands
  */
public class OperandLookup extends SymbolLookup {

    /**
    * The default constructor - invokes {@link #initialize() initialize()}
    */
    public OperandLookup() {
        initialize();
    }

    /**
     * Initialize the lookup table for operands
     */
    public void initialize() {
        if ((stringToID != null) || (idToString != null)) {
            return;
        }
        stringToID = new HashMap();
        idToString = new HashMap();
        addEntry("CELL_REFERENCE", TokenConstants.TREF);
        addEntry("CELL_AREA_REFERENCE", TokenConstants.TAREA);
        addEntry("INTEGER", TokenConstants.TNUM);
        addEntry("NUMBER", TokenConstants.TNUM);
        addEntry("STRING", TokenConstants.TSTRING);
        addEntry("NAME", TokenConstants.TNAME);
        addEntry("3D_CELL_REFERENCE", TokenConstants.TREF3D);
        addEntry("3D_CELL_AREA_REFERENCE", TokenConstants.TAREA3D);
    }
    
}
