/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "AxisWrapper.hxx"
#include "AxisHelper.hxx"
#include "TitleHelper.hxx"
#include "Chart2ModelContact.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"
#include "WrappedDirectStateProperty.hxx"
#include "GridWrapper.hxx"
#include "TitleWrapper.hxx"
#include "DisposeHelper.hxx"

#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>

#include "CharacterProperties.hxx"
#include "LineProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "WrappedCharacterHeightProperty.hxx"
#include "WrappedTextRotationProperty.hxx"
#include "WrappedGapwidthProperty.hxx"
#include "WrappedScaleProperty.hxx"
#include "WrappedDefaultProperty.hxx"
#include "WrappedNumberFormatProperty.hxx"
#include "WrappedScaleTextProperties.hxx"

#include <algorithm>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::chart::ContainerHelper;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;

namespace
{
static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Axis" ));

enum
{
    PROP_AXIS_MAX,
    PROP_AXIS_MIN,
    PROP_AXIS_STEPMAIN,
    PROP_AXIS_STEPHELP, //deprecated property use 'StepHelpCount' instead
    PROP_AXIS_STEPHELP_COUNT,
    PROP_AXIS_AUTO_MAX,
    PROP_AXIS_AUTO_MIN,
    PROP_AXIS_AUTO_STEPMAIN,
    PROP_AXIS_AUTO_STEPHELP,
    PROP_AXIS_TYPE,
    PROP_AXIS_TIME_INCREMENT,
    PROP_AXIS_EXPLICIT_TIME_INCREMENT,
    PROP_AXIS_LOGARITHMIC,
    PROP_AXIS_REVERSEDIRECTION,
    PROP_AXIS_VISIBLE,
    PROP_AXIS_CROSSOVER_POSITION,
    PROP_AXIS_CROSSOVER_VALUE,
    PROP_AXIS_ORIGIN,
    PROP_AXIS_AUTO_ORIGIN,
    PROP_AXIS_MARKS,
    PROP_AXIS_HELPMARKS,
    PROP_AXIS_MARK_POSITION,
    PROP_AXIS_DISPLAY_LABELS,
    PROP_AXIS_NUMBERFORMAT,
    PROP_AXIS_LINK_NUMBERFORMAT_TO_SOURCE,
    PROP_AXIS_LABEL_POSITION,
    PROP_AXIS_TEXT_ROTATION,
    PROP_AXIS_ARRANGE_ORDER,
    PROP_AXIS_TEXTBREAK,
    PROP_AXIS_CAN_OVERLAP,
    PROP_AXIS_STACKEDTEXT,
    PROP_AXIS_OVERLAP,
    PROP_AXIS_GAP_WIDTH
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    //Properties for scaling:
    rOutProperties.push_back(
        Property( C2U( "Max" ),
                  PROP_AXIS_MAX,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "Min" ),
                  PROP_AXIS_MIN,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "StepMain" ),
                  PROP_AXIS_STEPMAIN,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "StepHelpCount" ),
                  PROP_AXIS_STEPHELP_COUNT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    //deprecated property use 'StepHelpCount' instead
    rOutProperties.push_back(
        Property( C2U( "StepHelp" ),
                  PROP_AXIS_STEPHELP,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "AutoMax" ),
                  PROP_AXIS_AUTO_MAX,
                  ::getBooleanCppuType(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AutoMin" ),
                  PROP_AXIS_AUTO_MIN,
                  ::getBooleanCppuType(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AutoStepMain" ),
                  PROP_AXIS_AUTO_STEPMAIN,
                  ::getBooleanCppuType(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AutoStepHelp" ),
                  PROP_AXIS_AUTO_STEPHELP,
                  ::getBooleanCppuType(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AxisType" ),
                  PROP_AXIS_TYPE,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)), //type com::sun::star::chart::ChartAxisType
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TimeIncrement" ),
                  PROP_AXIS_TIME_INCREMENT,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::TimeIncrement * >(0)),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "ExplicitTimeIncrement" ),
                  PROP_AXIS_EXPLICIT_TIME_INCREMENT,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::TimeIncrement * >(0)),
                  beans::PropertyAttribute::READONLY |
                  beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "Logarithmic" ),
                  PROP_AXIS_LOGARITHMIC,
                  ::getBooleanCppuType(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ReverseDirection" ),
                  PROP_AXIS_REVERSEDIRECTION,
                  ::getBooleanCppuType(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    //todo: this property is missing in the API
    rOutProperties.push_back(
        Property( C2U( "Visible" ),
                  PROP_AXIS_VISIBLE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "CrossoverPosition" ),
                  PROP_AXIS_CROSSOVER_POSITION,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartAxisPosition * >(0)),
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "CrossoverValue" ),
                  PROP_AXIS_CROSSOVER_VALUE,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::MAYBEVOID ));


    rOutProperties.push_back(
        Property( C2U( "Origin" ),
                  PROP_AXIS_ORIGIN,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "AutoOrigin" ),
                  PROP_AXIS_AUTO_ORIGIN,
                  ::getBooleanCppuType(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    //Properties for interval marks:
    rOutProperties.push_back(
        Property( C2U( "Marks" ),
                  PROP_AXIS_MARKS,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "HelpMarks" ),
                  PROP_AXIS_HELPMARKS,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "MarkPosition" ),
                  PROP_AXIS_MARK_POSITION,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartAxisMarkPosition * >(0)),
                  beans::PropertyAttribute::MAYBEDEFAULT ));


    //Properties for labels:
    rOutProperties.push_back(
        Property( C2U( "DisplayLabels" ),
                  PROP_AXIS_DISPLAY_LABELS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberFormat" ),
                  PROP_AXIS_NUMBERFORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "LinkNumberFormatToSource" ),
                  PROP_AXIS_LINK_NUMBERFORMAT_TO_SOURCE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "LabelPosition" ),
                  PROP_AXIS_LABEL_POSITION,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartAxisLabelPosition * >(0)),
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextRotation" ),
                  PROP_AXIS_TEXT_ROTATION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ArrangeOrder" ),
                  PROP_AXIS_ARRANGE_ORDER,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartAxisArrangeOrderType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextBreak" ),
                  PROP_AXIS_TEXTBREAK,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextCanOverlap" ),
                  PROP_AXIS_CAN_OVERLAP,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "StackedText" ),
                  PROP_AXIS_STACKEDTEXT,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // Properties related to bar charts:
    rOutProperties.push_back(
        Property( C2U( "Overlap" ),
                  PROP_AXIS_OVERLAP,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "GapWidth" ),
                  PROP_AXIS_GAP_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticAxisWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        //::chart::NamedLineProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
        ::chart::wrapper::WrappedScaleTextProperties::addProperties( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticAxisWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticAxisWrapperPropertyArray_Initializer >
{
};

} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

AxisWrapper::AxisWrapper(
    tAxisType eType, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex ),
        m_eType( eType )
{
}

AxisWrapper::~AxisWrapper()
{
}

// ____ chart::XAxis ____
Reference< beans::XPropertySet > SAL_CALL AxisWrapper::getAxisTitle() throw (uno::RuntimeException)
{
    if( !m_xAxisTitle.is() )
    {
        TitleHelper::eTitleType eTitleType( TitleHelper::X_AXIS_TITLE    );
        switch( m_eType )
        {
            case X_AXIS:
                eTitleType = TitleHelper::X_AXIS_TITLE;
                break;
            case Y_AXIS:
                eTitleType = TitleHelper::Y_AXIS_TITLE;
                break;
            case Z_AXIS:
                eTitleType = TitleHelper::Z_AXIS_TITLE;
                break;
            case SECOND_X_AXIS:
                eTitleType = TitleHelper::SECONDARY_X_AXIS_TITLE;
                break;
            case SECOND_Y_AXIS:
                eTitleType = TitleHelper::SECONDARY_Y_AXIS_TITLE;
                break;
            default:
                return 0;
        }
        m_xAxisTitle = new TitleWrapper( eTitleType, m_spChart2ModelContact );
    }
    return m_xAxisTitle;
}
Reference< beans::XPropertySet > SAL_CALL AxisWrapper::getMajorGrid() throw (uno::RuntimeException)
{
    if( !m_xMajorGrid.is() )
    {
        GridWrapper::tGridType eGridType( GridWrapper::X_MAJOR_GRID );
        switch( m_eType )
        {
            case X_AXIS:
                eGridType = GridWrapper::X_MAJOR_GRID;
                break;
            case Y_AXIS:
                eGridType = GridWrapper::Y_MAJOR_GRID;
                break;
            case Z_AXIS:
                eGridType = GridWrapper::Z_MAJOR_GRID;
                break;
            default:
                return 0;
        }
        m_xMajorGrid = new GridWrapper( eGridType, m_spChart2ModelContact );
    }
    return m_xMajorGrid;
}
Reference< beans::XPropertySet > SAL_CALL AxisWrapper::getMinorGrid() throw (uno::RuntimeException)
{
    if( !m_xMinorGrid.is() )
    {
        GridWrapper::tGridType eGridType( GridWrapper::X_MAJOR_GRID );
        switch( m_eType )
        {
            case X_AXIS:
                eGridType = GridWrapper::X_MINOR_GRID;
                break;
            case Y_AXIS:
                eGridType = GridWrapper::Y_MINOR_GRID;
                break;
            case Z_AXIS:
                eGridType = GridWrapper::Z_MINOR_GRID;
                break;
            default:
                return 0;
        }
        m_xMinorGrid = new GridWrapper( eGridType, m_spChart2ModelContact );
    }
    return m_xMinorGrid;
}

// ____ XShape ____
awt::Point SAL_CALL AxisWrapper::getPosition()
    throw (uno::RuntimeException)
{
    awt::Point aResult( m_spChart2ModelContact->GetAxisPosition( this->getAxis() ) );
    return aResult;
}

void SAL_CALL AxisWrapper::setPosition( const awt::Point& /*aPosition*/ )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "trying to set position of Axis" );
}

