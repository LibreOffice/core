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

#include <svx/seclabel/SpifPolicy.hxx>

#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class SpifPolicyTest : public CppUnit::TestFixture
{
    void testParse();
    void testValidate();
    void testValidateRelationships();
    void testBuildLabel();
    void testMatchesLabel();
    void testPolicySet();
    void testWantsWatermark();
    void testMarkingModifiers();
    void testDeriveMarking();
    void testDeriveMarkingOwnership();
    void testMarkingMatrix();

    CPPUNIT_TEST_SUITE(SpifPolicyTest);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST(testValidate);
    CPPUNIT_TEST(testValidateRelationships);
    CPPUNIT_TEST(testBuildLabel);
    CPPUNIT_TEST(testMatchesLabel);
    CPPUNIT_TEST(testPolicySet);
    CPPUNIT_TEST(testWantsWatermark);
    CPPUNIT_TEST(testMarkingModifiers);
    CPPUNIT_TEST(testDeriveMarking);
    CPPUNIT_TEST(testDeriveMarkingOwnership);
    CPPUNIT_TEST(testMarkingMatrix);
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

    svx::seclabel::SpifPolicy aPolicy;
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
    CPPUNIT_ASSERT_EQUAL(
        u"SPIF Collabora SECRET CANADA // UNITED KINGDOM."_ustr,
        aPolicy.deriveMarking(aPolicy.buildLabel(u"SECRET"_ustr, aSelected, OUString(), OUString())));

    // INT is the 3rd selectable; its tag has no qualifiers, so it attaches as a plain
    // group, joined to the classification by a space.
    std::vector<bool> aIntOnly(3, false);
    aIntOnly[2] = true; // INT
    CPPUNIT_ASSERT_EQUAL(
        u"SPIF Collabora SECRET INT"_ustr,
        aPolicy.deriveMarking(aPolicy.buildLabel(u"SECRET"_ustr, aIntOnly, OUString(), OUString())));
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
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    using T = svx::seclabel::SpifViolationType;

    // Below minimum (0 < 1).
    auto aTooFew = aPolicy.validate(u"SECRET"_ustr, { false, false, false });
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTooFew.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(T::MinSelection), static_cast<int>(aTooFew[0].eType));
    CPPUNIT_ASSERT_EQUAL(u"Rel"_ustr, aTooFew[0].aName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTooFew[0].nSelected);

    // Within range (1 in 1..2).
    CPPUNIT_ASSERT(aPolicy.validate(u"SECRET"_ustr, { true, false, false }).empty());

    // Above maximum (3 > 2).
    auto aTooMany = aPolicy.validate(u"SECRET"_ustr, { true, true, true });
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTooMany.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(T::MaxSelection), static_cast<int>(aTooMany[0].eType));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aTooMany[0].nSelected);
}

void SpifPolicyTest::testValidateRelationships()
{
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">
  <spif:securityPolicyId name="T" id="1.2.3" />
  <spif:securityClassifications>
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4" />
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="Caveats" id="1.2.3.0">
      <spif:securityCategoryTag name="Cav" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="X" lacv="1" obsolete="false" />
        <spif:tagCategory name="Y" lacv="2" obsolete="false" />
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="Codeword" id="1.2.3.1">
      <spif:securityCategoryTag name="Cw" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="Z" lacv="3" obsolete="false">
          <spif:excludedCategory tagSetRef="Caveats" tagType="enumerated" lacv="1" />
          <spif:requiredCategory operation="oneOrMore">
            <spif:categoryGroup tagSetRef="Caveats" tagType="enumerated" lacv="2" />
          </spif:requiredCategory>
        </spif:tagCategory>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);

    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    using T = svx::seclabel::SpifViolationType;

    // Selectable order under SECRET: X(0), Y(1), Z(2).
    // Z alone: requiredCategory (needs Y) unmet.
    auto aZ = aPolicy.validate(u"SECRET"_ustr, { false, false, true });
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aZ.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(T::RequiredCategory), static_cast<int>(aZ[0].eType));
    CPPUNIT_ASSERT_EQUAL(u"Z"_ustr, aZ[0].aName);

    // Z + X: X is excluded by Z, and Y is still missing -> two violations.
    auto aZX = aPolicy.validate(u"SECRET"_ustr, { true, false, true });
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aZX.size());

    // Z + Y: exclusion satisfied (X not chosen) and requirement met.
    CPPUNIT_ASSERT(aPolicy.validate(u"SECRET"_ustr, { false, true, true }).empty());
}

