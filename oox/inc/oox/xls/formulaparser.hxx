/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formulaparser.hxx,v $
 * $Revision: 1.3.22.1 $
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

#ifndef OOX_XLS_FORMULAPARSER_HXX
#define OOX_XLS_FORMULAPARSER_HXX

#include "oox/xls/formulabase.hxx"

namespace oox {
namespace xls {

// ============================================================================

class FormulaParserImpl;

/** Import formula parser for OOX and BIFF filters.

    This class implements formula import for the OOX and BIFF filter. One
    instance is contained in the global filter data to prevent construction and
    destruction of internal buffers for every imported formula.
 */
class FormulaParser : public FormulaProcessorBase
{
public:
    explicit            FormulaParser( const WorkbookHelper& rHelper );
    virtual             ~FormulaParser();

    /** Converts an XML formula string. */
    void                importFormula(
                            FormulaContext& rContext,
                            const ::rtl::OUString& rFormulaString ) const;

    /** Imports and converts a OOBIN token array from the passed stream. */
    void                importFormula(
                            FormulaContext& rContext,
                            RecordInputStream& rStrm ) const;

    /** Imports and converts a BIFF token array from the passed stream.
        @param pnFmlaSize  Size of the token array. If 0 is passed, reads
        it from stream (1 byte in BIFF2, 2 bytes otherwise) first. */
    void                importFormula(
                            FormulaContext& rContext,
                            BiffInputStream& rStrm,
                            const sal_uInt16* pnFmlaSize = 0 ) const;

    /** Converts the passed BIFF error code to a similar formula. */
    void                convertErrorToFormula(
                            FormulaContext& rContext,
                            sal_uInt8 nErrorCode ) const;

    /** Converts the passed token index of a defined name to a formula calling that name. */
    void                convertNameToFormula(
                            FormulaContext& rContext,
                            sal_Int32 nTokenIndex ) const;

    /** Converts the passed number into a HYPERLINK formula with the passed URL. */
    void                convertNumberToHyperlink(
                            FormulaContext& rContext,
                            const ::rtl::OUString& rUrl,
                            double fValue ) const;

    /** Converts the passed XML formula to an OLE link target. */
    ::rtl::OUString     importOleTargetLink( const ::rtl::OUString& rFormulaString );

    /** Imports and converts an OLE link target from the passed stream. */
    ::rtl::OUString     importOleTargetLink( RecordInputStream& rStrm );

    /** Imports and converts an OLE link target from the passed stream. */
    ::rtl::OUString     importOleTargetLink(
                            BiffInputStream& rStrm,
                            const sal_uInt16* pnFmlaSize = 0 ) const;

private:
    ::std::auto_ptr< FormulaParserImpl > mxImpl;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

