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

#ifndef OOX_XLS_WORKSHEETBUFFER_HXX
#define OOX_XLS_WORKSHEETBUFFER_HXX

#include <utility>
#include "oox/helper/refmap.hxx"
#include "oox/helper/refvector.hxx"
#include "workbookhelper.hxx"

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

    /** Imports the attributes of a sheet element. */
    void                importSheet( const AttributeList& rAttribs );
    /** Imports the SHEET record from the passed BIFF12 stream. */
    void                importSheet( SequenceInputStream& rStrm );
    /** Inserts a new empty sheet into the document. Looks for an unused name.
         @return  Index of the new sheet in the Calc document. */
    sal_Int16           insertEmptySheet( const ::rtl::OUString& rPreferredName, bool bVisible );

    /** Returns the number of original sheets contained in the workbook. */
    sal_Int32           getWorksheetCount() const;
    /** Returns the OOXML relation identifier of the specified worksheet. */
    ::rtl::OUString     getWorksheetRelId( sal_Int32 nWorksheet ) const;

    /** Returns the Calc index of the specified worksheet. */
    sal_Int16           getCalcSheetIndex( sal_Int32 nWorksheet ) const;
    /** Returns the finalized name of the specified worksheet. */
    ::rtl::OUString     getCalcSheetName( sal_Int32 nWorksheet ) const;

    /** Returns the Calc index of the sheet with the passed original worksheet name. */
    sal_Int16           getCalcSheetIndex( const ::rtl::OUString& rWorksheetName ) const;
    /** Returns the finalized name of the sheet with the passed worksheet name. */
    ::rtl::OUString     getCalcSheetName( const ::rtl::OUString& rWorksheetName ) const;
    /** Converts sSheetNameRef (e.g. '#SheetName!A1' to '#SheetName.A1' )
        if sSheetNameRef doesn't start with '#' it is ignored and not modified
    */
    void                convertSheetNameRef( ::rtl::OUString& sSheetNameRef ) const;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
