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

#include <drawingml/chart/datatablecontext.hxx>
#include <drawingml/shapepropertiescontext.hxx>
#include <drawingml/textbodycontext.hxx>
#include <drawingml/chart/plotareamodel.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

namespace oox::drawingml::chart
{
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

DataTableContext::DataTableContext(ContextHandler2Helper& rParent, DataTableModel& rModel)
    : ContextBase<DataTableModel>(rParent, rModel)
{
}

DataTableContext::~DataTableContext() {}

ContextHandlerRef DataTableContext::onCreateContext(sal_Int32 nElement,
                                                    const AttributeList& rAttribs)
{
    switch (getCurrentElement())
    {
        case C_TOKEN(dTable):
            switch (nElement)
            {
                case C_TOKEN(showHorzBorder):
                    mrModel.mbShowHBorder = rAttribs.getBool(XML_val, false);
                    break;
                case C_TOKEN(showVertBorder):
                    mrModel.mbShowVBorder = rAttribs.getBool(XML_val, false);
                    break;
                case C_TOKEN(showOutline):
                    mrModel.mbShowOutline = rAttribs.getBool(XML_val, false);
                    break;
                case C_TOKEN(showKeys):
                    mrModel.mbShowKeys = rAttribs.getBool(XML_val, false);
                    break;
                case C_TOKEN(spPr):
                    return new ShapePropertiesContext(*this, mrModel.mxShapeProp.create());
                case C_TOKEN(txPr):
                    return new TextBodyContext(*this, mrModel.mxTextProp.create());
            }
            break;
    }
    return nullptr;
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
