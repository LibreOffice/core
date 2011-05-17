/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef CHART_WRAPPED_SCALE_PROPERTY_HXX
#define CHART_WRAPPED_SCALE_PROPERTY_HXX

#include "WrappedProperty.hxx"
#include "Chart2ModelContact.hxx"

#include <boost/shared_ptr.hpp>
#include <vector>

//.............................................................................
namespace chart
{
namespace wrapper
{

class WrappedScaleProperty : public WrappedProperty
{
public:
    enum tScaleProperty
    {
          SCALE_PROP_MAX
        , SCALE_PROP_MIN
        , SCALE_PROP_ORIGIN
        , SCALE_PROP_STEPMAIN
        , SCALE_PROP_STEPHELP //deprecated property
        , SCALE_PROP_STEPHELP_COUNT
        , SCALE_PROP_AUTO_MAX
        , SCALE_PROP_AUTO_MIN
        , SCALE_PROP_AUTO_ORIGIN
        , SCALE_PROP_AUTO_STEPMAIN
        , SCALE_PROP_AUTO_STEPHELP
        , SCALE_PROP_AXIS_TYPE
        , SCALE_PROP_DATE_INCREMENT
        , SCALE_PROP_EXPLICIT_DATE_INCREMENT
        , SCALE_PROP_LOGARITHMIC
        , SCALE_PROP_REVERSEDIRECTION
    };

public:
    WrappedScaleProperty( tScaleProperty eScaleProperty, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedScaleProperty();

    static void addWrappedProperties( std::vector< WrappedProperty* >& rList, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

protected: //methods
    void setPropertyValue( tScaleProperty eScaleProperty, const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any getPropertyValue( tScaleProperty eScaleProperty, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    tScaleProperty          m_eScaleProperty;

    mutable ::com::sun::star::uno::Any m_aOuterValue;
};

} //  namespace wrapper
} //  namespace chart
//.............................................................................

// CHART_WRAPPED_SCALE_PROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
