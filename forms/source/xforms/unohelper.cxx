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


#include "unohelper.hxx"

#include <osl/diagnose.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/processfactory.hxx>


using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Exception;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::Property;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::PropertyAttribute::READONLY;


void xforms::copy( const Reference<XPropertySet>& xFrom,
                   Reference<XPropertySet>& xTo )
{
    OSL_ENSURE( xFrom.is(), "no source" );
    OSL_ENSURE( xTo.is(), "no target" );

    // get property names & infos, and iterate over target properties
    Sequence<Property> aProperties =
        xTo->getPropertySetInfo()->getProperties();
    sal_Int32 nProperties = aProperties.getLength();
    const Property* pProperties = aProperties.getConstArray();
    Reference<XPropertySetInfo> xFromInfo = xFrom->getPropertySetInfo();
    for( sal_Int32 n = 0; n < nProperties; n++ )
    {
        const OUString& rName = pProperties[n].Name;

        // if both set have the property, copy the value
        // (catch and ignore exceptions, if any)
        if( xFromInfo->hasPropertyByName( rName ) )
        {
            try
            {
                Property aProperty = xFromInfo->getPropertyByName( rName );
                if ( ( aProperty.Attributes & READONLY ) == 0 )
                    xTo->setPropertyValue(rName, xFrom->getPropertyValue( rName ));
            }
            catch( const Exception& )
            {
                // ignore any errors; we'll copy as good as we can
            }
        }
        // else: no property? then ignore.
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
