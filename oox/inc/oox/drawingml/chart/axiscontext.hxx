/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: axiscontext.hxx,v $
 * $Revision: 1.4 $
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

#ifndef OOX_DRAWINGML_CHART_AXISCONTEXT_HXX
#define OOX_DRAWINGML_CHART_AXISCONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct AxisDispUnitsModel;

/** Handler for a value axis display units context (c:dispUnits element).
 */
class AxisDispUnitsContext : public ContextBase< AxisDispUnitsModel >
{
public:
    explicit            AxisDispUnitsContext( ::oox::core::ContextHandler2Helper& rParent, AxisDispUnitsModel& rModel );
    virtual             ~AxisDispUnitsContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct AxisModel;

/** Base class for axis context handlers (c:catAx, c:dateAx, c:serAx, c:valAx
    elements).
 */
class AxisContextBase : public ContextBase< AxisModel >
{
public:
    explicit            AxisContextBase( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~AxisContextBase();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a category axis context (c:catAx element).
 */
class CatAxisContext : public AxisContextBase
{
public:
    explicit            CatAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~CatAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a date axis context (c:dateAx element).
 */
class DateAxisContext : public AxisContextBase
{
public:
    explicit            DateAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~DateAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a series axis context (c:serAx element).
 */
class SerAxisContext : public AxisContextBase
{
public:
    explicit            SerAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~SerAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a value axis context (c:valAx element).
 */
class ValAxisContext : public AxisContextBase
{
public:
    explicit            ValAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~ValAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

