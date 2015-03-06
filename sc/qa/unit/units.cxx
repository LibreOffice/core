/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unitsimpl.hxx"
#include "utunit.hxx"

#include "formulacell.hxx"

#include "helper/qahelper.hxx"

#include <com/sun/star/util/NumberFormat.hpp>

using namespace sc::units;

// In order to be able to access the private members of UnitsImpl for
// testing, we need to be a friend of UnitsImpl. For this to work
// UnitsTest can't be a member of the anonymous namespace hence the
// need to use a namespace here.
namespace sc {
namespace units {
namespace test {

class UnitsTest:
    public ::test::BootstrapFixture
{
public:
    UnitsTest() {};
    virtual ~UnitsTest() {};

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    ::boost::shared_ptr< UnitsImpl > mpUnitsImpl;


    void testUTUnit();
    void testUnitVerification();

    void testUnitFromFormatStringExtraction();
    void testUnitValueStringSplitting();

    void testUnitFromHeaderExtraction();

    CPPUNIT_TEST_SUITE(UnitsTest);

    CPPUNIT_TEST(testUTUnit);
    CPPUNIT_TEST(testUnitVerification);

    CPPUNIT_TEST(testUnitFromFormatStringExtraction);
    CPPUNIT_TEST(testUnitValueStringSplitting);
    CPPUNIT_TEST(testUnitFromHeaderExtraction);

    CPPUNIT_TEST_SUITE_END();

private:
    ScDocument *mpDoc;
    ScDocShellRef m_xDocShRef;
};

void UnitsTest::setUp() {
    BootstrapFixture::setUp();

    ScDLL::Init();
    m_xDocShRef = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);

    mpDoc = &m_xDocShRef->GetDocument();

    mpUnitsImpl = UnitsImpl::GetUnits();
}

void UnitsTest::tearDown() {
    m_xDocShRef.Clear();
    BootstrapFixture::tearDown();
}

void UnitsTest::testUTUnit() {
    // Test that we can create units.
    UtUnit aDimensionless;
    CPPUNIT_ASSERT(UtUnit::createUnit("", aDimensionless, mpUnitsImpl->mpUnitSystem));
    // And test that an empty string does in fact map to the dimensionless unit one.
    // The documentation states that ut_is_dimensionless returns zero for dimensionless
    // units, however the sources (and udunits2's unit tests) suggest that zero is returned
    // for a unit WITH dimensions (as the method name would suggest).
    CPPUNIT_ASSERT(ut_is_dimensionless(aDimensionless.mpUnit.get()) != 0);

    // Test that we can't create garbage units
    UtUnit aGarbage;
    CPPUNIT_ASSERT(!UtUnit::createUnit("garbage", aGarbage, mpUnitsImpl->mpUnitSystem));

    // Do some addition, subtraction, comparison tests.
    UtUnit aM;
    UtUnit::createUnit("m", aM, mpUnitsImpl->mpUnitSystem);
    UtUnit aS;
    UtUnit::createUnit("s", aS, mpUnitsImpl->mpUnitSystem);
    UtUnit aM_S;
    UtUnit::createUnit("m/s", aM_S, mpUnitsImpl->mpUnitSystem);

    CPPUNIT_ASSERT(aM_S*aS == aM);
    CPPUNIT_ASSERT(aM/aS == aM_S);
}

void UnitsTest::testUnitVerification() {
    // Make sure we have at least one tab to work with
    mpDoc->EnsureTable(0);

    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    sal_uInt32 nKeyCM, nKeyKG, nKeyS, nKeyCM_S;

    // Used to return position of error in input string for PutEntry
    // -- not needed here.
    sal_Int32 nCheckPos;

    short nType = css::util::NumberFormat::DEFINED;

    OUString sCM = "#\"cm\"";
    pFormatter->PutEntry(sCM, nCheckPos, nType, nKeyCM);
    OUString sKG = "#\"kg\"";
    pFormatter->PutEntry(sKG, nCheckPos, nType, nKeyKG);
    OUString sS = "#\"s\"";
    pFormatter->PutEntry(sS, nCheckPos, nType, nKeyS);
    OUString sCM_S = "#\"cm/s\"";
    pFormatter->PutEntry(sCM_S, nCheckPos, nType, nKeyCM_S);

    // 1st column: 10cm, 20cm, 30cm
    ScAddress address(0, 0, 0);
    mpDoc->SetNumberFormat(address, nKeyCM);
    mpDoc->SetValue(address, 10);

    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyCM);
    mpDoc->SetValue(address, 20);

    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyCM);
    mpDoc->SetValue(address, 30);

