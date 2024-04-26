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

#include <sal/types.h>
#include <rtl/locale.h>
#include <rtl/ustring.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace rtl_locale
{
    // default locale for test purpose
    static void setDefaultLocale()
    {
        rtl_locale_setDefault(u"de", u"DE", /* OUString() */ u"hochdeutsch" );
    }

class getDefault : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp() override
    {
        // start message
        rtl_locale::setDefaultLocale();
    }

    void getDefault_001()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        CPPUNIT_ASSERT_MESSAGE("locale must not null", pData != nullptr);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(getDefault);
    CPPUNIT_TEST(getDefault_001);
    CPPUNIT_TEST_SUITE_END();
}; // class getDefault

class setDefault : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp() override
    {
        // start message
        rtl_locale::setDefaultLocale();
    }

    void tearDown() override
    {
        setDefaultLocale();
    }

    // insert your test code here.
    void setDefault_001()
    {
        rtl_locale_setDefault(u"en", u"US", u"");
        rtl_Locale* pData = rtl_locale_getDefault();
        CPPUNIT_ASSERT_MESSAGE("locale must not null", pData != nullptr);

        // be sure to not GPF
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(setDefault);
    CPPUNIT_TEST(setDefault_001);
    CPPUNIT_TEST_SUITE_END();
}; // class setDefault

class getLanguage : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp() override
    {
        // start message
        rtl_locale::setDefaultLocale();
    }

    // insert your test code here.
    void getLanguage_001()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        OUString suLanguage = pData->Language;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "locale language must be 'de'", u"de"_ustr, suLanguage );
    }
    void getLanguage_002()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        OUString suLanguage(rtl_locale_getLanguage(pData), SAL_NO_ACQUIRE);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "locale language must be 'de'", u"de"_ustr, suLanguage );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(getLanguage);
    CPPUNIT_TEST(getLanguage_001);
    CPPUNIT_TEST(getLanguage_002);
    CPPUNIT_TEST_SUITE_END();
}; // class getLanguage

class getCountry : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp() override
    {
        // start message
        rtl_locale::setDefaultLocale();
    }

    // insert your test code here.
    void getCountry_001()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        OUString suCountry = pData->Country;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "locale country must be 'DE'", u"DE"_ustr, suCountry );
    }
    void getCountry_002()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        OUString suCountry(rtl_locale_getCountry(pData), SAL_NO_ACQUIRE);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "locale country must be 'DE'", u"DE"_ustr, suCountry );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(getCountry);
    CPPUNIT_TEST(getCountry_001);
    CPPUNIT_TEST(getCountry_002);
    CPPUNIT_TEST_SUITE_END();
}; // class getCountry

class getVariant : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp() override
    {
        // start message
        rtl_locale::setDefaultLocale();
    }

    // insert your test code here.
    void getVariant_001()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        OUString suVariant = pData->Variant;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "locale variant must be 'hochdeutsch'", u"hochdeutsch"_ustr, suVariant );
    }
    void getVariant_002()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        OUString suVariant(rtl_locale_getVariant(pData), SAL_NO_ACQUIRE);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "locale variant must be 'hochdeutsch'", u"hochdeutsch"_ustr, suVariant );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(getVariant);
    CPPUNIT_TEST(getVariant_001);
    CPPUNIT_TEST(getVariant_002);
    CPPUNIT_TEST_SUITE_END();
}; // class getVariant

class hashCode : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp() override
    {
        // start message
        rtl_locale::setDefaultLocale();
    }

    // insert your test code here.
    void hashCode_001()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        sal_Int32 nHashCode = pData->HashCode;
        CPPUNIT_ASSERT_MESSAGE("locale hashcode must be 3831", nHashCode != 0);
    }
    void hashCode_002()
    {
        rtl_Locale* pData = rtl_locale_getDefault();
        sal_Int32 nHashCode = rtl_locale_hashCode(pData);
        CPPUNIT_ASSERT_MESSAGE("locale hashcode must be 3831", nHashCode != 0);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(hashCode);
    CPPUNIT_TEST(hashCode_001);
    CPPUNIT_TEST(hashCode_002);
    CPPUNIT_TEST_SUITE_END();
}; // class hashCode

class equals : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp() override
    {
        // start message
        rtl_locale::setDefaultLocale();
    }

    // insert your test code here.
    void equals_001()
    {
        rtl_Locale* pData1 = rtl_locale_register(u"en", u"US", u"");
        rtl_Locale* pData2 = rtl_locale_register(u"en", u"US", u"");

        bool bLocaleAreEqual = (pData1 == pData2);

        CPPUNIT_ASSERT_MESSAGE("check operator ==()", bLocaleAreEqual);
    }

    void equals_002()
    {
        rtl_Locale* pData1 = rtl_locale_register(u"en", u"US", u"");
        rtl_Locale* pData2 = rtl_locale_register(u"en", u"US", u"");

        sal_Int32 nEqual = rtl_locale_equals(pData1, pData2);
        CPPUNIT_ASSERT(nEqual != 0);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(equals);
    CPPUNIT_TEST(equals_001);
    CPPUNIT_TEST(equals_002);
    CPPUNIT_TEST_SUITE_END();
}; // class equals

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_locale::getDefault);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_locale::setDefault);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_locale::getLanguage);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_locale::getCountry);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_locale::getVariant);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_locale::hashCode);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_locale::equals);
} // namespace rtl_locale

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