awt::Size SAL_CALL AxisWrapper::getSize()
    throw (uno::RuntimeException)
{
    awt::Size aSize( m_spChart2ModelContact->GetAxisSize( this->getAxis() ) );
    return aSize;
}

void SAL_CALL AxisWrapper::setSize( const awt::Size& /*aSize*/ )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "trying to set size of Axis" );
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL AxisWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return C2U( "com.sun.star.chart.ChartAxis" );
}

// ____ XNumberFormatsSupplier ____
uno::Reference< beans::XPropertySet > SAL_CALL AxisWrapper::getNumberFormatSettings()
    throw (uno::RuntimeException)
{
    Reference< util::XNumberFormatsSupplier > xNumSuppl( m_spChart2ModelContact->getChartModel(), uno::UNO_QUERY );
    if( xNumSuppl.is() )
        return xNumSuppl->getNumberFormatSettings();

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< util::XNumberFormats > SAL_CALL AxisWrapper::getNumberFormats()
    throw (uno::RuntimeException)
{
    Reference< util::XNumberFormatsSupplier > xNumSuppl( m_spChart2ModelContact->getChartModel(), uno::UNO_QUERY );
    if( xNumSuppl.is() )
        return xNumSuppl->getNumberFormats();

    return uno::Reference< util::XNumberFormats >();
}

void AxisWrapper::getDimensionAndMainAxisBool( tAxisType eType, sal_Int32& rnDimensionIndex, sal_Bool& rbMainAxis )
{
    switch( eType )
    {
        case X_AXIS:
            rnDimensionIndex = 0; rbMainAxis = sal_True; break;
        case Y_AXIS:
            rnDimensionIndex = 1; rbMainAxis = sal_True; break;
        case Z_AXIS:
            rnDimensionIndex = 2; rbMainAxis = sal_True; break;
        case SECOND_X_AXIS:
            rnDimensionIndex = 0; rbMainAxis = sal_False; break;
        case SECOND_Y_AXIS:
            rnDimensionIndex = 1; rbMainAxis = sal_False; break;
    }
}

// ____ XComponent ____
void SAL_CALL AxisWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    DisposeHelper::DisposeAndClear( m_xAxisTitle );
    DisposeHelper::DisposeAndClear( m_xMajorGrid );
    DisposeHelper::DisposeAndClear( m_xMinorGrid );

    clearWrappedPropertySet();
}

