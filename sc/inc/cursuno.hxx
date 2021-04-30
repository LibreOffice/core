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

#include "cellsuno.hxx"
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>

class ScCellCursorObj final : public ScCellRangeObj,
                        public css::sheet::XSheetCellCursor,
                        public css::sheet::XUsedAreaCursor,
                        public css::table::XCellCursor
{
public:
                            ScCellCursorObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellCursorObj() override;

    virtual css::uno::Any SAL_CALL queryInterface(
                                const css::uno::Type & rType ) override;
    virtual void SAL_CALL   acquire() noexcept override;
    virtual void SAL_CALL   release() noexcept override;

                            // XSheetCellCursor
    virtual void SAL_CALL   collapseToCurrentRegion() override;
    virtual void SAL_CALL   collapseToCurrentArray() override;
    virtual void SAL_CALL   collapseToMergedArea() override;
    virtual void SAL_CALL   expandToEntireColumns() override;
    virtual void SAL_CALL   expandToEntireRows() override;
    virtual void SAL_CALL   collapseToSize( sal_Int32 nColumns, sal_Int32 nRows ) override;

                            // XUsedAreaCursor
    virtual void SAL_CALL   gotoStartOfUsedArea( sal_Bool bExpand ) override;
    virtual void SAL_CALL   gotoEndOfUsedArea( sal_Bool bExpand ) override;

                            // XCellCursor
    virtual void SAL_CALL   gotoStart() override;
    virtual void SAL_CALL   gotoEnd() override;
    virtual void SAL_CALL   gotoNext() override;
    virtual void SAL_CALL   gotoPrevious() override;
    virtual void SAL_CALL   gotoOffset( sal_Int32 nColumnOffset, sal_Int32 nRowOffset ) override;

                            // XSheetCellRange
    virtual css::uno::Reference< css::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() override;

                            // XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom ) override;
    using ScCellRangeObj::getCellRangeByName;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByName( const OUString& aRange ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
