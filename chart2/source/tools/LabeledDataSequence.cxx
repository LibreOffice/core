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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "LabeledDataSequence.hxx"
#include "ModifyListenerHelper.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

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

// ================================================================================

Sequence< OUString > LabeledDataSequence::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.data.LabeledDataSequence" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LabeledDataSequence,
                             C2U( "com.sun.star.comp.chart2.LabeledDataSequence" ))

// ================================================================================

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
