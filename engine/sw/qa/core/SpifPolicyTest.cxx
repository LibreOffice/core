/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#include <rtl/ustring.hxx>

#include <SpifPolicy.hxx>

#include <tools/stream.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class SpifPolicyTest : public CppUnit::TestFixture
{
    void testParse();

    CPPUNIT_TEST_SUITE(SpifPolicyTest);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST_SUITE_END();
};

void SpifPolicyTest::testParse()
{
    // Mirrors sixworks/spif-collabora.xml; the tag-set block must be skipped.
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">
  <spif:securityPolicyId name="SPIF Collabora" id="1.2.826.0.1310.1.2.0" />
  <spif:securityClassifications>
    <spif:securityClassification name="OFFICIAL" color="yellow" lacv="3" hierarchy="3" obsolete="false" />
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4" obsolete="false" />
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="Release Categories" id="1.2.826.0.1310.1.2.0.0">
      <spif:securityCategoryTag name="Releasable To" tagType="enumerated" enumType="permissive" singleSelection="false">
        <spif:tagCategory name="CANADA" lacv="4407630" obsolete="false" />
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);

    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);

    sw::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    CPPUNIT_ASSERT_EQUAL(u"SPIF Collabora"_ustr, aPolicy.aName);
    CPPUNIT_ASSERT_EQUAL(u"1.2.826.0.1310.1.2.0"_ustr, aPolicy.aId);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPolicy.aClassifications.size());

    CPPUNIT_ASSERT_EQUAL(u"OFFICIAL"_ustr, aPolicy.aClassifications[0].aName);
    CPPUNIT_ASSERT_EQUAL(u"yellow"_ustr, aPolicy.aClassifications[0].aColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aPolicy.aClassifications[0].nLacv);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aPolicy.aClassifications[0].nHierarchy);
    CPPUNIT_ASSERT(!aPolicy.aClassifications[0].bObsolete);

    CPPUNIT_ASSERT_EQUAL(u"SECRET"_ustr, aPolicy.aClassifications[1].aName);
    CPPUNIT_ASSERT_EQUAL(u"red"_ustr, aPolicy.aClassifications[1].aColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aPolicy.aClassifications[1].nLacv);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SpifPolicyTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
