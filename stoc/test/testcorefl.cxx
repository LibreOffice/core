/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include <sal/main.h>
#include <osl/diagnose.h>

#if OSL_DEBUG_LEVEL > 0
#define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
#else
#define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
#endif

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase1.hxx>

#include <ModuleA/XInterface1.hpp>
#include <ModuleC/XInterfaceA.hpp>
#include <ModuleC/XInterfaceB.hpp>
#include <ModuleC/ModuleC.hpp>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>

#include <rtl/ustrbuf.hxx>

#include <stdio.h>


using namespace cppu;
using namespace osl;
using namespace ModuleA;
using namespace ModuleB;
using namespace ModuleC;
using namespace ModuleA::ModuleB;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::reflection;
using namespace com::sun::star::container;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;


//==================================================================================================
class OInterfaceA : public WeakImplHelper1< XInterfaceA >
{
public:

    virtual void SAL_CALL methodA(void) throw (RuntimeException)
        {}

    virtual void SAL_CALL methodB(sal_Int16 /*aShort*/) throw (RuntimeException)
        {}
    virtual Sequence< StructB > SAL_CALL methodC(const StructC& /*aStructC*/, StructA& /*aStructA*/) throw (RuntimeException)
        { return Sequence< StructB >(); }
};

//==================================================================================================
static inline bool uik_equals( const Uik & rUik1, const Uik & rUik2 )
{
    return (rUik1.m_Data1 == rUik2.m_Data1 &&
            rUik1.m_Data2 == rUik2.m_Data2 &&
            rUik1.m_Data3 == rUik2.m_Data3 &&
            rUik1.m_Data4 == rUik2.m_Data4 &&
            rUik1.m_Data5 == rUik2.m_Data5);
}
//==================================================================================================
static sal_Bool test_corefl( const Reference< XIdlReflection > & xRefl )
{
    Reference< XIdlClass > xClass;
    Reference< XHierarchicalNameAccess > xHNameAccess( xRefl, UNO_QUERY );
    TEST_ENSHURE(xHNameAccess.is(), "### cannot get XHierarchicalNameAccess!" );

    TEST_ENSHURE(xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructA")))->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructA")), "test_RegCoreReflection(): error 2b");
    TEST_ENSHURE(xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.ExceptionB")))->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 2c");
    TEST_ENSHURE(xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.ModuleB.EnumA"))).is(), "test_RegCoreReflection(): error 2e");
    // const

    TEST_ENSHURE(*(const sal_Bool *)xHNameAccess->getByHierarchicalName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.aConstBoolean"))).getValue() == aConstBoolean, "test_RegCoreReflection(): error 4c");
    TEST_ENSHURE(*(const sal_Int8 *)xHNameAccess->getByHierarchicalName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.aConstByte"))).getValue() == aConstByte, "test_RegCoreReflection(): error 4e");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.aConstShort"))) == aConstShort, "test_RegCoreReflection(): error 4g");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.aConstUShort"))) == aConstUShort, "test_RegCoreReflection(): error 4i");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.aConstLong"))) == aConstLong, "test_RegCoreReflection(): error 4k");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.aConstULong"))) == aConstULong, "test_RegCoreReflection(): error 4m");
