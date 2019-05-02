/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <resource/sharedresources.hxx>
#include <strings.hrc>

#include <utility>
#include <vector>

using namespace css;

namespace connectivity_test
{
#define TEST_SOURCE_STRING NC_("TEST_SOURCE_STRING", "UnitTest")
#define TEST_SOURCE_ONE_SUBSTITUTION NC_("TEST_SOURCE_ONE_SUBSTITUTION", "One substitution $sub$")
#define TEST_SOURCE_TWO_SUBSTITUTION                                                               \
    NC_("TEST_SOURCE_TWO_SUBSTITUTION", "Two substitution $sub0$ $sub1$")
#define TEST_SOURCE_THREE_SUBSTITUTION                                                             \
    NC_("TEST_SOURCE_THREE_SUBSTITUTION", "Three substitution $sub0$ $sub1$ $sub2$")

class SharedResourcesTest : public test::BootstrapFixture
{
public:
    SharedResourcesTest();

    void testGetSourceString();
    void testGetSourceStringWithSubstitutionOne();
    void testGetSourceStringWithSubstitutionTwo();
    void testGetSourceStringWithSubstitutionThree();
    void testGetSourceStringWithSubstitutionVector();

    CPPUNIT_TEST_SUITE(SharedResourcesTest);

    CPPUNIT_TEST(testGetSourceString);
    CPPUNIT_TEST(testGetSourceStringWithSubstitutionOne);
    CPPUNIT_TEST(testGetSourceStringWithSubstitutionTwo);
    CPPUNIT_TEST(testGetSourceStringWithSubstitutionThree);
    CPPUNIT_TEST(testGetSourceStringWithSubstitutionVector);

    CPPUNIT_TEST_SUITE_END();

private:
    ::connectivity::SharedResources m_aResource;
};

SharedResourcesTest::SharedResourcesTest()
    : test::BootstrapFixture(false, false)
{
}

void SharedResourcesTest::testGetSourceString()
{
    CPPUNIT_ASSERT_EQUAL(OUString("UnitTest"), m_aResource.getResourceString(TEST_SOURCE_STRING));
}

void SharedResourcesTest::testGetSourceStringWithSubstitutionOne()
{
    CPPUNIT_ASSERT_EQUAL(OUString("One substitution UnitTest"),
                         m_aResource.getResourceStringWithSubstitution(TEST_SOURCE_ONE_SUBSTITUTION,
                                                                       "$sub$", "UnitTest"));
}

void SharedResourcesTest::testGetSourceStringWithSubstitutionTwo()
{
    CPPUNIT_ASSERT_EQUAL(OUString("Two substitution UnitTest1 UnitTest2"),
                         m_aResource.getResourceStringWithSubstitution(TEST_SOURCE_TWO_SUBSTITUTION,
                                                                       "$sub0$", "UnitTest1",
                                                                       "$sub1$", "UnitTest2"));
}

void SharedResourcesTest::testGetSourceStringWithSubstitutionThree()
{
    CPPUNIT_ASSERT_EQUAL(OUString("Three substitution UnitTest1 UnitTest2 UnitTest3"),
                         m_aResource.getResourceStringWithSubstitution(
                             TEST_SOURCE_THREE_SUBSTITUTION, "$sub0$", "UnitTest1", "$sub1$",
                             "UnitTest2", "$sub2$", "UnitTest3"));
}

void SharedResourcesTest::testGetSourceStringWithSubstitutionVector()
{
    std::vector<std::pair<const sal_Char*, OUString>> aStringToSubstitutes;
    aStringToSubstitutes.push_back(std::pair<const sal_Char*, OUString>("$sub0$", "vector0"));
    aStringToSubstitutes.push_back(std::pair<const sal_Char*, OUString>("$sub1$", "vector1"));
    aStringToSubstitutes.push_back(std::pair<const sal_Char*, OUString>("$sub2$", "vector2"));

    CPPUNIT_ASSERT_EQUAL(OUString("Three substitution vector0 vector1 vector2"),
                         m_aResource.getResourceStringWithSubstitution(
                             TEST_SOURCE_THREE_SUBSTITUTION, aStringToSubstitutes));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SharedResourcesTest);

} // namespace connectivity_test

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
