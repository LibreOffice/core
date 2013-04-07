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


#include "FormattedStringHelper.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

Sequence< Reference< chart2::XFormattedString > >
            FormattedStringHelper::createFormattedStringSequence(
                     const Reference< uno::XComponentContext > & xContext
                    , const OUString & rString
                    , const Reference< beans::XPropertySet > & xTextProperties ) throw()
{
    Reference< XFormattedString > xFormStr;
    try
    {
        if( xContext.is() )
        {
            xFormStr.set(
                xContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.chart2.FormattedString", xContext ),
                uno::UNO_QUERY_THROW );

            xFormStr->setString( rString );

            // set character properties
            comphelper::copyProperties(
                xTextProperties, Reference< beans::XPropertySet >( xFormStr, uno::UNO_QUERY ) );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return Sequence< Reference< XFormattedString > >( & xFormStr, 1 );
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
