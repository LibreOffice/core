/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xeformula.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:53:49 $
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

#ifndef SC_XEFORMULA_HXX
#define SC_XEFORMULA_HXX

#ifndef SC_XLFORMULA_HXX
#include "xlformula.hxx"
#endif
#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif

// External reference log =====================================================

/** Log entry for external references in a formula, used i.e. in change tracking. */
struct XclExpRefLogEntry
{
    const XclExpString* mpUrl;              /// URL of the document containing the first sheet.
    const XclExpString* mpFirstTab;         /// Name of the first sheet.
    const XclExpString* mpLastTab;          /// Name of the last sheet.
    sal_uInt16          mnFirstXclTab;      /// Calc index of the first sheet.
    sal_uInt16          mnLastXclTab;       /// Calc index of the last sheet.

    explicit            XclExpRefLogEntry();
};

/** Vector containing a log for all external references in a formula, used i.e. in change tracking. */
typedef ::std::vector< XclExpRefLogEntry > XclExpRefLog;

// Formula compiler ===========================================================

class ScRangeList;
class XclExpFmlaCompImpl;

/** The formula compiler to create Excel token arrays from Calc token arrays. */
class XclExpFormulaCompiler : protected XclExpRoot
{
public:
    explicit            XclExpFormulaCompiler( const XclExpRoot& rRoot );
    virtual             ~XclExpFormulaCompiler();

    /** Creates and returns the token array of a formula. */
    XclTokenArrayRef    CreateFormula(
                            XclFormulaType eType, const ScTokenArray& rScTokArr,
                            const ScAddress* pScBasePos = 0, XclExpRefLog* pRefLog = 0 );

    /** Creates and returns a token array containing a single cell address. */
    XclTokenArrayRef    CreateFormula( XclFormulaType eType, const ScAddress& rScPos );

    /** Creates and returns a token array containing a single cell range address. */
    XclTokenArrayRef    CreateFormula( XclFormulaType eType, const ScRange& rScRange );

    /** Creates and returns the token array for a cell range list. */
    XclTokenArrayRef    CreateFormula( XclFormulaType eType, const ScRangeList& rScRanges );

    /** Creates a single error token containing the passed error code. */
    XclTokenArrayRef    CreateErrorFormula( sal_uInt8 nErrCode );

    /** Creates a single token for a special cell reference.
        @descr  This is used for array formulas and shared formulas (token tExp),
            and multiple operation tables (token tTbl). */
    XclTokenArrayRef    CreateSpecialRefFormula( sal_uInt8 nTokenId, const XclAddress& rXclPos );

    /** Creates a single tNameXR token for a reference to an external name.
        @descr  This is used i.e. for linked macros in push buttons. */
    XclTokenArrayRef    CreateNameXFormula( sal_uInt16 nExtSheet, sal_uInt16 nExtName );

private:
    typedef ScfRef< XclExpFmlaCompImpl > XclExpFmlaCompImplRef;
    XclExpFmlaCompImplRef mxImpl;
};

// ============================================================================

#endif

