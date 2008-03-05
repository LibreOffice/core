/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: axiscontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:32:54 $
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

#include "oox/drawingml/chart/axiscontext.hxx"
#include "oox/drawingml/chart/axismodel.hxx"
#include "oox/drawingml/chart/layoutcontext.hxx"

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextWrapper;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

AxisContextBase::AxisContextBase( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    ChartContextBase< AxisModel >( rParent, rModel )
{
}

AxisContextBase::~AxisContextBase()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper AxisContextBase::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( axId ):
                getModel().getData().mnAxisId = rAttribs.getInteger( XML_val, -1 );
                return false;
            case C_TOKEN( crossAx ):
                getModel().getData().mnCrossAxisId = rAttribs.getInteger( XML_val, -1 );
                return false;
            case C_TOKEN( crosses ):
                getModel().getData().mnCrossMode = rAttribs.getToken( XML_val, XML_autoZero );
                return false;
            case C_TOKEN( crossesAt ):
                getModel().getData().mfCrossesAt = rAttribs.getDouble( XML_val, 0.0 );
                return false;
            case C_TOKEN( delete ):
                getModel().getData().mbDeleted = rAttribs.getBool( XML_val, true );
                return false;
            case C_TOKEN( majorTickMark ):
                getModel().getData().mnMajorTickMark = rAttribs.getToken( XML_val, XML_cross );
                return false;
            case C_TOKEN( minorTickMark ):
                getModel().getData().mnMinorTickMark = rAttribs.getToken( XML_val, XML_cross );
                return false;
            case C_TOKEN( numFmt ):
                getModel().getData().maFormatCode = rAttribs.getString( XML_formatCode );
                getModel().getData().mbSourceLinked = rAttribs.getBool( XML_sourceLinked, true );
                return false;
            case C_TOKEN( scaling ):
                return true;
            case C_TOKEN( tickLblPos ):
                getModel().getData().mnTickLabelPos = rAttribs.getToken( XML_val, XML_nextTo );
                return false;
        }
    }
    else switch( getCurrentElement() )
    {
        case C_TOKEN( scaling ):
            switch( nElement )
            {
                case C_TOKEN( logBase ):
                    getModel().getData().mfLogBase = rAttribs.getDouble( XML_val, 0.0 );
                    return false;
                case C_TOKEN( max ):
                    getModel().getData().mfMax = rAttribs.getDouble( XML_val, 0.0 );
                    return false;
                case C_TOKEN( min ):
                    getModel().getData().mfMin = rAttribs.getDouble( XML_val, 0.0 );
                    return false;
                case C_TOKEN( orientation ):
                    getModel().getData().mnOrientation = rAttribs.getToken( XML_val, XML_minMax );
                    return false;
            }
        break;
    }
    return false;
}

// ============================================================================

CatAxisContext::CatAxisContext( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    AxisContextBase( rParent, rModel )
{
}

CatAxisContext::~CatAxisContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper CatAxisContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( auto ):
                getModel().getData().mbAuto = rAttribs.getBool( XML_val, true );
                return false;
            case C_TOKEN( axPos ):
                getModel().getData().mnAxisPos = rAttribs.getToken( XML_val );
                return false;
            case C_TOKEN( lblAlgn ):
                getModel().getData().mnLabelAlign = rAttribs.getToken( XML_val, XML_ctr );
                return false;
            case C_TOKEN( lblOffset ):
                getModel().getData().mnLabelOffset = rAttribs.getInteger( XML_val, 100 );
                return false;
            case C_TOKEN( noMultiLvlLbl ):
                getModel().getData().mbNoMultiLevel = rAttribs.getBool( XML_val, true );
                return false;
            case C_TOKEN( tickLblSkip ):
                getModel().getData().mnTickLabelSkip = rAttribs.getInteger( XML_val, 0 );
                return false;
            case C_TOKEN( tickMarkSkip ):
                getModel().getData().mnTickMarkSkip = rAttribs.getInteger( XML_val, 0 );
                return false;
        }
    }
    return AxisContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

DateAxisContext::DateAxisContext( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    AxisContextBase( rParent, rModel )
{
}

DateAxisContext::~DateAxisContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper DateAxisContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( auto ):
                getModel().getData().mbAuto = rAttribs.getBool( XML_val, true );
                return false;
            case C_TOKEN( baseTimeUnit ):
                getModel().getData().mnBaseTimeUnit = rAttribs.getToken( XML_val, XML_days );
                return false;
            case C_TOKEN( lblOffset ):
                getModel().getData().mnLabelOffset = rAttribs.getInteger( XML_val, 100 );
                return false;
            case C_TOKEN( majorTimeUnit ):
                getModel().getData().mnMajorTimeUnit = rAttribs.getToken( XML_val, XML_days );
                return false;
            case C_TOKEN( majorUnit ):
                getModel().getData().mfMajorUnit = rAttribs.getDouble( XML_val, 0.0 );
                return false;
            case C_TOKEN( minorTimeUnit ):
                getModel().getData().mnMinorTimeUnit = rAttribs.getToken( XML_val, XML_days );
                return false;
            case C_TOKEN( minorUnit ):
                getModel().getData().mfMinorUnit = rAttribs.getDouble( XML_val, 0.0 );
                return false;
        }
    }
    return AxisContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

SerAxisContext::SerAxisContext( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    AxisContextBase( rParent, rModel )
{
}

SerAxisContext::~SerAxisContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper SerAxisContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( tickLblSkip ):
                getModel().getData().mnTickLabelSkip = rAttribs.getInteger( XML_val, 0 );
                return false;
            case C_TOKEN( tickMarkSkip ):
                getModel().getData().mnTickMarkSkip = rAttribs.getInteger( XML_val, 0 );
                return false;
        }
    }
    return AxisContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

ValAxisContext::ValAxisContext( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    AxisContextBase( rParent, rModel )
{
}

ValAxisContext::~ValAxisContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper ValAxisContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( crossBetween ):
                getModel().getData().mnCrossBetween = rAttribs.getToken( XML_val, XML_between );
                return false;
            case C_TOKEN( dispUnits ):
                return true;
            case C_TOKEN( majorUnit ):
                getModel().getData().mfMajorUnit = rAttribs.getDouble( XML_val, 0.0 );
                return false;
            case C_TOKEN( minorUnit ):
                getModel().getData().mfMinorUnit = rAttribs.getDouble( XML_val, 0.0 );
                return false;
        }
    }
    else switch( getCurrentElement() )
    {
        case C_TOKEN( dispUnits ):
            switch( nElement )
            {
                case C_TOKEN( builtInUnit ):
                    getModel().getData().mnBuiltInUnit = rAttribs.getToken( XML_val, XML_thousands );
                    return false;
                case C_TOKEN( custUnit ):
                    getModel().getData().mfCustomUnit = rAttribs.getDouble( XML_val, 0.0 );
                    return false;
            }
            return  (nElement == C_TOKEN( dispUnitsLbl ));

        case C_TOKEN( dispUnitsLbl ):
            switch( nElement )
            {
                case C_TOKEN( layout ):
                    return new LayoutContext( *this, getModel().createUnitLabelsLayout() );
            }
            return false;
    }
    return AxisContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

