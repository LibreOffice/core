/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <boost/optional.hpp>
#include <sal/types.h>
#include <comphelper/unwrapargs.hxx>
#include <comphelper/servicedecl.hxx>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include <sstream>

class VariadicTemplatesTest : public CppUnit::TestFixture
{
public:
    void testUnwrapArgs();
    void testServiceDecl();

    CPPUNIT_TEST_SUITE(VariadicTemplatesTest);
    CPPUNIT_TEST(testUnwrapArgs);
    CPPUNIT_TEST(testServiceDecl);
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

template < typename T0, typename T1, typename T2, typename T3, typename T4 >
inline void unwrapArgsBaseline(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > const& seq,
    T0& v0, T1& v1, T2& v2, T3& v3, T4& v4,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XInterface> const& xErrorContext =
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>() )
{
    ::detail::extract( seq, 0, v0, xErrorContext );
    ::detail::extract( seq, 1, v1, xErrorContext );
    ::detail::extract( seq, 2, v2, xErrorContext );
    ::detail::extract( seq, 3, v3, xErrorContext );
    ::detail::extract( seq, 4, v4, xErrorContext );
}

struct DummyStruct {
    sal_uInt32 m_x;

    DummyStruct( sal_uInt32 x ): m_x( x ) { }
    DummyStruct() : m_x( 0 ) { }

    void* getFactory( const char* ) const {
        if( m_x == 42 )
            return new int( m_x );
        return nullptr;
    }
};

}

void VariadicTemplatesTest::testUnwrapArgs() {
    OUString tmp1   = "Test1";
    sal_Int32 tmp2  = 42;
    sal_uInt32 tmp3 = 42;
    OUString tmp4   = "Test2";
    OUString tmp5   = "Test3";
    ::com::sun::star::uno::Any tmp6(
        ::com::sun::star::uno::makeAny( tmp1 )
        );
    ::com::sun::star::uno::Any tmp7(
        ::com::sun::star::uno::makeAny( tmp2 )
        );
    ::com::sun::star::uno::Any tmp8(
        ::com::sun::star::uno::makeAny( tmp3 )
        );
    ::com::sun::star::uno::Any tmp9(
        ::com::sun::star::uno::makeAny( tmp4 )
        );
    ::boost::optional< ::com::sun::star::uno::Any > tmp10(
        ::com::sun::star::uno::makeAny( tmp5 )
        );
    ::boost::optional< ::com::sun::star::uno::Any > tmp11(
        ::com::sun::star::uno::makeAny( tmp1 )
        );

    // test equality with the baseline and template specialization with
    // boost::optional< T >
    try {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq1(
            static_cast< sal_uInt32 >( 5 ) );
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq2(
            static_cast< sal_uInt32 >( 5 ) );

        // tmp11 should be ignored as it is ::boost::optional< T >
        ::comphelper::unwrapArgs( seq1, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11 );
        unwrapArgsBaseline( seq2, tmp6, tmp7, tmp8, tmp9, tmp10 );
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

    // test argument counting
    try {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq(
            static_cast< sal_uInt32 >( 4 ) );
        ::comphelper::unwrapArgs( seq, tmp6, tmp7, tmp10, tmp11, tmp10, tmp6 );
    }
    catch( ::com::sun::star::lang::IllegalArgumentException& err ) {
        CPPUNIT_ASSERT_EQUAL( static_cast< short >( 5 ), err.ArgumentPosition );
    }

    OUString test1( "Test2" );
    OUString test2( "Test2" );
    OUString test3( "Test3" );
    OUString test4( "Test4" );
    OUString test5( "Test5" );

    try {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq(
            static_cast< sal_uInt32 >( 4 ) );
        ::comphelper::unwrapArgs( seq, test1, test2, test3, test4, test5 );
    }
    catch( ::com::sun::star::lang::IllegalArgumentException& err1 ) {
        try {
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq(
                static_cast< sal_uInt32 >( 4 ) );
            unwrapArgsBaseline( seq, test1, test2, test3, test4, test5 );
            CPPUNIT_FAIL( "unwrapArgs failed while the baseline did not throw" );
        }
        catch( ::com::sun::star::lang::IllegalArgumentException& err2 ) {
            CPPUNIT_ASSERT_MESSAGE( "err1.ArgumentPosition == err2.ArgumentPosition",
                                    err1.ArgumentPosition == err2.ArgumentPosition );
        }
    }
}

void VariadicTemplatesTest::testServiceDecl() {
    DummyStruct dummy1( 42 );
    DummyStruct dummy2;
    DummyStruct dummy3;
    void* pRet = ::comphelper::service_decl::component_getFactoryHelper( "test",
                                                                         dummy3,
                                                                         dummy2,
                                                                         dummy1 );

    CPPUNIT_ASSERT_MESSAGE( "pRet != 0",
                            pRet != nullptr );

    sal_uInt32* pnRet = static_cast< sal_uInt32* >( pRet );

    CPPUNIT_ASSERT_MESSAGE( "*pnRet == 42",
                            *pnRet == 42 );
    delete pnRet;

    pRet = ::comphelper::service_decl::component_getFactoryHelper( "test",
                                                                   dummy1,
                                                                   dummy2,
                                                                   dummy2 );

    CPPUNIT_ASSERT_MESSAGE( "pRet != nullptr",
                            pRet != nullptr );

    pnRet = static_cast< sal_uInt32* >( pRet );

    CPPUNIT_ASSERT_MESSAGE( "*pnRet == 42",
                            *pnRet == 42 );

    delete pnRet;
}

CPPUNIT_TEST_SUITE_REGISTRATION(VariadicTemplatesTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
