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

#include <sal/main.h>
#include <cppuhelper/shlib.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppu/EnvDcp.hxx>

#include "../testcmp/TestComponent.hxx"

#include <iostream>
#include <cstring>

#ifndef SAL_DLLPREFIX
# define SAL_DLLPREFIX  ""
#endif


using namespace ::com::sun::star;


class MyKey : public cppu::WeakImplHelper<registry::XRegistryKey>

{
public:
    virtual rtl::OUString SAL_CALL getKeyName() throw (uno::RuntimeException) { return rtl::OUString(); };

    // Methods
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return sal_False;};
    virtual sal_Bool SAL_CALL isValid(  ) throw (uno::RuntimeException) {return sal_False;};
    virtual registry::RegistryKeyType SAL_CALL getKeyType( const rtl::OUString& /*rKeyName*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return registry::RegistryKeyType_KEY;};
    virtual registry::RegistryValueType SAL_CALL getValueType(  ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return registry::RegistryValueType_NOT_DEFINED;};
    virtual sal_Int32 SAL_CALL getLongValue(  ) throw (registry::InvalidRegistryException, registry::InvalidValueException, uno::RuntimeException) {return 0;};
    virtual void SAL_CALL setLongValue( sal_Int32 /*value*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual uno::Sequence< sal_Int32 > SAL_CALL getLongListValue(  ) throw (registry::InvalidRegistryException, registry::InvalidValueException, uno::RuntimeException) { return uno::Sequence<sal_Int32>(); };
    virtual void SAL_CALL setLongListValue( const uno::Sequence< sal_Int32 >& /*seqValue*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual rtl::OUString SAL_CALL getAsciiValue(  ) throw (registry::InvalidRegistryException, registry::InvalidValueException, uno::RuntimeException) {return rtl::OUString();};
    virtual void SAL_CALL setAsciiValue( const rtl::OUString& /*value*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual uno::Sequence< rtl::OUString > SAL_CALL getAsciiListValue(  ) throw (registry::InvalidRegistryException, registry::InvalidValueException, uno::RuntimeException) {return uno::Sequence<rtl::OUString>();};
    virtual void SAL_CALL setAsciiListValue( const uno::Sequence< rtl::OUString >& /*seqValue*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual rtl::OUString SAL_CALL getStringValue(  ) throw (registry::InvalidRegistryException, registry::InvalidValueException, uno::RuntimeException) {return rtl::OUString();};
    virtual void SAL_CALL setStringValue( const rtl::OUString& /*value*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual uno::Sequence< rtl::OUString > SAL_CALL getStringListValue(  ) throw (registry::InvalidRegistryException, registry::InvalidValueException, uno::RuntimeException) {return uno::Sequence<rtl::OUString>();};
    virtual void SAL_CALL setStringListValue( const uno::Sequence< rtl::OUString >& /*seqValue*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual uno::Sequence< sal_Int8 > SAL_CALL getBinaryValue(  ) throw (registry::InvalidRegistryException, registry::InvalidValueException, uno::RuntimeException) {return uno::Sequence<sal_Int8>();};
    virtual void SAL_CALL setBinaryValue( const uno::Sequence< sal_Int8 >& /*value*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual uno::Reference< registry::XRegistryKey > SAL_CALL openKey( const rtl::OUString& /*aKeyName*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return uno::Reference<registry::XRegistryKey>();};
    virtual uno::Reference< registry::XRegistryKey > SAL_CALL createKey( const rtl::OUString& /*aKeyName*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return uno::Reference<registry::XRegistryKey>();};
    virtual void SAL_CALL closeKey(  ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual void SAL_CALL deleteKey( const rtl::OUString& /*rKeyName*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual uno::Sequence< uno::Reference< registry::XRegistryKey > > SAL_CALL openKeys(  ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return uno::Sequence<uno::Reference<registry::XRegistryKey> >();};
    virtual uno::Sequence< rtl::OUString > SAL_CALL getKeyNames(  ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return uno::Sequence<rtl::OUString>();};
    virtual sal_Bool SAL_CALL createLink( const rtl::OUString& /*aLinkName*/, const rtl::OUString& /*aLinkTarget*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return sal_False;};
    virtual void SAL_CALL deleteLink( const rtl::OUString& /*rLinkName*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {};
    virtual rtl::OUString SAL_CALL getLinkTarget( const rtl::OUString& /*rLinkName*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return rtl::OUString();};
    virtual rtl::OUString SAL_CALL getResolvedName( const rtl::OUString& /*aKeyName*/ ) throw (registry::InvalidRegistryException, uno::RuntimeException) {return rtl::OUString();};
};



