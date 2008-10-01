/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataPoint.cxx,v $
 * $Revision: 1.9.44.1 $
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
#include "DataPoint.hxx"
#include "DataPointProperties.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::rtl::OUString;

// ____________________________________________________________

namespace
{
const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::DataPointProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}
} // anonymous namespace

// ____________________________________________________________

namespace chart
{

DataPoint::DataPoint( const uno::Reference< beans::XPropertySet > & rParentProperties ) :
        ::property::OPropertySet( m_aMutex ),
        m_xParentProperties( rParentProperties ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder()),
        m_bNoParentPropAllowed( false )
{}

DataPoint::DataPoint( const DataPoint & rOther ) :
        MutexContainer(),
        impl::DataPoint_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder()),
        m_bNoParentPropAllowed( true )
{
    // m_xParentProperties has to be set from outside, like in the method
    // DataSeries::createClone

    // add as listener to XPropertySet properties
    Reference< beans::XPropertySet > xPropertySet;
    uno::Any aValue;

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );

    m_bNoParentPropAllowed = false;
}

DataPoint::~DataPoint()
{
    try
    {
        // remove listener from XPropertySet properties
        Reference< beans::XPropertySet > xPropertySet;
        uno::Any aValue;

        getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X );
        if( ( aValue >>= xPropertySet )
            && xPropertySet.is())
            ModifyListenerHelper::removeListener( xPropertySet, m_xModifyEventForwarder );

        getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y );
        if( ( aValue >>= xPropertySet )
            && xPropertySet.is())
            ModifyListenerHelper::removeListener( xPropertySet, m_xModifyEventForwarder );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL DataPoint::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new DataPoint( *this ));
}

// ____ XChild ____
Reference< uno::XInterface > SAL_CALL DataPoint::getParent()
    throw (uno::RuntimeException)
{
    return Reference< uno::XInterface >( m_xParentProperties, uno::UNO_QUERY );
}

void SAL_CALL DataPoint::setParent(
    const Reference< uno::XInterface >& Parent )
    throw (lang::NoSupportException,
           uno::RuntimeException)
{
    m_xParentProperties.set( Parent, uno::UNO_QUERY );
}

// ____ OPropertySet ____
uno::Any DataPoint::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    // the value set at the data series is the default
    uno::Reference< beans::XFastPropertySet > xFast( m_xParentProperties, uno::UNO_QUERY );
    if( !xFast.is())
    {
        OSL_ENSURE( m_bNoParentPropAllowed, "data point needs a parent property set to provide values correctly" );
        return uno::Any();
    }

    return xFast->getFastPropertyValue( nHandle );
}

void SAL_CALL DataPoint::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    if(    nHandle == DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y
        || nHandle == DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X )
    {
        uno::Any aOldValue;
        Reference< util::XModifyBroadcaster > xBroadcaster;
        this->getFastPropertyValue( aOldValue, nHandle );
        if( aOldValue.hasValue() &&
            (aOldValue >>= xBroadcaster) &&
            xBroadcaster.is())
        {
            ModifyListenerHelper::removeListener( xBroadcaster, m_xModifyEventForwarder );
        }

        OSL_ASSERT( rValue.getValueType().getTypeClass() == uno::TypeClass_INTERFACE );
        if( rValue.hasValue() &&
            (rValue >>= xBroadcaster) &&
            xBroadcaster.is())
        {
            ModifyListenerHelper::addListener( xBroadcaster, m_xModifyEventForwarder );
        }
    }

    ::property::OPropertySet::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

::cppu::IPropertyArrayHelper & SAL_CALL DataPoint::getInfoHelper()
{
    return getInfoHelperConst();
}

::cppu::IPropertyArrayHelper & SAL_CALL DataPoint::getInfoHelperConst() const
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL
    DataPoint::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static Reference< beans::XPropertySetInfo > xInfo;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

// ____ XModifyBroadcaster ____
void SAL_CALL DataPoint::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL DataPoint::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL DataPoint::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL DataPoint::disposing( const lang::EventObject& )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void DataPoint::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void DataPoint::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

Sequence< OUString > DataPoint::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.DataPoint" );
    aServices[ 1 ] = C2U( "com.sun.star.chart2.DataPointProperties" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// needed by MSC compiler
using impl::DataPoint_Base;

IMPLEMENT_FORWARD_XINTERFACE2( DataPoint, DataPoint_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DataPoint, C2U( "com.sun.star.comp.chart.DataPoint" ));

} //  namespace chart
