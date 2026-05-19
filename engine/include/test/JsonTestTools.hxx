/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <test/testdllapi.hxx>

#include <rtl/string.hxx>
#include <tools/JsonPath.hxx>

#include <string_view>

/** Test helpers for asserting on JSON output.
 *
 *  These are thin assertion wrappers around tools::JsonPath. Test code
 *  can also call the JsonPath instance methods directly and assert with
 *  CPPUNIT_ASSERT_EQUAL.
 */
class OOO_DLLPUBLIC_TEST JsonTestTools
{
public:
    /// Parse JSON text into a JsonPath. CPPUNIT_FAILs on parse error.
    static tools::JsonPath parseJson(std::string_view sJson);

protected:
    JsonTestTools();
    virtual ~JsonTestTools();

    /// Assert that the value at sPath equals sExpected.
    static void assertJsonPath(const tools::JsonPath& rJson, std::string_view sPath,
                               std::string_view sExpected);

    /// Assert that the value at sPath equals nExpected.
    static void assertJsonPath(const tools::JsonPath& rJson, std::string_view sPath,
                               sal_Int64 nExpected);

    /// Assert that the value at sPath equals bExpected.
    static void assertJsonPathBool(const tools::JsonPath& rJson, std::string_view sPath,
                                   bool bExpected);

    /// Assert that the value at sPath equals fExpected within fDelta.
    static void assertJsonPathDouble(const tools::JsonPath& rJson, std::string_view sPath,
                                     double fExpected, double fDelta);

    /// Assert that sPath resolves to a node.
    static void assertJsonPathExists(const tools::JsonPath& rJson, std::string_view sPath);

    /// Assert that sPath does not resolve.
    static void assertJsonPathMissing(const tools::JsonPath& rJson, std::string_view sPath);

    /// Assert that the array or object at sPath has nExpected children.
    static void assertJsonPathSize(const tools::JsonPath& rJson, std::string_view sPath,
                                   size_t nExpected);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
