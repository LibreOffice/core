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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_AXISCONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_AXISCONTEXT_HXX

#include <drawingml/chart/chartcontextbase.hxx>

namespace oox {
namespace drawingml {
namespace chart {



struct AxisDispUnitsModel;

/** Handler for a value axis display units context (c:dispUnits element).
 */
class AxisDispUnitsContext : public ContextBase< AxisDispUnitsModel >
{
public:
    explicit            AxisDispUnitsContext( ::oox::core::ContextHandler2Helper& rParent, AxisDispUnitsModel& rModel );
    virtual             ~AxisDispUnitsContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



struct AxisModel;

/** Base class for axis context handlers (c:catAx, c:dateAx, c:serAx, c:valAx
    elements).
 */
class AxisContextBase : public ContextBase< AxisModel >
{
public:
    explicit            AxisContextBase( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~AxisContextBase();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a category axis context (c:catAx element).
 */
class CatAxisContext : public AxisContextBase
{
public:
    explicit            CatAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~CatAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a date axis context (c:dateAx element).
 */
class DateAxisContext : public AxisContextBase
{
public:
    explicit            DateAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~DateAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a series axis context (c:serAx element).
 */
class SerAxisContext : public AxisContextBase
{
public:
    explicit            SerAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~SerAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a value axis context (c:valAx element).
 */
class ValAxisContext : public AxisContextBase
{
public:
    explicit            ValAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~ValAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
