/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weak.hxx>
#include <org/libreoffice/embindtest/Struct.hpp>
#include <org/libreoffice/embindtest/XTest.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star::uno
{
class Any;
class XComponentContext;
}

namespace
{
class Test : public cppu::WeakImplHelper<org::libreoffice::embindtest::XTest>
{
public:
    sal_Bool SAL_CALL getBoolean() override { return true; }

    sal_Bool SAL_CALL isBoolean(sal_Bool value) override { return value; }

    sal_Int8 SAL_CALL getByte() override { return -12; }

    sal_Bool SAL_CALL isByte(sal_Int8 value) override { return value == -12; }

    sal_Int16 SAL_CALL getShort() override { return -1234; }

    sal_Bool SAL_CALL isShort(sal_Int16 value) override { return value == -1234; }

    sal_uInt16 SAL_CALL getUnsignedShort() override { return 54321; }

    sal_Bool SAL_CALL isUnsignedShort(sal_uInt16 value) override { return value == 54321; }

    sal_Int32 SAL_CALL getLong() override { return -123456; }

    sal_Bool SAL_CALL isLong(sal_Int32 value) override { return value == -123456; }

    sal_uInt32 SAL_CALL getUnsignedLong() override { return 3456789012; }

    sal_Bool SAL_CALL isUnsignedLong(sal_uInt32 value) override { return value == 3456789012; }

    sal_Int64 SAL_CALL getHyper() override { return -123456789; }

    sal_Bool SAL_CALL isHyper(sal_Int64 value) override { return value == -123456789; }

    sal_uInt64 SAL_CALL getUnsignedHyper() override { return 9876543210; }

    sal_Bool SAL_CALL isUnsignedHyper(sal_uInt64 value) override { return value == 9876543210; }

    float SAL_CALL getFloat() override { return -10.25; }

    sal_Bool SAL_CALL isFloat(float value) override { return value == -10.25; }

    double SAL_CALL getDouble() override { return 100.5; }

    sal_Bool SAL_CALL isDouble(double value) override { return value == 100.5; }

    OUString SAL_CALL getString() override { return u"hä"_ustr; }

    sal_Bool SAL_CALL isString(OUString const& value) override { return value == u"hä"; }

    org::libreoffice::embindtest::Struct SAL_CALL getStruct() override
    {
        return { -123456, 100.5, u"hä"_ustr };
    }

    sal_Bool SAL_CALL isStruct(org::libreoffice::embindtest::Struct const& value) override
    {
        return value.m1 == -123456 && value.m2 == 100.5 && value.m3 == u"hä";
    }
};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_embindtest_Test_get_implementation(css::uno::XComponentContext*,
                                                        css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
