/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OperatorLookup.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:07:06 $
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

import org.openoffice.xmerge.util.Debug;

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
