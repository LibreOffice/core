/************************************************************************
 *  FunctionLookup.java
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

import org.openoffice.xmerge.util.Debug;

import java.util.HashMap;

public class FunctionLookup extends SymbolLookup {

    private HashMap stringToArgs = null;

    /**
    * The default constructor - invokes {@link #initialize() initialize()}
     */
    public FunctionLookup() {
        initialize();
    }

    /**
     * Initialize the lookup table for functions
     */
    public void initialize() {
        if ((stringToID != null) || (idToString != null) || (stringToArgs !=null)) {
            return;
        }
        stringToID = new HashMap();
        idToString = new HashMap();
        stringToArgs = new HashMap();

                // Functions with Variable number of Arguments
                // Math and Trig
                addEntry("SUM", TokenConstants.TSUM, -1);
                addEntry("MIN", TokenConstants.TMIN, -1);
                addEntry("PRODUCT", TokenConstants.TPRODUCT, -1);
                addEntry("LOG", TokenConstants.TLOG, -1);
                addEntry("SUMIF", TokenConstants.TSUMIF, -1);
                addEntry("TRUNC", TokenConstants.TRUNC, -1);
                // Financial
                addEntry("DDB", TokenConstants.TDDB, -1);
                addEntry("FV", TokenConstants.TFV, -1);
                addEntry("IRR", TokenConstants.TIRR, -1);
                addEntry("NPER", TokenConstants.TNPER, -1);
                addEntry("NPV", TokenConstants.TNPV, -1);
                addEntry("PMT", TokenConstants.TPMT, -1);
                addEntry("PV", TokenConstants.TPV, -1);
                addEntry("RATE", TokenConstants.TRATE, -1);
                // Statistical
                addEntry("AVERAGE", TokenConstants.TAVERAGE, -1);
                addEntry("COUNT", TokenConstants.TCOUNT, -1);
                addEntry("COUNTA", TokenConstants.TCOUNTA, -1);
                addEntry("MAX", TokenConstants.TMAX, -1 );
                addEntry("MIN", TokenConstants.TMIN, -1);
                addEntry("STDEV", TokenConstants.TSTDEV, -1 );
                addEntry("STDEVP", TokenConstants.TSTDEVP, -1 );
                addEntry("VAR", TokenConstants.TVAR, -1);
                addEntry("VARP", TokenConstants.TVARP, -1);
                // Lookup
                addEntry("CHOOSE", TokenConstants.TCHOOSE, -1);
                addEntry("HLOOKUP", TokenConstants.THLOOKUP, -1);
                addEntry("INDEX", TokenConstants.TINDEX, -1);
                addEntry("MATCH", TokenConstants.TMATCH, -1) ;
                addEntry("VLOOKUP", TokenConstants.TVLOOKUP, -1);
                // Text
                addEntry("RIGHT", TokenConstants.TRIGHT, -1);
                addEntry("SUBSTITUTE", TokenConstants.TSUBSTITUTE, -1);
                addEntry("FIND", TokenConstants.TFIND, -1);
                addEntry("LEFT", TokenConstants.TLEFT, -1);
                // Logical
                addEntry("AND", TokenConstants.TAND, -1 );
                addEntry("IF", TokenConstants.TIF, -1) ;
                addEntry("OR", TokenConstants.TOR, -1);

                // Functions with Fixed number of Arguments
                // Math and Trig
                addEntry("ABS", TokenConstants.TABS, 1);
                addEntry("ACOS", TokenConstants.TACOS, 1);
                addEntry("ASIN", TokenConstants.TASIN, 1);
                addEntry("ATAN", TokenConstants.TATAN, 1);
                addEntry("ATAN2", TokenConstants.TATAN2, 1);
                addEntry("COS", TokenConstants.TCOS, 1);
                addEntry("COUNTIF", TokenConstants.TCOUNTIF, 1);
                addEntry("DEGREES", TokenConstants.TDEGREES, 1);
                addEntry("EXP", TokenConstants.TEXP, 1);
                addEntry("FACT", TokenConstants.TFACT, 1);
                addEntry("INT", TokenConstants.TINTE, 1);
                addEntry("LN", TokenConstants.TLN, 1);
                addEntry("LOG10", TokenConstants.TLOG10, 1);
                addEntry("MOD", TokenConstants.TMOD, 1);
                addEntry("PI", TokenConstants.TPI, 0);
                addEntry("POWER", TokenConstants.TPOWERF, 2);
                addEntry("RADIANS", TokenConstants.TRADIANS, 1);
                addEntry("RAND", TokenConstants.TRAND, 1);
                addEntry("ROUND", TokenConstants.TROUND, 1);
                addEntry("SQRT", TokenConstants.TSQRT, 1);
                addEntry("TAN", TokenConstants.TTAN, 1);
                addEntry("SIN", TokenConstants.TSIN, 1);
                // Financial
                addEntry("SLN", TokenConstants.TSLN, 3);
                addEntry("SYD", TokenConstants.TSYD, 4);
                // Date and Time
                addEntry("DATE", TokenConstants.TDATE, 3);
                addEntry("DATEVALUE", TokenConstants.TDATEVALUE, 1);
                addEntry("DAY", TokenConstants.TDAY, 1);
                addEntry("HOUR", TokenConstants.THOUR, 1);
                addEntry("MINUTE", TokenConstants.TMINUTE, 1 );
                addEntry("MONTH", TokenConstants.TMONTH, 1);
                addEntry("NOW", TokenConstants.TNOW, 0);
                addEntry("SECOND", TokenConstants.TSECOND, 1);
                addEntry("TIME", TokenConstants.TTIME, 3);
                addEntry("TIMEVALUE", TokenConstants.TTIMEVALUE, 1);
                addEntry("YEAR", TokenConstants.TYEAR, 1);
                // Statistical
                addEntry("COUNTBLANK", TokenConstants.TCOUNTBLANK, 1);
                // lookup
                addEntry("COLUMNS", TokenConstants.TCOLUMNS, 1);
                addEntry("ROWS", TokenConstants.TROWS, 1);
                // Database
                addEntry("DAVERAGE", TokenConstants.TDAVAERAGE, 3);
                addEntry("DCOUNT", TokenConstants.TDCOUNT, 3);
                addEntry("DCOUNTA", TokenConstants.TDCOUNTA, 2);
                addEntry("DGET", TokenConstants.TDGET, 3);
                addEntry("DMAX", TokenConstants.TDMAX, 3);
                addEntry("DMIN", TokenConstants.TDMIN, 3);
                addEntry("DPRODUCT", TokenConstants.TDPRODUCT, 3);
                addEntry("DSTDEV", TokenConstants.TDSTDEV, 3);
                addEntry("DSTDEVP", TokenConstants.TDSTDEVP, 3) ;
                addEntry("DSUM", TokenConstants.TDSUM, 3);
                addEntry("DVAR", TokenConstants.TDVAR, 3);
                addEntry("DVARP", TokenConstants.TDVARP, 3);
                // Text
                addEntry("EXACT", TokenConstants.TEXACT, 2);
                addEntry("LEN", TokenConstants.TLEN, 1);
                addEntry("LOWER", TokenConstants.TLOWER, 1);
                addEntry("MID", TokenConstants.TMID, 3); // ??????
                addEntry("PROPER", TokenConstants.TPROPER, 1);
                addEntry("REPLACE", TokenConstants.TREPLACE, 4);
                addEntry("REPT", TokenConstants.TREPT, 2);
                addEntry("T", TokenConstants.TT, 1);
                addEntry("TRIM", TokenConstants.TRIM, 1);
                addEntry("UPPER", TokenConstants.TUPPER, 1);
                addEntry("VALUE", TokenConstants.TVALUE, 1);
                // Logical
                addEntry("FALSE", TokenConstants.TFALSE, 0);
                addEntry("NOT", TokenConstants.TNOT, 1);
                addEntry("TRUE", TokenConstants.TTRUE, 0);
                // Informational
                addEntry("ERRORTYPE", TokenConstants.TERRORTYPE, 1);
                addEntry("ISBLANK", TokenConstants.TISBLANK, 1);
                addEntry("ISERR", TokenConstants.TISERR, 1);
                addEntry("ISERROR", TokenConstants.TISERROR, 1);
                addEntry("ISLOGICAL", TokenConstants.TISLOGICAL, 1);
                addEntry("ISNA", TokenConstants.TISNA, 1);
                addEntry("ISNONTEXT", TokenConstants.TISNONTEXT, 1);
                addEntry("ISNUMBER", TokenConstants.TISNUMBER, 1);
                addEntry("ISTEXT", TokenConstants.TISTEXT, 1);
                addEntry("N", TokenConstants.TN, 1);
                addEntry("NA", TokenConstants.TNA, 0);

    }

    /**
     * Associate a function with an identifier and specifiy the number of arguments for that function
     * @param symbol    The function string that will act as the key in the lookup table
     * @param id        The identifier for the function
     * @param args      The number of arguments this function requires
     */
    public void addEntry(String symbol, int id, int args) {
        addEntry(symbol, id);
        stringToArgs.put(symbol, new Integer(args));
    }

    /**
     * Retrieve the number of arguments for this function
     * @param   symbol  The function name
     * @return  The number of arguments required by this function
     */
    public int getArgCountFromString(String symbol) {
        return ((Integer)stringToArgs.get(symbol)).intValue();
    }
}
