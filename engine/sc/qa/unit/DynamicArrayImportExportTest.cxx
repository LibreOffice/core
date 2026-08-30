/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <document.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/XPath.hxx>
#include <tools/stream.hxx>

#include <map>
#include <memory>
#include <set>
#include <string_view>

namespace sc
{
namespace
{
// One formula as a sheet part stores it: the text of the f element, the array marking with its
// range, and the cm attribute that points at the dynamic-array metadata.
struct StoredFormula
{
    OUString aText;
    bool bArray = false;
    OUString aRange;
    OUString aMetadata;

    bool operator==(const StoredFormula& rOther) const = default;
};

// The formulas of one sheet part, keyed by the cell reference. An f element with no text of its
// own carries no formula: a shared-group member keeps its text in the master, and the cells a
// spill covers get an empty one. Those are left out.
using SheetFormulas = std::map<OUString, StoredFormula>;

OString toReport(std::u16string_view aText)
{
    return OUStringToOString(aText, RTL_TEXTENCODING_UTF8);
}

OString describe(const StoredFormula& rFormula)
{
    OString aDescription = "\"" + toReport(rFormula.aText) + "\"";
    if (rFormula.bArray)
        aDescription += " array over " + toReport(rFormula.aRange);
    if (!rFormula.aMetadata.isEmpty())
        aDescription += " cm=" + toReport(rFormula.aMetadata);
    return aDescription;
}

// Fail listing every cell the export wrote differently than the input file has it. A cell in
// rIgnored is a difference we know about, and one that stops differing is reported too, so the
// list keeps naming the differences that are really there.
void assertSameStoredFormulas(const SheetFormulas& rInput, const SheetFormulas& rWritten,
                              const OUString& rSheetPath, const std::set<OUString>& rIgnored)
{
    OString aReport;
    auto appendLine = [&aReport](const OString& rLine) {
        if (!aReport.isEmpty())
            aReport += "\n";
        aReport += rLine;
    };
    const OString aSheet = toReport(rSheetPath);

    for (const auto& rEntry : rInput)
    {
        const bool bIgnored = rIgnored.find(rEntry.first) != rIgnored.end();
        auto aWritten = rWritten.find(rEntry.first);
        if (aWritten == rWritten.end())
        {
            if (!bIgnored)
                appendLine(aSheet + " " + toReport(rEntry.first) + ": no formula written, the "
                           "input has " + describe(rEntry.second));
            continue;
        }
        if (aWritten->second == rEntry.second)
        {
            if (bIgnored)
                appendLine(aSheet + " " + toReport(rEntry.first)
                           + ": comes out as the input has it now, take it off the list");
            continue;
        }
        if (!bIgnored)
            appendLine(aSheet + " " + toReport(rEntry.first) + ": the input has "
                       + describe(rEntry.second) + ", we wrote " + describe(aWritten->second));
    }
    for (const auto& rEntry : rWritten)
    {
        if (rInput.find(rEntry.first) == rInput.end()
            && rIgnored.find(rEntry.first) == rIgnored.end())
        {
            appendLine(aSheet + " " + toReport(rEntry.first) + ": a formula appeared, "
                       + describe(rEntry.second));
        }
    }

    if (aReport.isEmpty())
        return;
    OString aLabel = "Formulas the export wrote differently:\n" + aReport;
    CPPUNIT_FAIL(aLabel.getStr());
}
}

class DynamicArrayImportExportTest : public ScModelTestBase
{
public:
    DynamicArrayImportExportTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }

protected:
    SheetFormulas collectStoredFormulas(const xmlDocUniquePtr& pSheet);
    SheetFormulas readInputSheet(std::u16string_view aInputName, const OUString& rSheetPath);
};

SheetFormulas DynamicArrayImportExportTest::collectStoredFormulas(const xmlDocUniquePtr& pSheet)
{
    SheetFormulas aFormulas;
    tools::XPath aXPath(pSheet.get(),
                        [this](xmlXPathContextPtr pContext) { registerNamespaces(pContext); });
    std::unique_ptr<tools::XPathObject> pCells = aXPath.create("//x:sheetData/x:row/x:c");
    CPPUNIT_ASSERT(pCells);
    for (int nIndex = 0; nIndex < pCells->count(); ++nIndex)
    {
        const OUString aReference = pCells->at(nIndex)->attribute("r");
        const OString aPath = "//x:sheetData/x:row/x:c[@r='"
                              + OUStringToOString(aReference, RTL_TEXTENCODING_UTF8) + "']/x:f";
        std::unique_ptr<tools::XPathObject> pFormula = aXPath.create(aPath);
        if (!pFormula || pFormula->count() != 1)
            continue;

        StoredFormula aFormula;
        aFormula.aText = pFormula->content();
        if (aFormula.aText.isEmpty())
            continue;
        aFormula.aMetadata = pCells->at(nIndex)->attribute("cm");
        aFormula.bArray = pFormula->attribute("t") == "array";
        // A shared-formula group also carries a ref, over the cells of the group. Only the
        // range of an array formula is part of what the formula means.
        if (aFormula.bArray)
            aFormula.aRange = pFormula->attribute("ref");
        aFormulas.emplace(aReference, aFormula);
    }
    return aFormulas;
}

