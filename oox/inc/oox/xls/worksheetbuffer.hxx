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



#ifndef OOX_XLS_WORKSHEETBUFFER_HXX
#define OOX_XLS_WORKSHEETBUFFER_HXX

#include <utility>
#include "oox/helper/refmap.hxx"
#include "oox/helper/refvector.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace i18n { class XCharacterClassification; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Contains data from the 'sheet' element describing a sheet in the workbook. */
struct SheetInfoModel
{
    ::rtl::OUString     maRelId;        /// Relation identifier for the sheet substream.
    ::rtl::OUString     maName;         /// Original name of the sheet.
    sal_Int64           mnBiffHandle;   /// BIFF record handle of the sheet substream.
    sal_Int32           mnSheetId;      /// Sheet identifier.
    sal_Int32           mnState;        /// Visibility state.

    explicit            SheetInfoModel();
};

// ============================================================================

/** Stores information about all sheets in a spreadsheet document.

    Information about sheets includes the sheet name, the visibility state, and
    for the OOXML filter, the relation identifier of the sheet used to obtain
    the related worksheet fragment.
 */
class WorksheetBuffer : public WorkbookHelper
{
public:
    explicit            WorksheetBuffer( const WorkbookHelper& rHelper );

    /** Returns the base file name without path and file extension. */
    static ::rtl::OUString getBaseFileName( const ::rtl::OUString& rUrl );

    /** Initializes the buffer for single sheet files (BIFF2-BIFF4). */
    void                initializeSingleSheet();

    /** Imports the attributes of a sheet element. */
    void                importSheet( const AttributeList& rAttribs );
    /** Imports the SHEET record from the passed BIFF12 stream. */
    void                importSheet( SequenceInputStream& rStrm );
    /** Imports the SHEET record from the passed BIFF stream. */
    void                importSheet( BiffInputStream& rStrm );
    /** Inserts a new empty sheet into the document. Looks for an unused name.
         @return  Index of the new sheet in the Calc document. */
    sal_Int16           insertEmptySheet( const ::rtl::OUString& rPreferredName, bool bVisible );

    /** Returns the number of original sheets contained in the workbook. */
    sal_Int32           getWorksheetCount() const;
    /** Returns the OOXML relation identifier of the specified worksheet. */
    ::rtl::OUString     getWorksheetRelId( sal_Int32 nWorksheet ) const;
    /** Returns the BIFF record handle of the associated sheet substream. */
    sal_Int64           getBiffRecordHandle( sal_Int32 nWorksheet ) const;

    /** Returns the Calc index of the specified worksheet. */
    sal_Int16           getCalcSheetIndex( sal_Int32 nWorksheet ) const;
    /** Returns the finalized name of the specified worksheet. */
    ::rtl::OUString     getCalcSheetName( sal_Int32 nWorksheet ) const;

    /** Returns the Calc index of the sheet with the passed original worksheet name. */
    sal_Int16           getCalcSheetIndex( const ::rtl::OUString& rWorksheetName ) const;
    /** Returns the finalized name of the sheet with the passed worksheet name. */
    ::rtl::OUString     getCalcSheetName( const ::rtl::OUString& rWorksheetName ) const;

private:
    struct SheetInfo : public SheetInfoModel
    {
        ::rtl::OUString     maCalcName;
        ::rtl::OUString     maCalcQuotedName;
        sal_Int16           mnCalcSheet;

        explicit            SheetInfo( const SheetInfoModel& rModel, sal_Int16 nCalcSheet, const ::rtl::OUString& rCalcName );
    };

    typedef ::std::pair< sal_Int16, ::rtl::OUString > IndexNamePair;

    /** Creates a new sheet in the Calc document. Does not insert anything in the own lists. */
    IndexNamePair       createSheet( const ::rtl::OUString& rPreferredName, sal_Int32 nSheetPos, bool bVisible );
    /** Creates a new sheet in the Calc document and inserts the related SheetInfo. */
    void                insertSheet( const SheetInfoModel& rModel );

private:
    typedef RefVector< SheetInfo > SheetInfoVector;
    SheetInfoVector     maSheetInfos;

    typedef RefMap< ::rtl::OUString, SheetInfo, IgnoreCaseCompare > SheetInfoMap;
    SheetInfoMap        maSheetInfosByName;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
