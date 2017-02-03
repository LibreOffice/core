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

#include "WrappedNumberFormatProperty.hxx"
#include "macros.hxx"
#include <unonames.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

namespace chart
{
namespace wrapper
{

WrappedNumberFormatProperty::WrappedNumberFormatProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
        : WrappedDirectStateProperty( CHART_UNONAME_NUMFMT, CHART_UNONAME_NUMFMT )
        , m_spChart2ModelContact(spChart2ModelContact)
{
}

WrappedNumberFormatProperty::~WrappedNumberFormatProperty()
{
}

void WrappedNumberFormatProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    sal_Int32 nFormat = 0;
    if( ! (rOuterValue >>= nFormat) )
        throw lang::IllegalArgumentException( "Property 'NumberFormat' requires value of type sal_Int32", nullptr, 0 );

    if(xInnerPropertySet.is())
        xInnerPropertySet->setPropertyValue(getInnerName(), this->convertOuterToInnerValue(rOuterValue));
}

Any WrappedNumberFormatProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    if( !xInnerPropertySet.is() )
    {
        OSL_FAIL("missing xInnerPropertySet in WrappedNumberFormatProperty::getPropertyValue");
        return Any();
    }
    Any aRet( xInnerPropertySet->getPropertyValue(getInnerName()));
    if( !aRet.hasValue() )
    {
        sal_Int32 nKey = 0;
        Reference< chart2::XDataSeries > xSeries( xInnerPropertySet, uno::UNO_QUERY );
        if( xSeries.is() )
            nKey = m_spChart2ModelContact->getExplicitNumberFormatKeyForSeries( xSeries );
        else
        {
            Reference< chart2::XAxis > xAxis( xInnerPropertySet, uno::UNO_QUERY );
            nKey = m_spChart2ModelContact->getExplicitNumberFormatKeyForAxis( xAxis );
        }
        aRet <<= nKey;
    }
    return aRet;
}

Any WrappedNumberFormatProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    return uno::Any( sal_Int32( 0 ) );
}

WrappedLinkNumberFormatProperty::WrappedLinkNumberFormatProperty() :
    WrappedDirectStateProperty(CHART_UNONAME_LINK_TO_SRC_NUMFMT, CHART_UNONAME_LINK_TO_SRC_NUMFMT)
{
}

WrappedLinkNumberFormatProperty::~WrappedLinkNumberFormatProperty()
{
}

void WrappedLinkNumberFormatProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    if( !xInnerPropertySet.is() )
    {
        OSL_FAIL("missing xInnerPropertySet in WrappedNumberFormatProperty::setPropertyValue");
        return;
    }

    xInnerPropertySet->setPropertyValue(getInnerName(), rOuterValue);
}

Any WrappedLinkNumberFormatProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    if( !xInnerPropertySet.is() )
    {
        OSL_FAIL("missing xInnerPropertySet in WrappedNumberFormatProperty::getPropertyValue");
        return getPropertyDefault(nullptr);
    }

    return xInnerPropertySet->getPropertyValue(getInnerName());
}

Any WrappedLinkNumberFormatProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    return uno::Any( true ); // bLink
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