void SpifPolicyTest::testBuildLabel()
{
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">
  <spif:securityPolicyId name="SPIF Collabora" id="1.2.826.0.1310.1.2.0" />
  <spif:securityClassifications>
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4" />
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="Release Categories" id="1.2.826.0.1310.1.2.0.0">
      <spif:securityCategoryTag name="Releasable To" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="CANADA" lacv="4407630" obsolete="false" />
        <spif:tagCategory name="UNITED KINGDOM" lacv="5591873" obsolete="false" />
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);

    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    const auto aLabel = aPolicy.buildLabel(u"SECRET"_ustr, { true, true },
                                           u"2026-06-21T10:00:00Z"_ustr, u"2027-06-21T10:00:00Z"_ustr);

    CPPUNIT_ASSERT_EQUAL(u"SPIF Collabora"_ustr, aLabel.aPolicyName);
    CPPUNIT_ASSERT_EQUAL(u"urn:oid:1.2.826.0.1310.1.2.0"_ustr, aLabel.aPolicyId);
    CPPUNIT_ASSERT_EQUAL(u"SECRET"_ustr, aLabel.aClassification);
    CPPUNIT_ASSERT_EQUAL(u"2026-06-21T10:00:00Z"_ustr, aLabel.aCreationDateTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aLabel.aCategories.size());
    CPPUNIT_ASSERT_EQUAL(u"Releasable To"_ustr, aLabel.aCategories[0].aTagName);
    CPPUNIT_ASSERT_EQUAL(u"PERMISSIVE"_ustr, aLabel.aCategories[0].aType);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aLabel.aCategories[0].aValues.size());
    CPPUNIT_ASSERT_EQUAL(u"CANADA"_ustr, aLabel.aCategories[0].aValues[0]);
    CPPUNIT_ASSERT_EQUAL(u"UNITED KINGDOM"_ustr, aLabel.aCategories[0].aValues[1]);
}

void SpifPolicyTest::testMatchesLabel()
{
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">
  <spif:securityPolicyId name="SPIF Collabora" id="1.2.826.0.1310.1.2.0" />
  <spif:securityClassifications>
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4" />
  </spif:securityClassifications>
</spif:SPIF>)xml"_ostr);
    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    // A label naming this policy's OID (urn:oid: prefixed) is ours to edit.
    svx::seclabel::StanagLabel aLabel;
    aLabel.aPolicyId = u"urn:oid:1.2.826.0.1310.1.2.0"_ustr;
    CPPUNIT_ASSERT(aPolicy.matchesLabel(aLabel));

    // The bare OID (no urn:oid: prefix) matches too.
    aLabel.aPolicyId = u"1.2.826.0.1310.1.2.0"_ustr;
    CPPUNIT_ASSERT(aPolicy.matchesLabel(aLabel));

    // A different OID is a foreign policy.
    aLabel.aPolicyId = u"urn:oid:1.2.826.0.1310.9.9.9"_ustr;
    CPPUNIT_ASSERT(!aPolicy.matchesLabel(aLabel));

    // An empty policy URI never matches.
    aLabel.aPolicyId.clear();
    CPPUNIT_ASSERT(!aPolicy.matchesLabel(aLabel));
}

