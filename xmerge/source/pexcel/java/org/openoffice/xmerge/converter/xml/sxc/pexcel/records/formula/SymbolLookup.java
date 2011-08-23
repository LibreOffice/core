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
 * This interface defines the attributes of a lookup table for this plugin.
 * Symbols will generally be either operators (_, -, *, etc) or funtion names.
 */
public abstract class SymbolLookup {

    protected HashMap stringToID = null;
    protected HashMap idToString = null;

    /**
     * Perform lookup table specific initialization. This would typically entail loading values into
     * the lookup table. It is best to optimize this process so that data is loaded statically and shared
     * across all instances of the lookup table.
     */
    abstract public void initialize();
    
    /**
     * Associate a symbol with a  numeric value in the lookup table
     * @param symbol	The symbol that will act as the key in the lookup table
     * @param value		The value to be associated with a given symbol
     */
    public void addEntry(String symbol, int id) {
        Integer iObj = new Integer(id);
        stringToID.put(symbol, iObj);
        idToString.put(iObj, symbol);
    }
    
    /**
     * Retrieve the symbol associated with a given identifier
     * @param	id	The identfier for which we need to retieve the symbol string
     * @return	The string associated with this identifier in the lookup table.
     */
    public String getStringFromID(int id) {
        return (String)idToString.get(new Integer(id));
    }
    
    /**
     * Retrieve the identifier associated with a given symbol
     * @param	symbol	The symbol for which we need to retieve the identifier
     * @throws UnsupportedFunctionException Thown when the symbol is not found in the lookup table
     * @return	The identifier associated with this string in the lookup table.
     */
    public int getIDFromString(String symbol) throws UnsupportedFunctionException {
        Integer i = (Integer)stringToID.get(symbol);
        if (i == null)
            throw new UnsupportedFunctionException("Token '" + symbol + "' not supported by Pocket Excel");
    
        return ((Integer)stringToID.get(symbol)).intValue();
    }
}
