/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tools/JsonPath.hxx>

namespace
{
class JsonPathTest : public CppUnit::TestFixture
{
};

constexpr std::string_view kSample = R"({
    "type": "vectortile",
    "part": 0,
    "slideWidth": 15874,
    "slideHeight": 8929,
    "ratio": 0.5,
    "visible": true,
    "primitives": [
        { "type": "backgroundcolor", "color": "#ffffff" },
        { "type": "polyPolygonColor", "color": "#4472c4" }
    ]
})";

CPPUNIT_TEST_FIXTURE(JsonPathTest, testParseMalformed)
{
    CPPUNIT_ASSERT(!tools::JsonPath::parse("{ not json").has_value());
    CPPUNIT_ASSERT(!tools::JsonPath::parse("").has_value());
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testParseSmallObject)
{
    // Smallest possible smoke test: a single-key object literal, no
    // whitespace, no helper string. If this fails, parse() itself is
    // broken (vs the sample document being mishandled somehow).
    auto oJson = tools::JsonPath::parse(R"({"a":1})");
    CPPUNIT_ASSERT(oJson.has_value());
    CPPUNIT_ASSERT_MESSAGE("/a should resolve after parse", oJson->has("/a"));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1), oJson->getInt("/a").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testHas)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT(oJson->has("/type"));
    CPPUNIT_ASSERT(oJson->has("type"));
    CPPUNIT_ASSERT(oJson->has("/primitives"));
    CPPUNIT_ASSERT(oJson->has("/primitives/0"));
    CPPUNIT_ASSERT(oJson->has("/primitives/0/type"));
    CPPUNIT_ASSERT(!oJson->has("/nonexistent"));
    CPPUNIT_ASSERT(!oJson->has("/primitives/5"));
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testGetString)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT_EQUAL("vectortile"_ostr, oJson->getString("/type").value_or(OString()));
    CPPUNIT_ASSERT_EQUAL("#ffffff"_ostr,
                         oJson->getString("/primitives/0/color").value_or(OString()));
    CPPUNIT_ASSERT(!oJson->getString("/nonexistent").has_value());
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testGetInt)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), oJson->getInt("/part").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(15874), oJson->getInt("/slideWidth").value_or(-1));
    CPPUNIT_ASSERT(!oJson->getInt("/nonexistent").has_value());
    // A string value is not an integer.
    CPPUNIT_ASSERT(!oJson->getInt("/type").has_value());
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testGetDouble)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, oJson->getDouble("/ratio").value_or(-1.0), 1e-9);
    // Integers are also valid doubles.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15874.0, oJson->getDouble("/slideWidth").value_or(-1.0), 1e-9);
    CPPUNIT_ASSERT(!oJson->getDouble("/nonexistent").has_value());
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testGetBool)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT_EQUAL(true, oJson->getBool("/visible").value_or(false));
    CPPUNIT_ASSERT(!oJson->getBool("/nonexistent").has_value());
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testGetSize)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT_EQUAL(size_t(2), oJson->getSize("/primitives").value_or(0));
    // The root object has 7 keys: type, part, slideWidth, slideHeight,
    // ratio, visible, primitives.
    CPPUNIT_ASSERT_EQUAL(size_t(7), oJson->getSize().value_or(0));
    CPPUNIT_ASSERT(!oJson->getSize("/nonexistent").has_value());
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testNumericIndex)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT_EQUAL("backgroundcolor"_ostr,
                         oJson->getString("/primitives/0/type").value_or(OString()));
    CPPUNIT_ASSERT_EQUAL("polyPolygonColor"_ostr,
                         oJson->getString("/primitives/1/type").value_or(OString()));
    // Out-of-range index is missing, not an error.
    CPPUNIT_ASSERT(!oJson->getString("/primitives/2/type").has_value());
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testNestedPath)
{
    static constexpr std::string_view sNested = R"({
        "a": { "b": { "c": { "d": "deep" } } }
    })";
    auto oJson = tools::JsonPath::parse(sNested);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT_EQUAL("deep"_ostr, oJson->getString("/a/b/c/d").value_or(OString()));
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testAtSharesOwnership)
{
    // Sub-paths obtained via at() must keep the underlying tree alive
    // even after the original JsonPath is destroyed.
    std::optional<tools::JsonPath> oSub;
    {
        auto oJson = tools::JsonPath::parse(kSample);
        CPPUNIT_ASSERT(oJson.has_value());
        oSub = oJson->at("/primitives/0");
    }
    CPPUNIT_ASSERT(oSub.has_value());
    CPPUNIT_ASSERT_EQUAL("backgroundcolor"_ostr, oSub->getString("type").value_or(OString()));
    CPPUNIT_ASSERT_EQUAL("#ffffff"_ostr, oSub->getString("color").value_or(OString()));
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testSubWrapsIteratedChild)
{
    // The typical use of sub() is iterating tree() directly and
    // wrapping each child for path queries.
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    auto oPrimitives = oJson->at("/primitives");
    CPPUNIT_ASSERT(oPrimitives.has_value());

    std::vector<OString> aTypes;
    for (const auto& rChild : oPrimitives->tree())
    {
        tools::JsonPath aChild = oPrimitives->sub(rChild.second);
        aTypes.push_back(aChild.getString("type").value_or(OString()));
    }
    CPPUNIT_ASSERT_EQUAL(size_t(2), aTypes.size());
    CPPUNIT_ASSERT_EQUAL("backgroundcolor"_ostr, aTypes[0]);
    CPPUNIT_ASSERT_EQUAL("polyPolygonColor"_ostr, aTypes[1]);
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testSubSharesOwnership)
{
    // A JsonPath obtained from sub() must keep the tree alive after
    // both the original root JsonPath and the intermediate at()
    // result are destroyed.
    std::optional<tools::JsonPath> oSub;
    {
        auto oJson = tools::JsonPath::parse(kSample);
        CPPUNIT_ASSERT(oJson.has_value());
        auto oPrimitives = oJson->at("/primitives");
        CPPUNIT_ASSERT(oPrimitives.has_value());
        // Pick the first child via iteration.
        const auto& rFirst = oPrimitives->tree().front().second;
        oSub = oPrimitives->sub(rFirst);
    }
    CPPUNIT_ASSERT(oSub.has_value());
    CPPUNIT_ASSERT_EQUAL("backgroundcolor"_ostr, oSub->getString("type").value_or(OString()));
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testLeadingSlashOptional)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    auto oWithSlash = oJson->getString("/primitives/0/type");
    auto oWithoutSlash = oJson->getString("primitives/0/type");
    CPPUNIT_ASSERT(oWithSlash.has_value());
    CPPUNIT_ASSERT(oWithoutSlash.has_value());
    CPPUNIT_ASSERT_EQUAL(*oWithSlash, *oWithoutSlash);
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testEmptySegmentsSkipped)
{
    // Empty path segments (leading "/", "//", trailing "/") are
    // silently skipped. "/primitives//0//type" must resolve the same
    // way as "/primitives/0/type".
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT_EQUAL("backgroundcolor"_ostr,
                         oJson->getString("/primitives//0//type").value_or(OString()));
    CPPUNIT_ASSERT_EQUAL("backgroundcolor"_ostr,
                         oJson->getString("/primitives/0/type/").value_or(OString()));
}

CPPUNIT_TEST_FIXTURE(JsonPathTest, testMissingPathReturnsNullopt)
{
    auto oJson = tools::JsonPath::parse(kSample);
    CPPUNIT_ASSERT(oJson.has_value());

    CPPUNIT_ASSERT(!oJson->at("/nope").has_value());
    CPPUNIT_ASSERT(!oJson->find("/nope/here"));
    CPPUNIT_ASSERT(!oJson->getInt("/nope").has_value());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