void SpifPolicyTest::testPolicySet()
{
    auto makeSpif = [](const char* pName, const char* pId) -> OString {
        return OString::Concat("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<spif:SPIF xmlns:spif=\"http://www.xmlspif.org/spif\" "
                               "schemaVersion=\"1.0\" version=\"1\">"
                               "<spif:securityPolicyId name=\"")
               + pName + "\" id=\"" + pId
               + "\" /><spif:securityClassifications>"
                 "<spif:securityClassification name=\"SECRET\" color=\"red\" lacv=\"4\" "
                 "hierarchy=\"4\" /></spif:securityClassifications></spif:SPIF>";
    };

    utl::TempFileNamed aTempDir(nullptr, /*bDirectory*/ true);
    const OUString sDir = aTempDir.GetURL();

    auto writeFile = [&sDir](const OUString& rName, const OString& rContent) {
        SvFileStream aOut(sDir + "/" + rName, StreamMode::WRITE | StreamMode::TRUNC);
        aOut.WriteBytes(rContent.getStr(), rContent.getLength());
    };
    writeFile(u"a.xml"_ustr, makeSpif("Policy A", "1.2.3"));
    writeFile(u"b.xml"_ustr, makeSpif("Policy B", "4.5.6"));
    writeFile(u"notspif.xml"_ustr, "<html/>"_ostr); // valid XML, wrong root -> skipped
    writeFile(u"readme.txt"_ustr, makeSpif("Policy C", "7.8.9")); // not *.xml -> skipped

    svx::seclabel::SpifPolicySet aSet;
    aSet.loadFromDir(sDir);

    // Only the two SPIF *.xml files load, in filename order.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aSet.aPolicies.size());
    CPPUNIT_ASSERT_EQUAL(u"Policy A"_ustr, aSet.aPolicies[0].aName);
    CPPUNIT_ASSERT_EQUAL(u"Policy B"_ustr, aSet.aPolicies[1].aName);

    // findByLabel resolves a label's OID to the provisioned policy.
    svx::seclabel::StanagLabel aLabel;
    aLabel.aPolicyId = u"urn:oid:4.5.6"_ustr;
    const svx::seclabel::SpifPolicy* pMatch = aSet.findByLabel(aLabel);
    CPPUNIT_ASSERT(pMatch);
    CPPUNIT_ASSERT_EQUAL(u"Policy B"_ustr, pMatch->aName);

    // An OID none of the policies declare is foreign.
    aLabel.aPolicyId = u"urn:oid:9.9.9"_ustr;
    CPPUNIT_ASSERT(!aSet.findByLabel(aLabel));

    // A later file declaring an already-loaded OID replaces that policy in place
    // instead of listing it twice. loadProvisioned relies on this: it scans the
    // system tree and then the user tree, so the user's copy of an org policy wins
    // while the listing order stays put.
    writeFile(u"c.xml"_ustr, makeSpif("Policy A (user)", "1.2.3"));
    svx::seclabel::SpifPolicySet aReloaded;
    aReloaded.loadFromDir(sDir);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aReloaded.aPolicies.size());
    CPPUNIT_ASSERT_EQUAL(u"Policy A (user)"_ustr, aReloaded.aPolicies[0].aName);
    CPPUNIT_ASSERT_EQUAL(u"Policy B"_ustr, aReloaded.aPolicies[1].aName);
}

