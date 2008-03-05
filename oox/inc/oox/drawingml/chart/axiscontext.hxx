/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: axiscontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:47:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_CHART_AXISCONTEXT_HXX
#define OOX_DRAWINGML_CHART_AXISCONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

class AxisModel;

class AxisContextBase : public ChartContextBase< AxisModel >
{
public:
    explicit            AxisContextBase(
                            ::oox::core::ContextHandler2Helper& rParent,
                            AxisModel& rModel );
    virtual             ~AxisContextBase();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a category axis context (c:catAx element).
 */
class CatAxisContext : public AxisContextBase
{
public:
    explicit            CatAxisContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            AxisModel& rModel );
    virtual             ~CatAxisContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a date axis context (c:dateAx element).
 */
class DateAxisContext : public AxisContextBase
{
public:
    explicit            DateAxisContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            AxisModel& rModel );
    virtual             ~DateAxisContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a series axis context (c:serAx element).
 */
class SerAxisContext : public AxisContextBase
{
public:
    explicit            SerAxisContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            AxisModel& rModel );
    virtual             ~SerAxisContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a value axis context (c:valAx element).
 */
class ValAxisContext : public AxisContextBase
{
public:
    explicit            ValAxisContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            AxisModel& rModel );
    virtual             ~ValAxisContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

