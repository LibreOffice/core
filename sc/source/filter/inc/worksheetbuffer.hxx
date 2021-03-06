/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <utility>
#include <oox/helper/refmap.hxx>
#include <oox/helper/refvector.hxx>
#include "workbookhelper.hxx"

namespace oox { class SequenceInputStream; }
namespace oox { class AttributeList; }

namespace oox::xls {

/** Contains data from the 'sheet' element describing a sheet in the workbook. */
struct SheetInfoModel
{
    OUString     maRelId;        /// Relation identifier for the sheet substream.
    OUString     maName;         /// Original name of the sheet.
    sal_Int32    mnSheetId;      /// Sheet identifier.
    sal_Int32    mnState;        /// Visibility state.

    explicit     SheetInfoModel();
};

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
    sal_Int16           insertEmptySheet( const OUString& rPreferredName );

    /** Returns the number of original sheets contained in the workbook. */
    sal_Int32           getWorksheetCount() const;
    /** Returns the number of all sheets, workbook + dummy ones (pivot table cache records ) */
    sal_Int32           getAllSheetCount() const;
    /** Returns the OOXML relation identifier of the specified worksheet. */
    OUString     getWorksheetRelId( sal_Int32 nWorksheet ) const;

    /** Returns the Calc index of the specified worksheet. */
    sal_Int16           getCalcSheetIndex( sal_Int32 nWorksheet ) const;
    /** Returns the finalized name of the specified worksheet. */
    OUString     getCalcSheetName( sal_Int32 nWorksheet ) const;

    /** Returns the Calc index of the sheet with the passed original worksheet name. */
    sal_Int16           getCalcSheetIndex( const OUString& rWorksheetName ) const;
    /** Returns the finalized name of the sheet with the passed worksheet name. */
    OUString     getCalcSheetName( const OUString& rWorksheetName ) const;
    /** Converts sSheetNameRef (e.g. '#SheetName!A1' to '#SheetName.A1' )
        if sSheetNameRef doesn't start with '#' it is ignored and not modified
    */
    void                convertSheetNameRef( OUString& sSheetNameRef ) const;
    void finalizeImport( sal_Int16 nActiveSheet );

private:
    struct SheetInfo : public SheetInfoModel
    {
        OUString            maCalcName;
        OUString            maCalcQuotedName;
        sal_Int16           mnCalcSheet;

        explicit            SheetInfo( const SheetInfoModel& rModel, sal_Int16 nCalcSheet, const OUString& rCalcName );
    };

    typedef ::std::pair< sal_Int16, OUString > IndexNamePair;

    /** Creates a new sheet in the Calc document. Does not insert anything in the own lists. */
    IndexNamePair       createSheet( const OUString& rPreferredName, sal_Int32 nSheetPos );
    /** Creates a new sheet in the Calc document and inserts the related SheetInfo. */
    void                insertSheet( const SheetInfoModel& rModel );

private:
    RefVector< SheetInfo >  maSheetInfos;
    RefMap< OUString, SheetInfo, IgnoreCaseCompare >  maSheetInfosByName;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
