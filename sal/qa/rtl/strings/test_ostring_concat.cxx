/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/string.hxx>

#include <typeinfo>

using namespace rtl;

namespace std
{
template< typename charT, typename traits > std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, const std::type_info& info )
{
    return stream << info.name();
}
} // namespace

namespace test { namespace ostring {

class StringConcat : public CppUnit::TestFixture
{
private:
    void check();
    void checkEnsureCapacity();

CPPUNIT_TEST_SUITE(StringConcat);
CPPUNIT_TEST(check);
CPPUNIT_TEST(checkEnsureCapacity);
CPPUNIT_TEST_SUITE_END();
};

#ifdef RTL_FAST_STRING
#define TYPES_ASSERT_EQUAL( a, b ) CPPUNIT_ASSERT_EQUAL( a, b )
#else
#define TYPES_ASSERT_EQUAL( a, b )
#endif
void test::ostring::StringConcat::check()
{
// All the extra () are to protect commas againsts being treated as separators of macro arguments.
    CPPUNIT_ASSERT_EQUAL( OString(), OString(OString() + OString()) );
    CPPUNIT_ASSERT_EQUAL( OString( "foobar" ), OString( OString( "foo" ) + OString( "bar" )));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OString, OString > )), typeid( OString( "foo" ) + OString( "bar" )));
    CPPUNIT_ASSERT_EQUAL( OString( "foobar" ), OString( OString( "foo" ) + "bar" ));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OString, const char[ 4 ] > )), typeid( OString( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( OString( "foobarbaz" ), OString( OString( "foo" ) + "bar" + "baz" ));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OStringConcat< OString, const char[ 4 ] >, const char[ 4 ] > )), typeid( OString( "foo" ) + "bar" + "baz" ));
    CPPUNIT_ASSERT_EQUAL( OString( "foobar" ), OString( OStringLiteral( "foo" ) + "bar" ));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OStringLiteral, const char[ 4 ] > )), typeid( OStringLiteral( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( OString( "foobar" ), OString( OStringLiteral( "foo" ) + (const char*)"bar" ));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OStringLiteral, const char* > )), typeid( OStringLiteral( "foo" ) + (const char*)"bar" ));
    const char d1[] = "xyz";
    char d2[] = "abc";
    const char* d3 = d1;
    char* d4 = d2;
    CPPUNIT_ASSERT_EQUAL( OString( "fooxyz" ), OString( OString( "foo" ) + d1 ));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OString, const char[ 4 ] > )), typeid( OString( "foo" ) + d1 ));
    CPPUNIT_ASSERT_EQUAL( OString( "fooabc" ), OString( OString( "foo" ) + d2 ));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OString, char[ 4 ] > )), typeid( OString( "foo" ) + d2 ));
    CPPUNIT_ASSERT_EQUAL( OString( "fooxyz" ), OString( OString( "foo" ) + d3 ));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OString, const char* > )), typeid( OString( "foo" ) + d3 ));
    CPPUNIT_ASSERT_EQUAL( OString( "fooabc" ), OString( OString( "foo" ) + d4 ));
    TYPES_ASSERT_EQUAL(( typeid( OStringConcat< OString, char* > )), typeid( OString( "foo" ) + d4 ));
}
#undef typeid

void test::ostring::StringConcat::checkEnsureCapacity()
{
    rtl_String* str = NULL;
    rtl_string_newFromLiteral( &str, "test", strlen( "test" ), 0 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, str->refCount );

    rtl_String* oldStr = str;
    rtl_string_ensureCapacity( &str, 4 ); // should be no-op
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, str->refCount );
    CPPUNIT_ASSERT( oldStr == str );

    rtl_string_acquire( oldStr );
    CPPUNIT_ASSERT_EQUAL( 2, str->refCount );
    rtl_string_ensureCapacity( &str, 4 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, str->refCount );
    // a copy was forced because of refcount
    CPPUNIT_ASSERT( oldStr != str );
    CPPUNIT_ASSERT( strcmp( oldStr->buffer, str->buffer ) == 0 );
    CPPUNIT_ASSERT_EQUAL( 1, oldStr->refCount );
    rtl_string_release( str );
    str = oldStr;

    rtl_string_acquire( oldStr );
    rtl_string_ensureCapacity( &str, 1024 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length ); // size is still 4
    CPPUNIT_ASSERT_EQUAL( 1, str->refCount );
    CPPUNIT_ASSERT( oldStr != str );
    CPPUNIT_ASSERT( strcmp( oldStr->buffer, str->buffer ) == 0 );
    CPPUNIT_ASSERT_EQUAL( 1, oldStr->refCount );
    strcpy( str->buffer, "01234567890123456789" ); // but there should be extra capacity
    str->length += 20;
    rtl_string_release( str );
    rtl_string_release( oldStr );
}

}} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::ostring::StringConcat);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
