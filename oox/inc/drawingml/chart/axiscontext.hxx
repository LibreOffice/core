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

namespace oox::drawingml::chart {


struct AxisDispUnitsModel;

/** Handler for a value axis display units context (c:dispUnits element).
 */
class AxisDispUnitsContext final : public ContextBase< AxisDispUnitsModel >
{
public:
    explicit            AxisDispUnitsContext( ::oox::core::ContextHandler2Helper& rParent, AxisDispUnitsModel& rModel );
    virtual             ~AxisDispUnitsContext() override;

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
    virtual             ~AxisContextBase() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a category axis context (c:catAx element).
 */
class CatAxisContext final : public AxisContextBase
{
public:
    explicit            CatAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~CatAxisContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a date axis context (c:dateAx element).
 */
class DateAxisContext final : public AxisContextBase
{
public:
    explicit            DateAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~DateAxisContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a series axis context (c:serAx element).
 */
class SerAxisContext final : public AxisContextBase
{
public:
    explicit            SerAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~SerAxisContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a value axis context (c:valAx element).
 */
class ValAxisContext final : public AxisContextBase
{
public:
    explicit            ValAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~ValAxisContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};

/** Handler for a chartex axis context (<cx:axis> element).
 */
class CxAxisContext final : public AxisContextBase
{
public:
    explicit            CxAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel, sal_Int32 nId );
    virtual             ~CxAxisContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


} // namespace oox::drawingml::chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
