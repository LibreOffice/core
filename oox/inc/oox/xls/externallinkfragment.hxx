/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: externallinkfragment.hxx,v $
 * $Revision: 1.4.20.2 $
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

#ifndef OOX_XLS_EXTERNALLINKFRAGMENT_HXX
#define OOX_XLS_EXTERNALLINKFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/externallinkbuffer.hxx"

namespace oox {
namespace xls {

class ExternalLink;

// ============================================================================

/** This class implements importing the sheetData element in external sheets.

    The sheetData element embedded in the externalBook element contains cached
    cells from externally linked sheets.
 */
class OoxExternalSheetDataContext : public OoxWorkbookContextBase
{
public:
    explicit            OoxExternalSheetDataContext(
                            OoxWorkbookFragmentBase& rFragment,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >& rxSheetCache );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

private:
    /** Imports cell settings from a c element. */
    void                importCell( const AttributeList& rAttribs );

    /** Imports the EXTCELL_BLANK from the passed stream. */
    void                importExtCellBlank( RecordInputStream& rStrm );
    /** Imports the EXTCELL_BOOL from the passed stream. */
    void                importExtCellBool( RecordInputStream& rStrm );
    /** Imports the EXTCELL_DOUBLE from the passed stream. */
    void                importExtCellDouble( RecordInputStream& rStrm );
    /** Imports the EXTCELL_ERROR from the passed stream. */
    void                importExtCellError( RecordInputStream& rStrm );
    /** Imports the EXTCELL_STRING from the passed stream. */
    void                importExtCellString( RecordInputStream& rStrm );

    /** Sets the passed cell value to the current position in the sheet cache. */
    void                setCellValue( const ::com::sun::star::uno::Any& rValue );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >
                        mxSheetCache;               /// The sheet cache used to store external cell values.
    ::com::sun::star::table::CellAddress maCurrPos; /// Position of current cell.
    sal_Int32           mnCurrType;                 /// Data type of current cell.
};

// ============================================================================

class OoxExternalLinkFragment : public OoxWorkbookFragmentBase
{
public:
    explicit            OoxExternalLinkFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            ExternalLink& rExtLink );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;

private:
    ::oox::core::ContextHandlerRef createSheetDataContext( sal_Int32 nSheetId );

private:
    ExternalLink&       mrExtLink;
    ExternalNameRef     mxExtName;
    ::rtl::OUString     maResultValue;
    sal_Int32           mnResultType;
};

// ============================================================================

class BiffExternalLinkFragment : public BiffWorkbookFragmentBase
{
public:
    explicit            BiffExternalLinkFragment( const BiffWorkbookFragmentBase& rParent, bool bImportDefNames );
    virtual             ~BiffExternalLinkFragment();

    /** Imports all records related to external links. */
    virtual bool        importFragment();

    /** Tries to import a record related to external links and defined names. */
    void                importRecord();

    /** Finalizes buffers related to external links and defined names. */
    void                finalizeImport();

private:
    void                importExternSheet();
    void                importExternalBook();
    void                importExternalName();
    void                importXct();
    void                importCrn();
    void                importDefinedName();

    /** Sets the passed cell value to the passed position in the sheet cache. */
    void                setCellValue( const BinAddress& rBinAddr, const ::com::sun::star::uno::Any& rValue );

private:
    ExternalLinkRef     mxExtLink;              /// Current external link.
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >
                        mxSheetCache;           /// The sheet cache used to store external cell values.
    bool                mbImportDefNames;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

