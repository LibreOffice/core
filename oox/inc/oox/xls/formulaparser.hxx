/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formulaparser.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
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

private:
    ::std::auto_ptr< FormulaParserImpl > mxImpl;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

