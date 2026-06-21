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

#include <StanagLabel.hxx>

#include <tools/stream.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class StanagLabelTest : public CppUnit::TestFixture
{
    void testToXml();
    void testToBindingXml();
    void testItemProps();
    void testParseRoundTrip();

    CPPUNIT_TEST_SUITE(StanagLabelTest);
    CPPUNIT_TEST(testToXml);
    CPPUNIT_TEST(testToBindingXml);
    CPPUNIT_TEST(testItemProps);
    CPPUNIT_TEST(testParseRoundTrip);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{
sw::seclabel::StanagLabel makeSampleLabel()
{
    sw::seclabel::StanagLabel aLabel;
    aLabel.aPolicyName = u"SPIF Collabora"_ustr;
    aLabel.aPolicyId = u"urn:oid:1.2.826.0.1310.1.2.0"_ustr;
    aLabel.aClassification = u"SECRET"_ustr;
    aLabel.aCreationDateTime = u"2026-06-21T10:00:00Z"_ustr;
    aLabel.aReviewDateTime = u"2027-06-21T10:00:00Z"_ustr;

    sw::seclabel::StanagCategory aCategory;
    aCategory.aTagName = u"Releasable To"_ustr;
    aCategory.aType = u"PERMISSIVE"_ustr;
    aCategory.aValues = { u"CANADA"_ustr, u"UNITED KINGDOM"_ustr };
    aLabel.aCategories.push_back(aCategory);
    return aLabel;
}
}

void StanagLabelTest::testToXml()
{
    const OUString aXml = makeSampleLabel().toXml();

    CPPUNIT_ASSERT(aXml.indexOf(u"urn:nato:stanag:4774:confidentialitymetadatalabel:1:0") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"ConfidentialityLabelVersion=\"1\"") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"ReviewDateTime=\"2027-06-21T10:00:00Z\"") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"URI=\"urn:oid:1.2.826.0.1310.1.2.0\"") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u">SPIF Collabora<") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u">SECRET<") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"TagName=\"Releasable To\"") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"Type=\"PERMISSIVE\"") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u">CANADA<") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u">UNITED KINGDOM<") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u">2026-06-21T10:00:00Z<") != -1);
}

void StanagLabelTest::testToBindingXml()
{
    const OUString aXml = makeSampleLabel().toBindingXml();

    // 4778 binding structure.
    CPPUNIT_ASSERT(aXml.indexOf(u"urn:nato:stanag:4778:bindinginformation:1:0") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"<BindingInformation") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"<MetadataBindingContainer") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"<Metadata") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"URI=\"\"") != -1); // DataReference binds the whole document

    // 4774 label embedded inside, re-declaring its own namespace.
    CPPUNIT_ASSERT(aXml.indexOf(u"<OriginatorConfidentialityLabel") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"urn:nato:stanag:4774:confidentialitymetadatalabel:1:0") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u">SECRET<") != -1);
}

void StanagLabelTest::testItemProps()
{
    const OUString aXml = sw::seclabel::buildItemProps(
        u"{B6E4D8A1-1A35-4F0E-9B7A-71F4C0F5E0D3}"_ustr,
        u"urn:nato:stanag:4778:bindinginformation:1:0"_ustr);

    CPPUNIT_ASSERT(aXml.indexOf(u"<ds:datastoreItem") != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"ds:itemID=\"{B6E4D8A1-1A35-4F0E-9B7A-71F4C0F5E0D3}\"") != -1);
    CPPUNIT_ASSERT(
        aXml.indexOf(
            u"xmlns:ds=\"http://schemas.openxmlformats.org/officeDocument/2006/customXml\"")
        != -1);
    CPPUNIT_ASSERT(aXml.indexOf(u"ds:uri=\"urn:nato:stanag:4778:bindinginformation:1:0\"") != -1);
}

void StanagLabelTest::testParseRoundTrip()
{
    // Binding form: build -> serialize -> parse back, fields must survive.
    const OString aBinding = OUStringToOString(makeSampleLabel().toBindingXml(),
                                               RTL_TEXTENCODING_UTF8);
    SvMemoryStream aStream(const_cast<char*>(aBinding.getStr()), aBinding.getLength(),
                           StreamMode::READ);

    sw::seclabel::StanagLabel aParsed;
    CPPUNIT_ASSERT(aParsed.parse(aStream));

    CPPUNIT_ASSERT_EQUAL(u"SPIF Collabora"_ustr, aParsed.aPolicyName);
    CPPUNIT_ASSERT_EQUAL(u"urn:oid:1.2.826.0.1310.1.2.0"_ustr, aParsed.aPolicyId);
    CPPUNIT_ASSERT_EQUAL(u"SECRET"_ustr, aParsed.aClassification);
    CPPUNIT_ASSERT_EQUAL(u"2026-06-21T10:00:00Z"_ustr, aParsed.aCreationDateTime);
    CPPUNIT_ASSERT_EQUAL(u"2027-06-21T10:00:00Z"_ustr, aParsed.aReviewDateTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aParsed.aCategories.size());
    CPPUNIT_ASSERT_EQUAL(u"Releasable To"_ustr, aParsed.aCategories[0].aTagName);
    CPPUNIT_ASSERT_EQUAL(u"PERMISSIVE"_ustr, aParsed.aCategories[0].aType);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aParsed.aCategories[0].aValues.size());
    CPPUNIT_ASSERT_EQUAL(u"CANADA"_ustr, aParsed.aCategories[0].aValues[0]);
    CPPUNIT_ASSERT_EQUAL(u"UNITED KINGDOM"_ustr, aParsed.aCategories[0].aValues[1]);

    // Standalone label form (root is the label, no binding wrapper).
    const OString aLabel = OUStringToOString(makeSampleLabel().toXml(), RTL_TEXTENCODING_UTF8);
    SvMemoryStream aLabelStream(const_cast<char*>(aLabel.getStr()), aLabel.getLength(),
                                StreamMode::READ);
    sw::seclabel::StanagLabel aParsed2;
    CPPUNIT_ASSERT(aParsed2.parse(aLabelStream));
    CPPUNIT_ASSERT_EQUAL(u"SECRET"_ustr, aParsed2.aClassification);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aParsed2.aCategories[0].aValues.size());
}

CPPUNIT_TEST_SUITE_REGISTRATION(StanagLabelTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
