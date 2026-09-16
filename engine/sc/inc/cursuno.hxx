/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "cellsuno.hxx"
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>

using ScCellCursorObj_BASE = cppu::ImplInheritanceHelper<ScCellRangeObj,
                                                         css::sheet::XSheetCellCursor,
                                                         css::sheet::XUsedAreaCursor,
                                                         css::table::XCellCursor>;
class ScCellCursorObj final : public ScCellCursorObj_BASE
{
public:
                            ScCellCursorObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellCursorObj() override;

                            // XSheetCellCursor
    virtual void   collapseToCurrentRegion() override;
    virtual void   collapseToCurrentArray() override;
    virtual void   collapseToMergedArea() override;
    virtual void   expandToEntireColumns() override;
    virtual void   expandToEntireRows() override;
    virtual void   collapseToSize( sal_Int32 nColumns, sal_Int32 nRows ) override;

                            // XUsedAreaCursor
    virtual void   gotoStartOfUsedArea( bool bExpand ) override;
    virtual void   gotoEndOfUsedArea( bool bExpand ) override;

                            // XCellCursor
    virtual void   gotoStart() override;
    virtual void   gotoEnd() override;
    virtual void   gotoNext() override;
    virtual void   gotoPrevious() override;
    virtual void   gotoOffset( sal_Int32 nColumnOffset, sal_Int32 nRowOffset ) override;

                            // XSheetCellRange
    virtual cpo::uno::Reference< css::sheet::XSpreadsheet >
                            getSpreadsheet() override;

                            // XCellRange
    virtual cpo::uno::Reference< css::table::XCell >
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual cpo::uno::Reference< css::table::XCellRange >
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom ) override;
    using ScCellRangeObj::getCellRangeByName;
    virtual cpo::uno::Reference< css::table::XCellRange >
                            getCellRangeByName( const OUString& aRange ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
