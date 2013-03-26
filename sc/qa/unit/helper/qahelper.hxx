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
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
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

#ifndef SC_QA_HELPER_HXX
#define SC_QA_HELPER_HXX

#include "helper/csv_handler.hxx"
#include "helper/debughelper.hxx"
#include "orcus/csv_parser.hpp"
#include <fstream>
#include <string>
#include <sstream>

#include <osl/detail/android-bootstrap.h>

bool testEqualsWithTolerance( long nVal1, long nVal2, long nTol )
{
    return ( labs( nVal1 - nVal2 ) <= nTol );
}

// Why is this here and not in osl, and using the already existing file
// handling APIs? Do we really want to add arbitrary new file handling
// wrappers here and there (and then having to handle the Android (and
// eventually perhaps iOS) special cases here, too)?  Please move this to osl,
// it sure looks gemerally useful. Or am I missing something?

void loadFile(const rtl::OUString& aFileName, std::string& aContent)
{
    rtl::OString aOFileName = rtl::OUStringToOString(aFileName, RTL_TEXTENCODING_UTF8);

#ifdef ANDROID
    const char *contents;
    size_t size;
    if (strncmp(aOFileName.getStr(), "/assets/", sizeof("/assets/")-1) == 0) {
        contents = (const char *) lo_apkentry(aOFileName.getStr(), &size);
        if (contents != 0) {
            aContent = std::string(contents, size);
            return;
        }
    }
#endif

    std::ifstream aFile(aOFileName.getStr());

    rtl::OStringBuffer aErrorMsg("Could not open csv file: ");
    aErrorMsg.append(aOFileName);
    CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), aFile);
    std::ostringstream aOStream;
    aOStream << aFile.rdbuf();
    aFile.close();
    aContent = aOStream.str();
}

void testFile(rtl::OUString& aFileName, ScDocument* pDoc, SCTAB nTab, StringType aStringFormat = StringValue)
{
    csv_handler aHandler(pDoc, nTab, aStringFormat);
    orcus::csv_parser_config aConfig;
    aConfig.delimiters.push_back(',');
    aConfig.delimiters.push_back(';');
    aConfig.text_qualifier = '"';
    aConfig.trim_cell_value = false;


    std::string aContent;
    loadFile(aFileName, aContent);
    orcus::csv_parser<csv_handler> parser ( &aContent[0], aContent.size() , aHandler, aConfig);
    try
    {
        parser.parse();
    }
    catch (const orcus::csv_parse_error& e)
    {
        std::cout << "reading csv content file failed: " << e.what() << std::endl;
        rtl::OStringBuffer aErrorMsg("csv parser error: ");
        aErrorMsg.append(e.what());
        CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), false);
    }
}

//need own handler because conditional formatting strings must be generated
void testCondFile(rtl::OUString& aFileName, ScDocument* pDoc, SCTAB nTab)
{
    conditional_format_handler aHandler(pDoc, nTab);
    orcus::csv_parser_config aConfig;
    aConfig.delimiters.push_back(',');
    aConfig.delimiters.push_back(';');
    aConfig.text_qualifier = '"';
    std::string aContent;
    loadFile(aFileName, aContent);
    orcus::csv_parser<conditional_format_handler> parser ( &aContent[0], aContent.size() , aHandler, aConfig);
    try
    {
        parser.parse();
    }
    catch (const orcus::csv_parse_error& e)
    {
        std::cout << "reading csv content file failed: " << e.what() << std::endl;
        rtl::OStringBuffer aErrorMsg("csv parser error: ");
        aErrorMsg.append(e.what());
        CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), false);
    }

}

#define ASSERT_DOUBLES_EQUAL( expected, result )    \
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (expected), (result), 1e-14 )

#define ASSERT_DOUBLES_EQUAL_MESSAGE( message, expected, result )   \
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( (message), (expected), (result), 1e-14 )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
