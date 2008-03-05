/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: externallinkbuffer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:03:51 $
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

#ifndef OOX_XLS_EXTERNALLINKBUFFER_HXX
#define OOX_XLS_EXTERNALLINKBUFFER_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/xls/defnamesbuffer.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XDDELink; }
} } }

namespace oox { namespace core {
    class Relations;
} }

namespace oox {
namespace xls {

// ============================================================================

/** Contains indexes for a range of sheets in the spreadsheet document. */
struct LinkSheetRange
{
    sal_Int32           mnFirst;        /// Index of the first sheet.
    sal_Int32           mnLast;         /// Index of the last sheet.
    bool                mbRel;          /// True = relative indexes.

    inline explicit     LinkSheetRange() { setDeleted(); }
    inline explicit     LinkSheetRange( sal_Int32 nFirst, sal_Int32 nLast ) { setRange( nFirst, nLast ); }

    /** Sets this struct to deleted state. */
    inline void         setDeleted() { mnFirst = mnLast = -1; mbRel = false; }
    /** Sets this struct to use current sheet state. */
    inline void         setRelative() { mnFirst = mnLast = 0; mbRel = true; }
    /** Sets the passed absolute sheet range to the memebers of this struct. */
    inline void         setRange( sal_Int32 nFirst, sal_Int32 nLast )
                            { mnFirst = ::std::min( nFirst, nLast ); mnLast = ::std::max( nFirst, nLast ); mbRel = false; }

    /** Returns true, if the sheet indexes are valid and different. */
    inline bool         is3dRange() const { return (0 <= mnFirst) && (mnFirst < mnLast); }
};

// ============================================================================

struct OoxExternalNameData
{
    bool                mbBuiltIn;          /// Name is a built-in name.
    bool                mbNotify;           /// Notify application on data change.
    bool                mbPreferPic;        /// Picture link.
    bool                mbStdDocName;       /// Name is the StdDocumentName for DDE.
    bool                mbOleObj;           /// Name is an OLE object.
    bool                mbIconified;        /// Iconified object link.

    explicit            OoxExternalNameData();
};

// ============================================================================

class ExternalLink;

class ExternalName : public DefinedNameBase
{
public:
    explicit            ExternalName(
                            const ExternalLink& rParentLink,
                            sal_Int32 nLocalSheet );

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
    void                importExternalName( RecordInputStream& rStrm );
    /** Imports the EXTERNALNAMEFLAGS record containing the settings of an external name. */
    void                importExternalNameFlags( RecordInputStream& rStrm );
    /** Imports the DDEITEMVALUES record containing the size of the DDE result matrix. */
    void                importDdeItemValues( RecordInputStream& rStrm );
    /** Imports the DDEITEM_BOOL record containing a boolean value in a link result. */
    void                importDdeItemBool( RecordInputStream& rStrm );
    /** Imports the DDEITEM_DOUBLE record containing a double value in a link result. */
    void                importDdeItemDouble( RecordInputStream& rStrm );
    /** Imports the DDEITEM_ERROR record containing an error code in a link result. */
    void                importDdeItemError( RecordInputStream& rStrm );
    /** Imports the DDEITEM_STRING record containing a string in a link result. */
    void                importDdeItemString( RecordInputStream& rStrm );

    /** Imports the EXTERNALNAME record from the passed stream. */
    void                importExternalName( BiffInputStream& rStrm );

    /** Returns true, if the name refers to an OLE object. */
    inline bool         isOleObject() const { return maOoxExtNameData.mbOleObj; }

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
    OoxExternalNameData maOoxExtNameData;   /// Additional name data.
    ResultMatrix        maResults;          /// DDE/OLE link results.
    ResultMatrix::iterator maCurrIt;        /// Current position in result matrix.
    sal_uInt32          mnStorageId;        /// OLE storage identifier.
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDDELink >
                        mxDdeLink;          /// Interface of a DDE link.
    bool                mbDdeLinkCreated;   /// True = already tried to create the DDE link.
};

typedef ::boost::shared_ptr< ExternalName > ExternalNameRef;

// ============================================================================

enum ExternalLinkType
{
    LINKTYPE_SELF,          /// Link refers to the current workbook.
    LINKTYPE_SAME,          /// Link refers to the current sheet.
    LINKTYPE_INTERNAL,      /// Link refers to a sheet in the own workbook.
    LINKTYPE_EXTERNAL,      /// Link refers to an external spreadsheet document.
    LINKTYPE_ANALYSIS,      /// Link refers to Analysis add-in.
    LINKTYPE_DDE,           /// DDE link.
    LINKTYPE_OLE,           /// OLE link.
    LINKTYPE_MAYBE_DDE_OLE, /// Could be DDE or OLE link (BIFF only).
    LINKTYPE_UNKNOWN        /// Unknown or unsupported link type.
};

// ============================================================================

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
    void                importExternalBook( const ::oox::core::Relations& rRelations, RecordInputStream& rStrm );
    /** Imports the EXTSHEETNAMES record containing the sheet names in an externally linked document. */
    void                importExtSheetNames( RecordInputStream& rStrm );
    /** Imports the EXTERNALNAME record describing an external name. */
    ExternalNameRef     importExternalName( RecordInputStream& rStrm );
    /** Imports the EXTERNALREF record from the passed stream. */
    void                importExternalRef( RecordInputStream& rStrm );
    /** Imports the EXTERNALSELF record from the passed stream. */
    void                importExternalSelf( RecordInputStream& rStrm );
    /** Imports the EXTERNALSAME record from the passed stream. */
    void                importExternalSame( RecordInputStream& rStrm );
    /** Imports the EXTERNALADDIN record from the passed stream. */
    void                importExternalAddin( RecordInputStream& rStrm );

