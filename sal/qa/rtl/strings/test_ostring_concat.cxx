/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// activate support for detecting errors instead of getting compile errors
#define RTL_STRING_UNITTEST_CONCAT

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <string>
#include <typeinfo>

bool rtl_string_unittest_invalid_concat = false;

using namespace rtl;

template<> inline std::string CppUnit::assertion_traits<std::type_info>::toString(
    std::type_info const & x)
{
    return x.name();
}

namespace test::ostring {

class StringConcat : public CppUnit::TestFixture
{
private:
    void checkConcat();
    void checkEnsureCapacity();
    void checkAppend();
    void checkInvalid();

CPPUNIT_TEST_SUITE(StringConcat);
CPPUNIT_TEST(checkConcat);
CPPUNIT_TEST(checkEnsureCapacity);
CPPUNIT_TEST(checkAppend);
CPPUNIT_TEST(checkInvalid);
CPPUNIT_TEST_SUITE_END();
};

void test::ostring::StringConcat::checkConcat()
{
// All the extra () are to protect commas against being treated as separators of macro arguments.
    CPPUNIT_ASSERT_EQUAL( OString(), OString(OString() + OString()));
    CPPUNIT_ASSERT_EQUAL( "foobar"_ostr, OString( "foo"_ostr + "bar"_ostr));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, OString > )), typeid( "foo"_ostr + "bar"_ostr));
    CPPUNIT_ASSERT_EQUAL( "foobar"_ostr, OString( "foo"_ostr + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, const char[ 4 ] > )), typeid( "foo"_ostr + "bar" ));
    CPPUNIT_ASSERT_EQUAL( "foobarbaz"_ostr, OString( "foo"_ostr + "bar" + "baz" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringConcat< OString, const char[ 4 ] >, const char[ 4 ] > )), typeid( "foo"_ostr + "bar" + "baz" ));
    CPPUNIT_ASSERT_EQUAL( "foobar"_ostr, OString( OStringBuffer( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringBuffer, const char[ 4 ] > )), typeid( OStringBuffer( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( "foobar"_ostr, OString( OStringLiteral( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringLiteral<4>, const char[ 4 ] > )), typeid( OStringLiteral( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( "foobar"_ostr, OString( OStringLiteral( "foo" ) + static_cast<const char*>("bar") ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringLiteral<4>, const char* > )), typeid( OStringLiteral( "foo" ) + static_cast<const char*>("bar") ));
    const char d1[] = "xyz";
    char d2[] = "abc";
    const char* d3 = d1;
    char* d4 = d2;
    CPPUNIT_ASSERT_EQUAL( "fooxyz"_ostr, OString( "foo"_ostr + d1 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, const char[ 4 ] > )), typeid( "foo"_ostr + d1 ));
    CPPUNIT_ASSERT_EQUAL( "fooabc"_ostr, OString( "foo"_ostr + d2 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, char[ 4 ] > )), typeid( "foo"_ostr + d2 ));
    CPPUNIT_ASSERT_EQUAL( "fooxyz"_ostr, OString( "foo"_ostr + d3 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, const char* > )), typeid( "foo"_ostr + d3 ));
    CPPUNIT_ASSERT_EQUAL( "fooabc"_ostr, OString( "foo"_ostr + d4 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, char* > )), typeid( "foo"_ostr + d4 ));
    CPPUNIT_ASSERT_EQUAL( "fooabc"_ostr, OString( "foo"_ostr + d4 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, char* > )), typeid( "foo"_ostr + d4 ));
    CPPUNIT_ASSERT_EQUAL( "foobar"_ostr, OString( OString::Concat( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringConcat< OStringConcatMarker, const char[ 4 ] >, const char[ 4 ] > )), typeid( OString::Concat( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( "xyzbar"_ostr, OString( OString::Concat( d1 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringConcat< OStringConcatMarker, const char[ 4 ] >, const char[ 4 ] > )), typeid( OString::Concat( d1 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( "abcbar"_ostr, OString( OString::Concat( d2 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringConcat< OStringConcatMarker, char[ 4 ] >, const char[ 4 ] > )), typeid( OString::Concat( d2 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( "xyzbar"_ostr, OString( OString::Concat( d3 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringConcat< OStringConcatMarker, const char* >, const char[ 4 ] > )), typeid( OString::Concat( d3 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( "abcbar"_ostr, OString( OString::Concat( d4 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OStringConcat< OStringConcatMarker, char* >, const char[ 4 ] > )), typeid( OString::Concat( d4 ) + "bar" ));

    CPPUNIT_ASSERT_EQUAL( "num10"_ostr, OString( "num"_ostr + OString::number( 10 )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, StringNumber< char, RTL_STR_MAX_VALUEOFINT32 > > )), typeid( "num"_ostr + OString::number( 10 )));
    CPPUNIT_ASSERT_EQUAL( "num10"_ostr, OString( "num"_ostr + OString::number( 10L )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, StringNumber< char, RTL_STR_MAX_VALUEOFINT64 > > )), typeid( "num"_ostr + OString::number( 10L )));
    CPPUNIT_ASSERT_EQUAL( "num10"_ostr, OString( "num"_ostr + OString::number( 10ULL )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, StringNumber< char, RTL_STR_MAX_VALUEOFUINT64 > > )), typeid( "num"_ostr + OString::number( 10ULL )));
    CPPUNIT_ASSERT_EQUAL( "num10.5"_ostr, OString( "num"_ostr + OString::number( 10.5f )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, OString > )), typeid( "num"_ostr + OString::number( 10.5f )));
    CPPUNIT_ASSERT_EQUAL( "num10.5"_ostr, OString( "num"_ostr + OString::number( 10.5 )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OStringConcat< OString, OString > )), typeid( "num"_ostr + OString::number( 10.5 )));
}

void test::ostring::StringConcat::checkEnsureCapacity()
{
    rtl_String* str = nullptr;
    rtl_string_newFromLiteral( &str, "test", strlen( "test" ), 0 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, int( str->refCount ));

    rtl_String* oldStr = str;
    rtl_string_ensureCapacity( &str, 4 ); // should be no-op
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, int( str->refCount ));
    CPPUNIT_ASSERT_EQUAL( str, oldStr );

    rtl_string_acquire( oldStr );
    CPPUNIT_ASSERT_EQUAL( 2, int( str->refCount ));
    rtl_string_ensureCapacity( &str, 4 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, int( str->refCount ));
    // a copy was forced because of refcount
    CPPUNIT_ASSERT( oldStr != str );
    CPPUNIT_ASSERT_EQUAL( 0, strcmp( oldStr->buffer, str->buffer ) );
    CPPUNIT_ASSERT_EQUAL( 1, int( oldStr->refCount ));
    rtl_string_release( str );
    str = oldStr;

    rtl_string_acquire( oldStr );
    rtl_string_ensureCapacity( &str, 1024 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length ); // size is still 4
    CPPUNIT_ASSERT_EQUAL( 1, int( str->refCount ));
    CPPUNIT_ASSERT( oldStr != str );
    CPPUNIT_ASSERT_EQUAL( 0, strcmp( oldStr->buffer, str->buffer ) );
    CPPUNIT_ASSERT_EQUAL( 1, int( oldStr->refCount ));
    strcpy( str->buffer, "01234567890123456789" ); // but there should be extra capacity
    str->length += 20;
    rtl_string_release( str );
    rtl_string_release( oldStr );
}

void test::ostring::StringConcat::checkAppend()
{
    OString str( "foo"_ostr );
    str += OStringLiteral( "bar" ) + "baz";
    CPPUNIT_ASSERT_EQUAL( "foobarbaz"_ostr, str );
    OStringBuffer buf( "foo" );
    buf.append( OStringLiteral( "bar" ) + "baz" );
    CPPUNIT_ASSERT_EQUAL( "foobarbaz"_ostr, buf.makeStringAndClear());
}

#define INVALID_CONCAT( expression ) \
    ( \
    rtl_string_unittest_invalid_concat = false, \
    ( void ) OString( expression ), \
    rtl_string_unittest_invalid_concat )

void test::ostring::StringConcat::checkInvalid()
{
    CPPUNIT_ASSERT( !INVALID_CONCAT( OString() + OString()));
    CPPUNIT_ASSERT( INVALID_CONCAT( "a"_ostr + u"b"_ustr));
    CPPUNIT_ASSERT( INVALID_CONCAT( "a"_ostr + OUStringBuffer( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( "a"_ostr + OUStringLiteral( u"b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( "a"_ostr + OUString::Concat( u"b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( "a"_ostr + 1 ));
    rtl_String* rs = nullptr;
    rtl_uString* rus = nullptr;
    CPPUNIT_ASSERT( INVALID_CONCAT( u"b"_ustr + rs ));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"b"_ustr + rus ));
    CPPUNIT_ASSERT( INVALID_CONCAT( "a"_ostr + OUString::number( 10 )));
    CPPUNIT_ASSERT( INVALID_CONCAT( OString::number( 0 ) + OUString::number( 10 )));

#if 0
    // Should fail to compile, to avoid use of OStringConcat lvalues that
    // contain dangling references to temporaries:
    auto const conc = OStringLiteral("foo") + "bar";
    (void) OString(conc);
#endif
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::ostring::StringConcat);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
