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
#pragma once

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include "charttoolsdllapi.hxx"

#include <map>
#include <memory>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::beans { class XPropertyState; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }

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

    const OUString& getOuterName() const { return m_aOuterName;}
    virtual OUString getInnerName() const;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::beans::PropertyVetoException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    virtual void setPropertyToDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    virtual css::beans::PropertyState getPropertyState( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const;

protected:
    virtual css::uno::Any convertInnerToOuterValue( const css::uno::Any& rInnerValue ) const;
    virtual css::uno::Any convertOuterToInnerValue( const css::uno::Any& rOuterValue ) const;

protected:
    OUString             m_aOuterName;
    OUString             m_aInnerName;
};

typedef std::map< sal_Int32, std::unique_ptr<const WrappedProperty> > tWrappedPropertyMap;

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
