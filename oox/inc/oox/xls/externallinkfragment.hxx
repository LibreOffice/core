/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OOX_XLS_EXTERNALLINKFRAGMENT_HXX
#define OOX_XLS_EXTERNALLINKFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/externallinkbuffer.hxx"

namespace oox {
namespace xls {

class ExternalLink;

// ============================================================================
// ============================================================================

/** This class implements importing the sheetData element in external sheets.

    The sheetData element embedded in the externalBook element contains cached
    cells from externally linked sheets.
 */
class ExternalSheetDataContext : public WorkbookContextBase
{
public:
    explicit            ExternalSheetDataContext(
                            WorkbookFragmentBase& rFragment,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >& rxSheetCache );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

private:
    /** Imports cell settings from a c element. */
    void                importCell( const AttributeList& rAttribs );

    /** Imports the EXTCELL_BLANK from the passed stream. */
    void                importExtCellBlank( SequenceInputStream& rStrm );
    /** Imports the EXTCELL_BOOL from the passed stream. */
    void                importExtCellBool( SequenceInputStream& rStrm );
    /** Imports the EXTCELL_DOUBLE from the passed stream. */
    void                importExtCellDouble( SequenceInputStream& rStrm );
    /** Imports the EXTCELL_ERROR from the passed stream. */
    void                importExtCellError( SequenceInputStream& rStrm );
    /** Imports the EXTCELL_STRING from the passed stream. */
    void                importExtCellString( SequenceInputStream& rStrm );

    /** Sets the passed cell value to the current position in the sheet cache. */
    void                setCellValue( const ::com::sun::star::uno::Any& rValue );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >
                        mxSheetCache;               /// The sheet cache used to store external cell values.
    ::com::sun::star::table::CellAddress maCurrPos; /// Position of current cell.
    sal_Int32           mnCurrType;                 /// Data type of current cell.
};

// ============================================================================

class ExternalLinkFragment : public WorkbookFragmentBase
{
public:
    explicit            ExternalLinkFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            ExternalLink& rExtLink );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );
    virtual void        onEndElement();

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

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
// ============================================================================

class BiffExternalSheetDataContext : public BiffWorkbookContextBase
{
public:
    explicit            BiffExternalSheetDataContext( const WorkbookHelper& rHelper, bool bImportDefNames );
    virtual             ~BiffExternalSheetDataContext();

    /** Tries to import a record related to external links and defined names. */
    virtual void        importRecord( BiffInputStream& rStrm );

private:
    void                importExternSheet( BiffInputStream& rStrm );
    void                importExternalBook( BiffInputStream& rStrm );
    void                importExternalName( BiffInputStream& rStrm );
    void                importXct( BiffInputStream& rStrm );
    void                importCrn( BiffInputStream& rStrm );
    void                importDefinedName( BiffInputStream& rStrm );

    /** Sets the passed cell value to the passed position in the sheet cache. */
    void                setCellValue( const BinAddress& rBinAddr, const ::com::sun::star::uno::Any& rValue );

private:
    ExternalLinkRef     mxExtLink;              /// Current external link.
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >
                        mxSheetCache;           /// The sheet cache used to store external cell values.
    bool                mbImportDefNames;
};

// ============================================================================

class BiffExternalLinkFragment : public BiffWorkbookFragmentBase
{
public:
    explicit            BiffExternalLinkFragment( const BiffWorkbookFragmentBase& rParent );

    /** Imports all records related to external links. */
    virtual bool        importFragment();
};

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace oox

#endif
