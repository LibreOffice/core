/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedDefaultProperty.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:06:39 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedDefaultProperty.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

WrappedDefaultProperty::WrappedDefaultProperty(
    const OUString& rOuterName, const OUString& rInnerName,
    const uno::Any& rNewOuterDefault ) :
        WrappedProperty( rOuterName, rInnerName ),
        m_aOuterDefaultValue( rNewOuterDefault )
{}

WrappedDefaultProperty::~WrappedDefaultProperty()
{}

void WrappedDefaultProperty::setPropertyToDefault(
    const Reference< beans::XPropertyState >& xInnerPropertyState ) const
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    Reference< beans::XPropertySet > xInnerPropSet( xInnerPropertyState, uno::UNO_QUERY );
    if( xInnerPropSet.is())
        this->setPropertyValue( m_aOuterDefaultValue, xInnerPropSet );
}

uno::Any WrappedDefaultProperty::getPropertyDefault(
    const Reference< beans::XPropertyState >& xInnerPropertyState ) const
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    return m_aOuterDefaultValue;
}

beans::PropertyState WrappedDefaultProperty::getPropertyState(
    const Reference< beans::XPropertyState >& xInnerPropertyState ) const
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    beans::PropertyState aState = beans::PropertyState_DIRECT_VALUE;
    try
    {
        Reference< beans::XPropertySet > xInnerProp( xInnerPropertyState, uno::UNO_QUERY_THROW );
        uno::Any aValue = this->getPropertyValue( xInnerProp );
        if( m_aOuterDefaultValue == this->convertInnerToOuterValue( aValue ))
            aState = beans::PropertyState_DEFAULT_VALUE;
    }
    catch( beans::UnknownPropertyException& ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return aState;
}

} //  namespace chart
