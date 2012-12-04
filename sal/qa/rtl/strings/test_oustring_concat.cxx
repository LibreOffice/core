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

#include <rtl/ustring.hxx>

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

namespace test { namespace oustring {

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
void test::oustring::StringConcat::check()
{
// All the extra () are to protect commas againsts being treated as separators of macro arguments.
    CPPUNIT_ASSERT_EQUAL( OUString(), OUString(OUString() + OUString()) );
    CPPUNIT_ASSERT_EQUAL( OUString( "foobar" ), OUString( OUString( "foo" ) + OUString( "bar" )));
    TYPES_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, OUString > )), typeid( OUString( "foo" ) + OUString( "bar" )));
    CPPUNIT_ASSERT_EQUAL( OUString( "foobar" ), OUString( OUString( "foo" ) + "bar" ));
    TYPES_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, const char[ 4 ] > )), typeid( OUString( "foo" ) + "bar" ));
    CPPUNIT_ASSERT_EQUAL( OUString( "foobarbaz" ), OUString( OUString( "foo" ) + "bar" + "baz" ));
    TYPES_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringConcat< OUString, const char[ 4 ] >, const char[ 4 ] > )), typeid( OUString( "foo" ) + "bar" + "baz" ));
    CPPUNIT_ASSERT_EQUAL( OUString( "foobar" ), OUString( OUStringLiteral( "foo" ) + "bar" ));
    TYPES_ASSERT_EQUAL(( typeid( OUStringConcat< OUStringLiteral, const char[ 4 ] > )), typeid( OUStringLiteral( "foo" ) + "bar" ));
    const char d1[] = "xyz";
    CPPUNIT_ASSERT_EQUAL( OUString( "fooxyz" ), OUString( OUString( "foo" ) + d1 ));
    TYPES_ASSERT_EQUAL(( typeid( OUStringConcat< OUString, const char[ 4 ] > )), typeid( OUString( "foo" ) + d1 ));
}

void test::oustring::StringConcat::checkEnsureCapacity()
{
    rtl_uString* str = NULL;
    rtl_uString_newFromLiteral( &str, "test", strlen( "test" ), 0 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, str->refCount );

    rtl_uString* oldStr = str;
    rtl_uString_ensureCapacity( &str, 4 ); // should be no-op
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, str->refCount );
    CPPUNIT_ASSERT( oldStr == str );

    rtl_uString_acquire( oldStr );
    CPPUNIT_ASSERT_EQUAL( 2, str->refCount );
    rtl_uString_ensureCapacity( &str, 4 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length );
    CPPUNIT_ASSERT_EQUAL( 1, str->refCount );
    // a copy was forced because of refcount
    CPPUNIT_ASSERT( oldStr != str );
    CPPUNIT_ASSERT( rtl_ustr_compare( oldStr->buffer, str->buffer ) == 0 );
    CPPUNIT_ASSERT_EQUAL( 1, oldStr->refCount );
    rtl_uString_release( str );
    str = oldStr;

    rtl_uString_acquire( oldStr );
    rtl_uString_ensureCapacity( &str, 1024 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4 ), str->length ); // size is still 4
    CPPUNIT_ASSERT_EQUAL( 1, str->refCount );
    CPPUNIT_ASSERT( oldStr != str );
    CPPUNIT_ASSERT( rtl_ustr_compare( oldStr->buffer, str->buffer ) == 0 );
    CPPUNIT_ASSERT_EQUAL( 1, oldStr->refCount );
    // but there should be extra capacity
    for( int i = 0;
         i < 20;
         ++i )
        str->buffer[ str->length + i ] = '0';
    str->length += 20;
    rtl_uString_release( str );
    rtl_uString_release( oldStr );
}


}} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::StringConcat);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
