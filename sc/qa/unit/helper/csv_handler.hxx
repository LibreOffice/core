/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *  Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <iostream>

#include "docsh.hxx"
#include "postit.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "cellform.hxx"

#define DEBUG_CSV_HANDLER 0

namespace {

rtl::OUString getConditionalFormatString(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    rtl::OUString aString;
    Color* pColor;
    ScBaseCell* pCell = pDoc->GetCell(ScAddress(nCol, nRow, nTab));
    const SfxItemSet* pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
    const ScPatternAttr* pPattern = pDoc->GetPattern(nCol, nRow, nTab);
    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
    ScCellFormat::GetString( pCell, nFormat, aString, &pColor, *pFormatter);
    return aString;
}

rtl::OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    rtl::OStringBuffer aString("Error in Table: ");
    aString.append(static_cast<sal_Int32>(nTab));
    aString.append(" Column: ");
    aString.append(static_cast<sal_Int32>(nCol));
    aString.append(" Row: ");
    aString.append(nRow);
    return aString.makeStringAndClear();
}

rtl::OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab, const rtl::OUString& rExpectedString, const rtl::OUString& rString)
{
    rtl::OStringBuffer aString(createErrorMessage(nCol, nRow, nTab));
    aString.append("; Expected: '");
    aString.append(rtl::OUStringToOString(rExpectedString, RTL_TEXTENCODING_UTF8));
    aString.append("' Found: '");
    aString.append(rtl::OUStringToOString(rString, RTL_TEXTENCODING_UTF8));
    aString.append("'");
    return aString.makeStringAndClear();
}

rtl::OString createErrorMessage(SCCOL nCol, SCROW nRow, SCTAB nTab, double aExpected, double aValue)
{
    rtl::OStringBuffer aString(createErrorMessage(nCol, nRow, nTab));
    aString.append("; Expected: '");
    aString.append(aExpected);
    aString.append("' Found: '");
    aString.append(aValue);
    aString.append("'");
    return aString.makeStringAndClear();

}

}

enum StringType { PureString, FormulaValue, StringValue };

class csv_handler
{
public:
    csv_handler(ScDocument* pDoc, SCTAB nTab, StringType eType = StringValue):
            mpDoc(pDoc),
            mnCol(0),
            mnRow(0),
            mnTab(nTab),
            meStringType(eType)  {}

    void begin_parse() {}

    void end_parse() {}

    void begin_row() {}

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
            rtl::OUString aCSVString(p, n, RTL_TEXTENCODING_UTF8);
            rtl::OUString aString;
            mpDoc->GetString(mnCol, mnRow, mnTab, aString);

#if DEBUG_CSV_HANDLER
                std::cout << "String: " << rtl::OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
                std::cout << "CSVString: " << rtl::OUStringToOString(aCSVString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
                std::cout << "result: " << (int)(aCSVString == aString) << std::endl;
#endif //DEBUG_CSV_HANDLER

            CPPUNIT_ASSERT_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab, aCSVString, aString).getStr(), aString == aCSVString);
        }
        else
        {
            char* pRemainingChars = NULL;
            std::string aStr(p, n);
            double nValue = strtod(&aStr[0], &pRemainingChars);
            if (*pRemainingChars)
            {
                rtl::OUString aString;
                switch (meStringType)
                {
                    case StringValue:
                        mpDoc->GetString(mnCol, mnRow, mnTab, aString);
                        break;
                    case FormulaValue:
                        mpDoc->GetFormula(mnCol, mnRow, mnTab, aString);
                        break;
                    default:
                        break;
                }
                rtl::OUString aCSVString(p, n, RTL_TEXTENCODING_UTF8);
#if DEBUG_CSV_HANDLER
                std::cout << "String: " << rtl::OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
                std::cout << "CSVString: " << rtl::OUStringToOString(aCSVString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
                std::cout << "result: " << (int)(aCSVString == aString) << std::endl;
#endif //DEBUG_CSV_HANDLER

                CPPUNIT_ASSERT_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab, aCSVString, aString).getStr(), aString == aCSVString);
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
                CPPUNIT_ASSERT_MESSAGE(createErrorMessage(mnCol, mnRow, mnTab, nValue, aValue).getStr(), aValue == nValue);
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

    void begin_parse() {}

    void end_parse() {}

    void begin_row() {}

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
        rtl::OUString aString = getConditionalFormatString(mpDoc, mnCol, mnRow, mnTab);
        rtl::OUString aCSVString(p, n, RTL_TEXTENCODING_UTF8);
#if DEBUG_CSV_HANDLER
        std::cout << "String: " << rtl::OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
        std::cout << "CSVString: " << rtl::OUStringToOString(aCSVString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
