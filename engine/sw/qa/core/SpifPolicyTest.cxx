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
    void testValidate();

    CPPUNIT_TEST_SUITE(SpifPolicyTest);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST(testValidate);
    CPPUNIT_TEST_SUITE_END();
};

void SpifPolicyTest::testParse()
{
    // Mirrors sixworks/spif-collabora.xml (policy id, classifications, tag sets).
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
      <spif:securityCategoryTag name="Releasable To" tagType="enumerated" enumType="permissive" singleSelection="false" minSelection="1" maxSelection="2">
        <spif:tagCategory name="CANADA" lacv="4407630" obsolete="false" />
        <spif:tagCategory name="UNITED KINGDOM" lacv="5591873" obsolete="false" />
        <spif:markingQualifier markingCode="pageTopBottom">
          <spif:qualifier markingQualifier="//" qualifierCode="separator" />
          <spif:qualifier markingQualifier="." qualifierCode="suffix" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="UK Restrictive Codeword - NTK" id="1.2.826.0.1310.1.2.0.4">
      <spif:securityCategoryTag name="UK Restrictive Codewords - NTK" tagType="enumerated" enumType="restrictive" singleSelection="false">
        <spif:tagCategory name="INT" lacv="21745403334774610" obsolete="false">
          <spif:excludedClass>OFFICIAL</spif:excludedClass>
        </spif:tagCategory>
        <spif:tagCategory name="OPS" lacv="21745403334774611" obsolete="true">
          <spif:excludedClass>OFFICIAL</spif:excludedClass>
        </spif:tagCategory>
        <spif:tagCategory name="REL" lacv="99" requiredClass="OFFICIAL" obsolete="false">
          <spif:excludedCategory tagSetRef="Release Categories" tagType="enumerated" lacv="4407630" />
          <spif:requiredCategory operation="oneOrMore">
            <spif:categoryGroup tagSetRef="Release Categories" tagType="enumerated" lacv="5591873" />
          </spif:requiredCategory>
        </spif:tagCategory>
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

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPolicy.aTagSets.size());

    const auto& rRelSet = aPolicy.aTagSets[0];
    CPPUNIT_ASSERT_EQUAL(u"Release Categories"_ustr, rRelSet.aName);
    CPPUNIT_ASSERT_EQUAL(u"1.2.826.0.1310.1.2.0.0"_ustr, rRelSet.aId);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRelSet.aTags.size());

    const auto& rRelTag = rRelSet.aTags[0];
    CPPUNIT_ASSERT_EQUAL(u"Releasable To"_ustr, rRelTag.aName);
    CPPUNIT_ASSERT_EQUAL(u"enumerated"_ustr, rRelTag.aTagType);
    CPPUNIT_ASSERT_EQUAL(u"permissive"_ustr, rRelTag.aEnumType);
    CPPUNIT_ASSERT(!rRelTag.bSingleSelection);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rRelTag.aCategories.size());
    CPPUNIT_ASSERT_EQUAL(u"CANADA"_ustr, rRelTag.aCategories[0].aName);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(4407630), rRelTag.aCategories[0].nLacv);
    CPPUNIT_ASSERT_EQUAL(u"UNITED KINGDOM"_ustr, rRelTag.aCategories[1].aName);
    CPPUNIT_ASSERT_EQUAL(u"//"_ustr, rRelTag.aMarkingSeparator);
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, rRelTag.aMarkingSuffix);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rRelTag.nMinSelection);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rRelTag.nMaxSelection);

    // lacv exceeding 32 bits must round-trip.
    const auto& rNtkTag = aPolicy.aTagSets[1].aTags[0];
    CPPUNIT_ASSERT_EQUAL(u"restrictive"_ustr, rNtkTag.aEnumType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), rNtkTag.nMinSelection); // unset => unbounded
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(21745403334774610), rNtkTag.aCategories[0].nLacv);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rNtkTag.aCategories[0].aExcludedClasses.size());
    CPPUNIT_ASSERT_EQUAL(u"OFFICIAL"_ustr, rNtkTag.aCategories[0].aExcludedClasses[0]);

    // isSelectable: obsolete or excluded-by-classification → not selectable.
    CPPUNIT_ASSERT(rNtkTag.aCategories[0].isSelectable(u"SECRET"_ustr)); // INT ok under SECRET
    CPPUNIT_ASSERT(!rNtkTag.aCategories[0].isSelectable(u"OFFICIAL"_ustr)); // INT excludes OFFICIAL
    CPPUNIT_ASSERT_EQUAL(u"OPS"_ustr, rNtkTag.aCategories[1].aName);
    CPPUNIT_ASSERT(!rNtkTag.aCategories[1].isSelectable(u"SECRET"_ustr)); // OPS obsolete
    CPPUNIT_ASSERT_EQUAL(u"REL"_ustr, rNtkTag.aCategories[2].aName);
    CPPUNIT_ASSERT_EQUAL(u"OFFICIAL"_ustr, rNtkTag.aCategories[2].aRequiredClass);
    CPPUNIT_ASSERT(rNtkTag.aCategories[2].isSelectable(u"OFFICIAL"_ustr));
    CPPUNIT_ASSERT(!rNtkTag.aCategories[2].isSelectable(u"SECRET"_ustr)); // requiredClass=OFFICIAL

    const auto& rRel = rNtkTag.aCategories[2];
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRel.aExcludedCategories.size());
    CPPUNIT_ASSERT_EQUAL(u"Release Categories"_ustr, rRel.aExcludedCategories[0].aTagSetRef);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(4407630), rRel.aExcludedCategories[0].nLacv);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRel.aRequiredCategories.size());
    CPPUNIT_ASSERT_EQUAL(u"oneOrMore"_ustr, rRel.aRequiredCategories[0].aOperation);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRel.aRequiredCategories[0].aCategories.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(5591873),
                         rRel.aRequiredCategories[0].aCategories[0].nLacv);

    // Marking derivation walks only categories selectable under the classification
    // (matching the dialog's filtered rows). Under SECRET: CANADA, UNITED KINGDOM,
    // INT (OPS is obsolete and skipped).
    std::vector<bool> aSelected(3, false);
    aSelected[0] = true; // CANADA
    aSelected[1] = true; // UNITED KINGDOM
    CPPUNIT_ASSERT_EQUAL(u"SECRET//CANADA UNITED KINGDOM."_ustr,
                         aPolicy.buildMarking(u"SECRET"_ustr, aSelected));

    // INT is the 3rd selectable; its tag has no qualifiers, so it attaches plainly.
    std::vector<bool> aIntOnly(3, false);
    aIntOnly[2] = true; // INT
    CPPUNIT_ASSERT_EQUAL(u"SECRETINT"_ustr, aPolicy.buildMarking(u"SECRET"_ustr, aIntOnly));
}

void SpifPolicyTest::testValidate()
{
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">
  <spif:securityPolicyId name="T" id="1.2.3" />
  <spif:securityClassifications>
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4" />
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="TS" id="1.2.3.0">
      <spif:securityCategoryTag name="Rel" tagType="enumerated" enumType="permissive" minSelection="1" maxSelection="2">
        <spif:tagCategory name="A" lacv="1" obsolete="false" />
        <spif:tagCategory name="B" lacv="2" obsolete="false" />
        <spif:tagCategory name="C" lacv="3" obsolete="false" />
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);

    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    sw::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    // Below minimum (0 < 1).
    auto aTooFew = aPolicy.validate(u"SECRET"_ustr, { false, false, false });
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTooFew.size());
    CPPUNIT_ASSERT_EQUAL(u"Rel"_ustr, aTooFew[0].aTagName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTooFew[0].nSelected);

    // Within range (1 in 1..2).
    CPPUNIT_ASSERT(aPolicy.validate(u"SECRET"_ustr, { true, false, false }).empty());

    // Above maximum (3 > 2).
    auto aTooMany = aPolicy.validate(u"SECRET"_ustr, { true, true, true });
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTooMany.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aTooMany[0].nSelected);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SpifPolicyTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
