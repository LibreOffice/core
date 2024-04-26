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
extern bool rtl_string_unittest_invalid_concat;

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include <string>
#include <typeinfo>

using namespace rtl;

template<> inline std::string CppUnit::assertion_traits<std::type_info>::toString(
    std::type_info const & x)
{
    return x.name();
}

namespace test::oustring {

class StringConcat : public CppUnit::TestFixture
{
private:
    void checkConcat();
    void checkConcatAsciiL();
    void checkEnsureCapacity();
    void checkAppend();
    void checkInvalid();

CPPUNIT_TEST_SUITE(StringConcat);
CPPUNIT_TEST(checkConcat);
CPPUNIT_TEST(checkConcatAsciiL);
CPPUNIT_TEST(checkEnsureCapacity);
CPPUNIT_TEST(checkAppend);
CPPUNIT_TEST(checkInvalid);
CPPUNIT_TEST_SUITE_END();
};

void test::oustring::StringConcat::checkConcat()
{
// All the extra () are to protect commas against being treated as separators of macro arguments.
    CPPUNIT_ASSERT_EQUAL( OUString(), OUString(OUString() + OUString()));
    CPPUNIT_ASSERT_EQUAL( u"foobar"_ustr, OUString( u"foo"_ustr + u"bar"_ustr));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUString > )), typeid( u"foo"_ustr + u"bar"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"foobar"_ustr, OUString( u"foo"_ustr + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, const char[ 4 ] > )), typeid( u"foo"_ustr + "bar" ));
    CPPUNIT_ASSERT_EQUAL( u"foobarbaz"_ustr, OUString( u"foo"_ustr + "bar" + "baz" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringConcat< OUString, const char[ 4 ] >, const char[ 4 ] > )), typeid( u"foo"_ustr + "bar" + "baz" ));
    CPPUNIT_ASSERT_EQUAL( u"foobar"_ustr, OUString( OUStringBuffer( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringBuffer, const char[ 4 ] > )), typeid( OUStringBuffer( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( u"foobar"_ustr, OUString( OUStringLiteral( u"foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringLiteral<4>, const char[ 4 ] > )), typeid( OUStringLiteral( u"foo" ) + "bar" ));
    const char d1[] = "xyz";
    CPPUNIT_ASSERT_EQUAL( u"fooxyz"_ustr, OUString( u"foo"_ustr + d1 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, const char[ 4 ] > )), typeid( u"foo"_ustr + d1 ));
    const sal_Unicode* d2 = u"xyz";
    CPPUNIT_ASSERT_EQUAL( u"fooxyz"_ustr, OUString( u"foo"_ustr + d2 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, const sal_Unicode* > )), typeid( u"foo"_ustr + d2 ));
    const sal_Unicode d3[] = u"xyz";
    CPPUNIT_ASSERT_EQUAL( u"foobar"_ustr, OUString( OUString::Concat( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringConcat< rtl::OUStringConcatMarker, const char[ 4 ] >, const char[ 4 ] > )), typeid( OUString::Concat( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( u"xyzbar"_ustr, OUString( OUString::Concat( d1 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringConcat< rtl::OUStringConcatMarker, const char[ 4 ] >, const char[ 4 ] > )), typeid( OUString::Concat( d1 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( u"foobar"_ustr, OUString( OUString::Concat( u"foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringConcat< rtl::OUStringConcatMarker, const sal_Unicode[ 4 ] >, const char[ 4 ] > )), typeid( OUString::Concat( u"foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( u"xyzbar"_ustr, OUString( OUString::Concat( d2 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringConcat< rtl::OUStringConcatMarker, const sal_Unicode* >, const char[ 4 ] > )), typeid( OUString::Concat( d2 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( u"xyzbar"_ustr, OUString( OUString::Concat( d3 ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringConcat< rtl::OUStringConcatMarker, const sal_Unicode[ 4 ] >, const char[ 4 ] > )), typeid( OUString::Concat( d3 ) + "bar" ));

    CPPUNIT_ASSERT_EQUAL( u"num10"_ustr, OUString( u"num"_ustr + OUString::number( 10 )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, StringNumber< sal_Unicode, RTL_USTR_MAX_VALUEOFINT32 > > )), typeid( u"num"_ustr + OUString::number( 10 )));
    CPPUNIT_ASSERT_EQUAL( u"num10"_ustr, OUString( u"num"_ustr + OUString::number( 10L )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, StringNumber< sal_Unicode, RTL_USTR_MAX_VALUEOFINT64 > > )), typeid( u"num"_ustr + OUString::number( 10L )));
    CPPUNIT_ASSERT_EQUAL( u"num10"_ustr, OUString( u"num"_ustr + OUString::number( 10ULL )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, StringNumber< sal_Unicode, RTL_USTR_MAX_VALUEOFUINT64 > > )), typeid( u"num"_ustr + OUString::number( 10ULL )));
    CPPUNIT_ASSERT_EQUAL( u"num10.5"_ustr, OUString( u"num"_ustr + OUString::number( 10.5f )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUString > )), typeid( u"num"_ustr + OUString::number( 10.5f )));
    CPPUNIT_ASSERT_EQUAL( u"num10.5"_ustr, OUString( u"num"_ustr + OUString::number( 10.5 )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUString > )), typeid( u"num"_ustr + OUString::number( 10.5 )));
}

void test::oustring::StringConcat::checkConcatAsciiL()
{
    {
        OUString s(u"foo"_ustr);
        s += "";
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, s);
    }
    {
        OUString s(u"foo"_ustr);
        s += "bar";
        CPPUNIT_ASSERT_EQUAL(u"foobar"_ustr, s);
    }
}

void test::oustring::StringConcat::checkEnsureCapacity()
{
    rtl_uString* str = nullptr;
    rtl_uString_newFromLiteral( &str, "test", strlen( "test" ), 0 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, int( str->refCount ));

    rtl_uString* oldStr = str;
    rtl_uString_ensureCapacity( &str, 4 ); // should be no-op
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, int( str->refCount ));
    CPPUNIT_ASSERT_EQUAL( str, oldStr );

    rtl_uString_acquire( oldStr );
    CPPUNIT_ASSERT_EQUAL( 2, int( str->refCount ));
    rtl_uString_ensureCapacity( &str, 4 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, int( str->refCount ));
    // a copy was forced because of refcount
    CPPUNIT_ASSERT( oldStr != str );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(0), rtl_ustr_compare( oldStr->buffer, str->buffer ) );
    CPPUNIT_ASSERT_EQUAL( 1, int( oldStr->refCount ));
    rtl_uString_release( str );
    str = oldStr;

    rtl_uString_acquire( oldStr );
    rtl_uString_ensureCapacity( &str, 1024 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length ); // size is still 4
    CPPUNIT_ASSERT_EQUAL( 1, int( str->refCount ));
    CPPUNIT_ASSERT( oldStr != str );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(0), rtl_ustr_compare( oldStr->buffer, str->buffer ) );
    CPPUNIT_ASSERT_EQUAL( 1, int( oldStr->refCount ));
    // but there should be extra capacity
    for( int i = 0;
         i < 20;
         ++i )
        str->buffer[ str->length + i ] = '0';
    str->length += 20;
    rtl_uString_release( str );
    rtl_uString_release( oldStr );
}

void test::oustring::StringConcat::checkAppend()
{
    OUString str( u"foo"_ustr );
    str += OUStringLiteral( u"bar" ) + "baz";
    CPPUNIT_ASSERT_EQUAL( u"foobarbaz"_ustr, str );
    OUStringBuffer buf( "foo" );
    buf.append( OUStringLiteral( u"bar" ) + "baz" );
    CPPUNIT_ASSERT_EQUAL( u"foobarbaz"_ustr, buf.makeStringAndClear());
}

#define INVALID_CONCAT( expression ) \
    ( \
    rtl_string_unittest_invalid_concat = false, \
    ( void ) OUString( expression ), \
    rtl_string_unittest_invalid_concat )

void test::oustring::StringConcat::checkInvalid()
{
    CPPUNIT_ASSERT( !INVALID_CONCAT( OUString() + OUString()));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + "b"_ostr));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + OStringBuffer( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + static_cast<const char*>("b") ));
    char d[] = "b";
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + d ));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + static_cast<char*>(d) ));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + OStringLiteral( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + OString::Concat( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + 1 ));
    rtl_String* rs = nullptr;
    rtl_uString* rus = nullptr;
    CPPUNIT_ASSERT( INVALID_CONCAT( u"b"_ustr + rs ));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"b"_ustr + rus ));
    CPPUNIT_ASSERT( INVALID_CONCAT( u"a"_ustr + OString::number( 10 )));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString::number( 0 ) + OString::number( 10 )));

#if 0
    // Should fail to compile, to avoid use of OUStringConcat lvalues that
    // contain dangling references to temporaries:
    auto const conc = OUStringLiteral("foo") + "bar";
    (void) OUString(conc);
#endif
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::StringConcat);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