// The formulas one sheet part of the input file holds.
SheetFormulas DynamicArrayImportExportTest::readInputSheet(std::u16string_view aInputName,
                                                           const OUString& rSheetPath)
{
    std::unique_ptr<SvStream> pStream(parseExportStream(createFileURL(aInputName), rSheetPath));
    xmlDocUniquePtr pSheet = parseXmlStream(pStream.get());
    return collectStoredFormulas(pSheet);
}

CPPUNIT_TEST_FIXTURE(DynamicArrayImportExportTest, testDroppedImplicitIntersectionIsPutBack)
{
    // OOXML expresses the @ of a plain =@ref# cell by leaving it out, so the import puts it
    // back. An array formula does not reduce its operand and keeps the spelling it was saved
    // with.
    createScDoc("functions/dynamic_array/xlsx/DynamicArrayFixture.xlsx");
    ScDocument* pDocument = getScDoc();

    // "Implicit intersection Operator" F44 is a plain _xlfn.ANCHORARRAY($G$45).
    CPPUNIT_ASSERT_EQUAL(u"=@$G$45#"_ustr, pDocument->GetFormula(5, 43, 1));

    // "Spill Operator" A34 is the same call as an array formula.
    CPPUNIT_ASSERT_EQUAL(u"=$H$5#"_ustr, pDocument->GetFormula(0, 33, 2));
}

CPPUNIT_TEST_FIXTURE(DynamicArrayImportExportTest, testDynamicArrayFixtureKeepsFormulaSpelling)
{
    // The export writes every formula the way the input file spells it, apart from the cells
    // listed here. Those come out with the same value but another spelling, and most of them
    // are something to fix.
    static constexpr std::u16string_view aFixture(
        u"functions/dynamic_array/xlsx/DynamicArrayFixture.xlsx");
    createScDoc("functions/dynamic_array/xlsx/DynamicArrayFixture.xlsx");
    // The spilling cells take their shape from a calculation, so the export sees what a user
    // would.
    getScDoc()->CalcAll();

    const OUString aSheet1 = u"xl/worksheets/sheet1.xml"_ustr;
    const OUString aSheet2 = u"xl/worksheets/sheet2.xml"_ustr;
    const OUString aSheet3 = u"xl/worksheets/sheet3.xml"_ustr;
    const SheetFormulas aInput1 = readInputSheet(aFixture, aSheet1);
    const SheetFormulas aInput2 = readInputSheet(aFixture, aSheet2);
    const SheetFormulas aInput3 = readInputSheet(aFixture, aSheet3);

    saveAndReload(TestFilter::XLSX);

    // TRUE comes out as TRUE(), and a range over one cell as the cell.
    assertSameStoredFormulas(aInput1, collectStoredFormulas(parseExport(aSheet1)), aSheet1,
        {
            u"A17"_ustr, u"A46"_ustr, u"B34"_ustr
        });
    // The @ of a single-cell array formula imports as a plain cell, so the array marking goes,
    // and parentheses the import strips stay stripped.
    assertSameStoredFormulas(aInput2, collectStoredFormulas(parseExport(aSheet2)), aSheet2,
        {
            u"A28"_ustr, u"A29"_ustr, u"A30"_ustr, u"A32"_ustr, u"A33"_ustr, u"A34"_ustr,
            u"I7"_ustr, u"I8"_ustr, u"I9"_ustr, u"I10"_ustr, u"J7"_ustr, u"J8"_ustr, u"J9"_ustr,
            u"J10"_ustr
        });
    // A space in front of an operand comes out in front of the call it is wrapped in.
    assertSameStoredFormulas(aInput3, collectStoredFormulas(parseExport(aSheet3)), aSheet3,
        {
            u"A46"_ustr, u"A50"_ustr, u"A81"_ustr, u"A85"_ustr, u"A89"_ustr, u"F105"_ustr
        });
}

CPPUNIT_TEST_FIXTURE(DynamicArrayImportExportTest, testUnionSpillIntersectionKeepsFormulaSpelling)
{
    // The same over the union, spill and intersection operator combinations. The listed cells
    // are the union list whose part carries an operator, TRUE as TRUE(), the name of an unknown
    // function in lower case, and the parentheses and array marking of a single-cell @.
    static constexpr std::u16string_view aFixture(
        u"functions/dynamic_array/xlsx/UnionSpillIntersectionOperatorTest.xlsx");
    createScDoc("functions/dynamic_array/xlsx/UnionSpillIntersectionOperatorTest.xlsx");
    getScDoc()->CalcAll();

    const OUString aSheet = u"xl/worksheets/sheet1.xml"_ustr;
    const SheetFormulas aInput = readInputSheet(aFixture, aSheet);

    saveAndReload(TestFilter::XLSX);

    assertSameStoredFormulas(aInput, collectStoredFormulas(parseExport(aSheet)), aSheet,
        {
            u"A51"_ustr, u"F67"_ustr, u"F68"_ustr, u"F71"_ustr, u"F75"_ustr, u"F77"_ustr,
            u"F80"_ustr, u"F89"_ustr, u"F91"_ustr, u"F97"_ustr, u"F110"_ustr, u"G26"_ustr,
            u"K112"_ustr, u"K113"_ustr, u"K114"_ustr, u"K115"_ustr, u"L112"_ustr, u"L113"_ustr,
            u"L114"_ustr, u"L115"_ustr, u"M6"_ustr, u"N59"_ustr, u"N112"_ustr, u"N113"_ustr,
            u"N114"_ustr, u"N115"_ustr, u"O59"_ustr
        });
}

} // namespace sc

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
