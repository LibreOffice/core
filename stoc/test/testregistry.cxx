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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sal/main.h>
#include <osl/module.hxx>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <registry/registry.hxx>


#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#if defined ( UNX )
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

namespace stoc_impreg
{
void SAL_CALL mergeKeys(
Reference< registry::XRegistryKey > const & xDest,
Reference< registry::XRegistryKey > const & xSource )
SAL_THROW( (registry::InvalidRegistryException, registry::MergeConflictException) );
}
static void mergeKeys(
Reference< registry::XSimpleRegistry > const & xDest,
OUString const & rBaseNode,
OUString const & rURL )
SAL_THROW( (registry::InvalidRegistryException, registry::MergeConflictException) )
{
Reference< registry::XRegistryKey > xDestRoot( xDest->getRootKey() );
Reference< registry::XRegistryKey > xDestKey;
if (rBaseNode.getLength())
{
xDestKey = xDestRoot->createKey( rBaseNode );
xDestRoot->closeKey();
}
else
{
xDestKey = xDestRoot;
}
Reference< registry::XSimpleRegistry > xSimReg( ::cppu::createSimpleRegistry() );
xSimReg->open( rURL, sal_True, sal_False );
OSL_ASSERT( xSimReg->isValid() );
Reference< registry::XRegistryKey > xSourceKey( xSimReg->getRootKey() );
::stoc_impreg::mergeKeys( xDestKey, xSourceKey );
xSourceKey->closeKey();
xSimReg->close();
xDestKey->closeKey();
}


OString userRegEnv("STAR_USER_REGISTRY=");

OUString getExePath()
{
OUString        exe;
OSL_VERIFY( osl_getExecutableFile( &exe.pData ) == osl_Process_E_None);
#if defined(WIN32) || defined(WNT)
exe = exe.copy(0, exe.getLength() - 16);
#else
exe = exe.copy(0, exe.getLength() - 12);
#endif
return exe;
}

void setStarUserRegistry()
{
Registry *myRegistry = new Registry();

RegistryKey rootKey, rKey, rKey2;

OUString userReg = getExePath();
userReg += "user.rdb";
if(myRegistry->open(userReg, REG_READWRITE))
{
OSL_VERIFY(!myRegistry->create(userReg));
}

OSL_VERIFY(!myRegistry->close());
delete myRegistry;

userRegEnv += OUStringToOString(userReg, RTL_TEXTENCODING_ASCII_US);
    putenv((char *)userRegEnv.getStr());
}

void setLinkInDefaultRegistry(const OUString& linkName, const OUString& linkTarget)
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey;

    OUString appReg = getExePath();
    appReg += "stoctest.rdb";

    OSL_VERIFY(!myRegistry->open(appReg, REG_READWRITE));
    OSL_VERIFY(!myRegistry->openRootKey(rootKey));

    OSL_VERIFY(!rootKey.createLink(linkName, linkTarget));

    OSL_VERIFY(!rootKey.closeKey());
    OSL_VERIFY(!myRegistry->close());

    delete myRegistry;
}


