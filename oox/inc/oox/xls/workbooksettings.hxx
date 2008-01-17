/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: workbooksettings.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
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

#ifndef OOX_XLS_WORKBOOKSETTINGS_HXX
#define OOX_XLS_WORKBOOKSETTINGS_HXX

#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** Global workbook settings. */
struct OoxWorkbookPrData
{
    ::rtl::OUString     maCodeName;             /// VBA codename for the workbook.
    sal_Int32           mnShowObjectMode;       /// Specifies how objects are shown.
    sal_Int32           mnUpdateLinksMode;      /// Specifies how external links are updated.
    sal_Int32           mnDefaultThemeVer;      /// Default theme version.
    bool                mbDateMode1904;         /// True = null date is 1904-01-01.
    bool                mbSaveExtLinkValues;    /// True = save cached cell values for external links.

    explicit            OoxWorkbookPrData();

    /** Sets OOBIN or BIFF object visibility mode. */
    void                setBinObjectMode( sal_uInt16 nObjMode );
};

// ============================================================================

/** Workbook calculation settings. */
struct OoxCalcPrData
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

    explicit            OoxCalcPrData();
};

// ============================================================================

class WorkbookSettings : public WorkbookHelper
{
public:
    explicit            WorkbookSettings( const WorkbookHelper& rHelper );

    /** Imports the workbookPr element containing global workbook settings. */
    void                importWorkbookPr( const AttributeList& rAttribs );
    /** Imports the calcPr element containing workbook calculation settings. */
    void                importCalcPr( const AttributeList& rAttribs );

    /** Imports the WORKBOOKPR record containing global workbook settings. */
    void                importWorkbookPr( RecordInputStream& rStrm );
    /** Imports the CALCPR record containing workbook calculation settings. */
    void                importCalcPr( RecordInputStream& rStrm );

    /** Sets the save external linked values flag, e.g. from the WSBOOL record. */
    void                setSaveExtLinkValues( bool bSaveExtLinks );
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

private:
    OoxWorkbookPrData   maOoxBookData;
    OoxCalcPrData       maOoxCalcData;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

