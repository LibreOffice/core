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

    void testCellConversion();

    CPPUNIT_TEST_SUITE(UnitsTest);

    CPPUNIT_TEST(testUTUnit);
    CPPUNIT_TEST(testUnitVerification);

    CPPUNIT_TEST(testUnitFromFormatStringExtraction);
    CPPUNIT_TEST(testUnitValueStringSplitting);
    CPPUNIT_TEST(testUnitFromHeaderExtraction);

    CPPUNIT_TEST(testCellConversion);

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

    // Set separators for formulae - for now just argument separators (no matrix separators)
    ScFormulaOptions aOptions;
    aOptions.SetFormulaSepArg(",");
    m_xDocShRef->SetFormulaOptions(aOptions);
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

    // Do some simple conversion testing
    UtUnit aCM;
    UtUnit::createUnit("cm", aCM, mpUnitsImpl->mpUnitSystem);
    CPPUNIT_ASSERT(aCM.areConvertibleTo(aM));
    CPPUNIT_ASSERT(!aCM.areConvertibleTo(aS));
    CPPUNIT_ASSERT(aCM.convertValueTo(0.0, aM) == 0.0); // 0 converts to 0
    CPPUNIT_ASSERT(aCM.convertValueTo(100.0, aM) == 1.0);
}

