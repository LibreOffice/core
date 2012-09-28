/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef OOX_XLS_EXTERNALLINKBUFFER_HXX
#define OOX_XLS_EXTERNALLINKBUFFER_HXX

#include <com/sun/star/sheet/ExternalLinkInfo.hpp>
#include "oox/helper/containerhelper.hxx"
#include "defnamesbuffer.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { struct DDEItemInfo; }
    namespace sheet { class XDDELink; }
    namespace sheet { class XExternalDocLink; }
    namespace sheet { class XExternalSheetCache; }
} } }

namespace oox { namespace core {
    class Relations;
} }

namespace oox {
namespace xls {

// ============================================================================

struct ExternalNameModel
{
    bool                mbBuiltIn;          /// Name is a built-in name.
    bool                mbNotify;           /// Notify application on data change.
    bool                mbPreferPic;        /// Picture link.
    bool                mbStdDocName;       /// Name is the StdDocumentName for DDE.
    bool                mbOleObj;           /// Name is an OLE object.
    bool                mbIconified;        /// Iconified object link.

    explicit            ExternalNameModel();
};

// ============================================================================

class ExternalLink;

class ExternalName : public DefinedNameBase
{
public:
    explicit            ExternalName( const ExternalLink& rParentLink );

    /** Appends the passed value to the result set. */
    template< typename Type >
    inline void         appendResultValue( const Type& rValue )
                            { if( maCurrIt != maResults.end() ) (*maCurrIt++) <<= rValue; }

    /** Imports the definedName element. */
    void                importDefinedName( const AttributeList& rAttribs );
    /** Imports the ddeItem element describing an item of a DDE link. */
    void                importDdeItem( const AttributeList& rAttribs );
    /** Imports the values element containing the size of the DDE result matrix. */
    void                importValues( const AttributeList& rAttribs );
    /** Imports the oleItem element describing an object of an OLE link. */
    void                importOleItem( const AttributeList& rAttribs );

    /** Imports the EXTERNALNAME record containing the name (only). */
    void                importExternalName( SequenceInputStream& rStrm );
    /** Imports the EXTERNALNAMEFLAGS record containing the settings of an external name. */
    void                importExternalNameFlags( SequenceInputStream& rStrm );
    /** Imports the DDEITEMVALUES record containing the size of the DDE result matrix. */
    void                importDdeItemValues( SequenceInputStream& rStrm );
    /** Imports the DDEITEM_BOOL record containing a boolean value in a link result. */
    void                importDdeItemBool( SequenceInputStream& rStrm );
    /** Imports the DDEITEM_DOUBLE record containing a double value in a link result. */
    void                importDdeItemDouble( SequenceInputStream& rStrm );
    /** Imports the DDEITEM_ERROR record containing an error code in a link result. */
    void                importDdeItemError( SequenceInputStream& rStrm );
    /** Imports the DDEITEM_STRING record containing a string in a link result. */
    void                importDdeItemString( SequenceInputStream& rStrm );

    /** Returns true, if the name refers to an OLE object. */
    inline bool         isOleObject() const { return maExtNameModel.mbOleObj; }

#if 0
    /** Returns the sheet cache index if this is a sheet-local external name. */
    sal_Int32           getSheetCacheIndex() const;
#endif

    /** Returns the DDE item info needed by the XML formula parser. */
    bool                getDdeItemInfo(
                            ::com::sun::star::sheet::DDEItemInfo& orItemInfo ) const;

    /** Returns the complete DDE link data of this DDE item. */
    bool                getDdeLinkData(
                            ::rtl::OUString& orDdeServer,
                            ::rtl::OUString& orDdeTopic,
                            ::rtl::OUString& orDdeItem );

private:
    /** Sets the size of the result matrix. */
    void                setResultSize( sal_Int32 nColumns, sal_Int32 nRows );

private:
    typedef Matrix< ::com::sun::star::uno::Any > ResultMatrix;

