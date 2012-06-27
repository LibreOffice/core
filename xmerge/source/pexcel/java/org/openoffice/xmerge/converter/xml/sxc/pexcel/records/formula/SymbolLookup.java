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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

import java.util.HashMap;

/**
 * This interface defines the attributes of a lookup table for this plugin.
 * Symbols will generally be either operators (_, -, *, etc) or funtion names.
 */
public abstract class SymbolLookup {

    protected HashMap<String, Integer> stringToID = null;
    protected HashMap<Integer, String> idToString = null;

    /**
     * Perform lookup table specific initialization. This would typically entail loading values into
     * the lookup table. It is best to optimize this process so that data is loaded statically and shared
     * across all instances of the lookup table.
     */
    abstract public void initialize();

    /**
     * Associate a symbol with a  numeric value in the lookup table
     * @param symbol    The symbol that will act as the key in the lookup table
     * @param id        The ID number to be associated with a given symbol
     */
    public void addEntry(String symbol, int id) {
        Integer iObj = new Integer(id);
        stringToID.put(symbol, iObj);
        idToString.put(iObj, symbol);
    }

    /**
     * Retrieve the symbol associated with a given identifier
     * @param   id  The identfier for which we need to retieve the symbol string
     * @return  The string associated with this identifier in the lookup table.
     */
    public String getStringFromID(int id) {
        return idToString.get(new Integer(id));
    }

    /**
     * Retrieve the identifier associated with a given symbol
     * @param   symbol  The symbol for which we need to retieve the identifier
     * @throws UnsupportedFunctionException Thown when the symbol is not found in the lookup table
     * @return  The identifier associated with this string in the lookup table.
     */
    public int getIDFromString(String symbol) throws UnsupportedFunctionException {
        Integer i = stringToID.get(symbol);
        if (i == null)
            throw new UnsupportedFunctionException("Token '" + symbol + "' not supported by Pocket Excel");

        return stringToID.get(symbol).intValue();
    }
}
