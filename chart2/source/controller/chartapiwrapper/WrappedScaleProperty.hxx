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
#ifndef CHART_WRAPPED_SCALE_PROPERTY_HXX
#define CHART_WRAPPED_SCALE_PROPERTY_HXX

#include "WrappedProperty.hxx"
#include "Chart2ModelContact.hxx"

#include <boost/shared_ptr.hpp>
#include <vector>

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

// CHART_WRAPPED_SCALE_PROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