static rtl::OUString s_test__cppu_loadSharedLibComponentFactory(char const * pServicePurpose)
{
    rtl::OUString result;

    rtl::OUString servicePurpose = rtl::OUString(pServicePurpose,
                                                 rtl_str_getLength(pServicePurpose),
                                                 RTL_TEXTENCODING_ASCII_US);

    result += rtl::OUString("\ts_test__cppu_loadSharedLibComponentFactory ");
    result += rtl::OUString("(\"");
    result += servicePurpose;
    result += rtl::OUString("\") - ");

    try {
        uno::Reference<uno::XInterface> xObject(
            cppu::loadSharedLibComponentFactory(
                rtl::OUString(SAL_DLLPREFIX "TestComponent.uno" SAL_DLLEXTENSION),
                rtl::OUString(),
                rtl::OUString("impl.test.TestComponent") + servicePurpose,
                uno::Reference<lang::XMultiServiceFactory>(),
                uno::Reference<registry::XRegistryKey>())
            );

        rtl::OUString envDcp_purpose(cppu::EnvDcp::getPurpose(g_envDcp));
        if (envDcp_purpose == servicePurpose)
            result += rtl::OUString("passed\n");

        else
        {
            result += rtl::OUString("FAILED - got: \"");
            result += envDcp_purpose;
            result += rtl::OUString("\"\n");
        }
    }
    catch(uno::Exception & exception) {
        result += rtl::OUString("FAILED - got: \"");
        result += exception.Message;
        result += rtl::OUString("\"\n");
    }

    return result;
}

static rtl::OUString s_test__cppu_writeSharedLibComponentInfo(char const * pServicePurpose)
{
    rtl::OUString result;

    rtl::OUString servicePurpose = rtl::OUString(pServicePurpose,
                                                 rtl_str_getLength(pServicePurpose),
                                                 RTL_TEXTENCODING_ASCII_US);

    result += rtl::OUString("\ts_test__cppu_writeSharedLibComponentInfo ");
    result += rtl::OUString("(\"");
    result += servicePurpose;
    result += rtl::OUString("\") - ");

    char buff[256];
    strcpy(buff, "TestComponent.uno=");
    strcat(buff, pServicePurpose);

    putenv(buff);

    try {
        cppu::writeSharedLibComponentInfo(
            rtl::OUString(SAL_DLLPREFIX "TestComponent.uno" SAL_DLLEXTENSION),
            rtl::OUString(),
            uno::Reference<lang::XMultiServiceFactory>(),
            uno::Reference<registry::XRegistryKey>(new MyKey)
            );

        rtl::OUString envDcp_purpose(cppu::EnvDcp::getPurpose(g_envDcp));
        if (envDcp_purpose == servicePurpose)
            result += rtl::OUString("passed\n");

        else
        {
            result += rtl::OUString("FAILED - got: \"");
            result += envDcp_purpose;
            result += rtl::OUString("\"\n");
        }
    }
    catch(uno::Exception & exception) {
        result += rtl::OUString("FAILED - got: \"");
        result += exception.Message;
        result += rtl::OUString("\"\n");
    }

    return result;
}


SAL_IMPLEMENT_MAIN_WITH_ARGS(/*argc*/, argv)
{
    int result = 0;

    rtl::OUString message;

    message += rtl::OUString(argv[0], rtl_str_getLength(argv[0]), RTL_TEXTENCODING_ASCII_US);
    message += rtl::OUString("\n");

    message += s_test__cppu_loadSharedLibComponentFactory(":unsafe");
    message += s_test__cppu_loadSharedLibComponentFactory(":affine");
    message += s_test__cppu_loadSharedLibComponentFactory("");

    message += s_test__cppu_writeSharedLibComponentInfo(":unsafe");
    message += s_test__cppu_writeSharedLibComponentInfo(":affine");
    message += s_test__cppu_writeSharedLibComponentInfo("");

    if (message.indexOf("FAILED") == -1)
        message += rtl::OUString("TESTS PASSED\n");

    else
    {
        message += rtl::OUString("TESTS _NOT_ PASSED\n");
        result = -1;
    }

    std::cout << rtl::OUStringToOString(message, RTL_TEXTENCODING_ASCII_US).getStr();

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
