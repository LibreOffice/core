/*************************************************************************
 *
 *  $RCSfile: cfgregistry.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:13:43 $
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
#include <memory.h>
#include <stdio.h>

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _VOS_CONDITN_HXX_
#include <vos/conditn.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;
using namespace ::vos;
using namespace ::cppu;
//using namespace ::configmgr;

//=============================================================================
//= a dirty littly class for printing ascii characters
//=============================================================================
class OAsciiOutput
{
protected:
    sal_Char*   m_pCharacters;

public:
    OAsciiOutput(const ::rtl::OUString& _rUnicodeChars);
    ~OAsciiOutput() { delete m_pCharacters; }

    const sal_Char* getCharacters() const { return m_pCharacters; }
};

//-----------------------------------------------------------------------------
OAsciiOutput::OAsciiOutput(const ::rtl::OUString& _rUnicodeChars)
{
    sal_Int32 nLen = _rUnicodeChars.getLength();
    m_pCharacters = new sal_Char[nLen + 1];
    sal_Char* pFillPtr = m_pCharacters;
    const sal_Unicode* pSourcePtr = _rUnicodeChars.getStr();
#ifdef DEBUG
    sal_Bool bAsserted = sal_False;
#endif
    for (sal_Int32 i=0; i<nLen; ++i, ++pFillPtr, ++pSourcePtr)
    {
        OSL_ENSHURE(bAsserted || !(bAsserted = (*pSourcePtr >= 0x80)),
            "OAsciiOutput::OAsciiOutput : non-ascii character found !");
        *pFillPtr = *reinterpret_cast<const sal_Char*>(pSourcePtr);
    }
    *pFillPtr = 0;
}

#define ASCII_STRING(rtlOUString)   OAsciiOutput(rtlOUString).getCharacters()
#define UNI_STRING(asciiString)     ::rtl::OUString::createFromAscii(asciiString)

//=============================================================================
//=============================================================================

#if (defined UNX) || (defined OS2)
void main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
#endif
{
    TimeValue aTimeout;
    aTimeout.Seconds = 5;
    aTimeout.Nanosec = 0;

    Reference< XMultiServiceFactory > xORB = createRegistryServiceFactory(
        ::rtl::OUString::createFromAscii("l:\\bin.a\\applicat.rdb"),
        ::rtl::OUString()
        );
    if (!xORB.is())
    {
        fprintf(stdout, "could not create the service factory !\n\n");
        return;
    }

    try
    {
        Reference< XSimpleRegistry > xConfigurationRegistry;
        printf("instantiating the configuration registry access\n\r");
        xConfigurationRegistry = xConfigurationRegistry.query(
                xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationRegistry"))
            );

        const sal_Char* pLayoutNode = "com.sun.star.Inet";
        printf("opening the registry access to %s\n\r", pLayoutNode);
        xConfigurationRegistry->open(UNI_STRING(pLayoutNode), sal_False, sal_False);

        printf("retrieving the root key, enumerating elements\n\r");
        Reference< XRegistryKey > xRoot = xConfigurationRegistry->getRootKey();
        Sequence< ::rtl::OUString > aKeyNames = xRoot->getKeyNames();
        const ::rtl::OUString* pKeyNames = aKeyNames.getConstArray();
        for (sal_Int32 i=0; i<aKeyNames.getLength(); ++i, ++pKeyNames)
            printf("\t%i\t%s\n\r", i, ASCII_STRING(*pKeyNames));

        const sal_Char* pUpdateKey = "Proxy/NoProxy";
        printf("retrieving the key for %s\n\r", pUpdateKey);
        Reference< XRegistryKey > xTabStopsKey = xRoot->openKey(UNI_STRING(pUpdateKey));

/*      xTabStopsKey->createKey(UNI_STRING("blupp"));

        Sequence< ::rtl::OUString > sLanguages = xTabStopsKey->getStringListValue();
        sLanguages.realloc(sLanguages.getLength() + 1);
        sLanguages[sLanguages.getLength() - 1] = UNI_STRING("ru");
        xTabStopsKey->setStringListValue(sLanguages);
*/

        ::rtl::OUString sTest = xTabStopsKey->getStringValue();
        xTabStopsKey->setStringValue(UNI_STRING("blimp"));

        printf("flushing the changes\n\r");
        Reference< XFlushable > xCommit(xConfigurationRegistry, UNO_QUERY);
        xCommit->flush();
    }
    catch(RuntimeException& e)
    {
        printf("\n\r\n\rcaught an RuntimeException :\n\r");
        printf("    exception message : %s\n\r", ASCII_STRING(e.Message));
        return;
    }
    catch(Exception& e)
    {
        printf("\n\r\n\rcaught an Exception :\n\r");
        printf("    exception message : %s\n\r", ASCII_STRING(e.Message));
        return;
    }
}