void SpifPolicyTest::testWantsWatermark()
{
    // One tag carries markingCode=waterMark; selecting its category triggers a
    // watermark, selecting only the other tag's category does not.
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">
  <spif:securityPolicyId name="T" id="1.2.3" />
  <spif:securityClassifications>
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4" />
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="Codeword" id="1.2.3.0">
      <spif:securityCategoryTag name="CW" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="TS" lacv="1" obsolete="false" />
        <spif:markingQualifier markingCode="waterMark">
          <spif:qualifier markingQualifier="//" qualifierCode="separator" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="Release" id="1.2.3.1">
      <spif:securityCategoryTag name="Rel" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="CANADA" lacv="2" obsolete="false" />
        <spif:markingQualifier markingCode="pageTopBottom">
          <spif:qualifier markingQualifier="//" qualifierCode="separator" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="Cover" id="1.2.3.2">
      <spif:securityCategoryTag name="Cov" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="COVER" lacv="3" obsolete="false" />
        <spif:markingQualifier markingCode="documentStart" />
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="End" id="1.2.3.3">
      <spif:securityCategoryTag name="EndTag" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="ENDP" lacv="4" obsolete="false" />
        <spif:markingQualifier markingCode="documentEnd" />
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="Portion" id="1.2.3.4">
      <spif:securityCategoryTag name="PortionTag" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="PORT" lacv="5" obsolete="false" />
        <spif:markingQualifier markingCode="portionMarking" />
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);
    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    // Each placement flag is parsed only onto the tag that declares it.
    CPPUNIT_ASSERT(aPolicy.aTagSets[0].aTags[0].bWatermark);
    CPPUNIT_ASSERT(!aPolicy.aTagSets[1].aTags[0].bWatermark);
    CPPUNIT_ASSERT(aPolicy.aTagSets[2].aTags[0].bDocumentStart);
    CPPUNIT_ASSERT(aPolicy.aTagSets[3].aTags[0].bDocumentEnd);
    CPPUNIT_ASSERT(aPolicy.aTagSets[4].aTags[0].bPortionMarking);

    // Selectable order under SECRET: TS(0), CANADA(1), COVER(2), ENDP(3), PORT(4).
    CPPUNIT_ASSERT(aPolicy.wantsWatermark(u"SECRET"_ustr, { true, false, false, false, false })); // TS
    CPPUNIT_ASSERT(
        !aPolicy.wantsWatermark(u"SECRET"_ustr, { false, true, false, false, false })); // CANADA
    CPPUNIT_ASSERT(!aPolicy.wantsWatermark(u"SECRET"_ustr, { false, false, false, false, false }));

    CPPUNIT_ASSERT(
        aPolicy.wantsDocumentStart(u"SECRET"_ustr, { false, false, true, false, false })); // COVER
    CPPUNIT_ASSERT(!aPolicy.wantsDocumentStart(u"SECRET"_ustr, { true, true, false, true, true }));

    CPPUNIT_ASSERT(
        aPolicy.wantsDocumentEnd(u"SECRET"_ustr, { false, false, false, true, false })); // ENDP
    CPPUNIT_ASSERT(!aPolicy.wantsDocumentEnd(u"SECRET"_ustr, { true, true, true, false, true }));

    CPPUNIT_ASSERT(
        aPolicy.wantsPortionMarking(u"SECRET"_ustr, { false, false, false, false, true })); // PORT
    CPPUNIT_ASSERT(!aPolicy.wantsPortionMarking(u"SECRET"_ustr, { true, true, true, true, false }));
}

