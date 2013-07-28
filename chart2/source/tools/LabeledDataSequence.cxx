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

#include "LabeledDataSequence.hxx"
#include "ModifyListenerHelper.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

LabeledDataSequence::LabeledDataSequence( const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

LabeledDataSequence::LabeledDataSequence(
    const uno::Reference< chart2::data::XDataSequence > & rValues ) :
        m_xData( rValues ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    ModifyListenerHelper::addListener( m_xData, m_xModifyEventForwarder );
}

LabeledDataSequence::LabeledDataSequence(
    const uno::Reference< chart2::data::XDataSequence > & rValues,
    const uno::Reference< chart2::data::XDataSequence > & rLabel ) :
        m_xData( rValues ),
        m_xLabel( rLabel ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    ModifyListenerHelper::addListener( m_xData, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( m_xLabel, m_xModifyEventForwarder );
}

LabeledDataSequence::~LabeledDataSequence()
{
    if( m_xModifyEventForwarder.is())
    {
        if( m_xData.is())
            ModifyListenerHelper::removeListener( m_xData, m_xModifyEventForwarder );
        if( m_xLabel.is())
            ModifyListenerHelper::removeListener( m_xLabel, m_xModifyEventForwarder );
    }
}

// ____ XLabeledDataSequence ____
uno::Reference< chart2::data::XDataSequence > SAL_CALL LabeledDataSequence::getValues()
    throw (uno::RuntimeException)
{
    return m_xData;
}

void SAL_CALL LabeledDataSequence::setValues(
    const uno::Reference< chart2::data::XDataSequence >& xSequence )
    throw (uno::RuntimeException)
{
    if( m_xData != xSequence )
    {
        ModifyListenerHelper::removeListener( m_xData, m_xModifyEventForwarder );
        m_xData = xSequence;
        ModifyListenerHelper::addListener( m_xData, m_xModifyEventForwarder );
    }
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL LabeledDataSequence::getLabel()
    throw (uno::RuntimeException)
{
    return m_xLabel;
}

void SAL_CALL LabeledDataSequence::setLabel(
    const uno::Reference< chart2::data::XDataSequence >& xSequence )
    throw (uno::RuntimeException)
{
    if( m_xLabel != xSequence )
    {
        ModifyListenerHelper::removeListener( m_xLabel, m_xModifyEventForwarder );
        m_xLabel = xSequence;
        ModifyListenerHelper::addListener( m_xLabel, m_xModifyEventForwarder );
    }
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL LabeledDataSequence::createClone()
    throw (uno::RuntimeException)
{
    uno::Reference< chart2::data::XDataSequence > xNewValues( m_xData );
    uno::Reference< chart2::data::XDataSequence > xNewLabel( m_xLabel );

    uno::Reference< util::XCloneable > xLabelCloneable( m_xLabel, uno::UNO_QUERY );
    if( xLabelCloneable.is())
        xNewLabel.set( xLabelCloneable->createClone(), uno::UNO_QUERY );

    uno::Reference< util::XCloneable > xValuesCloneable( m_xData, uno::UNO_QUERY );
    if( xValuesCloneable.is())
        xNewValues.set( xValuesCloneable->createClone(), uno::UNO_QUERY );

    return uno::Reference< util::XCloneable >(
        new LabeledDataSequence( xNewValues, xNewLabel ) );
}

// ____ XModifyBroadcaster ____
void SAL_CALL LabeledDataSequence::addModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL LabeledDataSequence::removeModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

Sequence< OUString > LabeledDataSequence::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = "com.sun.star.chart2.data.LabeledDataSequence";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LabeledDataSequence,
                             OUString("com.sun.star.comp.chart2.LabeledDataSequence") )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
