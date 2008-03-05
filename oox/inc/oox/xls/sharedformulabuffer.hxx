/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sharedformulabuffer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:07:32 $
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

#ifndef OOX_XLS_SHAREDFORMULABUFFER_HXX
#define OOX_XLS_SHAREDFORMULABUFFER_HXX

#include <map>
#include <memory>
#include "oox/xls/worksheethelper.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XFormulaTokens; }
    namespace sheet { class XNamedRange; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Formula context that supports shared formulas. */
class ExtCellFormulaContext : public SimpleFormulaContext, public WorksheetHelper
{
public:
    explicit            ExtCellFormulaContext(
                            const WorksheetHelper& rHelper,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaTokens >& rxTokens,
                            const ::com::sun::star::table::CellAddress& rCellPos );

    virtual void        setSharedFormula( const ::com::sun::star::table::CellAddress& rBaseAddr );
};

// ============================================================================

class SharedFormulaBuffer : public WorksheetHelper
{
public:
    explicit            SharedFormulaBuffer( const WorksheetHelper& rHelper );

    /** Imports a shared formula from a OOX formula string. */
    void                importSharedFmla( const ::rtl::OUString& rFormula,
                            const ::rtl::OUString& rSharedRange, sal_Int32 nId,
                            const ::com::sun::star::table::CellAddress& rBaseAddr );
    /** Imports a shared formula from a SHAREDFORMULA record in the passed stream */
    void                importSharedFmla( RecordInputStream& rStrm,
                            const ::com::sun::star::table::CellAddress& rBaseAddr );
    /** Imports a shared formula from a SHAREDFMLA record in the passed stream. */
    void                importSharedFmla( BiffInputStream& rStrm,
                            const ::com::sun::star::table::CellAddress& rBaseAddr );

    /** Inserts a shared formula with the passed base address into a cell
        described by the passed formula context. */
    void                setSharedFormulaCell(
                            ExtCellFormulaContext& rContext,
                            const ::com::sun::star::table::CellAddress& rBaseAddr );
    /** Inserts a shared formula with the passed base address into a cell
        described by the passed formula context. */
    void                setSharedFormulaCell(
                            ExtCellFormulaContext& rContext,
                            sal_Int32 nSharedId );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XNamedRange >
                        createDefinedName( const BinAddress& rMapKey );

    bool                implSetSharedFormulaCell(
                            ExtCellFormulaContext& rContext,
                            const BinAddress& rMapKey );

    void                updateCachedCell(
                            const ::com::sun::star::table::CellAddress& rBaseAddr,
                            const BinAddress& rMapKey );

private:
    typedef ::std::map< BinAddress, sal_Int32 >         TokenIndexMap;
    typedef ::std::auto_ptr< ExtCellFormulaContext >    ContextPtr;

    const ::rtl::OUString maIsSharedProp;   /// Property name for shared formula name flag.
    TokenIndexMap       maIndexMap;         /// Maps shared formula base address to defined name identifier.
    ContextPtr          mxLastContext;      /// Cached formula context for leading formula cell.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

