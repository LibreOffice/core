/*************************************************************************
 *
 *  $RCSfile: xlformula.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-09-08 15:49:21 $
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

#ifndef SC_XLFORMULA_HXX
#define SC_XLFORMULA_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif
#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

// Constants ==================================================================

const sal_uInt8 EXC_TOKID_EXP               = 0x01;
const sal_uInt8 EXC_TOKID_TBL               = 0x02;
const sal_uInt8 EXC_TOKID_ADD               = 0x03;
const sal_uInt8 EXC_TOKID_SUB               = 0x04;
const sal_uInt8 EXC_TOKID_MUL               = 0x05;
const sal_uInt8 EXC_TOKID_DIV               = 0x06;
const sal_uInt8 EXC_TOKID_POWER             = 0x07;
const sal_uInt8 EXC_TOKID_CONCAT            = 0x08;
const sal_uInt8 EXC_TOKID_LT                = 0x09;
const sal_uInt8 EXC_TOKID_LE                = 0x0A;
const sal_uInt8 EXC_TOKID_EQ                = 0x0B;
const sal_uInt8 EXC_TOKID_GE                = 0x0C;
const sal_uInt8 EXC_TOKID_GT                = 0x0D;
const sal_uInt8 EXC_TOKID_NE                = 0x0E;
const sal_uInt8 EXC_TOKID_ISECT             = 0x0F;
const sal_uInt8 EXC_TOKID_LIST              = 0x10;
const sal_uInt8 EXC_TOKID_RANGE             = 0x11;
const sal_uInt8 EXC_TOKID_UPLUS             = 0x12;
const sal_uInt8 EXC_TOKID_UMINUS            = 0x13;
const sal_uInt8 EXC_TOKID_PERCENT           = 0x14;
const sal_uInt8 EXC_TOKID_PAREN             = 0x15;
const sal_uInt8 EXC_TOKID_MISSARG           = 0x16;
const sal_uInt8 EXC_TOKID_STR               = 0x17;
const sal_uInt8 EXC_TOKID_EXTENDED          = 0x18;
const sal_uInt8 EXC_TOKID_ATTR              = 0x19;
const sal_uInt8 EXC_TOKID_SHEET             = 0x1A;
const sal_uInt8 EXC_TOKID_ENDSHEET          = 0x1B;
const sal_uInt8 EXC_TOKID_ERR               = 0x1C;
const sal_uInt8 EXC_TOKID_BOOL              = 0x1D;
const sal_uInt8 EXC_TOKID_INT               = 0x1E;
const sal_uInt8 EXC_TOKID_NUM               = 0x1F;

// Token array ================================================================

/** Contains all cell references that can be extracted from a multiple operations formula. */
struct XclMultipleOpRefs
{
    ScAddress           maFmlaScPos;        /// Position of the (first) formula cell.
    ScAddress           maColFirstScPos;
    ScAddress           maColRelScPos;
    ScAddress           maRowFirstScPos;
    ScAddress           maRowRelScPos;
    bool                mbDblRefMode;       /// true = One formula with row and column values.
};

// ----------------------------------------------------------------------------

class ScTokenArray;
class ScRangeData;
class XclRoot;

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
    static bool         GetString( String& rString, const ScTokenArray& rScTokArr );

    /** Parses the passed formula and tries to find a string token list, i.e. "abc";"def";"ghi".
        @descr  Returns the unquoted (!) strings in a single string, separated with the
        passed character. If a comma is specified, the function will return abc,def,ghi from
        the example above.
        @param rStringList  (out-parameter) All strings contained in the formula as list.
        @param cSep  List separator character.
        @return  true = String token list found, rString parameter is valid. */
    static bool         GetStringList( String& rStringList, const ScTokenArray& rScTokArr, sal_Unicode cSep );

    /** Tries to convert a formula that consists of a single string token to a list of strings.
        @descr  Example: The formula ="abc\ndef\nghi" will be converted to the formula
        ="abc";"def";"ghi", if the LF character is specified as separator.
        @param rScTokArr  (in/out-parameter) The token array to modify.
        @param cStringSep  The separator in the source string. */
    static void         ConvertStringToList( ScTokenArray& rScTokArr, sal_Unicode cStringSep );

    // shared formulas --------------------------------------------------------

    /** Tries to extract the definition of a shared formula from the passed token array.
        @descr  Shared formulas are stored as hidden defined names in Calc. This
        function looks if the passed token array consists of the reference to
        such a hidden defined name and returns its definition on success. */
    static const ScRangeData* GetSharedFormula( const XclRoot& rRoot, const ScTokenArray& rScTokArr );

    // multiple operations ----------------------------------------------------

    /** Parses the passed formula and tries to extract references of a multiple operation.
        @descr  Requires that the formula contains a single MULTIPLE.OPERATION function call.
        Spaces in the formula are silently ignored.
        @return  true = Multiple operation found, and all references successfully extracted. */
    static bool         GetMultipleOpRefs( XclMultipleOpRefs& rRefs, const ScTokenArray& rScTokArr );
};

// ============================================================================

#endif

