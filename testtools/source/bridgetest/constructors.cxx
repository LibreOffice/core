/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: constructors.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-01-26 17:40:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "sal/config.h"

#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppu/unotype.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "test/testtools/bridgetest/BadConstructorArguments.hpp"
#include "test/testtools/bridgetest/TestEnum.hpp"
#include "test/testtools/bridgetest/TestStruct.hpp"
#include "test/testtools/bridgetest/TestPolyStruct.hpp"
#include "uno/lbnames.h"

namespace {

namespace css = com::sun::star;

class Impl: public ::cppu::WeakImplHelper1< css::lang::XInitialization > {
public:
    Impl() {}

private:
    Impl(Impl &); // not defined
    void operator =(Impl &); // not defined

    virtual ~Impl() {}

    virtual void SAL_CALL initialize(
        css::uno::Sequence< css::uno::Any > const & arguments)
        throw (css::uno::Exception);
};

void Impl::initialize(css::uno::Sequence< css::uno::Any > const & arguments)
    throw (css::uno::Exception)
{
    ::sal_Bool arg0;
    ::sal_Int8 arg1;
    ::sal_Int16 arg2;
    ::sal_uInt16 arg3;
    ::sal_Int32 arg4;
    ::sal_uInt32 arg5;
    ::sal_Int64 arg6;
    ::sal_uInt64 arg7;
    float arg8;
    double arg9;
    ::rtl::OUString arg11;
    css::uno::Type arg12;
    ::sal_Bool arg13;
    css::uno::Sequence< ::sal_Bool > arg14;
    css::uno::Sequence< ::sal_Int8 > arg15;
    css::uno::Sequence< ::sal_Int16 > arg16;
    css::uno::Sequence< ::sal_uInt16 > arg17;
    css::uno::Sequence< ::sal_Int32 > arg18;
    css::uno::Sequence< ::sal_uInt32 > arg19;
    css::uno::Sequence< ::sal_Int64 > arg20;
    css::uno::Sequence< ::sal_uInt64 > arg21;
    css::uno::Sequence< float > arg22;
    css::uno::Sequence< double > arg23;
    css::uno::Sequence< ::rtl::OUString > arg25;
    css::uno::Sequence< css::uno::Type > arg26;
    css::uno::Sequence< css::uno::Any > arg27;
    ::sal_Bool arg27b;
    css::uno::Sequence< css::uno::Sequence< ::sal_Bool > > arg28;
    css::uno::Sequence< css::uno::Sequence< css::uno::Any > > arg29;
    ::sal_Bool arg29b;
    css::uno::Sequence< ::test::testtools::bridgetest::TestEnum > arg30;
    css::uno::Sequence< ::test::testtools::bridgetest::TestStruct > arg31;
    css::uno::Sequence<
        ::test::testtools::bridgetest::TestPolyStruct< sal_Bool > > arg32;
    css::uno::Sequence<
        ::test::testtools::bridgetest::TestPolyStruct< css::uno::Any > > arg33;
    ::sal_Bool arg33b;
    css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > arg34;
    ::test::testtools::bridgetest::TestEnum arg35;
    ::test::testtools::bridgetest::TestStruct arg36;
    ::test::testtools::bridgetest::TestPolyStruct< ::sal_Bool > arg37;
    ::test::testtools::bridgetest::TestPolyStruct< css::uno::Any > arg38;
    ::sal_Bool arg38b;
    css::uno::Reference< css::uno::XInterface > arg39;
    if (!(arguments.getLength() == 40
          && (arguments[0] >>= arg0) && arg0
          && (arguments[1] >>= arg1) && arg1 == SAL_MIN_INT8
          && (arguments[2] >>= arg2) && arg2 == SAL_MIN_INT16
          && (arguments[3].getValueType()
              == ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get())
          && (arguments[3] >>= arg3) && arg3 == SAL_MAX_UINT16
          && (arguments[4] >>= arg4) && arg4 == SAL_MIN_INT32
          && (arguments[5] >>= arg5) && arg5 == SAL_MAX_UINT32
          && (arguments[6] >>= arg6) && arg6 == SAL_MIN_INT64
          && (arguments[7] >>= arg7) && arg7 == SAL_MAX_UINT64
          && (arguments[8] >>= arg8) && arg8 == 0.123f
          && (arguments[9] >>= arg9) && arg9 == 0.456
          && (arguments[10].getValueType()
              == ::cppu::UnoType< ::cppu::UnoCharType >::get())
          && (*static_cast< ::sal_Unicode const * >(arguments[10].getValue())
              == 'X')
          && (arguments[11] >>= arg11)
          && arg11.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("test"))
          && (arguments[12] >>= arg12)
          && arg12 == ::cppu::UnoType< css::uno::Any >::get()
          && (arguments[13] >>= arg13) && arg13
          && (arguments[14] >>= arg14) && arg14.getLength() == 1 && arg14[0]
          && (arguments[15] >>= arg15) && arg15.getLength() == 1
          && arg15[0] == SAL_MIN_INT8
          && (arguments[16] >>= arg16) && arg16.getLength() == 1
          && arg16[0] == SAL_MIN_INT16
          && (arguments[17].getValueType()
              == ::cppu::UnoType<
                  ::cppu::UnoSequenceType <
                      ::cppu::UnoUnsignedShortType > >::get())
          && (arguments[17] >>= arg17) && arg17.getLength() == 1
          && arg17[0] == SAL_MAX_UINT16
          && (arguments[18] >>= arg18) && arg18.getLength() == 1
          && arg18[0] == SAL_MIN_INT32
          && (arguments[19] >>= arg19) && arg19.getLength() == 1
          && arg19[0] == SAL_MAX_UINT32
          && (arguments[20] >>= arg20) && arg20.getLength() == 1
          && arg20[0] == SAL_MIN_INT64
          && (arguments[21] >>= arg21) && arg21.getLength() == 1
          && arg21[0] == SAL_MAX_UINT64
          && (arguments[22] >>= arg22) && arg22.getLength() == 1
          && arg22[0] == 0.123f
          && (arguments[23] >>= arg23) && arg23.getLength() == 1
          && arg23[0] == 0.456
          && (arguments[24].getValueType()
              == ::cppu::UnoType<
                  ::cppu::UnoSequenceType< ::cppu::UnoCharType > >::get())
          && (static_cast< css::uno::Sequence< ::sal_Unicode > const * >(
                  arguments[24].getValue())->getLength()
              == 1)
          && ((*static_cast< css::uno::Sequence< ::sal_Unicode > const * >(
                   arguments[24].getValue()))[0]
              == 'X')
          && (arguments[25] >>= arg25) && arg25.getLength() == 1
          && arg25[0].equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("test"))
          && (arguments[26] >>= arg26) && arg26.getLength() == 1
          && arg26[0] == ::cppu::UnoType< css::uno::Any >::get()
          && (arguments[27] >>= arg27) && arg27.getLength() == 1
          && (arg27[0] >>= arg27b) && arg27b
          && (arguments[28] >>= arg28) && arg28.getLength() == 1
          && arg28[0].getLength() == 1 && arg28[0][0]
          && (arguments[29] >>= arg29) && arg29.getLength() == 1
          && arg29[0].getLength() == 1 && (arg29[0][0] >>= arg29b) && arg29b
          && (arguments[30] >>= arg30) && arg30.getLength() == 1
          && arg30[0] == ::test::testtools::bridgetest::TestEnum_TWO
          && (arguments[31] >>= arg31) && arg31.getLength() == 1
          && arg31[0].member == 10
          && (arguments[32] >>= arg32) && arg32.getLength() == 1
          && arg32[0].member
          && (arguments[33] >>= arg33) && arg33.getLength() == 1
          && (arg33[0].member >>= arg33b) && arg33b
          && (arguments[34] >>= arg34) && arg34.getLength() == 1
          && !arg34[0].is()
          && (arguments[35] >>= arg35)
          && arg35 == ::test::testtools::bridgetest::TestEnum_TWO
          && (arguments[36] >>= arg36) && arg36.member == 10
          && (arguments[37] >>= arg37) && arg37.member
          && (arguments[38] >>= arg38) && (arg38.member >>= arg38b) && arg38b
          && (arguments[39] >>= arg39) && !arg39.is()))
    {
        throw ::test::testtools::bridgetest::BadConstructorArguments();
    }
}

css::uno::Reference< css::uno::XInterface > SAL_CALL create(
    css::uno::Reference< css::uno::XComponentContext > const &)
    SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new Impl);
}

::rtl::OUString SAL_CALL getImplementationName() {
    return ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "comp.test.testtools.bridgetest.Constructors"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() {
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("test.testtools.bridgetest.Constructors"));
    return s;
}

::cppu::ImplementationEntry entries[] = {
    { &create, &getImplementationName, &getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 } };

}

extern "C" void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return ::cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    char const ** envTypeName, uno_Environment **)
{
    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" ::sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return ::cppu::component_writeInfoHelper(
        serviceManager, registryKey, entries);
}
