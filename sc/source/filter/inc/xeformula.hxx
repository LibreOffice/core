/*************************************************************************
 *
 *  $RCSfile: xeformula.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-09 15:07:37 $
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

#ifndef SC_XEFORMULA_HXX
#define SC_XEFORMULA_HXX

#ifndef SC_XLFORMULA_HXX
#include "xlformula.hxx"
#endif
#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif

class XclExpStream;

// Token array ================================================================

/** Binary representation of an Excel token array. */
class XclExpTokenArray
{
public:
    /** Creates an empty token array. */
    explicit            XclExpTokenArray( bool bVolatile = false );
    /** Creates a token array, swaps passed token vector into own data. */
    explicit            XclExpTokenArray( ScfUInt8Vec& rTokVec, bool bVolatile = false );

    /** Returns true, if the token array is empty. */
    inline bool         Empty() const { return maTokVec.empty(); }
    /** Returns the size of the token array in bytes. */
    sal_uInt16          GetSize() const;
    /** Returns read-only access to the byte vector storing token data. */
    inline const ScfUInt8Vec& GetTokenVec() const { return maTokVec; }
    /** Returns true, if the formula contains a volatile function. */
    inline bool         IsVolatile() const { return mbVolatile; }

    /** Swaps own token vector with passed token vector. */
    inline void         SwapTokenVec( ScfUInt8Vec& rTokVec ) { maTokVec.swap( rTokVec ); }

    /** Writes the size field of the token array. */
    void                WriteSize( XclExpStream& rStrm ) const;
    /** Writes the tokens of the token array (without size field). */
    void                WriteArray( XclExpStream& rStrm ) const;
    /** Writes size field and the tokens. */
    void                Write( XclExpStream& rStrm ) const;

private:
    ScfUInt8Vec         maTokVec;       /// Byte vector containing token data.
    bool                mbVolatile;     /// True = Formula contains volatile function.
};

/** Calls the Write() function at the passed token array. */
XclExpStream& operator<<( XclExpStream& rStrm, const XclExpTokenArray& rTokArr );

// Formula compiler ===========================================================

class XclExpFmlaCompImpl;

/** The formula compiler to create Excel token arrays from Calc token arrays. */
class XclExpFormulaCompiler : protected XclExpRoot
{
public:
    explicit            XclExpFormulaCompiler( const XclExpRoot& rRoot );

    /** Creates and returns the token array of a simple formula cell. */
    XclExpTokenArrayRef CreateCellFormula( const ScTokenArray& rScTokArr, const ScAddress& rScPos );

    /** Creates and returns the token array of a matrix formula cell. */
    XclExpTokenArrayRef CreateMatrixFormula( const ScTokenArray& rScTokArr, const ScAddress& rScPos );

    /** Creates and returns the token array of a shared formula cell. */
    XclExpTokenArrayRef CreateSharedFormula( const ScTokenArray& rScTokArr, const ScAddress& rScPos );

    /** Creates and returns the token array for a condition in a conditional format. */
    XclExpTokenArrayRef CreateCondFormula( const ScTokenArray& rScTokArr );

    /** Creates and returns the token array for a condition in data validation. */
    XclExpTokenArrayRef CreateDataValFormula( const ScTokenArray& rScTokArr );

    /** Creates and returns the token array for a source range in list validation. */
    XclExpTokenArrayRef CreateListValFormula( const ScTokenArray& rScTokArr );

    /** Creates and returns the token array for a defined name. */
    XclExpTokenArrayRef CreateNameFormula( const ScTokenArray& rScTokArr );

    /** Creates and returns a token array containing a single cell address.
        @param b3DRefOnly  True = Always export sheet reference; false = try to shorten own sheet references. */
    XclExpTokenArrayRef CreateCellRefFormula( const ScAddress& rScPos, bool b3DRefOnly );

    /** Creates and returns a token array containing a single cell range address.
        @param b3DRefOnly  True = Always export sheet reference; false = try to shorten own sheet references. */
    XclExpTokenArrayRef CreateRangeRefFormula( const ScRange& rScRange, bool b3DRefOnly );

    /** Creates and returns the token array for a cell range list.
        @param b3DRefOnly  True = Always export sheet reference; false = try to shorten own sheet references. */
    XclExpTokenArrayRef CreateRangeListFormula( const ScRangeList& rRangeList, bool b3DRefOnly );

    /** Creates a single error token containing the passed error code. */
    XclExpTokenArrayRef CreateErrorFormula( sal_uInt8 nErrCode );

    /** Creates a single token for a special cell reference.
        @descr  This is used for array formulas and shared formulas (token tExp),
            and multiple operation tables (token tTbl). */
    XclExpTokenArrayRef CreateSpecialRefFormula( sal_uInt8 nTokenId,
                            sal_uInt16 nXclCol, sal_uInt16 nXclRow );

private:
    typedef ScfRef< XclExpFmlaCompImpl > XclExpFmlaCompImplRef;
    XclExpFmlaCompImplRef mxImpl;
};

// ============================================================================

#endif

