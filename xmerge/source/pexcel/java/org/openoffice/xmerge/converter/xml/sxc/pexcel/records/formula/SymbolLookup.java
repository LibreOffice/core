/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SymbolLookup.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:07:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
     * @param symbol    The symbol that will act as the key in the lookup table
     * @param value     The value to be associated with a given symbol
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
        return (String)idToString.get(new Integer(id));
    }

    /**
     * Retrieve the identifier associated with a given symbol
     * @param   symbol  The symbol for which we need to retieve the identifier
     * @throws UnsupportedFunctionException Thown when the symbol is not found in the lookup table
     * @return  The identifier associated with this string in the lookup table.
     */
    public int getIDFromString(String symbol) throws UnsupportedFunctionException {
        Integer i = (Integer)stringToID.get(symbol);
        if (i == null)
            throw new UnsupportedFunctionException("Token '" + symbol + "' not supported by Pocket Excel");

        return ((Integer)stringToID.get(symbol)).intValue();
    }
}
