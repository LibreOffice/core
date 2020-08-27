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

namespace CppUnit
{
template<> struct assertion_traits<std::type_info>
{
    static bool equal(std::type_info const & x, std::type_info const & y) { return x == y; }

    static std::string toString(std::type_info const & x) { return x.name(); }
};
} // namespace

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
    CPPUNIT_ASSERT_EQUAL( OUString( "foobar" ), OUString( OUString( "foo" ) + OUString( "bar" )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUString > )), typeid( OUString( "foo" ) + OUString( "bar" )));
    CPPUNIT_ASSERT_EQUAL( OUString( "foobar" ), OUString( OUString( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, const char[ 4 ] > )), typeid( OUString( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( OUString( "foobarbaz" ), OUString( OUString( "foo" ) + "bar" + "baz" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringConcat< OUString, const char[ 4 ] >, const char[ 4 ] > )), typeid( OUString( "foo" ) + "bar" + "baz" ));
    CPPUNIT_ASSERT_EQUAL( OUString( "foobar" ), OUString( OUStringLiteral( u"foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringLiteral, const char[ 4 ] > )), typeid( OUStringLiteral( u"foo" ) + "bar" ));
    const char d1[] = "xyz";
    CPPUNIT_ASSERT_EQUAL( OUString( "fooxyz" ), OUString( OUString( "foo" ) + d1 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, const char[ 4 ] > )), typeid( OUString( "foo" ) + d1 ));
    const sal_Unicode* d2 = u"xyz";
    CPPUNIT_ASSERT_EQUAL( OUString( "fooxyz" ), OUString( OUString( "foo" ) + d2 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, const sal_Unicode* > )), typeid( OUString( "foo" ) + d2 ));
    CPPUNIT_ASSERT_EQUAL( OUString( "fooxyz" ), OUString( rtl::OUStringView( u"foo" ) + d2 ));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< rtl::OUStringView, const sal_Unicode* > )), typeid( rtl::OUStringView( u"foo" ) + d2 ));

    CPPUNIT_ASSERT_EQUAL( OUString( "num10" ), OUString( OUString( "num" ) + OUString::number( 10 )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUStringNumber< int > > )), typeid( OUString( "num" ) + OUString::number( 10 )));
    CPPUNIT_ASSERT_EQUAL( OUString( "num10" ), OUString( OUString( "num" ) + OUString::number( 10L )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUStringNumber< long long > > )), typeid( OUString( "num" ) + OUString::number( 10L )));
    CPPUNIT_ASSERT_EQUAL( OUString( "num10" ), OUString( OUString( "num" ) + OUString::number( 10ULL )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUStringNumber< unsigned long long > > )), typeid( OUString( "num" ) + OUString::number( 10ULL )));
    CPPUNIT_ASSERT_EQUAL( OUString( "num10.5" ), OUString( OUString( "num" ) + OUString::number( 10.5f )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUStringNumber< float > > )), typeid( OUString( "num" ) + OUString::number( 10.5f )));
    CPPUNIT_ASSERT_EQUAL( OUString( "num10.5" ), OUString( OUString( "num" ) + OUString::number( 10.5 )));
    CPPUNIT_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUStringNumber< double > > )), typeid( OUString( "num" ) + OUString::number( 10.5 )));
}

void test::oustring::StringConcat::checkConcatAsciiL()
{
    {
        OUString s("foo");
        s += "";
        CPPUNIT_ASSERT_EQUAL(OUString("foo"), s);
    }
    {
        OUString s("foo");
        s += "bar";
        CPPUNIT_ASSERT_EQUAL(OUString("foobar"), s);
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
    OUString str( "foo" );
    str += OUStringLiteral( u"bar" ) + "baz";
    CPPUNIT_ASSERT_EQUAL( OUString( "foobarbaz" ), str );
    OUStringBuffer buf( "foo" );
    buf.append( OUStringLiteral( u"bar" ) + "baz" );
    CPPUNIT_ASSERT_EQUAL( OUString( "foobarbaz" ), buf.makeStringAndClear());
}

#define INVALID_CONCAT( expression ) \
    ( \
    rtl_string_unittest_invalid_concat = false, \
    ( void ) OUString( expression ), \
    rtl_string_unittest_invalid_concat )

void test::oustring::StringConcat::checkInvalid()
{
    CPPUNIT_ASSERT( !INVALID_CONCAT( OUString() + OUString()));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + OUStringBuffer( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + OString( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + OStringBuffer( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + static_cast<const char*>("b") ));
    char d[] = "b";
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + d ));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + static_cast<char*>(d) ));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + OStringLiteral( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + rtl::OStringView( "b" )));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + 1 ));
    rtl_String* rs = nullptr;
    rtl_uString* rus = nullptr;
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "b" ) + rs ));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "b" ) + rus ));
    CPPUNIT_ASSERT( INVALID_CONCAT( OUString( "a" ) + OString::number( 10 )));
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
