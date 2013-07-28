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
#ifndef CHART_WRAPPED_SCENE_PROPERTY_HXX
#define CHART_WRAPPED_SCENE_PROPERTY_HXX

#include "WrappedProperty.hxx"
#include "Chart2ModelContact.hxx"

#include <boost/shared_ptr.hpp>
#include <vector>

namespace chart
{
namespace wrapper
{

class WrappedSceneProperty
{
public:
    static void addWrappedProperties( std::vector< WrappedProperty* >& rList
        , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
};

class WrappedD3DTransformMatrixProperty : public WrappedProperty
{
public:
    WrappedD3DTransformMatrixProperty(
        ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedD3DTransformMatrixProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private:
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
};

} //namespace wrapper
} //namespace chart

// CHART_WRAPPED_SCENE_PROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
