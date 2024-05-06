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

#include <LabeledDataSequence.hxx>
#include <ModifyListenerHelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <utility>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

LabeledDataSequence::LabeledDataSequence() :
        m_xModifyEventForwarder( new ModifyEventForwarder() )
{}

LabeledDataSequence::LabeledDataSequence(
    uno::Reference< chart2::data::XDataSequence > xValues ) :
        m_xData(std::move( xValues )),
        m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    ModifyListenerHelper::addListener( m_xData, m_xModifyEventForwarder );
}

LabeledDataSequence::LabeledDataSequence(
    uno::Reference< chart2::data::XDataSequence > xValues,
    uno::Reference< chart2::data::XDataSequence > xLabel ) :
        m_xData(std::move( xValues )),
        m_xLabel(std::move( xLabel )),
        m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    ModifyListenerHelper::addListener( m_xData, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( m_xLabel, m_xModifyEventForwarder );
}

LabeledDataSequence::LabeledDataSequence( const LabeledDataSequence& rSource ) :
    impl::LabeledDataSequence_Base(),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    uno::Reference< chart2::data::XDataSequence > xNewValues( rSource.m_xData );
    uno::Reference< chart2::data::XDataSequence > xNewLabel( rSource.m_xLabel );

    uno::Reference< util::XCloneable > xLabelCloneable( rSource.m_xLabel, uno::UNO_QUERY );
    if( xLabelCloneable.is())
        xNewLabel.set( xLabelCloneable->createClone(), uno::UNO_QUERY );

    uno::Reference< util::XCloneable > xValuesCloneable( rSource.m_xData, uno::UNO_QUERY );
    if( xValuesCloneable.is())
        xNewValues.set( xValuesCloneable->createClone(), uno::UNO_QUERY );

    m_xData = xNewValues;
    m_xLabel = xNewLabel;

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
{
    return m_xData;
}

void SAL_CALL LabeledDataSequence::setValues(
    const uno::Reference< chart2::data::XDataSequence >& xSequence )
{
    if( m_xData != xSequence )
    {
        ModifyListenerHelper::removeListener( m_xData, m_xModifyEventForwarder );
        m_xData = xSequence;
        ModifyListenerHelper::addListener( m_xData, m_xModifyEventForwarder );
    }
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL LabeledDataSequence::getLabel()
{
    return m_xLabel;
}

void SAL_CALL LabeledDataSequence::setLabel(
    const uno::Reference< chart2::data::XDataSequence >& xSequence )
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
{
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL LabeledDataSequence::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
}

OUString SAL_CALL LabeledDataSequence::getImplementationName()
{
    return u"com.sun.star.comp.chart2.LabeledDataSequence"_ustr;
}

sal_Bool SAL_CALL LabeledDataSequence::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LabeledDataSequence::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.data.LabeledDataSequence"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_LabeledDataSequence_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::LabeledDataSequence );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