void SpifPolicyTest::testMarkingModifiers()
{
    // markingData display codes: noNameDisplay (show phrase) on a classification and
    // a category, and suppressClassName (drop the classification from the marking).
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">
  <spif:securityPolicyId name="T" id="1.2.3" />
  <spif:securityClassifications>
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4">
      <spif:markingData phrase="S"><spif:code>noNameDisplay</spif:code></spif:markingData>
    </spif:securityClassification>
    <spif:securityClassification name="TOPSECRET" color="red" lacv="5" hierarchy="5">
      <spif:markingData><spif:code>suppressClassName</spif:code></spif:markingData>
    </spif:securityClassification>
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="Rel" id="1.2.3.0">
      <spif:securityCategoryTag name="Releasable" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="CANADA" lacv="1" obsolete="false" />
        <spif:tagCategory name="GBR" lacv="2" obsolete="false">
          <spif:markingData phrase="UK"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:markingQualifier markingCode="pageTopBottom">
          <spif:qualifier markingQualifier="//" qualifierCode="separator" />
          <spif:qualifier markingQualifier="." qualifierCode="suffix" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);
    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    CPPUNIT_ASSERT(aPolicy.aClassifications[0].bNoNameDisplay);
    CPPUNIT_ASSERT_EQUAL(u"S"_ustr, aPolicy.aClassifications[0].aMarkingPhrase);
    CPPUNIT_ASSERT(aPolicy.aClassifications[1].bSuppressClassName);
    CPPUNIT_ASSERT(aPolicy.aTagSets[0].aTags[0].aCategories[1].bNoNameDisplay);

    auto marking = [&aPolicy](const OUString& rClass, const std::vector<bool>& rSel) {
        return aPolicy.deriveMarking(aPolicy.buildLabel(rClass, rSel, OUString(), OUString()));
    };

    // noNameDisplay: the classification shows its phrase "S", not "SECRET".
    CPPUNIT_ASSERT_EQUAL(u"T S CANADA."_ustr, marking(u"SECRET"_ustr, { true, false }));
    // ... and the category GBR shows its phrase "UK".
    CPPUNIT_ASSERT_EQUAL(u"T S UK."_ustr, marking(u"SECRET"_ustr, { false, true }));

    // suppressClassName: the class is dropped, but the policy-name ownership still leads.
    CPPUNIT_ASSERT_EQUAL(u"T CANADA."_ustr, marking(u"TOPSECRET"_ustr, { true, false }));
    CPPUNIT_ASSERT_EQUAL(u"T CANADA // UK."_ustr, marking(u"TOPSECRET"_ustr, { true, true }));
}

void SpifPolicyTest::testDeriveMarking()
{
    // ADatP-4774.2-conformant marking from a label: per-group markingQualifier
    // (prefix + values joined by the normalized separator + suffix), groups in
    // policy tag-set order joined by a space, value order preserved (no sort),
    // phrase via noNameDisplay, suppressClassName drops the classification.
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">
  <spif:securityPolicyId name="T" id="1.2.3" />
  <spif:securityClassifications>
    <spif:securityClassification name="OFFICIAL" color="green" lacv="1" hierarchy="1" />
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4">
      <spif:markingData phrase="S"><spif:code>noNameDisplay</spif:code></spif:markingData>
    </spif:securityClassification>
    <spif:securityClassification name="TOPSECRET" color="red" lacv="5" hierarchy="5">
      <spif:markingData><spif:code>suppressClassName</spif:code></spif:markingData>
    </spif:securityClassification>
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="RelSet" id="1.2.3.1">
      <spif:securityCategoryTag name="Releasable To" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="JAP" lacv="1" obsolete="false">
          <spif:markingData phrase="Japan"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="AUS" lacv="2" obsolete="false">
          <spif:markingData phrase="Australia"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:markingQualifier markingCode="pageTop">
          <spif:qualifier markingQualifier="Releasable To " qualifierCode="prefix" />
          <spif:qualifier markingQualifier="," qualifierCode="separator" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="OnlySet" id="1.2.3.2">
      <spif:securityCategoryTag name="Only" tagType="permissive">
        <spif:tagCategory name="NOR" lacv="1" obsolete="false">
          <spif:markingData phrase="Norway"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="USA" lacv="2" obsolete="false">
          <spif:markingData phrase="United States"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:markingQualifier markingCode="pageTop">
          <spif:qualifier markingQualifier=" Only" qualifierCode="suffix" />
          <spif:qualifier markingQualifier="," qualifierCode="separator" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="CwSet" id="1.2.3.3">
      <spif:securityCategoryTag name="Codewords" tagType="restrictive">
        <spif:tagCategory name="ATOMIC" lacv="1" obsolete="false" />
        <spif:tagCategory name="TRIDENT" lacv="2" obsolete="false" />
        <spif:markingQualifier markingCode="pageTop">
          <spif:qualifier markingQualifier="//" qualifierCode="separator" />
          <spif:qualifier markingQualifier="." qualifierCode="suffix" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);
    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    // deriveMarking works off a built label. Selectable order (all selectable here):
    // JAP(0) AUS(1) | NOR(2) USA(3) | ATOMIC(4) TRIDENT(5).
    auto marking = [&aPolicy](const OUString& rClass, const std::vector<bool>& rSel) {
        return aPolicy.deriveMarking(aPolicy.buildLabel(rClass, rSel, OUString(), OUString()));
    };

    // Permissive prefix + comma-joined phrases (comma hugs: single trailing space).
    CPPUNIT_ASSERT_EQUAL(u"T OFFICIAL Releasable To Japan, Australia"_ustr,
                         marking(u"OFFICIAL"_ustr, { true, true, false, false, false, false }));

    // Suffix group; classification shows its noNameDisplay phrase "S".
    CPPUNIT_ASSERT_EQUAL(u"T S Norway, United States Only"_ustr,
                         marking(u"SECRET"_ustr, { false, false, true, true, false, false }));

    // "//" separator gets a space on each side; "." suffix hugs the values.
    CPPUNIT_ASSERT_EQUAL(u"T OFFICIAL ATOMIC // TRIDENT."_ustr,
                         marking(u"OFFICIAL"_ustr, { false, false, false, false, true, true }));

    // Groups render in policy tag-set order (Releasable To before Only), space-joined.
    CPPUNIT_ASSERT_EQUAL(u"T S Releasable To Japan Norway Only"_ustr,
                         marking(u"SECRET"_ustr, { true, false, true, false, false, false }));

    // suppressClassName: the classification is dropped, but the policy-name ownership
    // still leads.
    CPPUNIT_ASSERT_EQUAL(u"T ATOMIC."_ustr,
                         marking(u"TOPSECRET"_ustr, { false, false, false, false, true, false }));
}