    /** Imports the EXTERNSHEET record from the passed stream. */
    void                importExternSheet( BiffInputStream& rStrm );
    /** Imports the EXTERNALBOOK record from the passed stream. */
    void                importExternalBook( BiffInputStream& rStrm );
    /** Imports the EXTERNALNAME record from the passed stream. */
    void                importExternalName( BiffInputStream& rStrm );

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

    /** Returns the internal sheet index for the specified external sheet index. */
    sal_Int32           getSheetIndex( sal_Int32 nTabId = 0 ) const;
    /** Returns the sheet range for the specified sheets (BIFF only). */
    void                getSheetRange( LinkSheetRange& orSheetRange, sal_Int32 nTabId1, sal_Int32 nTabId2 ) const;
    /** Returns the external name with the passed zero-based index. */
    ExternalNameRef     getNameByIndex( sal_Int32 nIndex ) const;

private:
    void                setExternalTargetUrl( const ::rtl::OUString& rTargetUrl );
    void                setDdeOleTargetUrl( const ::rtl::OUString& rClassName, const ::rtl::OUString& rTargetUrl, ExternalLinkType eLinkType );
    ::rtl::OUString     parseBiffTargetUrl( const ::rtl::OUString& rBiffTargetUrl );

    ExternalNameRef     createExternalName();

private:
    typedef ::std::vector< sal_Int32 >  SheetIndexVec;
    typedef RefVector< ExternalName >   ExternalNameVec;

    ExternalLinkType    meLinkType;
    ::rtl::OUString     maRelId;
    ::rtl::OUString     maClassName;
    ::rtl::OUString     maTargetUrl;
    SheetIndexVec       maSheetIndexes;
    ExternalNameVec     maExtNames;
};

typedef ::boost::shared_ptr< ExternalLink > ExternalLinkRef;

// ============================================================================

/** Represents a REF entry in the OOBIN EXTERNALSHEETS or in the BIFF8
    EXTERNSHEET record.

    This struct is used to map ref identifiers to external books (OOBIN:
    EXTERNALREF records, BIFF8: EXTERNALBOOK records), and provides sheet
    indexes into the sheet list of the external document.
 */
struct OoxRefSheets
{
    sal_Int32           mnExtRefId;         /// Zero-based index into list of external documents.
    sal_Int32           mnTabId1;           /// Zero-based index to first sheet in external document.
    sal_Int32           mnTabId2;           /// Zero-based index to last sheet in external document.

    explicit            OoxRefSheets();

    void                readOobData( RecordInputStream& rStrm );
    void                readBiff8Data( BiffInputStream& rStrm );
};

// ----------------------------------------------------------------------------

class ExternalLinkBuffer : public WorkbookHelper
{
public:
    explicit            ExternalLinkBuffer( const WorkbookHelper& rHelper );

    /** Imports the externalReference element containing . */
    ExternalLinkRef     importExternalReference( const AttributeList& rAttribs );

    /** Imports the EXTERNALREF record from the passed stream. */
    ExternalLinkRef     importExternalRef( RecordInputStream& rStrm );
    /** Imports the EXTERNALSELF record from the passed stream. */
    void                importExternalSelf( RecordInputStream& rStrm );
    /** Imports the EXTERNALSAME record from the passed stream. */
    void                importExternalSame( RecordInputStream& rStrm );
    /** Imports the EXTERNALADDIN record from the passed stream. */
    void                importExternalAddin( RecordInputStream& rStrm );
    /** Imports the EXTERNALSHEETS record from the passed stream. */
    void                importExternalSheets( RecordInputStream& rStrm );

    /** Imports the EXTERNSHEET record from the passed stream. */
    ExternalLinkRef     importExternSheet( BiffInputStream& rStrm );
    /** Imports the EXTERNALBOOK record from the passed stream. */
    ExternalLinkRef     importExternalBook( BiffInputStream& rStrm );
    /** Imports the EXTERNALNAME record from the passed stream. */
    void                importExternalName( BiffInputStream& rStrm );
    /** Imports the BIFF8 EXTERNSHEET record from the passed stream. */
    void                importExternSheet8( BiffInputStream& rStrm );

    /** Returns the external link for the passed reference identifier. */
    ExternalLinkRef     getExternalLink( sal_Int32 nRefId ) const;

    /** Returns the sheet range for the specified reference (BIFF2-BIFF5 only). */
    LinkSheetRange      getSheetRange( sal_Int32 nRefId, sal_Int16 nTabId1, sal_Int16 nTabId2 ) const;
    /** Returns the sheet range for the specified reference (BIFF8 only). */
    LinkSheetRange      getSheetRange( sal_Int32 nRefId ) const;

private:
    /** Creates a new external link and inserts it into the list of links. */
    ExternalLinkRef     createExternalLink();

    /** Returns the specified sheet indexes for a reference identifier. */
    const OoxRefSheets* getRefSheets( sal_Int32 nRefId ) const;

private:
    typedef RefVector< ExternalLink >       ExternalLinkVec;
    typedef ::std::vector< OoxRefSheets >   OoxRefSheetsVec;

    ExternalLinkVec     maExtLinks;         /// List of external documents.
    OoxRefSheetsVec     maRefSheets;        /// Sheet indexes for reference ids.
    bool                mbUseRefSheets;     /// True = use maRefSheets list (OOBIN only).
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