//      TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.aConstFloat"))) == aConstFloat, "test_RegCoreReflection(): error 4o");
//      TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.aConstDouble"))) == aConstDouble, "test_RegCoreReflection(): error 4q");

    // Enums

    xClass = xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.ModuleB.EnumA")));

    TEST_ENSHURE(xClass.is(), "test_RegCoreReflection(): error 5");

    Sequence<Reference< XIdlField > > fields = xClass->getFields();

    TEST_ENSHURE(
        (fields.getLength() == 3) &&
        (fields.getArray()[0]->getName() == OUString( RTL_CONSTASCII_USTRINGPARAM("VAL_1") )) &&
        (*(EnumA*)fields.getArray()[0]->get(Any()).getValue() == EnumA_VAL_1) &&
        (fields.getArray()[1]->getName() == OUString( RTL_CONSTASCII_USTRINGPARAM("VAL_2") )) &&
        (*(EnumA*)fields.getArray()[1]->get(Any()).getValue() == EnumA_VAL_2) &&
        (fields.getArray()[2]->getName() == OUString( RTL_CONSTASCII_USTRINGPARAM("VAL_3") )) &&
        (*(EnumA*)fields.getArray()[2]->get(Any()).getValue() == EnumA_VAL_3),
        "test_RegCoreReflection(): error 6");


    // Interface

    Reference< XIdlClass > xA = xRefl->forName( OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceB")) );

    xClass = xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceB")));

    TEST_ENSHURE(xClass == xA, "test_RegCoreReflection(): error 7");
    TEST_ENSHURE(xClass.is(), "test_RegCoreReflection(): error 7a");

    typelib_TypeDescription * pTD = 0;
    OUString aModuleName( RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceB") );
    typelib_typedescription_getByName( &pTD, aModuleName.pData );
    TEST_ENSHURE( pTD, "### cannot get typedescription for ModuleC.XInterfaceB!" );

    TEST_ENSHURE( uik_equals( *(Uik *)&((typelib_InterfaceTypeDescription *)pTD)->aUik,
                              xClass->getUik() ),
                  "test_RegCoreReflection(): error 8" );
    typelib_typedescription_release( pTD );

    TEST_ENSHURE(xClass->getSuperclasses().getLength() == 1, "test_RegCoreReflection(): error 9");
    TEST_ENSHURE(xClass->getSuperclasses().getArray()[0]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceA")), "test_RegCoreReflection(): error 10");
    TEST_ENSHURE(xClass->getMethods().getLength() == 7, "test_RegCoreReflection(): error 11");
    TEST_ENSHURE(xA->getMethods().getLength() == 7, "test_RegCoreReflection(): error 11a");
    TEST_ENSHURE(xClass->getMethods().getArray()[3]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("methodA")), "test_RegCoreReflection(): 12");
    TEST_ENSHURE(xClass->getMethods().getArray()[3]->getReturnType()->getTypeClass() == TypeClass_VOID, "test_RegCoreReflection(): error 13");
    TEST_ENSHURE(xClass->getMethods().getArray()[3]->getParameterTypes().getLength() == 0, "test_RegCoreReflection(): error 14");
    TEST_ENSHURE(xClass->getMethods().getArray()[3]->getExceptionTypes().getLength() == 0, "test_RegCoreReflection(): error 15");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getName() == OUString( RTL_CONSTASCII_USTRINGPARAM("methodB") ), "test_RegCoreReflection(): error 16");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getMode() == MethodMode_ONEWAY, "test_RegCoreReflection(): error 16a");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getReturnType()->getTypeClass() == TypeClass_VOID, "test_RegCoreReflection(): error 16");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getParameterTypes().getLength() == 1, "test_RegCoreReflection(): error 17");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getParameterTypes().getArray()[0]->getTypeClass() == TypeClass_SHORT, "test_RegCoreReflection(): error 18");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getParameterInfos().getArray()[0].aName == OUString( RTL_CONSTASCII_USTRINGPARAM("aShort") ), "test_RegCoreReflection(): error 18a");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getParameterInfos().getArray()[0].aType == xRefl->forName( OUString( RTL_CONSTASCII_USTRINGPARAM("short") ) ), "test_RegCoreReflection(): error 18b");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getParameterInfos().getArray()[0].aMode == ParamMode_IN, "test_RegCoreReflection(): error 18c");
    TEST_ENSHURE(xClass->getMethods().getArray()[4]->getExceptionTypes().getLength() == 0, "test_RegCoreReflection(): error 19");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("methodC")), "test_RegCoreReflection(): error 20");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getMode() == MethodMode_TWOWAY, "test_RegCoreReflection(): error 20a");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getReturnType()->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 21");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getReturnType()->getComponentType()->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 22");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getReturnType()->getComponentType()->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructB")), "test_RegCoreReflection(): error 23");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getLength() == 2, "test_RegCoreReflection(): error 24");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[0]->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 25");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[0]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")), "test_RegCoreReflection(): error 26");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[1]->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 27");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[1]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructA")), "test_RegCoreReflection(): error 28");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getExceptionTypes().getLength() == 0, "test_RegCoreReflection(): error 29");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("methodD")), "test_RegCoreReflection(): error 30");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getReturnType()->getTypeClass() == TypeClass_INTERFACE, "test_RegCoreReflection(): error 31");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getReturnType()->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceA")), "test_RegCoreReflection(): error 32");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getParameterTypes().getLength() == 1, "test_RegCoreReflection(): error 33");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getParameterTypes().getArray()[0]->getTypeClass() == TypeClass_ENUM, "test_RegCoreReflection(): error 34");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getParameterTypes().getArray()[0]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.ModuleB.EnumA")), "test_RegCoreReflection(): error 35");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getLength() == 3, "test_RegCoreReflection(): error 36");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[0]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 37");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[0]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.ExceptionA")), "test_RegCoreReflection(): error 38");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[1]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 38");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[1]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.ExceptionB")), "test_RegCoreReflection(): error 39");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 40");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.ExceptionC")), "test_RegCoreReflection(): error 41");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getLength() == 3, "test_RegCoreReflection(): error 42");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[0]->getType()->getTypeClass() == TypeClass_BOOLEAN, "test_RegCoreReflection(): error 43");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[0]->getType()->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("boolean")), "test_RegCoreReflection(): error 43a");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[1]->getType()->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 44");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[1]->getType()->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")), "test_RegCoreReflection(): error 45");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[2]->getType()->getTypeClass() == TypeClass_INTERFACE, "test_RegCoreReflection(): error 46");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[2]->getType()->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.XInterface1")), "test_RegCoreReflection(): error 47");

    // SequenceReflections

    TEST_ENSHURE(xRefl->forName( OUString(RTL_CONSTASCII_USTRINGPARAM("[]ModuleA.StructA")) )->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 48");
    TEST_ENSHURE(xRefl->forName( OUString(RTL_CONSTASCII_USTRINGPARAM("[]ModuleA.StructA")) )->getComponentType().is(), "test_RegCoreReflection(): error 49");
    TEST_ENSHURE(xRefl->forName( OUString(RTL_CONSTASCII_USTRINGPARAM("[][]ModuleA.StructA")) )->getComponentType()->getComponentType()->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructA")), "test_RegCoreReflection(): error 50");
    TEST_ENSHURE(xRefl->forName( OUString(RTL_CONSTASCII_USTRINGPARAM("[]com.sun.star.uno.XInterface")) ) == xRefl->forName(OUString( RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC") ))->getField(OUString(RTL_CONSTASCII_USTRINGPARAM("aInterfaceSeq")))->getType(), "test_RegCoreReflection(): error 51");

    StructC aStructC;
    aStructC.aLong = aConstLong;
    aStructC.aShort = aConstShort;
    aStructC.aFloat = aConstFloat;
    aStructC.aDouble = aConstDouble;
    aStructC.aInterfaceSeq = Sequence<Reference<XInterface > >();

    Any aAny;

    xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")))->getField(OUString(RTL_CONSTASCII_USTRINGPARAM("aInterfaceSeq")))->getType()->createObject(aAny);

    TEST_ENSHURE(aAny.getValueType() == ::getCppuType( (const Sequence<Reference< XInterface > > *)0 ), "test_RegCoreReflection(): error 51a");

    Any aStructAny(&aStructC, ::getCppuType( (const StructC *) 0 ));

    sal_Int32 nLong = aConstLong * 2;
    aAny.setValue( &nLong, ::getCppuType( (const sal_Int32 *)0 ) );

    TEST_ENSHURE(*(sal_Int32*)xRefl->forName(OUString( RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructA") ))->getField(OUString( RTL_CONSTASCII_USTRINGPARAM("aLong") ))->get(
        Any(&aStructC, ::getCppuType( (const StructC *)0 ))).getValue() == aConstLong, "test_RegCoreReflection(): error 52");
    TEST_ENSHURE(xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructA")))->getField(OUString( RTL_CONSTASCII_USTRINGPARAM("aLong") ))->getAccessMode() == FieldAccessMode_READWRITE, "test_RegCoreReflection(): error 52a");
    Reference< XIdlField2 > rField ( xRefl->forName(OUString( RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC") ))->getField(OUString( RTL_CONSTASCII_USTRINGPARAM("aLong") )) , UNO_QUERY );
    rField->set(aStructAny, aAny);
    TEST_ENSHURE(*(sal_Int32*)xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructB")))->getField(OUString( RTL_CONSTASCII_USTRINGPARAM("aLong") ))->get(aStructAny).getValue() == *(sal_Int32*)aAny.getValue(), "test_RegCoreReflection(): error 53");

    xRefl->forName( OUString(RTL_CONSTASCII_USTRINGPARAM("[]ModuleA.StructA")) )->createObject(aAny);

    TEST_ENSHURE(aAny.getValueTypeName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]ModuleA.StructA")), "test_RegCoreReflection(): error 54");
    xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("[][]ModuleA.StructA")))->createObject(aAny);

    TEST_ENSHURE(aAny.getValueTypeName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("[][]ModuleA.StructA") ), "test_RegCoreReflection(): error 56");

//      xClass = xRefl->forName(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.XIntroTest") ));

//      TEST_ENSHURE(xClass.is(), "test_RegCoreReflection(): error 58");
//      TEST_ENSHURE(xClass->getMethod(OUString(RTL_CONSTASCII_USTRINGPARAM("getStrings")))->getReturnType()->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 59");
//      TEST_ENSHURE(xClass->getMethod(OUString(RTL_CONSTASCII_USTRINGPARAM("getStrings")))->getReturnType()->getComponentType()->getName() == OUString(RTL_CONSTASCII_USTRINGPARAM("string")), "test_RegCoreReflection(): error 60");

//      xClass->getMethod(OUString(RTL_CONSTASCII_USTRINGPARAM("getStrings")))->getReturnType()->createObject(aAny);
//      TEST_ENSHURE(aAny.getValueTypeName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("[]string") ),  "test_RegCoreReflection(): error 61");

    TEST_ENSHURE(xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("[][][]unsigned long")))->getComponentType()->getComponentType()->getComponentType()->getTypeClass() == TypeClass_UNSIGNED_LONG, "test_RegCoreReflection(): error 62");

    try
    {
        fprintf( stderr, "%1\n" );
        Any bla = xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")))->getField(OUString(RTL_CONSTASCII_USTRINGPARAM("aString")))->get(Any());
        TEST_ENSHURE(sal_False, "test_RegCoreReflection(): error 63");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    try
    {
        fprintf( stderr, "%2\n" );
        Any blup;
        blup <<= aStructC;
        Any gulp;
        rField = Reference< XIdlField2 > ( xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")))->getField(OUString(RTL_CONSTASCII_USTRINGPARAM("aString"))) , UNO_QUERY);
        rField->set( blup, gulp);
//          xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")))->getField(OUString(RTL_CONSTASCII_USTRINGPARAM("aString")))->set(blup, gulp);
        TEST_ENSHURE(sal_False, "test_RegCoreReflection(): error 64");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    try
    {
        fprintf( stderr, "%3\n" );
        Any gulp;
        gulp <<= 3.14f;
        Any blup;
        blup <<= aStructC;
        rField = Reference< XIdlField2 > (
            xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")))->getField(OUString(RTL_CONSTASCII_USTRINGPARAM("aString"))) , UNO_QUERY);
        xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")))->getField(OUString(RTL_CONSTASCII_USTRINGPARAM("aString")))->set(blup, gulp);
        TEST_ENSHURE(sal_False, "test_RegCoreReflection(): error 65");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    Any gulp;
    gulp <<= OUString(OUString(RTL_CONSTASCII_USTRINGPARAM("Test")));
    Any blup;
    blup <<= aStructC;
    xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC")))->getField(OUString(RTL_CONSTASCII_USTRINGPARAM("aString")))->set(blup, gulp);

    Reference< XInterfaceA > xAI = new OInterfaceA();

    try
    {
        Sequence< Any > params;
        fprintf( stderr, "%4\n" );

        Any a;
        a <<= xAI;
        Any bla = xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceA")))->getMethod(OUString(RTL_CONSTASCII_USTRINGPARAM("methodC")))->invoke(a, params);
        TEST_ENSHURE(sal_False, "test_RegCoreReflection(): error 66");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    StructA aStructA;

    {
        Sequence< Any > params(2);

        params.getArray()[0].setValue(&aStructC, ::getCppuType( (const StructC *)0 ));
        params.getArray()[1].setValue(&aStructC, ::getCppuType( (const StructC *)0 ));

        Any a;
        a <<= xAI;
        Any bla = xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceA")))->getMethod(OUString(RTL_CONSTASCII_USTRINGPARAM("methodC")))->invoke(a, params);
    }
    try
    {
        Sequence< Any > params(2);

        params.getArray()[0].setValue(&aStructA, ::getCppuType( (const StructA *)0 ));
        params.getArray()[1].setValue(&aStructA, ::getCppuType( (const StructA *)0 ));

        Any a;
        a <<= xAI;
        Any bla = xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceA")))->getMethod(OUString(RTL_CONSTASCII_USTRINGPARAM("methodC")))->invoke(a, params);
        TEST_ENSHURE(sal_False, "test_RegCoreReflection(): error 67");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    Sequence< Any > params(2);

    params.getArray()[0].setValue(&aStructC, ::getCppuType( (const StructC *)0 ));
    params.getArray()[1].setValue(&aStructA, ::getCppuType( (const StructA *)0 ));

    Any a;
    a <<= xAI;
    TEST_ENSHURE(xRefl->forName(OUString(RTL_CONSTASCII_USTRINGPARAM("ModuleC.XInterfaceA")))->getMethod(OUString(RTL_CONSTASCII_USTRINGPARAM("methodC")))->invoke(a, params).getValueType()
        == ::getCppuType( (const Sequence<StructB> *)0 ), "test_RegCoreReflection(): error 68");

    return sal_True;
}

SAL_IMPLEMENT_MAIN()
{
    sal_Bool bSucc = sal_False;
    try
    {
        OUString aLibName( RTL_CONSTASCII_USTRINGPARAM(
                               "reflection.uno" SAL_DLLEXTENSION) );

        Reference< XMultiServiceFactory > xMgr(
            createRegistryServiceFactory(
                OUString( RTL_CONSTASCII_USTRINGPARAM("stoctest.rdb") ) ) );
        Reference< XComponentContext > xContext;
        Reference< beans::XPropertySet > xProps( xMgr, UNO_QUERY );
        OSL_ASSERT( xProps.is() );
        xProps->getPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>=
            xContext;
        OSL_ASSERT( xContext.is() );

        Reference< XIdlReflection > xRefl;
        xContext->getValueByName(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/com.sun.star.reflection.theCoreReflection")) )
                        >>= xRefl;
        OSL_ENSURE(
            xRefl.is(), "### CoreReflection singleton not accessable!?" );

        bSucc = test_corefl( xRefl );

        Reference< XComponent >( xContext, UNO_QUERY )->dispose();
    }
    catch (Exception & rExc)
    {
        OSL_ENSURE( sal_False, "### exception occurred!" );
        OString aMsg(
            OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### exception occurred: " );
        OSL_TRACE( aMsg.getStr() );
        OSL_TRACE( "\n" );
    }

    printf( "testcorefl %s !\n", (bSucc ? "succeeded" : "failed") );
    return (bSucc ? 0 : -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
