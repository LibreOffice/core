/*************************************************************************
 *
 *  $RCSfile: services.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:04:04 $
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
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/mutex.hxx>
#include <sfx2/sfxuno.hxx>

#include "../dom/documentbuilder.hxx"
#include "../dom/saxbuilder.hxx"
#include "../xpath/xpathapi.hxx"
#include "../events/testlistener.hxx"

extern "C"
{
using namespace ::DOM;
using namespace ::DOM::events;
using namespace ::XPath;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

void SAL_CALL
component_getImplementationEnvironment(const sal_Char **ppEnvironmentTypeName, uno_Environment **ppEnvironment)
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL
component_writeInfo(void *pServiceManager, void *pRegistryKey)
{
    Reference< XRegistryKey > xKey(reinterpret_cast< XRegistryKey* >(pRegistryKey));
    Reference< XRegistryKey > xNewKey;
    OUString aImpl;

    // register DOM service
    aImpl =  OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += CDocumentBuilder::_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(CDocumentBuilder::_getSupportedServiceNames()[0]);

    // register DOM service
    aImpl =  OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += CSAXDocumentBuilder::_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(CSAXDocumentBuilder::_getSupportedServiceNames()[0]);

    // register XPath service
    aImpl = OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += CXPathAPI::_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(CXPathAPI::_getSupportedServiceNames()[0]);

    // register EventTest service
    aImpl = OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += CTestListener::_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(CTestListener::_getSupportedServiceNames()[0]);


    return sal_True;
}

void* SAL_CALL
component_getFactory(const sal_Char *pImplementationName, void *pServiceManager, void *pRegistryKey)
{
    void* pReturn = NULL ;
    if  ( pImplementationName && pServiceManager )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory >  xServiceManager(
            reinterpret_cast< XMultiServiceFactory* >(pServiceManager));

        if (CDocumentBuilder::_getImplementationName().compareToAscii( pImplementationName ) == 0 )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createOneInstanceFactory(
                    xServiceManager, CDocumentBuilder::_getImplementationName(),
                    CDocumentBuilder::_getInstance, CDocumentBuilder::_getSupportedServiceNames()));
        }
        else if (CSAXDocumentBuilder::_getImplementationName().compareToAscii( pImplementationName ) == 0 )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CSAXDocumentBuilder::_getImplementationName(),
                    CSAXDocumentBuilder::_getInstance, CSAXDocumentBuilder::_getSupportedServiceNames()));
        }
        else if (CXPathAPI::_getImplementationName().compareToAscii( pImplementationName ) == 0 )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CXPathAPI::_getImplementationName(),
                    CXPathAPI::_getInstance, CXPathAPI::_getSupportedServiceNames()));
        }
        else if (CTestListener::_getImplementationName().compareToAscii( pImplementationName ) == 0 )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CTestListener::_getImplementationName(),
                    CTestListener::_getInstance, CTestListener::_getSupportedServiceNames()));
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}

} // extern "C"
