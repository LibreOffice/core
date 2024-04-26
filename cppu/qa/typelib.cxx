/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/script/FinishReason.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppu/unotype.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <typelib/typedescription.h>
#include <typelib/typedescription.hxx>

// Test that typelib_typedescription_register as called from typelib_typedescription_complete
// returns a correct typelib_TypeDescription and keeps pointers from the original
// typelib_TypeDescription intact (see tdf#115399 "Data race in typelib_typedescription_register").
// This code uses sufficiently "obscure" types in typelib_static_*_type_init to make it unlikely
// that they are already instantiated and registered with the type description manager, which might
// cause inconsistencies.

namespace
{
class Test : public CppUnit::TestFixture
{
public:
    void testEnum()
    {
        typelib_TypeDescriptionReference* ref = nullptr;
        typelib_static_enum_type_init(&ref, "com.sun.star.script.MemberType", 0);
        CPPUNIT_ASSERT(ref != nullptr);
        typelib_TypeDescription* td = ref->pType;
        CPPUNIT_ASSERT(td != nullptr);
        typelib_typedescription_acquire(td);
        CPPUNIT_ASSERT(!td->bComplete);
        auto t = reinterpret_cast<typelib_EnumTypeDescription*>(td);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nDefaultEnumValue);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nEnumValues);
        CPPUNIT_ASSERT(t->ppEnumNames == nullptr);
        CPPUNIT_ASSERT(t->pEnumValues == nullptr);
        CPPUNIT_ASSERT(typelib_typedescription_complete(&td));
        CPPUNIT_ASSERT(td != nullptr);
        CPPUNIT_ASSERT(td->bComplete);
        t = reinterpret_cast<typelib_EnumTypeDescription*>(td);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nDefaultEnumValue);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), t->nEnumValues);
        CPPUNIT_ASSERT(t->ppEnumNames != nullptr);
        CPPUNIT_ASSERT_EQUAL(u"METHOD"_ustr, OUString::unacquired(&t->ppEnumNames[0]));
        CPPUNIT_ASSERT_EQUAL(u"PROPERTY"_ustr, OUString::unacquired(&t->ppEnumNames[1]));
        CPPUNIT_ASSERT_EQUAL(u"UNKNOWN"_ustr, OUString::unacquired(&t->ppEnumNames[2]));
        CPPUNIT_ASSERT(t->pEnumValues != nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->pEnumValues[0]);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), t->pEnumValues[1]);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), t->pEnumValues[2]);
        typelib_typedescription_release(td);
        typelib_typedescriptionreference_release(ref);
    }

    void testStruct()
    {
        auto const t0 = cppu::UnoType<css::lang::EventObject>::get();
        auto const t1 = cppu::UnoType<css::script::FinishReason>::get();
        auto const t2 = cppu::UnoType<OUString>::get();
        auto const t3 = cppu::UnoType<css::uno::Any>::get();
        typelib_TypeDescriptionReference* ref = nullptr;
        typelib_TypeDescriptionReference* members[3]
            = { t1.getTypeLibType(), t2.getTypeLibType(), t3.getTypeLibType() };
        typelib_static_struct_type_init(&ref, "com.sun.star.script.FinishEngineEvent",
                                        t0.getTypeLibType(), 3, members, nullptr);
        CPPUNIT_ASSERT(ref != nullptr);
        typelib_TypeDescription* td = ref->pType;
        CPPUNIT_ASSERT(td != nullptr);
        typelib_typedescription_acquire(td);
        CPPUNIT_ASSERT(!td->bComplete);
        auto t = reinterpret_cast<typelib_StructTypeDescription*>(td);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->aBase.pBaseTypeDescription->aBase).equals(t0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), t->aBase.nMembers);
        auto const offsets = t->aBase.pMemberOffsets;
        CPPUNIT_ASSERT(offsets != nullptr);
        auto const typerefs = t->aBase.ppTypeRefs;
        CPPUNIT_ASSERT(typerefs != nullptr);
        CPPUNIT_ASSERT_EQUAL(t1, css::uno::Type(typerefs[0]));
        CPPUNIT_ASSERT_EQUAL(t2, css::uno::Type(typerefs[1]));
        CPPUNIT_ASSERT_EQUAL(t3, css::uno::Type(typerefs[2]));
        CPPUNIT_ASSERT(t->aBase.ppMemberNames == nullptr);
        CPPUNIT_ASSERT(t->pParameterizedTypes == nullptr);
        CPPUNIT_ASSERT(typelib_typedescription_complete(&td));
        CPPUNIT_ASSERT(td != nullptr);
        CPPUNIT_ASSERT(td->bComplete);
        t = reinterpret_cast<typelib_StructTypeDescription*>(td);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->aBase.pBaseTypeDescription->aBase).equals(t0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), t->aBase.nMembers);
        CPPUNIT_ASSERT(t->aBase.pMemberOffsets != nullptr);
        CPPUNIT_ASSERT(t->aBase.ppTypeRefs != nullptr);
        CPPUNIT_ASSERT_EQUAL(t1, css::uno::Type(t->aBase.ppTypeRefs[0]));
        CPPUNIT_ASSERT_EQUAL(t2, css::uno::Type(t->aBase.ppTypeRefs[1]));
        CPPUNIT_ASSERT_EQUAL(t3, css::uno::Type(t->aBase.ppTypeRefs[2]));
        CPPUNIT_ASSERT(t->aBase.ppMemberNames != nullptr);
        CPPUNIT_ASSERT_EQUAL(u"Finish"_ustr, OUString::unacquired(&t->aBase.ppMemberNames[0]));
        CPPUNIT_ASSERT_EQUAL(u"ErrorMessage"_ustr,
                             OUString::unacquired(&t->aBase.ppMemberNames[1]));
        CPPUNIT_ASSERT_EQUAL(u"Return"_ustr, OUString::unacquired(&t->aBase.ppMemberNames[2]));
        CPPUNIT_ASSERT(t->pParameterizedTypes == nullptr);
        // `offsets` and `typerefs` must still be valid:
        CPPUNIT_ASSERT_EQUAL(t->aBase.pMemberOffsets[0], offsets[0]);
        CPPUNIT_ASSERT_EQUAL(t->aBase.pMemberOffsets[1], offsets[1]);
        CPPUNIT_ASSERT_EQUAL(t->aBase.pMemberOffsets[2], offsets[2]);
        CPPUNIT_ASSERT_EQUAL(css::uno::Type(t->aBase.ppTypeRefs[0]), css::uno::Type(typerefs[0]));
        CPPUNIT_ASSERT_EQUAL(css::uno::Type(t->aBase.ppTypeRefs[1]), css::uno::Type(typerefs[1]));
        CPPUNIT_ASSERT_EQUAL(css::uno::Type(t->aBase.ppTypeRefs[2]), css::uno::Type(typerefs[2]));
        typelib_typedescription_release(td);
        typelib_typedescriptionreference_release(ref);
    }

    void testPolyStruct()
    {
        auto const t1 = cppu::UnoType<bool>::get();
        auto const t2 = cppu::UnoType<sal_Int32>::get();
        typelib_TypeDescriptionReference* ref = nullptr;
        typelib_TypeDescriptionReference* members[2] = { t1.getTypeLibType(), t2.getTypeLibType() };
        sal_Bool const param[2] = { false, true };
        typelib_static_struct_type_init(&ref, "com.sun.star.beans.Optional<long>", nullptr, 2,
                                        members, param);
        CPPUNIT_ASSERT(ref != nullptr);
        typelib_TypeDescription* td = ref->pType;
        CPPUNIT_ASSERT(td != nullptr);
        typelib_typedescription_acquire(td);
        CPPUNIT_ASSERT(!td->bComplete);
        auto t = reinterpret_cast<typelib_StructTypeDescription*>(td);
        CPPUNIT_ASSERT(t->aBase.pBaseTypeDescription == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), t->aBase.nMembers);
        auto const offsets = t->aBase.pMemberOffsets;
        CPPUNIT_ASSERT(offsets != nullptr);
        auto const typerefs = t->aBase.ppTypeRefs;
        CPPUNIT_ASSERT(typerefs != nullptr);
        CPPUNIT_ASSERT_EQUAL(t1, css::uno::Type(typerefs[0]));
        CPPUNIT_ASSERT_EQUAL(t2, css::uno::Type(typerefs[1]));
        CPPUNIT_ASSERT(t->aBase.ppMemberNames == nullptr);
        CPPUNIT_ASSERT(t->pParameterizedTypes != nullptr);
        CPPUNIT_ASSERT_EQUAL(param[0], t->pParameterizedTypes[0]);
        CPPUNIT_ASSERT_EQUAL(param[1], t->pParameterizedTypes[1]);
        CPPUNIT_ASSERT(typelib_typedescription_complete(&td));
        CPPUNIT_ASSERT(td != nullptr);
        CPPUNIT_ASSERT(td->bComplete);
        t = reinterpret_cast<typelib_StructTypeDescription*>(td);
        CPPUNIT_ASSERT(t->aBase.pBaseTypeDescription == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), t->aBase.nMembers);
        CPPUNIT_ASSERT(t->aBase.pMemberOffsets != nullptr);
        CPPUNIT_ASSERT(t->aBase.ppTypeRefs != nullptr);
        CPPUNIT_ASSERT_EQUAL(t1, css::uno::Type(t->aBase.ppTypeRefs[0]));
        CPPUNIT_ASSERT_EQUAL(t2, css::uno::Type(t->aBase.ppTypeRefs[1]));
        CPPUNIT_ASSERT(t->aBase.ppMemberNames != nullptr);
        CPPUNIT_ASSERT_EQUAL(u"IsPresent"_ustr, OUString::unacquired(&t->aBase.ppMemberNames[0]));
        CPPUNIT_ASSERT_EQUAL(u"Value"_ustr, OUString::unacquired(&t->aBase.ppMemberNames[1]));
        CPPUNIT_ASSERT(t->pParameterizedTypes != nullptr);
        CPPUNIT_ASSERT_EQUAL(param[0], t->pParameterizedTypes[0]);
        CPPUNIT_ASSERT_EQUAL(param[1], t->pParameterizedTypes[1]);
        // `offsets` and `typerefs` must still be valid:
        CPPUNIT_ASSERT_EQUAL(t->aBase.pMemberOffsets[0], offsets[0]);
        CPPUNIT_ASSERT_EQUAL(t->aBase.pMemberOffsets[1], offsets[1]);
        CPPUNIT_ASSERT_EQUAL(css::uno::Type(t->aBase.ppTypeRefs[0]), css::uno::Type(typerefs[0]));
        CPPUNIT_ASSERT_EQUAL(css::uno::Type(t->aBase.ppTypeRefs[1]), css::uno::Type(typerefs[1]));
        typelib_typedescription_release(td);
        typelib_typedescriptionreference_release(ref);
    }

    void testInterface()
    {
        auto const t0 = cppu::UnoType<css::uno::XInterface>::get();
        typelib_TypeDescriptionReference* ref = nullptr;
        typelib_TypeDescriptionReference* bases[1] = { t0.getTypeLibType() };
        typelib_static_mi_interface_type_init(&ref, "com.sun.star.script.XTypeConverter", 1, bases);
        CPPUNIT_ASSERT(ref != nullptr);
        typelib_TypeDescription* td = ref->pType;
        CPPUNIT_ASSERT(td != nullptr);
        typelib_typedescription_acquire(td);
        CPPUNIT_ASSERT(!td->bComplete);
        auto t = reinterpret_cast<typelib_InterfaceTypeDescription*>(td);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->pBaseTypeDescription->aBase).equals(t0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nMembers);
        CPPUNIT_ASSERT(t->ppMembers == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nAllMembers);
        CPPUNIT_ASSERT(t->ppAllMembers == nullptr);
        CPPUNIT_ASSERT(t->pMapMemberIndexToFunctionIndex == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nMapFunctionIndexToMemberIndex);
        CPPUNIT_ASSERT(t->pMapFunctionIndexToMemberIndex == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), t->nBaseTypes);
        auto const basetypes = t->ppBaseTypes;
        CPPUNIT_ASSERT(basetypes != nullptr);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&basetypes[0]->aBase).equals(t0));
        CPPUNIT_ASSERT(typelib_typedescription_complete(&td));
        CPPUNIT_ASSERT(td != nullptr);
        CPPUNIT_ASSERT(td->bComplete);
        t = reinterpret_cast<typelib_InterfaceTypeDescription*>(td);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->pBaseTypeDescription->aBase).equals(t0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), t->nMembers);
        CPPUNIT_ASSERT(t->ppMembers != nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), t->nAllMembers);
        CPPUNIT_ASSERT(t->ppAllMembers != nullptr);
        CPPUNIT_ASSERT(t->pMapMemberIndexToFunctionIndex != nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), t->nMapFunctionIndexToMemberIndex);
        CPPUNIT_ASSERT(t->pMapFunctionIndexToMemberIndex != nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), t->nBaseTypes);
        CPPUNIT_ASSERT(t->ppBaseTypes != nullptr);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->ppBaseTypes[0]->aBase).equals(t0));
        // `basetypes` must still be valid:
        CPPUNIT_ASSERT(
            css::uno::TypeDescription(&basetypes[0]->aBase).equals(&t->ppBaseTypes[0]->aBase));
        typelib_typedescription_release(td);
        typelib_typedescriptionreference_release(ref);
    }

    void testMultiInterface()
    {
        auto const t1 = cppu::UnoType<css::io::XTextInputStream>::get();
        auto const t2 = cppu::UnoType<css::io::XActiveDataSink>::get();
        typelib_TypeDescriptionReference* ref = nullptr;
        typelib_TypeDescriptionReference* bases[2] = { t1.getTypeLibType(), t2.getTypeLibType() };
        typelib_static_mi_interface_type_init(&ref, "com.sun.star.io.XTextInputStream2", 2, bases);
        CPPUNIT_ASSERT(ref != nullptr);
        typelib_TypeDescription* td = ref->pType;
        CPPUNIT_ASSERT(td != nullptr);
        typelib_typedescription_acquire(td);
        CPPUNIT_ASSERT(!td->bComplete);
        auto t = reinterpret_cast<typelib_InterfaceTypeDescription*>(td);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->pBaseTypeDescription->aBase).equals(t1));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nMembers);
        CPPUNIT_ASSERT(t->ppMembers == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nAllMembers);
        CPPUNIT_ASSERT(t->ppAllMembers == nullptr);
        CPPUNIT_ASSERT(t->pMapMemberIndexToFunctionIndex == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nMapFunctionIndexToMemberIndex);
        CPPUNIT_ASSERT(t->pMapFunctionIndexToMemberIndex == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), t->nBaseTypes);
        auto const basetypes = t->ppBaseTypes;
        CPPUNIT_ASSERT(basetypes != nullptr);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&basetypes[0]->aBase).equals(t1));
        CPPUNIT_ASSERT(css::uno::TypeDescription(&basetypes[1]->aBase).equals(t2));
        CPPUNIT_ASSERT(typelib_typedescription_complete(&td));
        CPPUNIT_ASSERT(td != nullptr);
        CPPUNIT_ASSERT(td->bComplete);
        t = reinterpret_cast<typelib_InterfaceTypeDescription*>(td);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->pBaseTypeDescription->aBase).equals(t1));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), t->nMembers);
        CPPUNIT_ASSERT(t->ppMembers == nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), t->nAllMembers);
        CPPUNIT_ASSERT(t->ppAllMembers != nullptr);
        CPPUNIT_ASSERT(t->pMapMemberIndexToFunctionIndex != nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), t->nMapFunctionIndexToMemberIndex);
        CPPUNIT_ASSERT(t->pMapFunctionIndexToMemberIndex != nullptr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), t->nBaseTypes);
        CPPUNIT_ASSERT(t->ppBaseTypes != nullptr);
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->ppBaseTypes[0]->aBase).equals(t1));
        CPPUNIT_ASSERT(css::uno::TypeDescription(&t->ppBaseTypes[1]->aBase).equals(t2));
        // `basetypes` must still be valid:
        CPPUNIT_ASSERT(
            css::uno::TypeDescription(&basetypes[0]->aBase).equals(&t->ppBaseTypes[0]->aBase));
        CPPUNIT_ASSERT(
            css::uno::TypeDescription(&basetypes[1]->aBase).equals(&t->ppBaseTypes[1]->aBase));
        typelib_typedescription_release(td);
        typelib_typedescriptionreference_release(ref);
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testEnum);
    CPPUNIT_TEST(testStruct);
    CPPUNIT_TEST(testPolyStruct);
    CPPUNIT_TEST(testInterface);
    CPPUNIT_TEST(testMultiInterface);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