void UnitsTest::testUnitVerification() {
    // Make sure we have at least one tab to work with
    mpDoc->EnsureTable(0);

    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    sal_uInt32 nKeyCM, nKeyM, nKeyKG, nKeyS, nKeyCM_S;

    // Used to return position of error in input string for PutEntry
    // -- not needed here.
    sal_Int32 nCheckPos;

    short nType = css::util::NumberFormat::DEFINED;

    OUString sM = "#\"m\"";
    pFormatter->PutEntry(sM, nCheckPos, nType, nKeyM);
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

    // 4th column: 5cm/s, 10cm/s, 15cm/s
    address = ScAddress(3, 0, 0);
    mpDoc->SetNumberFormat(address, nKeyCM_S);
    mpDoc->SetValue(address, 5);

    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyCM_S);
    mpDoc->SetValue(address, 10);

    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyCM_S);
    mpDoc->SetValue(address, 15);

    // 5th column: 1m
    address = ScAddress(4, 0, 0);
    mpDoc->SetNumberFormat(address, nKeyM);
    mpDoc->SetValue(address, 1);

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

    // Test that addition of scaled units works (cm + 100*m)
    address = ScAddress(0, 10, 0);
    mpDoc->SetFormula(address, "=A1+100*E1");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));
    // 10cm + 100*1m = 110cm
    CPPUNIT_ASSERT(mpDoc->GetValue(address) == 110);

    // But addition of them unscaled fails (cm + m)
    address = ScAddress(0, 11, 0);
    mpDoc->SetFormula(address, "=A1+E1");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // As does wrong scaling (cm+m/50)
    address = ScAddress(0, 12, 0);
    mpDoc->SetFormula(address, "=A1+E1/50");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // And scaling doesn't help when adding incompatible units (kg + 100*m)
    address = ScAddress(0, 13, 0);
    mpDoc->SetFormula(address, "=B1+100*E1");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // Test Ranges:
    // SUM("kg")
    address.IncRow();
    mpDoc->SetFormula(address, "=SUM(B1:B3)");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // SUM("cm"&"kg")
    address.IncRow();
    mpDoc->SetFormula(address, "=SUM(A1:B3)");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // SUM("cm"&"kg") - multiple ranges
    address.IncRow();
    // TODO: by default / when testing: separator is...?
    mpDoc->SetFormula(address, "=SUM(A1:A3,B1:B3)");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // SUM("cm")+SUM("kg")
    address.IncRow();
    mpDoc->SetFormula(address, "=SUM(A1:A3)+SUM(B1:B3)");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // SUM("cm")/SUM("s")+SUM("cm/s")
    address.IncRow();
    mpDoc->SetFormula(address, "=SUM(A1:A3)/SUM(C1:C3)+SUM(D1:D3)");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // PRODUCT("cm/","s")+"cm"
    address.IncRow();
    mpDoc->SetFormula(address, "=PRODUCT(C1:D1)+A1");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // PRODUCT("cm/","s")+"kg"
    address.IncRow();
    mpDoc->SetFormula(address, "=PRODUCT(C1:D1)+B1");
    pCell = mpDoc->GetFormulaCell(address);
    pTokens = pCell->GetCode();
    CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

    // Do a quick sanity check for all the other supported functions
    // The following all expect identically united inputs, and should
    // preserve that unit:
    std::vector<OUString> aPreservingFuncs{ "SUM", "AVERAGE", "MIN", "MAX" };
    for (const OUString& aFunc: aPreservingFuncs) {
        // FOO(cm) + cm
        address.IncRow();
        mpDoc->SetFormula(address, "=" + aFunc + "(A1:A2)+A3");
        pCell = mpDoc->GetFormulaCell(address);
        pTokens = pCell->GetCode();
        CPPUNIT_ASSERT(mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));

        // FOO(cm) + kg
        address.IncRow();
        mpDoc->SetFormula(address, "=" + aFunc + "(A1:A2)+B3");
        pCell = mpDoc->GetFormulaCell(address);
        pTokens = pCell->GetCode();
        CPPUNIT_ASSERT(!mpUnitsImpl->verifyFormula(pTokens, address, mpDoc));
    }
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
    OUString sUnitString;

    OUString sEmpty = "";
    CPPUNIT_ASSERT(!mpUnitsImpl->extractUnitFromHeaderString(sEmpty, aUnit, sUnitString));
    CPPUNIT_ASSERT(aUnit == UtUnit());
    CPPUNIT_ASSERT(sUnitString.isEmpty());

    OUString sSimple = "bla bla [cm/s]";
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitFromHeaderString(sSimple, aUnit, sUnitString));
    // We need to test in Units (rather than testing Unit::getString()) as
    // any given unit can have multiple string representations (and utunits defaults to
    // representing e.g. cm as (I think) "0.01m").
    UtUnit aTestUnit;
    CPPUNIT_ASSERT(UtUnit::createUnit("cm/s", aTestUnit, mpUnitsImpl->mpUnitSystem));
    CPPUNIT_ASSERT(aUnit == aTestUnit);
    CPPUNIT_ASSERT(sUnitString == "cm/s");

    OUString sFreeStanding = "bla bla kg/h";
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitFromHeaderString(sFreeStanding, aUnit, sUnitString));
    CPPUNIT_ASSERT(UtUnit::createUnit("kg/h", aTestUnit, mpUnitsImpl->mpUnitSystem));
    CPPUNIT_ASSERT(aUnit == aTestUnit);
    CPPUNIT_ASSERT(sUnitString == "kg/h");

    OUString sFreeStandingWithSpaces = "bla bla m / s";
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitFromHeaderString(sFreeStandingWithSpaces, aUnit, sUnitString));
    CPPUNIT_ASSERT(UtUnit::createUnit("m/s", aTestUnit, mpUnitsImpl->mpUnitSystem));
    CPPUNIT_ASSERT(aUnit == aTestUnit);
    CPPUNIT_ASSERT(sUnitString == "m/s");

    OUString sOperatorSeparated = "bla bla / t/s";
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitFromHeaderString(sOperatorSeparated, aUnit, sUnitString));
    CPPUNIT_ASSERT(UtUnit::createUnit("t/s", aTestUnit, mpUnitsImpl->mpUnitSystem));
    CPPUNIT_ASSERT(aUnit == aTestUnit);
    CPPUNIT_ASSERT(sUnitString == "t/s");


    OUString sRoundBrackets = "bla bla (t/h)";
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitFromHeaderString(sRoundBrackets, aUnit, sUnitString));
    CPPUNIT_ASSERT(UtUnit::createUnit("t/h", aTestUnit, mpUnitsImpl->mpUnitSystem));
    CPPUNIT_ASSERT(aUnit == aTestUnit);
    CPPUNIT_ASSERT(sUnitString == "(t/h)");

    // This becomes more of a nightmare to support, so let's not bother for now.
    // OUString sFreeStandingMixedSpaces = "bla bla m /s* kg";
    // CPPUNIT_ASSERT(mpUnitsImpl->extractUnitFromHeaderString(sFreeStanding, aUnit, sUnitString));
    // CPPUNIT_ASSERT(UtUnit::createUnit("m/s", aTestUnit, mpUnitsImpl->mpUnitSystem));
    // CPPUNIT_ASSERT(aUnit == aTestUnit);
}

