/*************************************************************************
 *
 *  $RCSfile: unohelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:57:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
                if ( ( aProperty.Attributes && READONLY ) == 0 )
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