void SpifPolicyTest::testDeriveMarkingOwnership()
{
    // Ownership prefix from replacePolicy: a Context value replaces it (NATO/KFOR),
    // TOP SECRET's classification replacePolicy overrides it (COSMIC), Context=NATO
    // deduplicates naturally (its phrase is "NATO"), and the "Releasable" Context
    // value (noNameDisplay with an empty phrase) is suppressed from the marking.
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="2.0" version="1">
  <spif:securityPolicyId name="NATO" id="1.3.26.1.3.1" />
  <spif:securityClassifications>
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4" />
    <spif:securityClassification name="TOPSECRET" color="red" lacv="5" hierarchy="5">
      <spif:markingData phrase="COSMIC"><spif:code>replacePolicy</spif:code></spif:markingData>
    </spif:securityClassification>
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="CtxSet" id="1.3.26.1.4.4">
      <spif:securityCategoryTag name="Context" tagType="permissive">
        <spif:tagCategory name="NATO" lacv="1" obsolete="false">
          <spif:markingData phrase="NATO"><spif:code>replacePolicy</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="KFOR" lacv="2" obsolete="false">
          <spif:markingData phrase="NATO/KFOR"><spif:code>replacePolicy</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="Releasable" lacv="3" obsolete="false">
          <spif:markingData><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="RelSet" id="1.3.26.1.4.2">
      <spif:securityCategoryTag name="Releasable To" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="JAP" lacv="1" obsolete="false">
          <spif:markingData phrase="Japan"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:markingQualifier markingCode="pageTop">
          <spif:qualifier markingQualifier="Releasable To " qualifierCode="prefix" />
          <spif:qualifier markingQualifier="," qualifierCode="separator" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);
    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    CPPUNIT_ASSERT_EQUAL(u"COSMIC"_ustr, aPolicy.aClassifications[1].aReplacePolicyPhrase);
    CPPUNIT_ASSERT_EQUAL(u"NATO/KFOR"_ustr,
                         aPolicy.aTagSets[0].aTags[0].aCategories[1].aReplacePolicyPhrase);

    auto marking = [&aPolicy](const OUString& rClass, const std::vector<bool>& rSel) {
        return aPolicy.deriveMarking(aPolicy.buildLabel(rClass, rSel, OUString(), OUString()));
    };

    // Selectable order: NATO(0) KFOR(1) Releasable(2) | JAP(3).
    // Context=NATO -> ownership "NATO"; the Context group renders nothing (its value
    // fed ownership); Releasable To renders normally.
    CPPUNIT_ASSERT_EQUAL(u"NATO SECRET Releasable To Japan"_ustr,
                         marking(u"SECRET"_ustr, { true, false, false, true }));

    // Context=KFOR -> ownership "NATO/KFOR".
    CPPUNIT_ASSERT_EQUAL(u"NATO/KFOR SECRET Releasable To Japan"_ustr,
                         marking(u"SECRET"_ustr, { false, true, false, true }));

    // TOP SECRET: the classification replacePolicy (COSMIC) overrides the Context.
    CPPUNIT_ASSERT_EQUAL(u"COSMIC TOPSECRET"_ustr,
                         marking(u"TOPSECRET"_ustr, { true, false, false, false }));

    // The suppressed "Releasable" Context value contributes nothing.
    CPPUNIT_ASSERT_EQUAL(u"NATO SECRET Releasable To Japan"_ustr,
                         marking(u"SECRET"_ustr, { true, false, true, true }));

    // No replacePolicy value selected -> ownership falls back to the policy name.
    CPPUNIT_ASSERT_EQUAL(u"NATO SECRET Releasable To Japan"_ustr,
                         marking(u"SECRET"_ustr, { false, false, false, true }));
}

