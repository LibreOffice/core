/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_QA_UNIT_HELPER_CSV_HANDLER_HXX
#define INCLUDED_SC_QA_UNIT_HELPER_CSV_HANDLER_HXX

#include <iostream>

#include "docsh.hxx"
#include "postit.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "cellform.hxx"
#include "cellvalue.hxx"

#include <rtl/strbuf.hxx>

#include <test/bootstrapfixture.hxx>

#define DEBUG_CSV_HANDLER 0

namespace {

OUString getConditionalFormatString(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    OUString aString;
    Color* pColor;
    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, nTab));
    if (aCell.isEmpty())
        return aString;

    const SfxItemSet* pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
    const ScPatternAttr* pPattern = pDoc->GetPattern(nCol, nRow, nTab);
    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
    aString = ScCellFormat::GetString(*pDoc, ScAddress(nCol, nRow, nTab), nFormat, &pColor, *pFormatter);
    return aString;
}

OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    OStringBuffer aString("Error in Table: ");
    aString.append(static_cast<sal_Int32>(nTab));
    aString.append(" Column: ");
    aString.append(static_cast<sal_Int32>(nCol));
    aString.append(" Row: ");
    aString.append(nRow);
    return aString.makeStringAndClear();
}

OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab, const OUString& rExpectedString, const OUString& rString)
{
    OStringBuffer aString(createErrorMessage(nCol, nRow, nTab));
    aString.append("; Expected: '");
    aString.append(OUStringToOString(rExpectedString, RTL_TEXTENCODING_UTF8));
    aString.append("' Found: '");
    aString.append(OUStringToOString(rString, RTL_TEXTENCODING_UTF8));
    aString.append("'");
    return aString.makeStringAndClear();
}

OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab, double aExpected, double aValue)
{
    OStringBuffer aString(createErrorMessage(nCol, nRow, nTab));
    aString.append("; Expected: '");
    aString.append(aExpected);
    aString.append("' Found: '");
    aString.append(aValue);
    aString.append("'");
    return aString.makeStringAndClear();

}

}

class csv_handler
{
public:
    csv_handler(ScDocument* pDoc, SCTAB nTab, StringType eType = StringValue):
            mpDoc(pDoc),
            mnCol(0),
            mnRow(0),
            mnTab(nTab),
            meStringType(eType)  {}

    static void begin_parse() {}

    static void end_parse() {}

    static void begin_row() {}

    void end_row()
    {
        ++mnRow;
        mnCol = 0;
    }

    void cell(const char* p, size_t n)
    {
#if DEBUG_CSV_HANDLER
        std::cout << "Col: " << mnCol << " Row: " << mnRow << std::endl;
#endif //DEBUG_CSV_HANDLER
        if (n == 0)
        {
            // Empty cell.
            if (!mpDoc->GetString(mnCol, mnRow, mnTab).isEmpty())
            {
                // cell in the document is not empty.
                CPPUNIT_ASSERT_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab).getStr(), false);
            }
        }
        else if (meStringType == PureString)
        {
            OUString aCSVString(p, n, RTL_TEXTENCODING_UTF8);
            OUString aString = mpDoc->GetString(mnCol, mnRow, mnTab);

#if DEBUG_CSV_HANDLER
                std::cout << "String: " << OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
                std::cout << "CSVString: " << OUStringToOString(aCSVString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
                std::cout << "result: " << (int)(aCSVString == aString) << std::endl;
#endif //DEBUG_CSV_HANDLER

            CPPUNIT_ASSERT_EQUAL_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab, aCSVString, aString).getStr(), aCSVString, aString);
        }
        else
        {
            char* pRemainingChars = nullptr;
            std::string aStr(p, n);
            double nValue = strtod(&aStr[0], &pRemainingChars);
            if (*pRemainingChars)
            {
                OUString aString;
                switch (meStringType)
                {
                    case StringValue:
                        aString = mpDoc->GetString(mnCol, mnRow, mnTab);
                        break;
                    case FormulaValue:
                        mpDoc->GetFormula(mnCol, mnRow, mnTab, aString);
                        break;
                    default:
                        break;
                }
                OUString aCSVString(p, n, RTL_TEXTENCODING_UTF8);
#if DEBUG_CSV_HANDLER
                std::cout << "String: " << OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
                std::cout << "CSVString: " << OUStringToOString(aCSVString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
                std::cout << "result: " << (int)(aCSVString == aString) << std::endl;
#endif //DEBUG_CSV_HANDLER

                CPPUNIT_ASSERT_EQUAL_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab, aCSVString, aString).getStr(), aCSVString, aString);
            }
            else
            {
                double aValue;
                mpDoc->GetValue(mnCol, mnRow, mnTab, aValue);
#if DEBUG_CSV_HANDLER
                std::cout << "Value: " << aValue << std::endl;
                std::cout << "CSVValue: " << nValue << std::endl;
                std::cout << "result: " << (int)(aValue == nValue) << std::endl;
#endif //DEBUG_CSV_HANDLER
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab, nValue, aValue).getStr(), nValue, aValue, 1e-10);
            }
        }
        ++mnCol;
    }

private:
    ScDocument* mpDoc;
    SCCOL mnCol;
    SCROW mnRow;
    SCTAB mnTab;
    StringType meStringType;
};

class conditional_format_handler
{
public:
    conditional_format_handler(ScDocument* pDoc, SCTAB nTab):
        mpDoc(pDoc),
        mnCol(0),
        mnRow(0),
        mnTab(nTab) {}

    static void begin_parse() {}

    static void end_parse() {}

    static void begin_row() {}

    void end_row()
    {
        ++mnRow;
        mnCol = 0;
    }

    void cell(const char* p, size_t n)
    {
#if DEBUG_CSV_HANDLER
        std::cout << "Col: " << mnCol << " Row: " << mnRow << std::endl;
#endif //DEBUG_CSV_HANDLER
        OUString aString = getConditionalFormatString(mpDoc, mnCol, mnRow, mnTab);
        OUString aCSVString(p, n, RTL_TEXTENCODING_UTF8);
#if DEBUG_CSV_HANDLER
        std::cout << "String: " << OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
        std::cout << "CSVString: " << OUStringToOString(aCSVString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
        std::cout << "result: " << (int)(aCSVString == aString) << std::endl;
#endif //DEBUG_CSV_HANDLER
        CPPUNIT_ASSERT_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab, aCSVString, aString).getStr(), aString == aCSVString );
        ++mnCol;
    }

private:
    ScDocument* mpDoc;
    SCCOL mnCol;
    SCROW mnRow;
    SCTAB mnTab;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
