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

#ifndef OOX_XLS_SCENARIOCONTEXT_HXX
#define OOX_XLS_SCENARIOCONTEXT_HXX

#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

class Scenario;
class SheetScenarios;

// ============================================================================

class OoxScenarioContext : public OoxWorksheetContextBase
{
public:
    explicit            OoxScenarioContext( OoxWorksheetContextBase& rParent, SheetScenarios& rSheetScenarios );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

private:
    Scenario&           mrScenario;
};

// ============================================================================

class OoxScenariosContext : public OoxWorksheetContextBase
{
public:
    explicit            OoxScenariosContext( OoxWorksheetFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

private:
    SheetScenarios&     mrSheetScenarios;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

