/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <optional>
#include <sal/types.h>
#include <comphelper/unwrapargs.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sstream>

class VariadicTemplatesTest : public CppUnit::TestFixture
{
public:
    void testUnwrapArgs();

    CPPUNIT_TEST_SUITE(VariadicTemplatesTest);
// This unit test sometimes generates a compiler crash on macos Intel. Unfortunately the crash
// is sometimes in the codegen phase, so it's not obvious how to fix it.
#if !(defined(MACOSX) && defined __x86_64__ && __clang_major__ <= 14)
    CPPUNIT_TEST(testUnwrapArgs);
#endif
    CPPUNIT_TEST_SUITE_END();
};

#if !(defined(MACOSX) && defined __x86_64__ && __clang_major__ <= 14)

namespace {

namespace detail {

template <typename T>
void extract(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> const& seq,
    sal_Int32 nArg, T & v,
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
    const& xErrorContext )
{
    if (nArg >= seq.getLength()) {
        throw ::com::sun::star::lang::IllegalArgumentException(
            u"No such argument available!"_ustr,
            xErrorContext, static_cast<sal_Int16>(nArg) );
    }
    if (! fromAny(seq[nArg], &v)) {
        throw ::com::sun::star::lang::IllegalArgumentException(
            "Cannot extract ANY { "
            + seq[nArg].getValueType().getTypeName()
            + " } to " + ::cppu::UnoType<T>::get().getTypeName(),
            xErrorContext,
            static_cast<sal_Int16>(nArg) );
    }
}

template <typename T>
void extract(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> const& seq,
    sal_Int32 nArg, ::std::optional<T> & v,
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
void unwrapArgsBaseline(
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

}

void VariadicTemplatesTest::testUnwrapArgs() {
    OUString tmp1   = u"Test1"_ustr;
    sal_Int32 tmp2  = 42;
    sal_uInt32 tmp3 = 42;
    ::com::sun::star::uno::Any tmp6(
        tmp1
        );
    ::com::sun::star::uno::Any tmp7(
        tmp2
        );
    ::com::sun::star::uno::Any tmp8(
        tmp3
        );
    ::com::sun::star::uno::Any tmp9(
        u"Test2"_ustr
        );
    ::std::optional< ::com::sun::star::uno::Any > tmp10(
        u"Test3"_ustr
        );
    ::std::optional< ::com::sun::star::uno::Any > tmp11(
        tmp1
        );

    // test equality with the baseline and template specialization with
    // std::optional< T >
    try {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq1(
            static_cast< sal_uInt32 >( 5 ) );
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > seq2(
            static_cast< sal_uInt32 >( 5 ) );

        // tmp11 should be ignored as it is ::std::optional< T >
        ::comphelper::unwrapArgs( seq1, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11 );
        unwrapArgsBaseline( seq2, tmp6, tmp7, tmp8, tmp9, tmp10 );
        ::com::sun::star::uno::Any* p1 = seq1.getArray();
        ::com::sun::star::uno::Any* p2 = seq2.getArray();

        for( sal_Int32 i = 0; i < seq1.getLength() && i < seq2.getLength(); ++i ) {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "seq1 and seq2 are equal",
                                    p1[i], p2[i] );
        }
        CPPUNIT_ASSERT_MESSAGE( "seq1 and seq2 are equal",
                                bool(seq1 == seq2) );
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

    OUString test1( u"Test2"_ustr );
    OUString test2( u"Test2"_ustr );
    OUString test3( u"Test3"_ustr );
    OUString test4( u"Test4"_ustr );
    OUString test5( u"Test5"_ustr );

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
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "err1.ArgumentPosition == err2.ArgumentPosition",
                                    err1.ArgumentPosition, err2.ArgumentPosition );
        }
    }
}
#endif

CPPUNIT_TEST_SUITE_REGISTRATION(VariadicTemplatesTest);

CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