void test_SimpleRegistry(
    OUString const & testreg,
    OUString const & testreg2,
    bool bMergeDifferently = true )
{
    Reference<XInterface> xIFace;
    Module module;

    OUString dllName(
        "simplereg.uno" SAL_DLLEXTENSION );

    if (module.load(dllName))
    {
        // try to get provider from module
        component_getFactoryFunc pCompFactoryFunc = (component_getFactoryFunc)
            module.getFunctionSymbol( OUString(COMPONENT_GETFACTORY) );

        if (pCompFactoryFunc)
        {
            XSingleServiceFactory * pRet = (XSingleServiceFactory *)
                (*pCompFactoryFunc)(
                    "com.sun.star.comp.stoc.SimpleRegistry", 0, 0 );
            if (pRet)
            {
                xIFace = pRet;
                pRet->release();
            }
        }
    }

    OSL_ENSURE( xIFace.is(), "test_SimpleRegistry error1");

    Reference<XSingleServiceFactory> xFactory( Reference<XSingleServiceFactory>::query(xIFace) );
     xIFace.clear();

    OSL_ENSURE( xFactory.is(), "testloader error11");

    Reference<XInterface> xIFace2 = xFactory->createInstance();
    xFactory.clear();

    OSL_ENSURE( xIFace2.is(), "testloader error12");

    Reference<XServiceInfo> xServInfo( Reference<XServiceInfo>::query(xIFace2) );

    OSL_ENSURE( xServInfo.is(), "test_SimpleRegistry error2");

    OSL_ENSURE( xServInfo->getImplementationName() == "com.sun.star.comp.stoc.SimpleRegistry", "test_SimpleRegistry error3");
    OSL_ENSURE( xServInfo->supportsService(OUString( "com.sun.star.registry.SimpleRegistry")), "test_SimpleRegistry error4");
    OSL_ENSURE( xServInfo->getSupportedServiceNames().getLength() == 1, "test_SimpleRegistry error5");
    xServInfo.clear();

    Reference<XSimpleRegistry> xReg( Reference<XSimpleRegistry>::query(xIFace2) );
    xIFace2.clear();

    OSL_ENSURE( xReg.is(), "test_SimpleRegistry error6");

    try
    {
        xReg->open(testreg, sal_False, sal_True);

        OSL_ENSURE( xReg->isValid() != sal_False, "test_SimpleRegistry error 7" );
        OSL_ENSURE( xReg->isReadOnly() == sal_False, "test_SimpleRegistry error 8" );

        Reference<XRegistryKey> xRootKey(xReg->getRootKey());
        OSL_ENSURE( xRootKey->isValid(), "test_SimpleRegistry error 9" );

        Reference<XRegistryKey> xKey = xRootKey->createKey(OUString( "FirstKey" ));

        Reference<XRegistryKey> xSubKey = xKey->createKey(OUString( "FirstSubKey" ));
        xSubKey->setLongValue(123456789);

        xSubKey = xKey->createKey(OUString( "SecondSubKey" ));
        xSubKey->setAsciiValue(OUString( "ich bin ein acsii value" ));

        xSubKey = xKey->createKey(OUString( "ThirdSubKey" ));
        xSubKey->setStringValue(OUString( "ich bin ein unicode value" ));

        xSubKey = xKey->createKey(OUString( "FourthSubKey" ));
        Sequence<sal_Int8> aSeq((sal_Int8*)"ich bin ein binary value", 25);
        xSubKey->setBinaryValue(aSeq);

        Sequence<OUString> seqNames = xKey->getKeyNames();
        Sequence< Reference<XRegistryKey> > seqKeys = xKey->openKeys();

        OUString name;
        for (sal_Int32 i=0; i < seqNames.getLength(); i++)
        {
            name = seqNames.getArray()[i];
            xSubKey = seqKeys.getArray()[i];

            if (name == OUString( "/FirstKey/FirstSubKey" ))
            {
                OSL_ENSURE( xSubKey->getLongValue() == 123456789,
                            "test_SimpleRegistry error 10" );
            } else
            if (name == OUString( "/FirstKey/SecondSubKey" ))
            {
                OSL_ENSURE( xSubKey->getAsciiValue() == OUString( "ich bin ein acsii value" ),
                            "test_SimpleRegistry error 11" );
            } else
            if (name == OUString( "/FirstKey/ThirdSubKey" ))
            {
                OSL_ENSURE( xSubKey->getStringValue() == OUString( "ich bin ein unicode value" ),
                            "test_SimpleRegistry error 12" );
            } else
            if (name == OUString( "/FirstKey/FourthSubKey" ))
            {
                Sequence<sal_Int8> seqByte = xSubKey->getBinaryValue();
                OSL_ENSURE(!strcmp(((const char*)seqByte.getArray()), "ich bin ein binary value"),
                            "test_SimpleRegistry error 13" );
            }

            seqKeys.getArray()[i]->closeKey();
        }

        xKey->closeKey();

        xRootKey->deleteKey(OUString( "FirstKey" ));
        xRootKey->createKey(OUString( "SecondFirstKey" ));

        xKey = xRootKey->createKey(OUString( "SecondKey" ));
        sal_Int32 pLongs[3] = {123, 456, 789};
        Sequence<sal_Int32> seqLongs(pLongs, 3);
        xKey->setLongListValue(seqLongs);

        Sequence<sal_Int32> seqLongs2;
        seqLongs2 = xKey->getLongListValue();
        OSL_ENSURE( seqLongs.getLength() == 3, "test_SimpleRegistry error 14" );
        OSL_ENSURE( seqLongs.getArray()[0] == 123, "test_SimpleRegistry error 15" );
        OSL_ENSURE( seqLongs.getArray()[1] == 456, "test_SimpleRegistry error 16" );
        OSL_ENSURE( seqLongs.getArray()[2] == 789, "test_SimpleRegistry error 17" );


        xKey = xRootKey->createKey(OUString( "ThirdKey" ));
        OUString pAscii[3];
        pAscii[0] = OUString( "Hallo" );
        pAscii[1] = OUString( "jetzt komm" );
        pAscii[2] = OUString( "ich" );

        Sequence<OUString> seqAscii(pAscii, 3);
        xKey->setAsciiListValue(seqAscii);

        Sequence<OUString> seqAscii2;
        seqAscii2 = xKey->getAsciiListValue();
        OSL_ENSURE( seqAscii2.getLength() == 3, "test_SimpleRegistry error 18" );
        OSL_ENSURE( seqAscii2.getArray()[0] == OUString( "Hallo" ), "test_SimpleRegistry error 19");
        OSL_ENSURE( seqAscii2.getArray()[1] == OUString( "jetzt komm" ), "test_SimpleRegistry error 20");
        OSL_ENSURE( seqAscii2.getArray()[2] == OUString( "ich" ), "test_SimpleRegistry error 21");

        xKey = xRootKey->createKey(OUString( "FourthKey" ));
        OUString pUnicode[3];
        pUnicode[0] = OUString( "Hallo" );
        pUnicode[1] = OUString( "jetzt komm" );
        pUnicode[2] = OUString( "ich als unicode" );

        Sequence<OUString> seqUnicode(pUnicode, 3);
        xKey->setStringListValue(seqUnicode);

        Sequence<OUString> seqUnicode2;
        seqUnicode2 = xKey->getStringListValue();
        OSL_ENSURE( seqUnicode2.getLength() == 3, "test_SimpleRegistry error 22" );
        OSL_ENSURE( seqUnicode2.getArray()[0] == OUString( "Hallo" ), "test_SimpleRegistry error 23");
        OSL_ENSURE( seqUnicode2.getArray()[1] == OUString( "jetzt komm" ), "test_SimpleRegistry error 24");
        OSL_ENSURE( seqUnicode2.getArray()[2] == OUString( "ich als unicode" ), "test_SimpleRegistry error 25");


        xReg->open(testreg2, sal_False, sal_True);
        OSL_ENSURE( xReg->isValid() != sal_False, "test_SimpleRegistry error 25" );
        xRootKey = xReg->getRootKey();
        xKey = xRootKey->createKey(OUString( "ThirdKey/FirstSubKey/WithSubSubKey" ));
        xKey->closeKey();
        OSL_VERIFY(
            xRootKey->createLink(
                OUString( "LinkTest" ),
                OUString( "/ThirdKey/FirstSubKey/WithSubSubKey" )) );
        xRootKey->closeKey();
        xReg->close();

        xReg->open(testreg, sal_False, sal_False);
        OSL_ENSURE( xReg->isValid() != sal_False, "test_SimpleRegistry error 26" );

        if (bMergeDifferently)
        {
            mergeKeys(
                xReg,
                OUString(),
                testreg2 );
        }
        else
        {
            xReg->mergeKey(OUString(), testreg2);
        }

        xRootKey = xReg->getRootKey();
        xKey = xRootKey->openKey( OUString( "LinkTest" ) );
        OSL_ENSURE( xKey.is() && xKey->isValid() && xKey->getKeyName() == "/ThirdKey/FirstSubKey/WithSubSubKey", "test_SimpleRegistry error 1213" );
        xKey->closeKey();
        OSL_ENSURE(
            xRootKey->getKeyType( OUString( "LinkTest" ) ) ==
            registry::RegistryKeyType_LINK,
            "test_SimpleRegistry error 1214" );

        xKey = xRootKey->openKey(OUString( "FirstKey/SecondSubKey" ));
        OSL_ENSURE( !xKey.is(), "test_SimpleRegistry error 27" );

        // Test Links
        xKey = xRootKey->createKey(OUString( "FifthKey" ));
        xKey->createLink(OUString( "MyFirstLink" ),
                         OUString( "/ThirdKey/FirstSubKey" ));

        xKey = xRootKey->openKey(OUString( "/FifthKey/MyFirstLink" ));
        OSL_ENSURE( xKey->isValid(), "test_SimpleRegistry error 27" );
        OSL_ENSURE( xKey->getKeyName() == OUString( "/ThirdKey/FirstSubKey" ), "test_SimpleRegistry error 28" );

        xKey->createLink(OUString( "/WithSubSubKey/MyFourthLink" ),
                         OUString( "/FourthKey/MySecondLink" ));

        OSL_ENSURE( xKey->getLinkTarget(OUString( "/WithSubSubKey/MyFourthLink" ))
                     == OUString( "/FourthKey/MySecondLink" ), "test_SimpleRegistry error 29" );

        try
        {
            OSL_ENSURE( xKey->getResolvedName(OUString( "/WithSubSubKey/MyFourthLink/BlaBlaBla" ))
                         == OUString( "/FourthKey/MySecondLink/BlaBlaBla" ), "test_SimpleRegistry error 30" );
        }
        catch(InvalidRegistryException&)
        {
        }

        xRootKey->createLink(OUString( "/FourthKey/MySecondLink" ),
                             OUString( "/SixthKey/MyThirdLink" ));
        xKey = xRootKey->createKey(OUString( "SixthKey" ));
        xKey->createLink(OUString( "MyThirdLink" ),
                         OUString( "/FourthKey/MySecondLink" ));

        xKey = xRootKey->createKey(OUString( "/SixthKey/SixthSubKey" ));

        try
        {
            xRootKey->openKey(OUString( "/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink" ));
        }
        catch(InvalidRegistryException&)
        {
        }

        OSL_ENSURE( xRootKey->getLinkTarget(OUString( "/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink" ))
                     == OUString( "/FourthKey/MySecondLink" ), "test_SimpleRegistry error 31" );

        xRootKey->deleteLink(OUString( "/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink" ));

        xRootKey->createLink(OUString( "/FourthKey/MySecondLink" ),
                             OUString( "/ThirdKey/FirstSubKey/WithSubSubKey" ));

        xKey = xRootKey->openKey(OUString( "SixthKey" ));
        seqNames = xKey->getKeyNames();
        seqKeys = xKey->openKeys();

        OSL_ENSURE( seqNames.getArray()[0] == OUString( "/SixthKey/SixthSubKey" ),
                      "test_SimpleRegistry error 32" );
        OSL_ENSURE( seqNames.getArray()[1] == OUString( "/SixthKey/MyThirdLink" ),
                      "test_SimpleRegistry error 33" );

        OSL_ENSURE( seqKeys.getArray()[0]->getKeyName() == OUString( "/SixthKey/SixthSubKey" ),
                      "test_SimpleRegistry error 34" );
        OSL_ENSURE( seqKeys.getArray()[1]->getKeyName() == OUString( "/ThirdKey/FirstSubKey/WithSubSubKey" ),
                      "test_SimpleRegistry error 35" );

        xRootKey->deleteLink(OUString( "/FourthKey/MySecondLink" ));
        xRootKey->closeKey();
    }
    catch(InvalidRegistryException&)
    {
        OSL_ENSURE(0, "exception InvalidRegistryExcption raised while doing test_SimpleRegistry");
    }
    catch(InvalidValueException&)
    {
        OSL_ENSURE(0, "exception InvalidValueExcption raised while doing test_SimpleRegistry()");
    }

    xReg.clear();

    printf("Test SimpleRegistry, OK!\n");
}


