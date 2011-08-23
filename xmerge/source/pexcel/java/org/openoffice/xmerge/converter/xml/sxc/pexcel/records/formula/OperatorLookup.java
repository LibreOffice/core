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
  * A lookup table containing information about operators
  */
public class OperatorLookup extends SymbolLookup {

    /**
    * The default constructor - invokes {@link #initialize() initialize()}
    */
    public OperatorLookup() {
        initialize();
    }

    /**
     * Initialize the lookup table for operators
     */
    public void initialize() {
        if ((stringToID != null) || (idToString != null)) {
            return;
        }
        stringToID = new HashMap();
        idToString = new HashMap();
        addEntry("UNARY_PLUS", TokenConstants.TUPLUS);
        addEntry("UNARY_MINUS", TokenConstants.TUMINUS);
        addEntry("%", TokenConstants.TPERCENT);
        addEntry("+", TokenConstants.TADD);
        addEntry("-", TokenConstants.TSUB);
        addEntry("*", TokenConstants.TMUL);
        addEntry("/", TokenConstants.TDIV);
        addEntry(",", TokenConstants.TARGSEP);
        addEntry("^", TokenConstants.TPOWER);
        addEntry("&", TokenConstants.TCONCAT);
        addEntry("(", TokenConstants.TPAREN);
        addEntry(")", TokenConstants.TCLOSEPAREN);
        addEntry("<", TokenConstants.TLESS);
        addEntry(">", TokenConstants.TGREATER);
        addEntry(">=", TokenConstants.TGTEQUALS);
        addEntry("<=", TokenConstants.TLESSEQUALS);
        addEntry("=", TokenConstants.TEQUALS);
        addEntry("<>", TokenConstants.TNEQUALS);
    }
    
}
