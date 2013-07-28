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
#ifndef CHART_WRAPPEDPROPERTY_HXX
#define CHART_WRAPPEDPROPERTY_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include "charttoolsdllapi.hxx"

#include <map>

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS WrappedProperty
{
    /** The property visible to the outer PropertySet in the PropertySetWrapper may have a different name, type and value than
    a corresponding property of the inner PropertySet. Use this class to do the conversion between the two.
    */
public:
    WrappedProperty( const OUString& rOuterName, const OUString& rInnerName );
    virtual ~WrappedProperty();

    const OUString& getOuterName() const;
    virtual OUString getInnerName() const;

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual void setPropertyToDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::PropertyState getPropertyState( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

protected:
    virtual ::com::sun::star::uno::Any convertInnerToOuterValue( const ::com::sun::star::uno::Any& rInnerValue ) const;
    virtual ::com::sun::star::uno::Any convertOuterToInnerValue( const ::com::sun::star::uno::Any& rOuterValue ) const;

protected:
    OUString             m_aOuterName;
    OUString             m_aInnerName;
};

typedef ::std::map< sal_Int32, const WrappedProperty* > tWrappedPropertyMap;

} //namespace chart

// CHART_WRAPPEDPROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
