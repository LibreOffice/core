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

#define DEBUG_CSV_HANDLER 0

class csv_handler
{
public:
    csv_handler(ScDocument* pDoc, SCTAB nTab):
            mpDoc(pDoc),
            mnCol(0),
            mnRow(0),
            mnTab(nTab)     {}

    void begin_parse()
    {

    }

    void end_parse()
    {

    }

    void begin_row()
    {

    }

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
        char* pRemainingChars = NULL;
        std::string aStr(p, n);
        double nValue = strtod(&aStr[0], &pRemainingChars);
        if (*pRemainingChars)
        {
            rtl::OUString aString;
            mpDoc->GetString(mnCol, mnRow, mnTab, aString);
            rtl::OUString aCSVString(p, n, RTL_TEXTENCODING_UTF8);
#if DEBUG_CSV_HANDLER
            std::cout << "String: " << rtl::OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
            std::cout << "CSVString: " << rtl::OUStringToOString(aCSVString, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
            std::cout << "result: " << (int)(aCSVString == aString) << std::endl;
#endif //DEBUG_CSV_HANDLER

            CPPUNIT_ASSERT_MESSAGE("content is not correct in cell", aString == aCSVString);
        }
        else
        {
            double aValue;
            mpDoc->GetValue(mnCol, mnRow, mnTab, aValue);
            CPPUNIT_ASSERT_MESSAGE("content is not correct in cell", aValue == nValue);
        }
        ++mnCol;
    }

private:
    ScDocument* mpDoc;
    SCCOL mnCol;
    SCROW mnRow;
    SCTAB mnTab;
};
