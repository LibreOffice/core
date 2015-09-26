/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <comphelper/unwrapargs.hxx>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include <sstream>

class VariadicTemplatesTest : public CppUnit::TestFixture
{
public:
    void testUnwrapArgs();

    CPPUNIT_TEST_SUITE(VariadicTemplatesTest);
    CPPUNIT_TEST(testUnwrapArgs);
    CPPUNIT_TEST_SUITE_END();
};

namespace {

namespace detail {

template <typename T>
inline void extract(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> const& seq,
    sal_Int32 nArg, T & v,
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
    const& xErrorContext )
{
    if (nArg >= seq.getLength()) {
        throw ::com::sun::star::lang::IllegalArgumentException(
            OUString( "No such argument available!"),
            xErrorContext, static_cast<sal_Int16>(nArg) );
    }
    if (! (seq[nArg] >>= v)) {
        OUStringBuffer buf;
        buf.append( "Cannot extract ANY { " );
        buf.append( seq[nArg].getValueType().getTypeName() );
        buf.append( " } to " );
        buf.append( ::cppu::UnoType<T>::get().getTypeName() );
        buf.append( static_cast<sal_Unicode>('!') );
        throw ::com::sun::star::lang::IllegalArgumentException(
            buf.makeStringAndClear(), xErrorContext,
            static_cast<sal_Int16>(nArg) );
    }
}

template <typename T>
inline void extract(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> const& seq,
    sal_Int32 nArg, ::boost::optional<T> & v,
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
    const& xErrorContext )
{
    if (nArg < seq.getLength()) {
        T t;
        extract( seq, nArg, t, xErrorContext );
        v = t;
    }
}

} // namespace detail

template < typename T1, typename T2, typename T3, typename T4 >
inline void unwrapArgsOrig(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > const& seq,
    T1& v0, T2& v1, T3& v2, T4& v3,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XInterface> const& xErrorContext =
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>() )
{
    ::detail::extract( seq, 0, v0, xErrorContext );
    ::detail::extract( seq, 1, v1, xErrorContext );
    ::detail::extract( seq, 2, v2, xErrorContext );
    ::detail::extract( seq, 3, v3, xErrorContext );
}

}

void VariadicTemplatesTest::testUnwrapArgs() {
    OUString tmp1   = "Test1";
    sal_Int32 tmp2  = 42;
    sal_uInt32 tmp3 = 42;
    OUString tmp4   = "Test2";
    OUString tmp5   = "Test3";
    ::boost::optional< ::com::sun::star::uno::Any > tmp6(
        ::com::sun::star::uno::makeAny( tmp1 )
        );
    ::boost::optional< ::com::sun::star::uno::Any > tmp7(
        ::com::sun::star::uno::makeAny( tmp2 )
        );
    ::boost::optional< ::com::sun::star::uno::Any > tmp8(
        ::com::sun::star::uno::makeAny( tmp3 )
        );
    ::boost::optional< ::com::sun::star::uno::Any > tmp9(
        ::com::sun::star::uno::makeAny( tmp4 )
        );
    ::boost::optional< ::com::sun::star::uno::Any > tmp10(
        ::com::sun::star::uno::makeAny( tmp5 )
        );
    try {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq1(
            static_cast< sal_uInt32 >( 4 ) );
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq2(
            static_cast< sal_uInt32 >( 4 ) );
        ::comphelper::unwrapArgs( seq1, tmp6, tmp7, tmp8, tmp9 );
        unwrapArgsOrig( seq2, tmp6, tmp7, tmp8, tmp10 );
        ::com::sun::star::uno::Any* p1 = seq1.getArray();
        ::com::sun::star::uno::Any* p2 = seq2.getArray();
        for( sal_Int16 i = 0; i < seq1.getLength() && i < seq2.getLength(); ++i ) {
            CPPUNIT_ASSERT_MESSAGE( "seq1 and seq2 are equal",
                                    p1[i] == p2[i] );
        }
        CPPUNIT_ASSERT_MESSAGE( "seq1 and seq2 are equal",
                                seq1 == seq2 );
    }
    catch( ::com::sun::star::lang::IllegalArgumentException& err ) {
        std::stringstream ss;
        ss << "IllegalArgumentException when unwrapping arguments at: " <<
            err.ArgumentPosition;
        CPPUNIT_FAIL( ss.str() );
    }

    try {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq(
            static_cast< sal_uInt32 >( 4 ) );
        ::comphelper::unwrapArgs( seq, tmp6, tmp7, tmp8, tmp9, tmp10 );
    }
    catch( ::com::sun::star::lang::IllegalArgumentException& err ) {
        CPPUNIT_ASSERT_EQUAL( err.ArgumentPosition, static_cast< short >( 4 ) );
    }

    try {
        ::boost::optional< bool > test1( true );
        ::boost::optional< OUString > test2( "Test2" );
        ::boost::optional< OUString > test3( "Test3" );
        ::boost::optional< OUString > test4( "Test4" );

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq(
            static_cast< sal_uInt32 >( 4 ) );
        ::comphelper::unwrapArgs( seq, test1, test2, test3, test4 );
    }
    catch( ::com::sun::star::lang::IllegalArgumentException& err ) {
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(VariadicTemplatesTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
