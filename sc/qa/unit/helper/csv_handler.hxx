/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <string_view>

#include "qahelper.hxx"

#include <patattr.hxx>
#include <document.hxx>
#include <cellform.hxx>
#include <cellvalue.hxx>

#define DEBUG_CSV_HANDLER 0

inline OUString getConditionalFormatString(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    OUString aString;
    const Color* pColor;
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

inline OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    return "Error in Table: " +
        OString::number(static_cast<sal_Int32>(nTab)) +
        " Column: " +
        OString::number(static_cast<sal_Int32>(nCol)) +
        " Row: " +
        OString::number(nRow);
}

inline OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab, std::u16string_view rExpectedString, std::u16string_view rString)
{
    return createErrorMessage(nCol, nRow, nTab) + "; Expected: '"
        + OUStringToOString(rExpectedString, RTL_TEXTENCODING_UTF8) + "' Found: '"
        + OUStringToOString(rString, RTL_TEXTENCODING_UTF8) + "'";
}

inline OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab, double aExpected, double aValue)
{
    return createErrorMessage(nCol, nRow, nTab) + "; Expected: '" + OString::number(aExpected)
        + "' Found: '" + OString::number(aValue) + "'";

}

class csv_handler
{
public:
    csv_handler(ScDocument* pDoc, SCTAB nTab, StringType eType):
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

    void cell(const char* p, size_t n, bool /*transient*/)
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
        else if (meStringType == StringType::PureString)
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
                    case StringType::StringValue:
                        aString = mpDoc->GetString(mnCol, mnRow, mnTab);
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

    void cell(const char* p, size_t n, bool /*transient*/)
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab, aCSVString, aString).getStr(), aCSVString, aString );
        ++mnCol;
    }

private:
    ScDocument* mpDoc;
    SCCOL mnCol;
    SCROW mnRow;
    SCTAB mnTab;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
