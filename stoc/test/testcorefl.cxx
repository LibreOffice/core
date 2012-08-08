/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/main.h>
#include <osl/diagnose.h>

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
    OSL_ENSURE(xHNameAccess.is(), "### cannot get XHierarchicalNameAccess!" );

    OSL_ENSURE(xRefl->forName(OUString("ModuleA.StructA"))->getName() == OUString("ModuleA.StructA"), "test_RegCoreReflection(): error 2b");
    OSL_ENSURE(xRefl->forName(OUString("ModuleA.ExceptionB"))->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 2c");
    OSL_ENSURE(xRefl->forName(OUString("ModuleA.ModuleB.EnumA")).is(), "test_RegCoreReflection(): error 2e");


    OSL_ENSURE(*(const sal_Bool *)xHNameAccess->getByHierarchicalName(OUString("ModuleC.aConstBoolean")).getValue() == aConstBoolean, "test_RegCoreReflection(): error 4c");
    OSL_ENSURE(*(const sal_Int8 *)xHNameAccess->getByHierarchicalName(OUString("ModuleC.aConstByte")).getValue() == aConstByte, "test_RegCoreReflection(): error 4e");
    OSL_ENSURE(xHNameAccess->getByHierarchicalName(OUString("ModuleC.aConstShort")) == aConstShort, "test_RegCoreReflection(): error 4g");
    OSL_ENSURE(xHNameAccess->getByHierarchicalName(OUString("ModuleC.aConstUShort")) == aConstUShort, "test_RegCoreReflection(): error 4i");
    OSL_ENSURE(xHNameAccess->getByHierarchicalName(OUString("ModuleC.aConstLong")) == aConstLong, "test_RegCoreReflection(): error 4k");
    OSL_ENSURE(xHNameAccess->getByHierarchicalName(OUString("ModuleC.aConstULong")) == aConstULong, "test_RegCoreReflection(): error 4m");
//      OSL_ENSURE(xHNameAccess->getByHierarchicalName(OUString("ModuleC.aConstFloat")) == aConstFloat, "test_RegCoreReflection(): error 4o");
//      OSL_ENSURE(xHNameAccess->getByHierarchicalName(OUString("ModuleC.aConstDouble")) == aConstDouble, "test_RegCoreReflection(): error 4q");

    // Enums

    xClass = xRefl->forName(OUString("ModuleA.ModuleB.EnumA"));

    OSL_ENSURE(xClass.is(), "test_RegCoreReflection(): error 5");

    Sequence<Reference< XIdlField > > fields = xClass->getFields();

    OSL_ENSURE(
        (fields.getLength() == 3) &&
        (fields.getArray()[0]->getName() == OUString( "VAL_1" )) &&
        (*(EnumA*)fields.getArray()[0]->get(Any()).getValue() == EnumA_VAL_1) &&
        (fields.getArray()[1]->getName() == OUString( "VAL_2" )) &&
        (*(EnumA*)fields.getArray()[1]->get(Any()).getValue() == EnumA_VAL_2) &&
        (fields.getArray()[2]->getName() == OUString( "VAL_3" )) &&
        (*(EnumA*)fields.getArray()[2]->get(Any()).getValue() == EnumA_VAL_3),
        "test_RegCoreReflection(): error 6");


    // Interface

    Reference< XIdlClass > xA = xRefl->forName( OUString("ModuleC.XInterfaceB") );

    xClass = xRefl->forName(OUString("ModuleC.XInterfaceB"));

    OSL_ENSURE(xClass == xA, "test_RegCoreReflection(): error 7");
    OSL_ENSURE(xClass.is(), "test_RegCoreReflection(): error 7a");

    typelib_TypeDescription * pTD = 0;
    OUString aModuleName( "ModuleC.XInterfaceB" );
    typelib_typedescription_getByName( &pTD, aModuleName.pData );
    OSL_ENSURE( pTD, "### cannot get typedescription for ModuleC.XInterfaceB!" );

    OSL_ENSURE( uik_equals( *(Uik *)&((typelib_InterfaceTypeDescription *)pTD)->aUik,
                              xClass->getUik() ),
                  "test_RegCoreReflection(): error 8" );
    typelib_typedescription_release( pTD );

    OSL_ENSURE(xClass->getSuperclasses().getLength() == 1, "test_RegCoreReflection(): error 9");
    OSL_ENSURE(xClass->getSuperclasses().getArray()[0]->getName() == OUString("ModuleC.XInterfaceA"), "test_RegCoreReflection(): error 10");
    OSL_ENSURE(xClass->getMethods().getLength() == 7, "test_RegCoreReflection(): error 11");
    OSL_ENSURE(xA->getMethods().getLength() == 7, "test_RegCoreReflection(): error 11a");
    OSL_ENSURE(xClass->getMethods().getArray()[3]->getName() == OUString("methodA"), "test_RegCoreReflection(): 12");
    OSL_ENSURE(xClass->getMethods().getArray()[3]->getReturnType()->getTypeClass() == TypeClass_VOID, "test_RegCoreReflection(): error 13");
    OSL_ENSURE(xClass->getMethods().getArray()[3]->getParameterTypes().getLength() == 0, "test_RegCoreReflection(): error 14");
    OSL_ENSURE(xClass->getMethods().getArray()[3]->getExceptionTypes().getLength() == 0, "test_RegCoreReflection(): error 15");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getName() == OUString( "methodB" ), "test_RegCoreReflection(): error 16");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getMode() == MethodMode_ONEWAY, "test_RegCoreReflection(): error 16a");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getReturnType()->getTypeClass() == TypeClass_VOID, "test_RegCoreReflection(): error 16");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getParameterTypes().getLength() == 1, "test_RegCoreReflection(): error 17");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getParameterTypes().getArray()[0]->getTypeClass() == TypeClass_SHORT, "test_RegCoreReflection(): error 18");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getParameterInfos().getArray()[0].aName == OUString( "aShort" ), "test_RegCoreReflection(): error 18a");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getParameterInfos().getArray()[0].aType == xRefl->forName( OUString( "short" ) ), "test_RegCoreReflection(): error 18b");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getParameterInfos().getArray()[0].aMode == ParamMode_IN, "test_RegCoreReflection(): error 18c");
    OSL_ENSURE(xClass->getMethods().getArray()[4]->getExceptionTypes().getLength() == 0, "test_RegCoreReflection(): error 19");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getName() == OUString("methodC"), "test_RegCoreReflection(): error 20");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getMode() == MethodMode_TWOWAY, "test_RegCoreReflection(): error 20a");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getReturnType()->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 21");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getReturnType()->getComponentType()->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 22");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getReturnType()->getComponentType()->getName() == OUString("ModuleA.StructB"), "test_RegCoreReflection(): error 23");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getParameterTypes().getLength() == 2, "test_RegCoreReflection(): error 24");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[0]->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 25");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[0]->getName() == OUString("ModuleA.StructC"), "test_RegCoreReflection(): error 26");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[1]->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 27");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[1]->getName() == OUString("ModuleA.StructA"), "test_RegCoreReflection(): error 28");
    OSL_ENSURE(xClass->getMethods().getArray()[5]->getExceptionTypes().getLength() == 0, "test_RegCoreReflection(): error 29");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getName() == OUString("methodD"), "test_RegCoreReflection(): error 30");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getReturnType()->getTypeClass() == TypeClass_INTERFACE, "test_RegCoreReflection(): error 31");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getReturnType()->getName() == OUString("ModuleC.XInterfaceA"), "test_RegCoreReflection(): error 32");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getParameterTypes().getLength() == 1, "test_RegCoreReflection(): error 33");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getParameterTypes().getArray()[0]->getTypeClass() == TypeClass_ENUM, "test_RegCoreReflection(): error 34");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getParameterTypes().getArray()[0]->getName() == OUString("ModuleA.ModuleB.EnumA"), "test_RegCoreReflection(): error 35");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getLength() == 3, "test_RegCoreReflection(): error 36");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[0]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 37");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[0]->getName() == OUString("ModuleA.ExceptionA"), "test_RegCoreReflection(): error 38");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[1]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 38");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[1]->getName() == OUString("ModuleA.ExceptionB"), "test_RegCoreReflection(): error 39");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 40");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getName() == OUString("ModuleA.ExceptionC"), "test_RegCoreReflection(): error 41");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getLength() == 3, "test_RegCoreReflection(): error 42");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[0]->getType()->getTypeClass() == TypeClass_BOOLEAN, "test_RegCoreReflection(): error 43");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[0]->getType()->getName() == OUString("boolean"), "test_RegCoreReflection(): error 43a");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[1]->getType()->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 44");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[1]->getType()->getName() == OUString("ModuleA.StructC"), "test_RegCoreReflection(): error 45");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[2]->getType()->getTypeClass() == TypeClass_INTERFACE, "test_RegCoreReflection(): error 46");
    OSL_ENSURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[2]->getType()->getName() == OUString("ModuleA.XInterface1"), "test_RegCoreReflection(): error 47");

    // SequenceReflections

    OSL_ENSURE(xRefl->forName( OUString("[]ModuleA.StructA") )->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 48");
    OSL_ENSURE(xRefl->forName( OUString("[]ModuleA.StructA") )->getComponentType().is(), "test_RegCoreReflection(): error 49");
    OSL_ENSURE(xRefl->forName( OUString("[][]ModuleA.StructA") )->getComponentType()->getComponentType()->getName() == OUString("ModuleA.StructA"), "test_RegCoreReflection(): error 50");
    OSL_ENSURE(xRefl->forName( OUString("[]com.sun.star.uno.XInterface") ) == xRefl->forName(OUString( "ModuleA.StructC" ))->getField(OUString("aInterfaceSeq"))->getType(), "test_RegCoreReflection(): error 51");

    StructC aStructC;
    aStructC.aLong = aConstLong;
    aStructC.aShort = aConstShort;
    aStructC.aFloat = aConstFloat;
    aStructC.aDouble = aConstDouble;
    aStructC.aInterfaceSeq = Sequence<Reference<XInterface > >();

    Any aAny;

    xRefl->forName(OUString("ModuleA.StructC"))->getField(OUString("aInterfaceSeq"))->getType()->createObject(aAny);

    OSL_ENSURE(aAny.getValueType() == ::getCppuType( (const Sequence<Reference< XInterface > > *)0 ), "test_RegCoreReflection(): error 51a");

    Any aStructAny(&aStructC, ::getCppuType( (const StructC *) 0 ));

    sal_Int32 nLong = aConstLong * 2;
    aAny.setValue( &nLong, ::getCppuType( (const sal_Int32 *)0 ) );

    OSL_ENSURE(*(sal_Int32*)xRefl->forName(OUString( "ModuleA.StructA" ))->getField(OUString( "aLong" ))->get(
        Any(&aStructC, ::getCppuType( (const StructC *)0 ))).getValue() == aConstLong, "test_RegCoreReflection(): error 52");
    OSL_ENSURE(xRefl->forName(OUString("ModuleA.StructA"))->getField(OUString( "aLong" ))->getAccessMode() == FieldAccessMode_READWRITE, "test_RegCoreReflection(): error 52a");
    Reference< XIdlField2 > rField ( xRefl->forName(OUString( "ModuleA.StructC" ))->getField(OUString( "aLong" )) , UNO_QUERY );
    rField->set(aStructAny, aAny);
    OSL_ENSURE(*(sal_Int32*)xRefl->forName(OUString("ModuleA.StructB"))->getField(OUString( "aLong" ))->get(aStructAny).getValue() == *(sal_Int32*)aAny.getValue(), "test_RegCoreReflection(): error 53");

    xRefl->forName( OUString("[]ModuleA.StructA") )->createObject(aAny);

    OSL_ENSURE( aAny.getValueTypeName() == "[]ModuleA.StructA", "test_RegCoreReflection(): error 54" );
    xRefl->forName(OUString("[][]ModuleA.StructA"))->createObject(aAny);

    OSL_ENSURE( aAny.getValueTypeName() == "[][]ModuleA.StructA", "test_RegCoreReflection(): error 56" );

//      xClass = xRefl->forName(OUString( "com.sun.star.beans.XIntroTest" ));

//      OSL_ENSURE(xClass.is(), "test_RegCoreReflection(): error 58");
//      OSL_ENSURE(xClass->getMethod(OUString("getStrings"))->getReturnType()->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 59");
//      OSL_ENSURE(xClass->getMethod(OUString("getStrings"))->getReturnType()->getComponentType()->getName() == OUString("string"), "test_RegCoreReflection(): error 60");

//      xClass->getMethod(OUString("getStrings"))->getReturnType()->createObject(aAny);
//      OSL_ENSURE( aAny.getValueTypeName() == "[]string",  "test_RegCoreReflection(): error 61" );

    OSL_ENSURE(xRefl->forName(OUString("[][][]unsigned long"))->getComponentType()->getComponentType()->getComponentType()->getTypeClass() == TypeClass_UNSIGNED_LONG, "test_RegCoreReflection(): error 62");

    try
    {
        fprintf( stderr, "%1\n" );
        Any bla = xRefl->forName(OUString("ModuleA.StructC"))->getField(OUString("aString"))->get(Any());
        OSL_ENSURE(sal_False, "test_RegCoreReflection(): error 63");
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
        rField = Reference< XIdlField2 > ( xRefl->forName(OUString("ModuleA.StructC"))->getField(OUString("aString")) , UNO_QUERY);
        rField->set( blup, gulp);
//          xRefl->forName(OUString("ModuleA.StructC"))->getField(OUString("aString"))->set(blup, gulp);
        OSL_ENSURE(sal_False, "test_RegCoreReflection(): error 64");
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
            xRefl->forName(OUString("ModuleA.StructC"))->getField(OUString("aString")) , UNO_QUERY);
        xRefl->forName(OUString("ModuleA.StructC"))->getField(OUString("aString"))->set(blup, gulp);
        OSL_ENSURE(sal_False, "test_RegCoreReflection(): error 65");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    Any gulp;
    gulp <<= OUString(OUString("Test"));
    Any blup;
    blup <<= aStructC;
    xRefl->forName(OUString("ModuleA.StructC"))->getField(OUString("aString"))->set(blup, gulp);

    Reference< XInterfaceA > xAI = new OInterfaceA();

    try
    {
        Sequence< Any > params;
        fprintf( stderr, "%4\n" );

        Any a;
        a <<= xAI;
        Any bla = xRefl->forName(OUString("ModuleC.XInterfaceA"))->getMethod(OUString("methodC"))->invoke(a, params);
        OSL_ENSURE(sal_False, "test_RegCoreReflection(): error 66");
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
        Any bla = xRefl->forName(OUString("ModuleC.XInterfaceA"))->getMethod(OUString("methodC"))->invoke(a, params);
    }
    try
    {
        Sequence< Any > params(2);

        params.getArray()[0].setValue(&aStructA, ::getCppuType( (const StructA *)0 ));
        params.getArray()[1].setValue(&aStructA, ::getCppuType( (const StructA *)0 ));

        Any a;
        a <<= xAI;
        Any bla = xRefl->forName(OUString("ModuleC.XInterfaceA"))->getMethod(OUString("methodC"))->invoke(a, params);
        OSL_ENSURE(sal_False, "test_RegCoreReflection(): error 67");
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
    bool result = (xRefl->forName(OUString("ModuleC.XInterfaceA"))->getMethod(OUString("methodC"))->invoke(a, params).getValueType()
                    == ::getCppuType( (const Sequence<StructB> *)0 )); (void)result;
    OSL_ENSURE(result, "test_RegCoreReflection(): error 68");

    return sal_True;
}

SAL_IMPLEMENT_MAIN()
{
    sal_Bool bSucc = sal_False;
    try
    {
        OUString aLibName( "reflection.uno" SAL_DLLEXTENSION );

        Reference< XMultiServiceFactory > xMgr(
            createRegistryServiceFactory(
                OUString( "stoctest.rdb" ) ) );
        Reference< XComponentContext > xContext;
        Reference< beans::XPropertySet > xProps( xMgr, UNO_QUERY );
        OSL_ASSERT( xProps.is() );
        xProps->getPropertyValue(
            OUString( "DefaultContext" ) ) >>=
            xContext;
        OSL_ASSERT( xContext.is() );

        Reference< XIdlReflection > xRefl;
        xContext->getValueByName(
            OUString( "/singletons/com.sun.star.reflection.theCoreReflection") )
                        >>= xRefl;
        OSL_ENSURE(
            xRefl.is(), "### CoreReflection singleton not accessable!?" );

        bSucc = test_corefl( xRefl );

        Reference< XComponent >( xContext, UNO_QUERY )->dispose();
    }
    catch (const Exception & rExc)
    {
        OSL_FAIL( "### exception occurred!" );
        OString aMsg(
            OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### exception occurred: " );
        OSL_TRACE( "%s", aMsg.getStr() );
        OSL_TRACE( "\n" );
    }

    printf( "testcorefl %s !\n", (bSucc ? "succeeded" : "failed") );
    return (bSucc ? 0 : -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
