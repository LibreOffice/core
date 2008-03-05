/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: workbookhelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:10:14 $
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

#ifndef OOX_XLS_WORKBOOKHELPER_HXX
#define OOX_XLS_WORKBOOKHELPER_HXX

#include <boost/shared_ptr.hpp>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "oox/xls/biffhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace container { class XNameAccess; }
    namespace container { class XNameContainer; }
    namespace awt { class XDevice; }
    namespace sheet { class XSpreadsheetDocument; }
    namespace sheet { class XSpreadsheet; }
    namespace sheet { class XNamedRanges; }
    namespace sheet { class XDatabaseRanges; }
    namespace style { class XStyle; }
} } }

namespace oox {
    class AttributeList;
    class SegmentProgressBar;
    class RecordInputStream;
}

namespace oox { namespace core {
    class BinaryFilterBase;
    class FilterBase;
    class FragmentHandler;
    class XmlFilterBase;
} }

namespace oox {
namespace xls {

// ============================================================================

/** An enumeration for all supported spreadsheet filter types. */
enum FilterType
{
    FILTER_OOX,         /// MS Excel OOXML (Office Open XML) or OOBIN.
    FILTER_BIFF,        /// MS Excel BIFF (Binary Interchange File Format).
    FILTER_UNKNOWN      /// Unknown filter type.
};

// ============================================================================

#if OSL_DEBUG_LEVEL > 0

class WorkbookHelperDebug
{
public:
    inline explicit     WorkbookHelperDebug( sal_Int32& rnCount ) : mrnCount( rnCount ) { ++mrnCount; }
    inline explicit     WorkbookHelperDebug( const WorkbookHelperDebug& rCopy ) : mrnCount( rCopy.mrnCount ) { ++mrnCount; }
    virtual             ~WorkbookHelperDebug() { --mrnCount; }
private:
    sal_Int32&          mrnCount;
};

#endif

// ============================================================================

class WorkbookData;
class WorkbookSettings;
class ViewSettings;
class WorksheetBuffer;
class ThemeBuffer;
class StylesBuffer;
class SharedStringsBuffer;
class ExternalLinkBuffer;
class DefinedNamesBuffer;
class TableBuffer;
class WebQueryBuffer;
class PivotTableBuffer;
class FormulaParser;
class UnitConverter;
class AddressConverter;
class StylesPropertyHelper;
class PageSettingsPropertyHelper;
class ValidationPropertyHelper;

/** Helper class to provice access to global workbook data.

    All classes derived from this helper class will have access to a singleton
    object of type WorkbookData containing global workbook settings, buffers,
    converters, etc. Nearly all classes in this filter implementation are
    derived directly or indirectly from this class.
 */
class WorkbookHelper
#if OSL_DEBUG_LEVEL > 0
    : private WorkbookHelperDebug
#endif
{
public:
    /*implicit*/        WorkbookHelper( WorkbookData& rBookData );
    virtual             ~WorkbookHelper();

    // filter -----------------------------------------------------------------

    /** Returns the base filter object (base class of all filters). */
    ::oox::core::FilterBase& getBaseFilter() const;
    /** Returns the file type of the current filter. */
    FilterType          getFilterType() const;
    /** Returns the filter progress bar. */
    SegmentProgressBar& getProgressBar() const;
    /** Returns true, if the file is a multi-sheet document, or false if single-sheet. */
    bool                isWorkbookFile() const;

    /** Final conversion after importing the workbook. */
    void                finalizeWorkbookImport();

    // document model ---------------------------------------------------------

    /** Returns a reference to the source/target spreadsheet document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                        getDocument() const;
    /** Returns a reference to the specified spreadsheet in the document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
                        getSheet( sal_Int32 nSheet ) const;
    /** Returns the reference device of the document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >
                        getReferenceDevice() const;
    /** Returns the container for defined names from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XNamedRanges >
                        getNamedRanges() const;
    /** Returns the container for database ranges from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDatabaseRanges >
                        getDatabaseRanges() const;
    /** Returns the container for DDE links from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        getDdeLinks() const;

    /** Returns the cell or page styles container from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        getStyleFamily( bool bPageStyles ) const;
    /** Returns the specified cell or page style from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
                        getStyleObject( const ::rtl::OUString& rStyleName, bool bPageStyle ) const;

    /** Creates a com.sun.star.style.Style object and returns its final name. */
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
                        createStyleObject(
                            ::rtl::OUString& orStyleName,
                            bool bPageStyle,
                            bool bRenameOldExisting = false );

    // buffers ----------------------------------------------------------------

    /** Returns the global workbook settings object. */
    WorkbookSettings&   getWorkbookSettings() const;
    /** Returns the workbook and sheet view settings object. */
    ViewSettings&       getViewSettings() const;
    /** Returns the worksheet buffer containing sheet names and properties. */
    WorksheetBuffer&    getWorksheets() const;
    /** Returns the office theme object read from the theme substorage. */
    ThemeBuffer&        getTheme() const;
    /** Returns all cell formatting objects read from the styles substream. */
    StylesBuffer&       getStyles() const;
    /** Returns the shared strings read from the shared strings substream. */
    SharedStringsBuffer& getSharedStrings() const;
    /** Returns the external links read from the external links substream. */
    ExternalLinkBuffer& getExternalLinks() const;
    /** Returns the defined names read from the workbook globals. */
    DefinedNamesBuffer& getDefinedNames() const;
    /** Returns the tables collection (equivalent to Calc's database ranges). */
    TableBuffer&        getTables() const;
    /** Returns the web queries. */
    WebQueryBuffer&     getWebQueries() const;
    /** Returns the pivot tables. */
    PivotTableBuffer&   getPivotTables() const;

    // converters -------------------------------------------------------------

    /** Returns the import formula parser. */
    FormulaParser&      getFormulaParser() const;
    /** Returns the measurement unit converter. */
    UnitConverter&      getUnitConverter() const;
    /** Returns the converter for string to cell address/range conversion. */
    AddressConverter&   getAddressConverter() const;
    /** Returns the converter for properties related to cell styles. */
    StylesPropertyHelper& getStylesPropertyHelper() const;
    /** Returns the converter for properties related to page and print settings. */
    PageSettingsPropertyHelper& getPageSettingsPropertyHelper() const;
    /** Returns the converter for properties related to data validation. */
    ValidationPropertyHelper& getValidationPropertyHelper() const;

    // OOX specific -----------------------------------------------------------

    /** Returns the base OOX filter object.
        Must not be called, if current filter is not the OOX filter. */
    ::oox::core::XmlFilterBase& getOoxFilter() const;

    /** Imports a fragment using the passed fragment handler, which contains
        the full path to the fragment stream. */
    bool                importOoxFragment( const ::rtl::Reference< ::oox::core::FragmentHandler >& rxHandler );

    // BIFF specific ----------------------------------------------------------

    /** Returns the base BIFF filter object. */
    ::oox::core::BinaryFilterBase& getBiffFilter() const;
    /** Returns the BIFF type in binary filter. */
    BiffType            getBiff() const;

    /** Returns the text encoding used to import/export byte strings. */
    rtl_TextEncoding    getTextEncoding() const;
    /** Sets the text encoding to import/export byte strings. */
    void                setTextEncoding( rtl_TextEncoding eTextEnc );
    /** Sets code page read from a CODEPAGE record for byte string import. */
    void                setCodePage( sal_uInt16 nCodePage );
    /** Sets text encoding from the default application font, if CODEPAGE record is missing. */
    void                setAppFontEncoding( rtl_TextEncoding eAppFontEnc );

    /** Enables workbook file mode, used for BIFF4 workspace files. */
    void                setIsWorkbookFile();
    /** Recreates global buffers that are used per sheet in specific BIFF versions. */
    void                createBuffersPerSheet();

    /** Looks for a password provided via API, or queries it via GUI. */
    ::rtl::OUString     queryPassword() const;

private:
    WorkbookData&       mrBookData;
};

// ============================================================================

namespace prv {

typedef ::boost::shared_ptr< WorkbookData > WorkbookDataRef;

struct WorkbookDataOwner
{
    explicit            WorkbookDataOwner( WorkbookDataRef xBookData );
    virtual             ~WorkbookDataOwner();
    WorkbookDataRef     mxBookData;
};

} // namespace prv

// ----------------------------------------------------------------------------

class WorkbookHelperRoot : private prv::WorkbookDataOwner, public WorkbookHelper
{
public:
    explicit            WorkbookHelperRoot( ::oox::core::XmlFilterBase& rFilter );
    explicit            WorkbookHelperRoot( ::oox::core::BinaryFilterBase& rFilter, BiffType eBiff );

    /** Returns true, if this helper refers to a valid document. */
    bool                isValid() const;

private:
                        WorkbookHelperRoot( const WorkbookHelperRoot& );
    WorkbookHelperRoot& operator=( const WorkbookHelperRoot& );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

