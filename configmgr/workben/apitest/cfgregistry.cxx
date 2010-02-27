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
#include "precompiled_configmgr.hxx"
#include <memory.h>
#include <stdio.h>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#include <vos/conditn.hxx>
#include <osl/diagnose.h>

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
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bAsserted = sal_False;
#endif
    for (sal_Int32 i=0; i<nLen; ++i, ++pFillPtr, ++pSourcePtr)
    {
        OSL_ENSURE(bAsserted || !(bAsserted = (*pSourcePtr >= 0x80)),
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
