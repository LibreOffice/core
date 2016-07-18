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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_WORKBOOKSETTINGS_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_WORKBOOKSETTINGS_HXX

#include "workbookhelper.hxx"

namespace com { namespace sun { namespace star { namespace util { struct Date; } } } }

namespace oox {
namespace xls {

/** Settings for workbook write protection. */
struct FileSharingModel
{
    OUString     maUserName;             /// User who added the write protection password.
    sal_uInt16          mnPasswordHash;         /// Hash value of the write protection password.
    bool                mbRecommendReadOnly;    /// True = recommend read-only mode on opening.

    explicit            FileSharingModel();
};

/** Global workbook settings. */
struct WorkbookSettingsModel
{
    OUString     maCodeName;             /// VBA codename for the workbook.
    sal_Int32           mnShowObjectMode;       /// Specifies how objects are shown.
    sal_Int32           mnUpdateLinksMode;      /// Specifies how external links are updated.
    sal_Int32           mnDefaultThemeVer;      /// Default theme version.
    bool                mbDateMode1904;         /// True = null date is 1904-01-01.
    bool                mbDateCompatibility;    /// False = null date is 1899-12-30.
    bool                mbSaveExtLinkValues;    /// True = save cached cell values for external links.

    explicit            WorkbookSettingsModel();

    /** Sets BIFF object visibility mode. */
    void                setBiffObjectMode( sal_uInt16 nObjMode );
};

/** Workbook calculation settings. */
struct CalcSettingsModel
{
    double              mfIterateDelta;         /// Minimum change in circular references.
    sal_Int32           mnCalcId;               /// Calculation engine identifier.
    sal_Int32           mnRefMode;              /// Cell reference mode: A1 or R1C1.
    sal_Int32           mnCalcMode;             /// Automatic or manual recalculation.
    sal_Int32           mnIterateCount;         /// Number of iterations in circular references.
    sal_Int32           mnProcCount;            /// Number of processors for concurrent calculation.
    bool                mbCalcOnSave;           /// True = always recalculate formulas before save.
    bool                mbCalcCompleted;        /// True = formulas have been recalculated before save.
    bool                mbFullPrecision;        /// True = use full precision on calculation.
    bool                mbIterate;              /// True = allow circular references.
    bool                mbConcurrent;           /// True = concurrent calculation enabled.
    bool                mbUseNlr;               /// True = use natural language references in formulas.

    explicit            CalcSettingsModel();
};

class WorkbookSettings : public WorkbookHelper
{
public:
    explicit            WorkbookSettings( const WorkbookHelper& rHelper );

    /** Imports the fileSharing element containing write protection settings. */
    void                importFileSharing( const AttributeList& rAttribs );
    /** Imports the workbookPr element containing global workbook settings. */
    void                importWorkbookPr( const AttributeList& rAttribs );
    /** Imports the calcPr element containing workbook calculation settings. */
    void                importCalcPr( const AttributeList& rAttribs );

    /** Imports the FILESHARING record containing write protection settings. */
    void                importFileSharing( SequenceInputStream& rStrm );
    /** Imports the WORKBOOKPR record containing global workbook settings. */
    void                importWorkbookPr( SequenceInputStream& rStrm );
    /** Imports the CALCPR record containing workbook calculation settings. */
    void                importCalcPr( SequenceInputStream& rStrm );

    /** Converts the imported workbook settings. */
    void                finalizeImport();

    /** Returns the show objects mode (considered a view setting in Calc). */
    sal_Int16           getApiShowObjectMode() const;
    /** Returns the nulldate of this workbook. */
    css::util::Date const & getNullDate() const;

private:
    /** Updates date mode and unit converter nulldate. */
    void                setDateMode( bool bDateMode1904, bool bDateCompatibility=true );

private:
    FileSharingModel    maFileSharing;
    WorkbookSettingsModel maBookSettings;
    CalcSettingsModel   maCalcSettings;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
