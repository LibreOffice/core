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

#ifndef OOX_XLS_WORKBOOKSETTINGS_HXX
#define OOX_XLS_WORKBOOKSETTINGS_HXX

#include "oox/xls/workbookhelper.hxx"

namespace com { namespace sun { namespace star { namespace util { struct Date; } } } }

namespace oox {
namespace xls {

// ============================================================================

/** Settings for workbook write protection. */
struct FileSharingModel
{
    ::rtl::OUString     maUserName;             /// User who added the write protection password.
    sal_uInt16          mnPasswordHash;         /// Hash value of the write protection password.
    bool                mbRecommendReadOnly;    /// True = recommend read-only mode on opening.

    explicit            FileSharingModel();
};

// ============================================================================

/** Global workbook settings. */
struct WorkbookSettingsModel
{
    ::rtl::OUString     maCodeName;             /// VBA codename for the workbook.
    sal_Int32           mnShowObjectMode;       /// Specifies how objects are shown.
    sal_Int32           mnUpdateLinksMode;      /// Specifies how external links are updated.
    sal_Int32           mnDefaultThemeVer;      /// Default theme version.
    bool                mbDateMode1904;         /// True = null date is 1904-01-01.
    bool                mbSaveExtLinkValues;    /// True = save cached cell values for external links.

    explicit            WorkbookSettingsModel();

    /** Sets OOBIN or BIFF object visibility mode. */
    void                setBinObjectMode( sal_uInt16 nObjMode );
};

// ============================================================================

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

// ============================================================================

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
    void                importFileSharing( RecordInputStream& rStrm );
    /** Imports the WORKBOOKPR record containing global workbook settings. */
    void                importWorkbookPr( RecordInputStream& rStrm );
    /** Imports the CALCPR record containing workbook calculation settings. */
    void                importCalcPr( RecordInputStream& rStrm );

    /** Sets the save external linked values flag, e.g. from the WSBOOL record. */
    void                setSaveExtLinkValues( bool bSaveExtLinks );
    /** Imports the FILESHARING record. */
    void                importFileSharing( BiffInputStream& rStrm );
    /** Imports the BOOKBOOL record. */
    void                importBookBool( BiffInputStream& rStrm );
    /** Imports the CALCCOUNT record. */
    void                importCalcCount( BiffInputStream& rStrm );
    /** Imports the CALCMODE record. */
    void                importCalcMode( BiffInputStream& rStrm );
    /** Imports the CODENAME record. */
    void                importCodeName( BiffInputStream& rStrm );
    /** Imports the DATEMODE record. */
    void                importDateMode( BiffInputStream& rStrm );
    /** Imports the DELTA record. */
    void                importDelta( BiffInputStream& rStrm );
    /** Imports the HIDEOBJ record. */
    void                importHideObj( BiffInputStream& rStrm );
    /** Imports the ITERATION record. */
    void                importIteration( BiffInputStream& rStrm );
    /** Imports the PRECISION record. */
    void                importPrecision( BiffInputStream& rStrm );
    /** Imports the REFMODE record. */
    void                importRefMode( BiffInputStream& rStrm );
    /** Imports the SAVERECALC record. */
    void                importSaveRecalc( BiffInputStream& rStrm );
    /** Imports the UNCALCED record. */
    void                importUncalced( BiffInputStream& rStrm );
    /** Imports the USESELFS record. */
    void                importUsesElfs( BiffInputStream& rStrm );

    /** Converts the imported workbook settings. */
    void                finalizeImport();

    /** Returns the show objects mode (considered a view setting in Calc). */
    sal_Int16           getApiShowObjectMode() const;
    /** Returns the nulldate of this workbook. */
    ::com::sun::star::util::Date getNullDate() const;

private:
    /** Updates date mode and unit converter nulldate. */
    void                setDateMode( bool bDateMode1904 );

private:
    FileSharingModel    maFileSharing;
    WorkbookSettingsModel maBookSettings;
    CalcSettingsModel   maCalcSettings;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

