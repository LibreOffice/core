/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unohelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:05:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