    // 2nd column: 1kg, 2kg, 3kg
    address = ScAddress(1, 0, 0);
    mpDoc->SetNumberFormat(address, nKeyKG);
    mpDoc->SetValue(address, 1);

    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyKG);
    mpDoc->SetValue(address, 2);

    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyKG);
    mpDoc->SetValue(address, 3);

    // 3rd column: 1s, 2s, 3s
    address = ScAddress(2, 0, 0);
    mpDoc->SetNumberFormat(address, nKeyS);
    mpDoc->SetValue(address, 1);

    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyS);
    mpDoc->SetValue(address, 2);

    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyS);
    mpDoc->SetValue(address, 3);

    // 4th column: 5cm/s
    address = ScAddress(3, 0, 0);
    mpDoc->SetNumberFormat(address, nKeyCM_S);
    mpDoc->SetValue(address, 5);

    ScFormulaCell* pCell;
    ScTokenArray* pTokens;

    // Test that addition of the same unit is successful
    address = ScAddress(0, 4, 0);
    mpDoc->SetFormula(address, "=A1+A2");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // Test that addition of different units fails
    address = ScAddress(0, 6, 0);
    mpDoc->SetFormula(address, "=A1+B1");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // Test that addition and multiplication works (i.e. kg*s+kg*s)
    address = ScAddress(0, 7, 0);
    mpDoc->SetFormula(address, "=A1*B1+A2*B2");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // Test another combination (i.e. cm/s+'cm/s')
    address = ScAddress(0, 8, 0);
    mpDoc->SetFormula(address, "=A1/C1+D1");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // Test that another combination fails (cm*kg/s+'cm/s')
    address = ScAddress(0, 9, 0);
    mpDoc->SetFormula(address, "=A1*B1/C1+D1");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));
}

void UnitsTest::testUnitFromFormatStringExtraction() {
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitStringFromFormat("\"weight: \"0.0\"kg\"") == "kg");
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitStringFromFormat("#\"cm\"") == "cm");
}

void UnitsTest::testUnitValueStringSplitting() {
    OUString sValue, sUnit;

    OUString sEmptyString = "";
    CPPUNIT_ASSERT(!mpUnitsImpl->splitUnitsFromInputString(sEmptyString, sValue, sUnit));
    CPPUNIT_ASSERT(sValue.isEmpty());
    CPPUNIT_ASSERT(sUnit.isEmpty());

    OUString sNumberOnlyString = "10";
    CPPUNIT_ASSERT(!mpUnitsImpl->splitUnitsFromInputString(sNumberOnlyString, sValue, sUnit));
    CPPUNIT_ASSERT(sValue == "10");
    CPPUNIT_ASSERT(sUnit.isEmpty());

    OUString sTextOnlyString = "hello world";
    CPPUNIT_ASSERT(!mpUnitsImpl->splitUnitsFromInputString(sTextOnlyString, sValue, sUnit));
    CPPUNIT_ASSERT(sValue == "hello world");
    CPPUNIT_ASSERT(sUnit.isEmpty());

    OUString sDeceptiveInput = "30garbage";
    CPPUNIT_ASSERT(!mpUnitsImpl->splitUnitsFromInputString(sDeceptiveInput, sValue, sUnit));
    CPPUNIT_ASSERT(sValue == "30garbage");
    CPPUNIT_ASSERT(sUnit.isEmpty());

    OUString sUnitOnly = "cm";
    CPPUNIT_ASSERT(!mpUnitsImpl->splitUnitsFromInputString(sUnitOnly, sValue, sUnit));
    CPPUNIT_ASSERT(sValue == "cm");
    CPPUNIT_ASSERT(sUnit.isEmpty());

    OUString sSimpleUnitedValue = "20m/s";
    CPPUNIT_ASSERT(mpUnitsImpl->splitUnitsFromInputString(sSimpleUnitedValue, sValue, sUnit));
    CPPUNIT_ASSERT(sValue == "20");
    CPPUNIT_ASSERT(sUnit == "m/s");

    OUString sMultipleTokens = "40E-4kg";
    CPPUNIT_ASSERT(mpUnitsImpl->splitUnitsFromInputString(sMultipleTokens, sValue, sUnit));
    CPPUNIT_ASSERT(sValue == "40E-4");
    CPPUNIT_ASSERT(sUnit == "kg");
}

void UnitsTest::testUnitFromHeaderExtraction() {
    UtUnit aUnit;

    OUString sEmpty = "";
    CPPUNIT_ASSERT(!mpUnitsImpl->extractUnitFromHeaderString(sEmpty, aUnit));

    OUString sSimple = "bla bla [cm/s]";
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitFromHeaderString(sSimple, aUnit));
    // We need to test in Units (rather than testing Unit::getString()) as
    // any given unit can have multiple string representations (and utunits defaults to
    // representing e.g. cm as (I think) "0.01m").
    UtUnit aTestUnit;
    CPPUNIT_ASSERT(UtUnit::createUnit("cm/s", aTestUnit, mpUnitsImpl->mpUnitSystem));
    CPPUNIT_ASSERT(aUnit == aTestUnit);
}

CPPUNIT_TEST_SUITE_REGISTRATION(UnitsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

}}} // namespace sc::units::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
