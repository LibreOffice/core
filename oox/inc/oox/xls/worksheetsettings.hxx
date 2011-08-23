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

#ifndef OOX_XLS_WORKSHEETSETTINGS_HXX
#define OOX_XLS_WORKSHEETSETTINGS_HXX

#include "oox/xls/worksheethelper.hxx"
#include "oox/xls/richstring.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** Sheet and outline settings. */
struct SheetSettingsModel
{
    ::rtl::OUString     maCodeName;             /// VBA module codename.
    Color               maTabColor;             /// Sheet tab color.
    bool                mbFilterMode;           /// True = sheet contains active filter.
    bool                mbApplyStyles;          /// True = automatic styles when creating outlines.
    bool                mbSummaryBelow;         /// True = row outline symbols below group.
    bool                mbSummaryRight;         /// True = column outline symbols right of group.

    explicit            SheetSettingsModel();
};

// ============================================================================

/** Sheet protection settings. */
struct SheetProtectionModel
{
    sal_uInt16          mnPasswordHash;         /// Hash value from sheet protection password.
    bool                mbSheet;                /// True = sheet protection enabled, locked cells are protcted.
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

    explicit            SheetProtectionModel();
};

// ============================================================================

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
    /** Imports protection settings from the sheetProtection element of a chart sheet. */
    void                importChartProtection( const AttributeList& rAttribs );
    /** Imports phonetic settings from the phoneticPr element. */
    void                importPhoneticPr( const AttributeList& rAttribs );

    /** Imports sheet properties from the SHEETPR record. */
    void                importSheetPr( RecordInputStream& rStrm );
    /** Imports sheet properties from the CHARTSHEETPR record. */
    void                importChartSheetPr( RecordInputStream& rStrm );
    /** Imports sheet protection settings from the SHEETPROTECTION record. */
    void                importSheetProtection( RecordInputStream& rStrm );
    /** Imports chart sheet protection settings from the CHARTPROTECTION record. */
    void                importChartProtection( RecordInputStream& rStrm );
    /** Imports phonetic settings from the PHONETICPR record. */
    void                importPhoneticPr( RecordInputStream& rStrm );

    /** Imports sheet properties from a SHEETPR record. */
    void                importSheetPr( BiffInputStream& rStrm );
    /** Imports protection status from the PROTECT record. */
    void                importProtect( BiffInputStream& rStrm );
    /** Imports object protection status from the OBJECTPROTECT record. */
    void                importObjectProtect( BiffInputStream& rStrm );
    /** Imports scenario protection status from the SCENPROTECT record. */
    void                importScenProtect( BiffInputStream& rStrm );
    /** Imports sheet password hash from the PASSWORD record. */
    void                importPassword( BiffInputStream& rStrm );
    /** Imports protection settings from the SHEETPROTECTION record. */
    void                importSheetProtection( BiffInputStream& rStrm );
    /** Imports the VBA code module name from the CODENAME record. */
    void                importCodeName( BiffInputStream& rStrm );
    /** Imports phonetic settings from the PHONETICPR record. */
    void                importPhoneticPr( BiffInputStream& rStrm );

    /** Converts the imported worksheet settings. */
    void                finalizeImport();

private:
    PhoneticSettings    maPhoneticSett;
    SheetSettingsModel  maSheetSettings;
    SheetProtectionModel maSheetProt;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

