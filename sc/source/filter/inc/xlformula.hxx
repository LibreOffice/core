/*************************************************************************
 *
 *  $RCSfile: xlformula.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:38:18 $
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

// ============================================================================

#ifndef SC_XLFORMULA_HXX
#define SC_XLFORMULA_HXX

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif
#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif


// Token array ================================================================

/** A helper that extends an ScTokenArray with additional Excel specific functions.
    @descr  The purpose to not add these functions to ScTokenArray is to prevent code
    changes in low-level Calc headers and to keep the Excel specific source code in
    the filter directory. Deriving from ScTokenArray is not viable because that would
    need expensive copy-constructions of the token arrays. */
class XclTokenArrayHelper : ScfNoInstance
{
public:
    // strings and string lists -----------------------------------------------

    /** Parses the passed formula and tries to find a single string token, i.e. "abc".
        @param rString  (out-parameter) The string contained in the formula.
        @return  true = String token found, rString parameter is valid. */
    static bool                 GetString( String& rString, const ScTokenArray& rTokenArray );

    /** Parses the passed formula and tries to find a string token list, i.e. "abc";"def";"ghi".
        @descr  Returns the unquoted (!) strings in a single string, separated with the
        passed character. If a comma is specified, the function will return abc,def,ghi from
        the example above.
        @param rStringList  (out-parameter) All strings contained in the formula as list.
        @param cSep  List separator character.
        @return  true = String token list found, rString parameter is valid. */
    static bool                 GetStringList( String& rStringList, const ScTokenArray& rTokenArray, sal_Unicode cSep );

    /** Tries to convert a formula that consists of a single string token to a list of strings.
        @descr  Example: The formula ="abc\ndef\nghi" will be converted to the formula
        ="abc";"def";"ghi", if the LF character is specified as separator.
        @param rTokenArray  (in/out-parameter) The token array to modify.
        @param cStringSep  The separator in the source string. */
    static void                 ConvertStringToList( ScTokenArray& rTokenArray, sal_Unicode cStringSep );

    // multiple operations ----------------------------------------------------

    /** Parses the passed formula and tries to extract references of a multiple operation.
        @descr  Requires that the formula contains a single MULTIPLE.OPERATION function call.
        Spaces in the formula are silently ignored.
        @param rbIsDoubleRefMode  (out-parameter) true, if the multiple operation contains one
        formula, and row and column values.
        @return  true = Multiple operation found, and all references successfully extracted. */
    static bool                 GetMultipleOpRefs(
                                    ScAddress& rFormula,
                                    ScAddress& rColFirstPos, ScAddress& rColRelPos,
                                    ScAddress& rRowFirstPos, ScAddress& rRowRelPos,
                                    bool& rbIsDoubleRefMode,
                                    const ScTokenArray& rTokenArray );
};


// ============================================================================

#endif