void SpifPolicyTest::testMarkingMatrix()
{
    // The six worked examples from ADatP-4774.2 Chapter 6, reproduced from a NATO-like
    // policy. Exercises ownership (Context replacePolicy -> NATO/<ctx>), the Only suffix
    // and Releasable-to prefix, the Administrative "-STAFF" hug, trigraph->full-name via
    // phrases, group ordering (Only before Releasable to), and preserved value order.
    static const OString aSpif(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="2.0" version="1">
  <spif:securityPolicyId name="NATO" id="1.3.26.1.3.1" />
  <spif:securityClassifications>
    <spif:securityClassification name="RESTRICTED" color="blue" lacv="2" hierarchy="2" />
    <spif:securityClassification name="CONFIDENTIAL" color="green" lacv="3" hierarchy="3" />
    <spif:securityClassification name="SECRET" color="red" lacv="4" hierarchy="4" />
  </spif:securityClassifications>
  <spif:securityCategoryTagSets>
    <spif:securityCategoryTagSet name="CtxSet" id="1.3.26.1.4.4">
      <spif:securityCategoryTag name="Context" tagType="permissive">
        <spif:tagCategory name="NATO" lacv="1" obsolete="false">
          <spif:markingData phrase="NATO"><spif:code>replacePolicy</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="KFOR" lacv="2" obsolete="false">
          <spif:markingData phrase="NATO/KFOR"><spif:code>replacePolicy</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="EAPC" lacv="3" obsolete="false">
          <spif:markingData phrase="NATO/EAPC"><spif:code>replacePolicy</spif:code></spif:markingData>
        </spif:tagCategory>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="AdminSet" id="1.3.26.1.4.3">
      <spif:securityCategoryTag name="Administrative" tagType="tagType7">
        <spif:tagCategory name="STAFF" lacv="1" obsolete="false" />
        <spif:markingQualifier markingCode="pageTop">
          <spif:qualifier markingQualifier="-" qualifierCode="prefix" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="OnlySet" id="1.3.26.1.4.5">
      <spif:securityCategoryTag name="Only" tagType="permissive">
        <spif:tagCategory name="NATO" lacv="1" obsolete="false" />
        <spif:tagCategory name="IRL" lacv="2" obsolete="false">
          <spif:markingData phrase="Ireland"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="UKR" lacv="3" obsolete="false">
          <spif:markingData phrase="Ukraine"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="NOR" lacv="4" obsolete="false">
          <spif:markingData phrase="Norway"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="USA" lacv="5" obsolete="false">
          <spif:markingData phrase="United States"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:markingQualifier markingCode="pageTop">
          <spif:qualifier markingQualifier=" Only" qualifierCode="suffix" />
          <spif:qualifier markingQualifier="," qualifierCode="separator" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
    <spif:securityCategoryTagSet name="RelSet" id="1.3.26.1.4.2">
      <spif:securityCategoryTag name="Releasable To" tagType="enumerated" enumType="permissive">
        <spif:tagCategory name="JAP" lacv="1" obsolete="false">
          <spif:markingData phrase="Japan"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="AUS" lacv="2" obsolete="false">
          <spif:markingData phrase="Australia"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="PFP" lacv="3" obsolete="false" />
        <spif:tagCategory name="SWE" lacv="4" obsolete="false">
          <spif:markingData phrase="Sweden"><spif:code>noNameDisplay</spif:code></spif:markingData>
        </spif:tagCategory>
        <spif:tagCategory name="RESOLUTE SUPPORT" lacv="5" obsolete="false" />
        <spif:markingQualifier markingCode="pageTop">
          <spif:qualifier markingQualifier="Releasable to " qualifierCode="prefix" />
          <spif:qualifier markingQualifier="," qualifierCode="separator" />
        </spif:markingQualifier>
      </spif:securityCategoryTag>
    </spif:securityCategoryTagSet>
  </spif:securityCategoryTagSets>
</spif:SPIF>)xml"_ostr);
    SvMemoryStream aStream(const_cast<char*>(aSpif.getStr()), aSpif.getLength(), StreamMode::READ);
    svx::seclabel::SpifPolicy aPolicy;
    CPPUNIT_ASSERT(aPolicy.parse(aStream));

    // Selectable indices (all selectable): Context NATO(0) KFOR(1) EAPC(2) |
    // Administrative STAFF(3) | Only NATO(4) IRL(5) UKR(6) NOR(7) USA(8) |
    // Releasable To JAP(9) AUS(10) PFP(11) SWE(12) RESOLUTE SUPPORT(13).
    auto sel = [](std::initializer_list<int> aIdx) {
        std::vector<bool> aVec(14, false);
        for (int i : aIdx)
            aVec[i] = true;
        return aVec;
    };
    auto marking = [&aPolicy](const OUString& rClass, const std::vector<bool>& rSel) {
        return aPolicy.deriveMarking(aPolicy.buildLabel(rClass, rSel, OUString(), OUString()));
    };

    CPPUNIT_ASSERT_EQUAL(u"NATO RESTRICTED"_ustr, marking(u"RESTRICTED"_ustr, sel({ 0 })));
    CPPUNIT_ASSERT_EQUAL(u"NATO CONFIDENTIAL-STAFF"_ustr,
                         marking(u"CONFIDENTIAL"_ustr, sel({ 0, 3 })));
    CPPUNIT_ASSERT_EQUAL(u"NATO RESTRICTED Releasable to Japan, Australia, PFP"_ustr,
                         marking(u"RESTRICTED"_ustr, sel({ 0, 9, 10, 11 })));
    CPPUNIT_ASSERT_EQUAL(u"NATO/KFOR CONFIDENTIAL NATO, Ireland, Ukraine Only"_ustr,
                         marking(u"CONFIDENTIAL"_ustr, sel({ 1, 4, 5, 6 })));
    CPPUNIT_ASSERT_EQUAL(u"NATO SECRET Norway, United States Only Releasable to Sweden"_ustr,
                         marking(u"SECRET"_ustr, sel({ 0, 7, 8, 12 })));
    CPPUNIT_ASSERT_EQUAL(u"NATO/EAPC CONFIDENTIAL Releasable to RESOLUTE SUPPORT"_ustr,
                         marking(u"CONFIDENTIAL"_ustr, sel({ 2, 13 })));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SpifPolicyTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