    const ExternalLink& mrParentLink;       /// External link this name belongs to.
    ExternalNameModel   maExtNameModel;     /// Additional name data.
    ResultMatrix        maResults;          /// DDE/OLE link results.
    ResultMatrix::iterator maCurrIt;        /// Current position in result matrix.
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDDELink >
                        mxDdeLink;          /// Interface of a DDE link.
    bool                mbDdeLinkCreated;   /// True = already tried to create the DDE link.
};

typedef ::boost::shared_ptr< ExternalName > ExternalNameRef;

// ============================================================================

/** Contains indexes for a range of sheets in the spreadsheet document. */
class LinkSheetRange
{
public:
    inline explicit     LinkSheetRange() { setDeleted(); }
    inline explicit     LinkSheetRange( sal_Int32 nFirst, sal_Int32 nLast ) { setRange( nFirst, nLast ); }
    inline explicit     LinkSheetRange( sal_Int32 nDocLink, sal_Int32 nFirst, sal_Int32 nLast ) { setExternalRange( nDocLink, nFirst, nLast ); }

    /** Sets this struct to deleted state. */
    void                setDeleted();
    /** Sets this struct to "use current sheet" state. */
    void                setSameSheet();
    /** Sets the passed absolute sheet range to the members of this struct. */
    void                setRange( sal_Int32 nFirst, sal_Int32 nLast );
    /** Sets the passed external sheet cache range to the members of this struct. */
    void                setExternalRange( sal_Int32 nDocLink, sal_Int32 nFirst, sal_Int32 nLast );

    /** Returns true, if the sheet indexes are valid and different. */
    inline bool         isDeleted() const { return mnFirst < 0; }
    /** Returns true, if the sheet range points to an external document. */
    inline bool         isExternal() const { return !isDeleted() && (meType == LINKSHEETRANGE_EXTERNAL); }
    /** Returns true, if the sheet indexes are valid and different. */
    inline bool         isSameSheet() const { return meType == LINKSHEETRANGE_SAMESHEET; }
    /** Returns true, if the sheet indexes are valid and different. */
    inline bool         is3dRange() const { return (0 <= mnFirst) && (mnFirst < mnLast); }

    inline sal_Int32    getDocLinkIndex() const { return mnDocLink; }
    inline sal_Int32    getFirstSheet() const { return mnFirst; }
    inline sal_Int32    getLastSheet() const { return mnLast; }

private:
    enum LinkSheetRangeType
    {
        LINKSHEETRANGE_INTERNAL,    /// Sheet range in the own document.
        LINKSHEETRANGE_EXTERNAL,    /// Sheet range in an external document.
        LINKSHEETRANGE_SAMESHEET    /// Current sheet depending on context.
    };

    LinkSheetRangeType  meType;         /// Link sheet range type.
    sal_Int32           mnDocLink;      /// Document link token index for external links.
    sal_Int32           mnFirst;        /// Index of the first sheet or index of first external sheet cache.
    sal_Int32           mnLast;         /// Index of the last sheet or index of last external sheet cache.
};

// ============================================================================

enum ExternalLinkType
{
    LINKTYPE_SELF,          /// Link refers to the current workbook.
    LINKTYPE_SAME,          /// Link refers to the current sheet.
    LINKTYPE_INTERNAL,      /// Link refers to a sheet in the own workbook.
    LINKTYPE_EXTERNAL,      /// Link refers to an external spreadsheet document.
    LINKTYPE_ANALYSIS,      /// Link refers to the Analysis add-in.
    LINKTYPE_LIBRARY,       /// Link refers to an external add-in.
    LINKTYPE_DDE,           /// DDE link.
    LINKTYPE_OLE,           /// OLE link.
    LINKTYPE_MAYBE_DDE_OLE, /// Could be DDE or OLE link (BIFF only).
    LINKTYPE_UNKNOWN        /// Unknown or unsupported link type.
};

// ----------------------------------------------------------------------------

class ExternalLink : public WorkbookHelper
{
public:
    explicit            ExternalLink( const WorkbookHelper& rHelper );

