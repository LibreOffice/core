/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/JsonTestTools.hxx>
#include <cppunit/TestAssert.h>
#include <string>

JsonTestTools::JsonTestTools() = default;

JsonTestTools::~JsonTestTools() = default;

namespace
{
OString pathLabel(std::string_view sPath, std::string_view sDetail)
{
    OString aPath(sPath.data(), sPath.size());
    OString aDetail(sDetail.data(), sDetail.size());
    return "JSON path '" + aPath + "': " + aDetail;
}
}

tools::JsonPath JsonTestTools::parseJson(std::string_view sJson)
{
    auto oJson = tools::JsonPath::parse(sJson);
    CPPUNIT_ASSERT_MESSAGE("JSON parse error", oJson.has_value());
    return *oJson;
}

void JsonTestTools::assertJsonPath(const tools::JsonPath& rJson, std::string_view sPath,
                                   std::string_view sExpected)
{
    auto oActual = rJson.getString(sPath);
    CPPUNIT_ASSERT_MESSAGE(pathLabel(sPath, "missing or not a string").getStr(),
                           oActual.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pathLabel(sPath, "value mismatch").getStr(),
                                 OString(sExpected.data(), sExpected.size()), *oActual);
}

void JsonTestTools::assertJsonPath(const tools::JsonPath& rJson, std::string_view sPath,
                                   sal_Int64 nExpected)
{
    auto oActual = rJson.getInt(sPath);
    CPPUNIT_ASSERT_MESSAGE(pathLabel(sPath, "missing or not an integer").getStr(),
                           oActual.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pathLabel(sPath, "value mismatch").getStr(), nExpected, *oActual);
}

void JsonTestTools::assertJsonPathBool(const tools::JsonPath& rJson, std::string_view sPath,
                                       bool bExpected)
{
    auto oActual = rJson.getBool(sPath);
    CPPUNIT_ASSERT_MESSAGE(pathLabel(sPath, "missing or not a bool").getStr(), oActual.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pathLabel(sPath, "value mismatch").getStr(), bExpected, *oActual);
}

void JsonTestTools::assertJsonPathDouble(const tools::JsonPath& rJson, std::string_view sPath,
                                         double fExpected, double fDelta)
{
    auto oActual = rJson.getDouble(sPath);
    CPPUNIT_ASSERT_MESSAGE(pathLabel(sPath, "missing or not a number").getStr(),
                           oActual.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(pathLabel(sPath, "value mismatch").getStr(), fExpected,
                                         *oActual, fDelta);
}

void JsonTestTools::assertJsonPathExists(const tools::JsonPath& rJson, std::string_view sPath)
{
    CPPUNIT_ASSERT_MESSAGE(pathLabel(sPath, "expected path to exist but it does not").getStr(),
                           rJson.has(sPath));
}

void JsonTestTools::assertJsonPathMissing(const tools::JsonPath& rJson, std::string_view sPath)
{
    CPPUNIT_ASSERT_MESSAGE(pathLabel(sPath, "expected path to be missing but it exists").getStr(),
                           !rJson.has(sPath));
}

void JsonTestTools::assertJsonPathSize(const tools::JsonPath& rJson, std::string_view sPath,
                                       size_t nExpected)
{
    auto oSize = rJson.getSize(sPath);
    CPPUNIT_ASSERT_MESSAGE(pathLabel(sPath, "missing").getStr(), oSize.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pathLabel(sPath, "size mismatch").getStr(), nExpected, *oSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
