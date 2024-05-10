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

#include <drawingml/chart/datatableconverter.hxx>

#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XDataTable.hpp>
#include <drawingml/chart/plotareamodel.hxx>
#include <oox/token/properties.hxx>

using namespace css;

namespace oox::drawingml::chart
{
DataTableConverter::DataTableConverter(const ConverterRoot& rParent, DataTableModel& rModel)
    : ConverterBase<DataTableModel>(rParent, rModel)
{
}

DataTableConverter::~DataTableConverter() = default;

void DataTableConverter::convertFromModel(uno::Reference<chart2::XDiagram> const& rxDiagram)
{
    if (!rxDiagram.is())
        return;

    try
    {
        uno::Reference<chart2::XDataTable> xDataTable(
            createInstance(u"com.sun.star.chart2.DataTable"_ustr), uno::UNO_QUERY_THROW);
        rxDiagram->setDataTable(xDataTable);

        PropertySet aPropSet(xDataTable);
        if (mrModel.mbShowHBorder)
            aPropSet.setProperty(PROP_HBorder, mrModel.mbShowHBorder);
        if (mrModel.mbShowVBorder)
            aPropSet.setProperty(PROP_VBorder, mrModel.mbShowVBorder);
        if (mrModel.mbShowOutline)
            aPropSet.setProperty(PROP_Outline, mrModel.mbShowOutline);
        if (mrModel.mbShowKeys)
            aPropSet.setProperty(PROP_Keys, mrModel.mbShowKeys);

        getFormatter().convertFormatting(aPropSet, mrModel.mxShapeProp, mrModel.mxTextProp,
                                         OBJECTTYPE_DATATABLE);
    }
    catch (uno::Exception&)
    {
    }
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
