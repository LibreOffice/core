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
using rtl::OUString;

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
                    C2U("com.sun.star.chart2.FormattedString"), xContext ),
                uno::UNO_QUERY_THROW );

            xFormStr->setString( rString );

            // set character properties
            comphelper::copyProperties(
                xTextProperties, Reference< beans::XPropertySet >( xFormStr, uno::UNO_QUERY ) );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return Sequence< Reference< XFormattedString > >( & xFormStr, 1 );
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