void UnitsTest::testCellConversion() {
    // We test both isCellConversionRecommended, and convertCellToHeaderUnit
    // since their arguments are essentially shared / dependent.
    mpDoc->EnsureTable(0);

    // Set up a column with a normal header and a few data values
    ScAddress address(20, 0, 0);
    mpDoc->SetString(address, "length [m]");

    address.IncRow();
    mpDoc->SetValue(address, 1);
    address.IncRow();
    mpDoc->SetValue(address, 2);
    address.IncRow();
    mpDoc->SetValue(address, 3);

    ScAddress aHeaderAddress;
    OUString sHeaderUnit, sCellUnit;

    // Test that we don't expect conversion for an non-united value cell
    CPPUNIT_ASSERT(!mpUnitsImpl->isCellConversionRecommended(address, mpDoc, sHeaderUnit, aHeaderAddress, sCellUnit));
    CPPUNIT_ASSERT(sHeaderUnit.isEmpty());
    CPPUNIT_ASSERT(sCellUnit.isEmpty());
    CPPUNIT_ASSERT(!aHeaderAddress.IsValid());

    // And now set up cells with local units
    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    sal_uInt32 nKeyCM, nKeyKG;

    sal_Int32 nCheckPos; // Passed by reference - unused
    short nType = css::util::NumberFormat::DEFINED;

    OUString sCM = "#\"cm\"";
    pFormatter->PutEntry(sCM, nCheckPos, nType, nKeyCM);
    OUString sKG = "#\"kg\"";
    pFormatter->PutEntry(sKG, nCheckPos, nType, nKeyKG);

    // First united cell: "cm" (convertible to "m")
    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyCM);
    mpDoc->SetValue(address, 100);

    CPPUNIT_ASSERT(mpUnitsImpl->isCellConversionRecommended(address, mpDoc, sHeaderUnit, aHeaderAddress, sCellUnit));
    CPPUNIT_ASSERT(sHeaderUnit == "m");
    CPPUNIT_ASSERT(sCellUnit == "cm");
    CPPUNIT_ASSERT(aHeaderAddress == ScAddress(20, 0, 0));

    // Test conversion (From cm to m)
    CPPUNIT_ASSERT(mpUnitsImpl->convertCellToHeaderUnit(address, mpDoc, sHeaderUnit, sCellUnit));
    CPPUNIT_ASSERT(mpDoc->GetValue(address) == 1);
    CPPUNIT_ASSERT(mpDoc->GetNumberFormat(address) == 0);

    // Second united cell: "kg" (not convertible to "m")
    address.IncRow();
    mpDoc->SetNumberFormat(address, nKeyKG);
    mpDoc->SetValue(address, 50);

    CPPUNIT_ASSERT(!mpUnitsImpl->isCellConversionRecommended(address, mpDoc, sHeaderUnit, aHeaderAddress, sCellUnit));
    CPPUNIT_ASSERT(sHeaderUnit.isEmpty());
    CPPUNIT_ASSERT(sCellUnit.isEmpty());
    CPPUNIT_ASSERT(!aHeaderAddress.IsValid());

    // We specifically don't test conversion with invalid units since that would be nonsensical
    // (i.e. would require us passing in made up arguments, where it's specifically necessary
    // to pass in the output of isCellConversionRecommended).
}

CPPUNIT_TEST_SUITE_REGISTRATION(UnitsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

}}} // namespace sc::units::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
