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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_WORKSHEETSETTINGS_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_WORKSHEETSETTINGS_HXX

#include "richstring.hxx"
#include "worksheethelper.hxx"
#include "tabprotection.hxx"

namespace oox {
namespace xls {

/** Sheet and outline settings. */
struct SheetSettingsModel
{
    OUString     maCodeName;             /// VBA module codename.
    Color               maTabColor;             /// Sheet tab color.
    bool                mbFilterMode;           /// True = sheet contains active filter.
    bool                mbApplyStyles;          /// True = automatic styles when creating outlines.
    bool                mbSummaryBelow;         /// True = row outline symbols below group.
    bool                mbSummaryRight;         /// True = column outline symbols right of group.

    explicit            SheetSettingsModel();
};

/** Sheet protection settings. */
struct SheetProtectionModel
{
    sal_uInt16          mnPasswordHash;         /// Hash value from sheet protection password.
    bool                mbSheet;                /// True = sheet protection enabled, locked cells are protected.
    bool                mbObjects;              /// True = objects locked.
    bool                mbScenarios;            /// True = scenarios locked.
    bool                mbFormatCells;          /// True = format cells locked.
    bool                mbFormatColumns;        /// True = format columns locked.
    bool                mbFormatRows;           /// True = format rows locked.
    bool                mbInsertColumns;        /// True = insert columns locked.
    bool                mbInsertRows;           /// True = insert rows locked.
    bool                mbInsertHyperlinks;     /// True = insert hyperlinks locked.
    bool                mbDeleteColumns;        /// True = delete columns locked.
    bool                mbDeleteRows;           /// True = delete rows locked.
    bool                mbSelectLocked;         /// True = select locked cells locked.
    bool                mbSort;                 /// True = sorting locked.
    bool                mbAutoFilter;           /// True = autofilters locked.
    bool                mbPivotTables;          /// True = pivot tables locked.
    bool                mbSelectUnlocked;       /// True = select unlocked cells locked.

    ::std::vector< ScEnhancedProtection >   maEnhancedProtections;

    explicit            SheetProtectionModel();
};

class WorksheetSettings : public WorksheetHelper
{
public:
    explicit            WorksheetSettings( const WorksheetHelper& rHelper );

    /** Imports sheet settings from the sheetPr element. */
    void                importSheetPr( const AttributeList& rAttribs );
    /** Imports chart sheet settings from the sheetPr element. */
    void                importChartSheetPr( const AttributeList& rAttribs );
    /** Imports the sheet tab color from the tabColor element. */
    void                importTabColor( const AttributeList& rAttribs );
    /** Imports outline settings from the outlinePr element. */
    void                importOutlinePr( const AttributeList& rAttribs );
    /** Imports protection settings from the sheetProtection element. */
    void                importSheetProtection( const AttributeList& rAttribs );
    /** Imports enhanced protection settings from the protectedRanges element. */
    static void         importProtectedRanges( const AttributeList& rAttribs );
    /** Imports enhanced protection settings from the protectedRange element. */
    void                importProtectedRange( const AttributeList& rAttribs );
    /** Imports protection settings from the sheetProtection element of a chart sheet. */
    void                importChartProtection( const AttributeList& rAttribs );
    /** Imports phonetic settings from the phoneticPr element. */
    void                importPhoneticPr( const AttributeList& rAttribs );

    /** Imports sheet properties from the SHEETPR record. */
    void                importSheetPr( SequenceInputStream& rStrm );
    /** Imports sheet properties from the CHARTSHEETPR record. */
    void                importChartSheetPr( SequenceInputStream& rStrm );
    /** Imports sheet protection settings from the SHEETPROTECTION record. */
    void                importSheetProtection( SequenceInputStream& rStrm );
    /** Imports chart sheet protection settings from the CHARTPROTECTION record. */
    void                importChartProtection( SequenceInputStream& rStrm );
    /** Imports phonetic settings from the PHONETICPR record. */
    void                importPhoneticPr( SequenceInputStream& rStrm );

    /** Converts the imported worksheet settings. */
    void                finalizeImport();

private:
    PhoneticSettings    maPhoneticSett;
    SheetSettingsModel  maSheetSettings;
    SheetProtectionModel maSheetProt;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
