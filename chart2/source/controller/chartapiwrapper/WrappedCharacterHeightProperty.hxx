/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedCharacterHeightProperty.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:21:16 $
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
#ifndef CHART_WRAPPED_CHARACTERHEIGHT_PROPERTY_HXX
#define CHART_WRAPPED_CHARACTERHEIGHT_PROPERTY_HXX

#include "WrappedProperty.hxx"

#include <vector>

//.............................................................................
namespace chart
{
namespace wrapper
{

class ReferenceSizePropertyProvider;

class WrappedCharacterHeightProperty_Base : public WrappedProperty
{
public:
    WrappedCharacterHeightProperty_Base( const ::rtl::OUString& rOuterEqualsInnerName, ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedCharacterHeightProperty_Base();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::PropertyState getPropertyState( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

protected:
    virtual ::com::sun::star::uno::Any convertInnerToOuterValue( const ::com::sun::star::uno::Any& rInnerValue ) const;
    virtual ::com::sun::star::uno::Any convertOuterToInnerValue( const ::com::sun::star::uno::Any& rOuterValue ) const;

protected:
    ReferenceSizePropertyProvider*  m_pRefSizePropProvider;
};

//-----------------------------------------------------------------------------

class WrappedCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    WrappedCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedCharacterHeightProperty();

    static void addWrappedProperties( std::vector< WrappedProperty* >& rList, ReferenceSizePropertyProvider* pRefSizePropProvider );
};

//-----------------------------------------------------------------------------

class WrappedAsianCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    WrappedAsianCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedAsianCharacterHeightProperty();
};

//-----------------------------------------------------------------------------

class WrappedComplexCharacterHeightProperty : public WrappedCharacterHeightProperty_Base
{
public:
    WrappedComplexCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider );
    virtual ~WrappedComplexCharacterHeightProperty();
};

} //namespace wrapper
} //namespace chart
//.............................................................................

// CHART_WRAPPED_CHARACTERHEIGHT_PROPERTY_HXX
#endif