    /** Imports the externalReference element containing the relation identifier. */
    void                importExternalReference( const AttributeList& rAttribs );
    /** Imports the externalBook element describing an externally linked document. */
    void                importExternalBook( const ::oox::core::Relations& rRelations, const AttributeList& rAttribs );
    /** Imports the sheetName element containing the sheet name in an externally linked document. */
    void                importSheetName( const AttributeList& rAttribs );
    /** Imports the definedName element describing an external name. */
    void                importDefinedName( const AttributeList& rAttribs );
    /** Imports the ddeLink element describing a DDE link. */
    void                importDdeLink( const AttributeList& rAttribs );
    /** Imports the ddeItem element describing an item of a DDE link. */
    ExternalNameRef     importDdeItem( const AttributeList& rAttribs );
    /** Imports the oleLink element describing an OLE link. */
    void                importOleLink( const ::oox::core::Relations& rRelations, const AttributeList& rAttribs );
    /** Imports the oleItem element describing an object of an OLE link. */
    ExternalNameRef     importOleItem( const AttributeList& rAttribs );

    /** Imports the EXTERNALBOOK record describing an externally linked document, DDE link, or OLE link. */
    void                importExternalBook( const ::oox::core::Relations& rRelations, SequenceInputStream& rStrm );
    /** Imports the EXTSHEETNAMES record containing the sheet names in an externally linked document. */
    void                importExtSheetNames( SequenceInputStream& rStrm );
    /** Imports the EXTERNALNAME record describing an external name. */
    ExternalNameRef     importExternalName( SequenceInputStream& rStrm );
    /** Imports the EXTERNALREF record from the passed stream. */
    void                importExternalRef( SequenceInputStream& rStrm );
    /** Imports the EXTERNALSELF record from the passed stream. */
    void                importExternalSelf( SequenceInputStream& rStrm );
    /** Imports the EXTERNALSAME record from the passed stream. */
    void                importExternalSame( SequenceInputStream& rStrm );
    /** Imports the EXTERNALADDIN record from the passed stream. */
    void                importExternalAddin( SequenceInputStream& rStrm );

    /** Sets the link type to 'self reference'. */
    inline void         setSelfLinkType() { meLinkType = LINKTYPE_SELF; }

    /** Returns the type of this external link. */
    inline ExternalLinkType getLinkType() const { return meLinkType; }
    /** Returns true, if the link refers to the current workbook. */
    inline bool         isInternalLink() const { return (meLinkType == LINKTYPE_SELF) || (meLinkType == LINKTYPE_INTERNAL); }

    /** Returns the relation identifier for the external link fragment. */
    inline const ::rtl::OUString& getRelId() const { return maRelId; }
    /** Returns the class name of this external link. */
    inline const ::rtl::OUString& getClassName() const { return maClassName; }
    /** Returns the target URL of this external link. */
    inline const ::rtl::OUString& getTargetUrl() const { return maTargetUrl; }
    /** Returns the link info needed by the XML formula parser. */
    ::com::sun::star::sheet::ExternalLinkInfo getLinkInfo() const;

    /** Returns the type of the external library if this is a library link. */
    FunctionLibraryType getFuncLibraryType() const;

    /** Returns the token index of the external document. */
    sal_Int32           getDocumentLinkIndex() const;
    /** Returns the external sheet cache index or for the passed sheet. */
    sal_Int32           getSheetCacheIndex( sal_Int32 nTabId = 0 ) const;
    /** Returns the sheet cache of the external sheet with the passed index. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >
                        getSheetCache( sal_Int32 nTabId ) const;

    /** Returns the internal sheet range or range of external sheet caches for the passed sheet range (BIFF only). */
    void                getSheetRange( LinkSheetRange& orSheetRange, sal_Int32 nTabId1, sal_Int32 nTabId2 ) const;

    /** Returns the external name with the passed zero-based index. */
    ExternalNameRef     getNameByIndex( sal_Int32 nIndex ) const;

private:
    void                setExternalTargetUrl( const ::rtl::OUString& rTargetUrl, const ::rtl::OUString& rTargetType );
    void                setDdeOleTargetUrl( const ::rtl::OUString& rClassName, const ::rtl::OUString& rTargetUrl, ExternalLinkType eLinkType );
    void                parseExternalReference( const ::oox::core::Relations& rRelations, const ::rtl::OUString& rRelId );

    /** Creates an external locument link and the sheet cache for the passed sheet name. */
    void                insertExternalSheet( const ::rtl::OUString& rSheetName );

    ExternalNameRef     createExternalName();

private:
    typedef ::std::vector< sal_Int16 >  Int16Vector;
    typedef ::std::vector< sal_Int32 >  Int32Vector;
    typedef RefVector< ExternalName >   ExternalNameVector;

