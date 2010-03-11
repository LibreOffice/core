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

#ifndef OOX_XLS_SCENARIOBUFFER_HXX
#define OOX_XLS_SCENARIOBUFFER_HXX

#include <com/sun/star/table/CellAddress.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct ScenarioCellModel
{
    ::com::sun::star::table::CellAddress maPos;
    ::rtl::OUString     maValue;
    sal_Int32           mnNumFmtId;
    bool                mbDeleted;

    explicit            ScenarioCellModel();
};

// ----------------------------------------------------------------------------

struct ScenarioModel
{
    ::rtl::OUString     maName;             /// Name of the scenario.
    ::rtl::OUString     maComment;          /// Comment.
    ::rtl::OUString     maUser;             /// Name of user created the scenario.
    bool                mbLocked;           /// True = input cell values locked.
    bool                mbHidden;           /// True = scenario is hidden.

    explicit            ScenarioModel();
};

// ----------------------------------------------------------------------------

class Scenario : public WorkbookHelper
{
public:
    explicit            Scenario( const WorkbookHelper& rHelper, sal_Int16 nSheet );

    /** Imports a scenario definition from a scenario element. */
    void                importScenario( const AttributeList& rAttribs );
    /** Imports a new cell for this scenario from a inputCells element. */
    void                importInputCells( const AttributeList& rAttribs );

    /** Imports a scenario definition from a SCENARIO record. */
    void                importScenario( RecordInputStream& rStrm );
    /** Imports a new cell for this scenario from a INPUTCELLS record. */
    void                importInputCells( RecordInputStream& rStrm );

    /** Imports a scenario definition from a SCENARIO record. */
    void                importScenario( BiffInputStream& rStrm );

    /** Creates the scenario in the Calc document. */
    void                finalizeImport();

private:
    typedef ::std::vector< ScenarioCellModel > ScenarioCellVector;

    ScenarioCellVector  maCells;            /// Scenario cells.
    ScenarioModel       maModel;            /// Scenario model data.
    sal_Int16           mnSheet;            /// Index of the sheet this scenario is based on.
};

// ============================================================================

struct SheetScenariosModel
{
    sal_Int32           mnCurrent;          /// Selected scenario.
    sal_Int32           mnShown;            /// Visible scenario.

    explicit            SheetScenariosModel();
};

// ----------------------------------------------------------------------------

class SheetScenarios : public WorkbookHelper
{
public:
    explicit            SheetScenarios( const WorkbookHelper& rHelper, sal_Int16 nSheet );

    /** Imports sheet scenario settings from a scenarios element. */
    void                importScenarios( const AttributeList& rAttribs );
    /** Imports sheet scenario settings from a SCENARIOS record. */
    void                importScenarios( RecordInputStream& rStrm );
    /** Imports sheet scenario settings from a SCENARIOS record. */
    void                importScenarios( BiffInputStream& rStrm );

    /** Creates and returns a new scenario in this collection. */
    Scenario&           createScenario();

    /** Creates all scenarios in the Calc sheet. */
    void                finalizeImport();

private:
    typedef RefVector< Scenario > ScenarioVector;
    ScenarioVector      maScenarios;
    SheetScenariosModel maModel;
    sal_Int16           mnSheet;
};

// ============================================================================

class ScenarioBuffer : public WorkbookHelper
{
public:
    explicit            ScenarioBuffer( const WorkbookHelper& rHelper );

    /** Creates and returns a scenario collection for the passed sheet. */
    SheetScenarios&     createSheetScenarios( sal_Int16 nSheet );

    /** Creates all scenarios in the Calc dcument. */
    void                finalizeImport();

private:
    typedef RefMap< sal_Int16, SheetScenarios, ::std::greater< sal_Int16 > > SheetScenariosMap;
    SheetScenariosMap   maSheetScenarios;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

