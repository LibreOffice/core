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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_WRAPPEDTEXTROTATIONPROPERTY_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_WRAPPEDTEXTROTATIONPROPERTY_HXX

#include "WrappedProperty.hxx"

namespace chart
{

class WrappedTextRotationProperty : public WrappedProperty
{
public:
    explicit WrappedTextRotationProperty( bool bDirectState=false );
    virtual ~WrappedTextRotationProperty();

    virtual ::com::sun::star::beans::PropertyState getPropertyState( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException) override;

protected:
    virtual ::com::sun::star::uno::Any convertInnerToOuterValue( const ::com::sun::star::uno::Any& rInnerValue ) const override;
    virtual ::com::sun::star::uno::Any convertOuterToInnerValue( const ::com::sun::star::uno::Any& rOuterValue ) const override;

    bool m_bDirectState;
};

} //namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_WRAPPEDTEXTROTATIONPROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
