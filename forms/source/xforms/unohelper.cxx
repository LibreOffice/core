/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unohelper.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_forms.hxx"

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
#include <unotools/processfactory.hxx>


using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Exception;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::Property;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::PropertyAttribute::READONLY;
using rtl::OUString;


Reference<XInterface> xforms::createInstance( const OUString& sServiceName )
{
    Reference<XMultiServiceFactory> xFactory = utl::getProcessServiceFactory();
    OSL_ENSURE( xFactory.is(), "can't get service factory" );

    Reference<XInterface> xInstance = xFactory->createInstance( sServiceName );
    OSL_ENSURE( xInstance.is(), "failed to create instance" );

    return xInstance;
}

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