void SAL_CALL AxisWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL AxisWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ================================================================================

//ReferenceSizePropertyProvider
void AxisWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp( this->getAxis(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        if( xProp->getPropertyValue( C2U("ReferencePageSize") ).hasValue() )
            xProp->setPropertyValue( C2U("ReferencePageSize"), uno::makeAny(
            m_spChart2ModelContact->GetPageSize() ));
    }
}
Any AxisWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp( this->getAxis(), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue( C2U("ReferencePageSize") );
    return aRet;
}
awt::Size AxisWrapper::getCurrentSizeForReference()
{
    return m_spChart2ModelContact->GetPageSize();
}

// ================================================================================

Reference< chart2::XAxis > AxisWrapper::getAxis()
{
    Reference< chart2::XAxis > xAxis;
    try
    {
        sal_Int32 nDimensionIndex = 0;
        sal_Bool  bMainAxis = sal_True;
        AxisWrapper::getDimensionAndMainAxisBool( m_eType, nDimensionIndex, bMainAxis );

        Reference< XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        xAxis = AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram );
        if( !xAxis.is() )
        {
            xAxis = AxisHelper::createAxis( nDimensionIndex, bMainAxis, xDiagram, m_spChart2ModelContact->m_xContext );
            Reference< beans::XPropertySet > xProp( xAxis, uno::UNO_QUERY );
            if( xProp.is() )
                xProp->setPropertyValue( C2U( "Show" ), uno::makeAny( sal_False ) );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return xAxis;
}

// WrappedPropertySet
Reference< beans::XPropertySet > AxisWrapper::getInnerPropertySet()
{
    return Reference< beans::XPropertySet >( this->getAxis(), uno::UNO_QUERY );
}

const Sequence< beans::Property >& AxisWrapper::getPropertySequence()
{
    return *StaticAxisWrapperPropertyArray::get();
}

const std::vector< WrappedProperty* > AxisWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

    aWrappedProperties.push_back( new WrappedTextRotationProperty() );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "Marks" ), C2U( "MajorTickmarks" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "HelpMarks" ), C2U( "MinorTickmarks" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "TextCanOverlap" ), C2U( "TextOverlap" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "ArrangeOrder" ), C2U( "ArrangeOrder" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "Visible" ), C2U( "Show" ) ) );
    aWrappedProperties.push_back( new WrappedDirectStateProperty( C2U( "DisplayLabels" ), C2U( "DisplayLabels" ) ) );
    aWrappedProperties.push_back( new WrappedDirectStateProperty( C2U( "TextBreak" ), C2U( "TextBreak" ) ) );
    WrappedNumberFormatProperty* pWrappedNumberFormatProperty = new WrappedNumberFormatProperty( m_spChart2ModelContact );
    aWrappedProperties.push_back( pWrappedNumberFormatProperty );
    aWrappedProperties.push_back( new WrappedLinkNumberFormatProperty(pWrappedNumberFormatProperty) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "StackedText" ), C2U( "StackCharacters" ) ) );
    aWrappedProperties.push_back( new WrappedDirectStateProperty( C2U( "CrossoverPosition" ), C2U( "CrossoverPosition" ) ) );
    {
        WrappedGapwidthProperty* pWrappedGapwidthProperty( new WrappedGapwidthProperty( m_spChart2ModelContact ) );
        WrappedBarOverlapProperty* pWrappedBarOverlapProperty( new WrappedBarOverlapProperty( m_spChart2ModelContact ) );
        sal_Int32 nDimensionIndex = 0;
        sal_Bool  bMainAxis = sal_True;
        sal_Int32 nAxisIndex = 0;
        AxisWrapper::getDimensionAndMainAxisBool( m_eType, nDimensionIndex, bMainAxis );
        if( !bMainAxis )
            nAxisIndex = 1;
        pWrappedGapwidthProperty->setDimensionAndAxisIndex( nDimensionIndex, nAxisIndex );
        pWrappedBarOverlapProperty->setDimensionAndAxisIndex( nDimensionIndex, nAxisIndex );
        aWrappedProperties.push_back( pWrappedGapwidthProperty );
        aWrappedProperties.push_back( pWrappedBarOverlapProperty );
    }

    WrappedScaleProperty::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    return aWrappedProperties;
}

// ================================================================================

Sequence< OUString > AxisWrapper::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartAxis" );
    aServices[ 1 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 2 ] = C2U( "com.sun.star.style.CharacterProperties" );
//     aServices[ 3 ] = C2U( "com.sun.star.beans.PropertySet" );
//     aServices[ 4 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( AxisWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart
