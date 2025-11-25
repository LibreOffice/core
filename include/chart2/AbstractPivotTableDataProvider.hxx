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

#include <chart2/chart2apidllapi.h>
#include <com/sun/star/chart2/data/PivotTableFieldEntry.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <vector>

namespace chart2api
{
/**
 * Data provider specific for pivot chart data.
 */
class CHART2API_DLLPUBLIC SAL_LOPLUGIN_ANNOTATE("crosscast") AbstractPivotTableDataProvider
{
public:
    /** names of column fields from the associated pivot table
     */
    virtual const std::vector<::css::chart2::data::PivotTableFieldEntry>&
    getColumnFields() const = 0;
    /** names of row fields from the associated pivot table
     */
    virtual const std::vector<::css::chart2::data::PivotTableFieldEntry>& getRowFields() const = 0;
    /** names of page fields from the associated pivot table
     */
    virtual const std::vector<::css::chart2::data::PivotTableFieldEntry>& getPageFields() const = 0;
    /** names of data fields from the associated pivot table
     */
    virtual const std::vector<::css::chart2::data::PivotTableFieldEntry>& getDataFields() const = 0;
    /** get the associated pivot table name
     */
    virtual const OUString& getPivotTableName() const = 0;
    /** set the associated pivot table name
     */
    virtual void setPivotTableName(const OUString& sPivotTableName) = 0;
    /** check if the associated pivot table exists
     */
    virtual bool hasPivotTable() const = 0;
    /** creates a single data sequence of values for the given data series index.
     *
     *  @param nIndex
     *      index of the data series
     */
    virtual ::css::uno::Reference<::css::chart2::data::XDataSequence>
    createDataSequenceOfValuesByIndex(sal_Int32 nIndex) = 0;
    /** creates a single data sequence of label(s) for the given data series index.
     *
     *  @param nIndex
     *      index of the data series
     */
    virtual ::css::uno::Reference<::css::chart2::data::XDataSequence>
    createDataSequenceOfLabelsByIndex(sal_Int32 nIndex) = 0;
    /** creates a single data sequence of categories.
     */
    virtual ::css::uno::Reference<::css::chart2::data::XDataSequence>
    createDataSequenceOfCategories() = 0;
    /** field output description: either "- all -", "- multiple -", or specific value
     *
     * @param nDimensionIndex
     *     dimension index of the field
     */
    virtual OUString getFieldOutputDescription(sal_Int32 nDimensionIndex) const = 0;

    virtual ~AbstractPivotTableDataProvider();
};

} // namespace chart2api

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