    ExternalLinkType    meLinkType;         /// Type of this link object.
    FunctionLibraryType meFuncLibType;      /// Type of the function library, if link type is LINKTYPE_LIBRARY.
    ::rtl::OUString     maRelId;            /// Relation identifier for the external link fragment.
    ::rtl::OUString     maClassName;        /// DDE service, OLE class name.
    ::rtl::OUString     maTargetUrl;        /// Target link, DDE topic, OLE target.
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalDocLink >
                        mxDocLink;          /// Interface for an external document.
    Int16Vector         maCalcSheets;       /// Internal sheet indexes.
    Int32Vector         maSheetCaches;      /// External sheet cache indexes.
    ExternalNameVector  maExtNames;         /// Defined names in external document.
};

typedef ::boost::shared_ptr< ExternalLink > ExternalLinkRef;

// ============================================================================

/** Represents a REF entry in the BIFF12 EXTERNALSHEETS or in the BIFF8
    EXTERNSHEET record.

    This struct is used to map ref identifiers to external books (BIFF12:
    EXTERNALREF records, BIFF8: EXTERNALBOOK records), and provides sheet
    indexes into the sheet list of the external document.
 */
struct RefSheetsModel
{
    sal_Int32           mnExtRefId;         /// Zero-based index into list of external documents.
    sal_Int32           mnTabId1;           /// Zero-based index to first sheet in external document.
    sal_Int32           mnTabId2;           /// Zero-based index to last sheet in external document.

    explicit            RefSheetsModel();

    void                readBiff12Data( SequenceInputStream& rStrm );
};

// ----------------------------------------------------------------------------

class ExternalLinkBuffer : public WorkbookHelper
{
public:
    explicit            ExternalLinkBuffer( const WorkbookHelper& rHelper );

    /** Imports the externalReference element containing . */
    ExternalLinkRef     importExternalReference( const AttributeList& rAttribs );

    /** Imports the EXTERNALREF record from the passed stream. */
    ExternalLinkRef     importExternalRef( SequenceInputStream& rStrm );
    /** Imports the EXTERNALSELF record from the passed stream. */
    void                importExternalSelf( SequenceInputStream& rStrm );
    /** Imports the EXTERNALSAME record from the passed stream. */
    void                importExternalSame( SequenceInputStream& rStrm );
    /** Imports the EXTERNALADDIN record from the passed stream. */
    void                importExternalAddin( SequenceInputStream& rStrm );
    /** Imports the EXTERNALSHEETS record from the passed stream. */
    void                importExternalSheets( SequenceInputStream& rStrm );

    /** Returns the sequence of link infos needed by the XML formula parser. */
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::ExternalLinkInfo >
                        getLinkInfos() const;

    /** Returns the external link for the passed reference identifier. */
    ExternalLinkRef     getExternalLink( sal_Int32 nRefId, bool bUseRefSheets = true ) const;

    /** Returns the sheet range for the specified reference (BIFF2-BIFF5 only). */
    LinkSheetRange      getSheetRange( sal_Int32 nRefId, sal_Int16 nTabId1, sal_Int16 nTabId2 ) const;
    /** Returns the sheet range for the specified reference (BIFF8 only). */
    LinkSheetRange      getSheetRange( sal_Int32 nRefId ) const;

private:
    /** Creates a new external link and inserts it into the list of links. */
    ExternalLinkRef     createExternalLink();

    /** Returns the specified sheet indexes for a reference identifier. */
    const RefSheetsModel* getRefSheets( sal_Int32 nRefId ) const;

private:
    typedef RefVector< ExternalLink >       ExternalLinkVec;
    typedef ::std::vector< RefSheetsModel > RefSheetsModelVec;

    ExternalLinkRef     mxSelfRef;          /// Implicit self reference at index 0.
    ExternalLinkVec     maLinks;            /// List of link structures for all kinds of links.
    ExternalLinkVec     maExtLinks;         /// Real external links needed for formula parser.
    RefSheetsModelVec   maRefSheets;        /// Sheet indexes for reference ids.
    bool                mbUseRefSheets;     /// True = use maRefSheets list (BIFF12 only).
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
