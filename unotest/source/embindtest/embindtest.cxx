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

    sal_Unicode SAL_CALL getChar() override { return u'Ö'; }

    sal_Bool SAL_CALL isChar(sal_Unicode value) override { return value == u'Ö'; }

    OUString SAL_CALL getString() override { return u"hä"_ustr; }

    sal_Bool SAL_CALL isString(OUString const& value) override { return value == u"hä"; }

    org::libreoffice::embindtest::Enum SAL_CALL getEnum() override
    {
        return org::libreoffice::embindtest::Enum_E_2;
    }

    sal_Bool SAL_CALL isEnum(org::libreoffice::embindtest::Enum value) override
    {
        return value == org::libreoffice::embindtest::Enum_E_2;
    }

    org::libreoffice::embindtest::Struct SAL_CALL getStruct() override
    {
        return { -123456, 100.5, u"hä"_ustr };
    }

    sal_Bool SAL_CALL isStruct(org::libreoffice::embindtest::Struct const& value) override
    {
        return value.m1 == -123456 && value.m2 == 100.5 && value.m3 == u"hä";
    }

    css::uno::Sequence<sal_Bool> SAL_CALL getSequenceBoolean() override
    {
        return { true, true, false };
    }

    sal_Bool SAL_CALL isSequenceBoolean(css::uno::Sequence<sal_Bool> const& value) override
    {
        return value == css::uno::Sequence<sal_Bool>{ true, true, false };
    }

    css::uno::Sequence<sal_Int8> SAL_CALL getSequenceByte() override { return { -12, 1, 12 }; }

    sal_Bool SAL_CALL isSequenceByte(css::uno::Sequence<sal_Int8> const& value) override
    {
        return value == css::uno::Sequence<sal_Int8>{ -12, 1, 12 };
    }

    css::uno::Sequence<sal_Int16> SAL_CALL getSequenceShort() override
    {
        return { -1234, 1, 1234 };
    }

    sal_Bool SAL_CALL isSequenceShort(css::uno::Sequence<sal_Int16> const& value) override
    {
        return value == css::uno::Sequence<sal_Int16>{ -1234, 1, 1234 };
    }

    css::uno::Sequence<sal_uInt16> SAL_CALL getSequenceUnsignedShort() override
    {
        return { 1, 10, 54321 };
    }

    sal_Bool SAL_CALL isSequenceUnsignedShort(css::uno::Sequence<sal_uInt16> const& value) override
    {
        return value == css::uno::Sequence<sal_uInt16>{ 1, 10, 54321 };
    }

    css::uno::Sequence<sal_Int32> SAL_CALL getSequenceLong() override
    {
        return { -123456, 1, 123456 };
    }

    sal_Bool SAL_CALL isSequenceLong(css::uno::Sequence<sal_Int32> const& value) override
    {
        return value == css::uno::Sequence<sal_Int32>{ -123456, 1, 123456 };
    }

    css::uno::Sequence<sal_uInt32> SAL_CALL getSequenceUnsignedLong() override
    {
        return { 1, 10, 3456789012 };
    }

    sal_Bool SAL_CALL isSequenceUnsignedLong(css::uno::Sequence<sal_uInt32> const& value) override
    {
        return value == css::uno::Sequence<sal_uInt32>{ 1, 10, 3456789012 };
    }

    css::uno::Sequence<sal_Int64> SAL_CALL getSequenceHyper() override
    {
        return { -123456789, 1, 123456789 };
    }

    sal_Bool SAL_CALL isSequenceHyper(css::uno::Sequence<sal_Int64> const& value) override
    {
        return value == css::uno::Sequence<sal_Int64>{ -123456789, 1, 123456789 };
    }

    css::uno::Sequence<sal_uInt64> SAL_CALL getSequenceUnsignedHyper() override
    {
        return { 1, 10, 9876543210 };
    }

    sal_Bool SAL_CALL isSequenceUnsignedHyper(css::uno::Sequence<sal_uInt64> const& value) override
    {
        return value == css::uno::Sequence<sal_uInt64>{ 1, 10, 9876543210 };
    }

    css::uno::Sequence<float> SAL_CALL getSequenceFloat() override
    {
        return { -10.25, 1.5, 10.75 };
    }

    sal_Bool SAL_CALL isSequenceFloat(css::uno::Sequence<float> const& value) override
    {
        return value == css::uno::Sequence<float>{ -10.25, 1.5, 10.75 };
    }

    css::uno::Sequence<double> SAL_CALL getSequenceDouble() override
    {
        return { -100.5, 1.25, 100.75 };
    }

    sal_Bool SAL_CALL isSequenceDouble(css::uno::Sequence<double> const& value) override
    {
        return value == css::uno::Sequence<double>{ -100.5, 1.25, 100.75 };
    }

    css::uno::Sequence<sal_Unicode> SAL_CALL getSequenceChar() override
    {
        return { 'a', 'B', u'Ö' };
    }

    sal_Bool SAL_CALL isSequenceChar(css::uno::Sequence<sal_Unicode> const& value) override
    {
        return value == css::uno::Sequence<sal_Unicode>{ 'a', 'B', u'Ö' };
    }

    css::uno::Sequence<OUString> SAL_CALL getSequenceString() override
    {
        return { u"foo"_ustr, u"barr"_ustr, u"bazzz"_ustr };
    }

    sal_Bool SAL_CALL isSequenceString(css::uno::Sequence<OUString> const& value) override
    {
        return value == css::uno::Sequence<OUString>{ u"foo"_ustr, u"barr"_ustr, u"bazzz"_ustr };
    }

    css::uno::Sequence<css::uno::Sequence<OUString>> SAL_CALL getSequenceSequenceString() override
    {
        return { {}, { u"foo"_ustr, u"barr"_ustr }, { u"baz"_ustr } };
    }

    sal_Bool SAL_CALL
    isSequenceSequenceString(css::uno::Sequence<css::uno::Sequence<OUString>> const& value) override
    {
        return value
               == css::uno::Sequence<css::uno::Sequence<OUString>>{ {},
                                                                    { u"foo"_ustr, u"barr"_ustr },
                                                                    { u"baz"_ustr } };
    }

    css::uno::Sequence<org::libreoffice::embindtest::Enum> SAL_CALL getSequenceEnum() override
    {
        return { org::libreoffice::embindtest::Enum_E_2, org::libreoffice::embindtest::Enum_E3,
                 org::libreoffice::embindtest::Enum_E_10 };
    }

    sal_Bool SAL_CALL
    isSequenceEnum(css::uno::Sequence<org::libreoffice::embindtest::Enum> const& value) override
    {
        return value
               == css::uno::Sequence<org::libreoffice::embindtest::Enum>{
                      org::libreoffice::embindtest::Enum_E_2, org::libreoffice::embindtest::Enum_E3,
                      org::libreoffice::embindtest::Enum_E_10
                  };
    }

    css::uno::Sequence<org::libreoffice::embindtest::Struct> SAL_CALL getSequenceStruct() override
    {
        return { { -123456, -100.5, u"foo"_ustr },
                 { 1, 1.25, u"barr"_ustr },
                 { 123456, 100.75, u"bazzz"_ustr } };
    }

    sal_Bool SAL_CALL
    isSequenceStruct(css::uno::Sequence<org::libreoffice::embindtest::Struct> const& value) override
    {
        return value
               == css::uno::Sequence<org::libreoffice::embindtest::Struct>{
                      { -123456, -100.5, u"foo"_ustr },
                      { 1, 1.25, u"barr"_ustr },
                      { 123456, 100.75, u"bazzz"_ustr }
                  };
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