void test_DefaultRegistry(
    OUString const & testreg,
    OUString const & testreg2,
    bool bMergeDifferently = false )
{
    // Test NestedRegistry
    OUString exePath( getExePath() );
    OUString userRdb(exePath);
    OUString applicatRdb(exePath);

    userRdb += "user.rdb";
    applicatRdb += "stoctest.rdb";

    Reference < XMultiServiceFactory > rSMgr  = ::cppu::createRegistryServiceFactory( userRdb, applicatRdb, sal_False, OUString());
                                                                                      //OUString("//./e:/src596/stoc/wntmsci3/bin") );

    Reference< XPropertySet > xPropSet( rSMgr, UNO_QUERY);
    OSL_ENSURE( xPropSet.is(), "test_DefaultRegistry error0");

    Any aPropertyAny( xPropSet->getPropertyValue( OUString("Registry") ) );
    OSL_ENSURE( aPropertyAny.hasValue(), "test_DefaultRegistry error1");

    Reference<XSimpleRegistry> xReg;
    aPropertyAny >>= xReg;
    OSL_ENSURE( xReg.is(), "test_DefaultRegistry error1a");

    Reference<XServiceInfo> xServInfo( Reference<XServiceInfo>::query(xReg) );

    OSL_ENSURE( xServInfo.is(), "test_DefaultRegistry error2");

    OSL_ENSURE( xServInfo->getImplementationName() == OUString( "com.sun.star.comp.stoc.NestedRegistry" ), "test_DefualtRegistry error3");
    OSL_ENSURE( xServInfo->supportsService(OUString( "com.sun.star.registry.NestedRegistry" )), "test_DefaultRegistry error4");
    OSL_ENSURE( xServInfo->getSupportedServiceNames().getLength() == 1, "test_DefaultRegistry error5");
    xServInfo.clear();

    OSL_ENSURE( xReg.is(), "test_DefaultRegistry error6");

    try
    {
        Reference<XRegistryKey> xRootKey(xReg->getRootKey());

        Reference<XRegistryKey> xKey = xRootKey->openKey(OUString( "/UCR/com/sun/star/registry/XSimpleRegistry" ));

        OSL_ENSURE( xKey->getKeyName() == OUString( "/UCR/com/sun/star/registry/XSimpleRegistry" ),
                     "test_DefaultRegistry error 7" );

        if (bMergeDifferently)
        {
            mergeKeys(
                xReg,
                OUString( "Test" ),
                testreg );
        }
        else
        {
            xReg->mergeKey(OUString( "Test" ), testreg );
        }

        xKey = xRootKey->openKey(OUString( "Test/ThirdKey/FirstSubKey/WithSubSubKey" ));
        if (xKey.is())
            xKey->setLongValue(123456789);

        xKey = xRootKey->openKey(OUString( "Test/ThirdKey/FirstSubKey" ));
        if (xKey.is())
        {
            xKey->createKey(OUString( "SecondSubSubKey" ));

            Sequence<OUString> seqNames = xKey->getKeyNames();

            OSL_ENSURE( seqNames.getLength() == 2, "test_DefaultRegistry error 8" );
        }

        xKey = xRootKey->openKey(OUString( "/Test/ThirdKey" ));
        if (xKey.is())
        {
            RegistryValueType valueType = xKey->getValueType();
            OSL_ENSURE( valueType == RegistryValueType_ASCIILIST, "test_DefaultRegistry error 9" );

            Sequence<OUString> seqValue = xKey->getAsciiListValue();

            OSL_ENSURE( seqValue.getLength() == 3, "test_DefaultRegistry error 10" );
            OSL_ENSURE( seqValue.getArray()[0] == OUString( "Hallo" ),
                          "test_DefaultRegistry error 11" );
            OSL_ENSURE( seqValue.getArray()[1] == OUString( "jetzt komm" ),
                          "test_DefaultRegistry error 12" );
            OSL_ENSURE( seqValue.getArray()[2] == OUString( "ich" ),
                          "test_DefaultRegistry error 13" );

            Sequence<sal_Int32> seqLong(3);
            seqLong.getArray()[0] = 1234;
            seqLong.getArray()[1] = 4567;
            seqLong.getArray()[2] = 7890;

            xKey->setLongListValue(seqLong);

            Sequence<sal_Int32> seqLongValue = xKey->getLongListValue();

            OSL_ENSURE( seqLongValue.getLength() == 3, "test_DefaultRegistry error 14" );
            OSL_ENSURE( seqLongValue.getArray()[0] == 1234, "test_DefaultRegistry error 15" );
            OSL_ENSURE( seqLongValue.getArray()[1] == 4567, "test_DefaultRegistry error 16" );
            OSL_ENSURE( seqLongValue.getArray()[2] == 7890, "test_DefaultRegistry error 17" );
        }

        // Test Links
        xKey = xRootKey->createKey(OUString( "/Test/FifthKey" ));
        xKey->createLink(OUString( "MyFirstLink" ),
                         OUString( "/Test/ThirdKey/FirstSubKey" ));

        xKey = xRootKey->openKey(OUString( "/Test/FifthKey/MyFirstLink" ));
        OSL_ENSURE( xKey->isValid(), "test_DefaultRegistry error 18" );
        OSL_ENSURE( xKey->getKeyName() == OUString( "/Test/ThirdKey/FirstSubKey" ),
                      "test_DefaultRegistry error 19" );

        xKey->createLink(OUString( "/WithSubSubKey/MyFourthLink" ),
                         OUString( "/Test/FourthKey/MySecondLink" ));

        OSL_ENSURE( xKey->getLinkTarget(OUString( "/WithSubSubKey/MyFourthLink" ))
                     == OUString( "/Test/FourthKey/MySecondLink" ),
                      "test_DefaultRegistry error 20" );

        try
        {
            OSL_ENSURE( xKey->getResolvedName(OUString( "/WithSubSubKey/MyFourthLink/BlaBlaBla" ))
                          == OUString( "/Test/FourthKey/MySecondLink/BlaBlaBla" ),
                          "test_DefaultRegistry error 21" );
        }
        catch(InvalidRegistryException&)
        {
        }

        xRootKey->createLink(OUString( "/Test/FourthKey/MySecondLink" ),
                             OUString( "/Test/SixthKey/MyThirdLink" ));
        xKey = xRootKey->createKey(OUString( "/Test/SixthKey" ));
        xKey->createLink(OUString( "MyThirdLink" ),
                         OUString( "/Test/FourthKey/MySecondLink" ));

        try
        {
            xRootKey->openKey(OUString( "/Test/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink" ));
        }
        catch(InvalidRegistryException&)
        {
            printf("test InvalidRegistryExcption OK!\n");
        }

        OSL_ENSURE( xRootKey->getLinkTarget(OUString( "/Test/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink" ))
                     == OUString( "/Test/FourthKey/MySecondLink" ),
                      "test_DefaultRegistry error 22" );

        xRootKey->deleteLink(OUString( "/Test/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink" ));

        xKey = xRootKey->openKey(OUString( "/Test/DefaultLink/SecondSubSubKey" ));
        if (xKey.is())
        {
            OSL_ENSURE( xKey->getKeyName() == OUString( "/Test/ThirdKey/FirstSubKey/SecondSubSubKey" ), "test_DefaultRegistry error 23" );
        }
        xKey = xRootKey->createKey(OUString( "/Test/DefaultLink/ThirdSubSubKey" ));
        if (xKey.is())
        {
            OSL_ENSURE( xKey->getKeyName() == OUString( "/Test/ThirdKey/FirstSubKey/ThirdSubSubKey" ),
                          "test_DefaultRegistry error 24" );
        }

        xKey = xRootKey->openKey(OUString( "Test" ));
        OSL_ENSURE( xKey->isValid(), "test_DefaultRegistry error 25" );

        xRootKey->deleteKey(OUString( "Test" ));

        if (bMergeDifferently)
        {
            mergeKeys(
                xReg,
                OUString( "AllFromTestreg2" ),
                testreg2);
        }
        else
        {
            xReg->mergeKey(OUString( "AllFromTestreg2" ),
                           testreg2);
        }

        xKey = xRootKey->openKey(OUString( "/AllFromTestreg2/ThirdKey/FirstSubKey" ));
        if (xKey.is())
        {
            xRootKey->deleteKey(OUString( "/AllFromTestreg2" ));
        }

    }
    catch(InvalidRegistryException&)
    {
        OSL_ENSURE(0, "exception InvalidRegistryExcption raised while doing test_DefaultRegistry");
    }
    catch(InvalidValueException&)
    {
        OSL_ENSURE(0, "exception InvalidValueExcption raised while doing test_DefaultRegistry()");
    }
    try
    {
        xReg->close();
    }
    catch(const InvalidRegistryException& e)
    {
        (void)e;
        OSL_ENSURE(0, OUStringToOString(e.Message,RTL_TEXTENCODING_ASCII_US).getStr());
    }


    xReg.clear();

    // shutdown
    Reference< ::com::sun::star::lang::XComponent > xComp( rSMgr, UNO_QUERY );
    OSL_ENSURE( xComp.is(), "### serivce manager has to implement XComponent!" );
    xComp->dispose();

    printf("Test DefaultRegistry, OK!\n");
}


SAL_IMPLEMENT_MAIN()
{
//  setStarUserRegistry();
     setLinkInDefaultRegistry(OUString("/Test/DefaultLink"),
                              OUString("/Test/FifthKey/MyFirstLink"));

    OUString reg1( "testreg1.rdb" );
    OUString reg2( "testreg2.rdb" );
    OUString areg1( "atestreg1.rdb" );
    OUString areg2( "atestreg2.rdb" );

      test_SimpleRegistry( reg1, reg2 );
      test_DefaultRegistry( reg1, reg2 );
      test_SimpleRegistry( areg1, areg2, true ); // use different merge
      test_DefaultRegistry( areg1, areg2, true );

    Reference< XSimpleRegistry > xSimReg( ::cppu::createSimpleRegistry() );
    xSimReg->open( reg1, sal_False, sal_True );
    xSimReg->destroy();
    xSimReg->open( reg2, sal_False, sal_True );
    xSimReg->destroy();
    xSimReg->open( areg1, sal_False, sal_True );
    xSimReg->destroy();
    xSimReg->open( areg2, sal_False, sal_True );
    xSimReg->destroy();
    return(0);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
