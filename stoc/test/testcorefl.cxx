/*************************************************************************
 *
 *  $RCSfile: testcorefl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <osl/diagnose.h>

#ifdef _DEBUG
#define TEST_ENSHURE(c, m)   OSL_ENSHURE(c, m)
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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>

#include <rtl/ustrbuf.hxx>

#include <stdio.h>


using namespace rtl;
using namespace cppu;
using namespace osl;
using namespace ModuleA;
using namespace ModuleB;
using namespace ModuleC;
using namespace ModuleA::ModuleB;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::reflection;
using namespace com::sun::star::container;


//==================================================================================================
class OInterfaceA : public WeakImplHelper1< XInterfaceA >
{
public:

    virtual void SAL_CALL methodA(void)
        {}

    virtual void SAL_CALL methodB(sal_Int16 aShort)
        {}
    virtual Sequence< StructB > SAL_CALL methodC(const StructC& aStructC, StructA& aStructA)
        { return Sequence< StructB >(); }
};

//==================================================================================================
static inline uik_equals( const Uik & rUik1, const Uik & rUik2 )
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

    TEST_ENSHURE(xRefl->forName(OUString::createFromAscii("ModuleA.StructA"))->getName() == OUString::createFromAscii("ModuleA.StructA"), "test_RegCoreReflection(): error 2b");
    TEST_ENSHURE(xRefl->forName(OUString::createFromAscii("ModuleA.ExceptionB"))->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 2c");
    TEST_ENSHURE(xRefl->forName(OUString::createFromAscii("ModuleA.ModuleB.EnumA")).is(), "test_RegCoreReflection(): error 2e");
    // const

    TEST_ENSHURE(*(const sal_Bool *)xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstBoolean")).getValue() == aConstBoolean, "test_RegCoreReflection(): error 4c");
    TEST_ENSHURE(*(const sal_Int8 *)xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstByte")).getValue() == aConstByte, "test_RegCoreReflection(): error 4e");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstShort")) == aConstShort, "test_RegCoreReflection(): error 4g");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstUShort")) == aConstUShort, "test_RegCoreReflection(): error 4i");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstLong")) == aConstLong, "test_RegCoreReflection(): error 4k");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstULong")) == aConstULong, "test_RegCoreReflection(): error 4m");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstFloat")) == aConstFloat, "test_RegCoreReflection(): error 4o");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstDouble")) == aConstDouble, "test_RegCoreReflection(): error 4q");
    TEST_ENSHURE(xHNameAccess->getByHierarchicalName(OUString::createFromAscii("ModuleC.aConstString")) == aConstString, "test_RegCoreReflection(): error 4s");

    // Enums

    xClass = xRefl->forName(OUString::createFromAscii("ModuleA.ModuleB.EnumA"));

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

    Reference< XIdlClass > xA = xRefl->forName( OUString::createFromAscii("ModuleC.XInterfaceB") );

    xClass = xRefl->forName(OUString::createFromAscii("ModuleC.XInterfaceB"));

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
    TEST_ENSHURE(xClass->getSuperclasses().getArray()[0]->getName() == OUString::createFromAscii("ModuleC.XInterfaceA"), "test_RegCoreReflection(): error 10");
    TEST_ENSHURE(xClass->getMethods().getLength() == 7, "test_RegCoreReflection(): error 11");
    TEST_ENSHURE(xA->getMethods().getLength() == 7, "test_RegCoreReflection(): error 11a");
    TEST_ENSHURE(xClass->getMethods().getArray()[3]->getName() == OUString::createFromAscii("methodA"), "test_RegCoreReflection(): 12");
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
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getName() == OUString::createFromAscii("methodC"), "test_RegCoreReflection(): error 20");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getMode() == MethodMode_TWOWAY, "test_RegCoreReflection(): error 20a");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getReturnType()->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 21");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getReturnType()->getComponentType()->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 22");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getReturnType()->getComponentType()->getName() == OUString::createFromAscii("ModuleA.StructB"), "test_RegCoreReflection(): error 23");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getLength() == 2, "test_RegCoreReflection(): error 24");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[0]->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 25");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[0]->getName() == OUString::createFromAscii("ModuleA.StructC"), "test_RegCoreReflection(): error 26");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[1]->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 27");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getParameterTypes().getArray()[1]->getName() == OUString::createFromAscii("ModuleA.StructA"), "test_RegCoreReflection(): error 28");
    TEST_ENSHURE(xClass->getMethods().getArray()[5]->getExceptionTypes().getLength() == 0, "test_RegCoreReflection(): error 29");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getName() == OUString::createFromAscii("methodD"), "test_RegCoreReflection(): error 30");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getReturnType()->getTypeClass() == TypeClass_INTERFACE, "test_RegCoreReflection(): error 31");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getReturnType()->getName() == OUString::createFromAscii("ModuleC.XInterfaceA"), "test_RegCoreReflection(): error 32");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getParameterTypes().getLength() == 1, "test_RegCoreReflection(): error 33");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getParameterTypes().getArray()[0]->getTypeClass() == TypeClass_ENUM, "test_RegCoreReflection(): error 34");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getParameterTypes().getArray()[0]->getName() == OUString::createFromAscii("ModuleA.ModuleB.EnumA"), "test_RegCoreReflection(): error 35");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getLength() == 3, "test_RegCoreReflection(): error 36");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[0]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 37");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[0]->getName() == OUString::createFromAscii("ModuleA.ExceptionA"), "test_RegCoreReflection(): error 38");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[1]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 38");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[1]->getName() == OUString::createFromAscii("ModuleA.ExceptionB"), "test_RegCoreReflection(): error 39");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getTypeClass() == TypeClass_EXCEPTION, "test_RegCoreReflection(): error 40");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getName() == OUString::createFromAscii("ModuleA.ExceptionC"), "test_RegCoreReflection(): error 41");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getLength() == 3, "test_RegCoreReflection(): error 42");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[0]->getType()->getTypeClass() == TypeClass_BOOLEAN, "test_RegCoreReflection(): error 43");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[0]->getType()->getName() == OUString::createFromAscii("boolean"), "test_RegCoreReflection(): error 43a");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[1]->getType()->getTypeClass() == TypeClass_STRUCT, "test_RegCoreReflection(): error 44");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[1]->getType()->getName() == OUString::createFromAscii("ModuleA.StructC"), "test_RegCoreReflection(): error 45");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[2]->getType()->getTypeClass() == TypeClass_INTERFACE, "test_RegCoreReflection(): error 46");
    TEST_ENSHURE(xClass->getMethods().getArray()[6]->getExceptionTypes().getArray()[2]->getFields().getArray()[2]->getType()->getName() == OUString::createFromAscii("ModuleA.XInterface1"), "test_RegCoreReflection(): error 47");

    // SequenceReflections

    TEST_ENSHURE(xRefl->forName( OUString::createFromAscii("[]ModuleA.StructA") )->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 48");
    TEST_ENSHURE(xRefl->forName( OUString::createFromAscii("[]ModuleA.StructA") )->getComponentType().is(), "test_RegCoreReflection(): error 49");
    TEST_ENSHURE(xRefl->forName( OUString::createFromAscii("[][]ModuleA.StructA") )->getComponentType()->getComponentType()->getName() == OUString::createFromAscii("ModuleA.StructA"), "test_RegCoreReflection(): error 50");
    TEST_ENSHURE(xRefl->forName( OUString::createFromAscii("[]com.sun.star.uno.XInterface") ) == xRefl->forName(OUString( RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC") ))->getField(OUString::createFromAscii("aInterfaceSeq"))->getType(), "test_RegCoreReflection(): error 51");

    StructC aStructC;
    aStructC.aLong = aConstLong;
    aStructC.aShort = aConstShort;
    aStructC.aFloat = aConstFloat;
    aStructC.aDouble = aConstDouble;
    aStructC.aString = aConstString;
    aStructC.aInterfaceSeq = Sequence<Reference<XInterface > >();

    Any aAny;

    xRefl->forName(OUString::createFromAscii("ModuleA.StructC"))->getField(OUString::createFromAscii("aInterfaceSeq"))->getType()->createObject(aAny);

    TEST_ENSHURE(aAny.getValueType() == ::getCppuType( (const Sequence<Reference< XInterface > > *)0 ), "test_RegCoreReflection(): error 51a");

    Any aStructAny(&aStructC, ::getCppuType( (const StructC *) 0 ));

    sal_Int32 nLong = aConstLong * 2;
    aAny.setValue( &nLong, ::getCppuType( (const sal_Int32 *)0 ) );

    TEST_ENSHURE(*(sal_Int32*)xRefl->forName(OUString( RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructA") ))->getField(OUString( RTL_CONSTASCII_USTRINGPARAM("aLong") ))->get(
        Any(&aStructC, ::getCppuType( (const StructC *)0 ))).getValue() == aConstLong, "test_RegCoreReflection(): error 52");
    TEST_ENSHURE(xRefl->forName(OUString::createFromAscii("ModuleA.StructA"))->getField(OUString( RTL_CONSTASCII_USTRINGPARAM("aLong") ))->getAccessMode() == FieldAccessMode_READWRITE, "test_RegCoreReflection(): error 52a");
    xRefl->forName(OUString( RTL_CONSTASCII_USTRINGPARAM("ModuleA.StructC") ))->getField(OUString( RTL_CONSTASCII_USTRINGPARAM("aLong") ))->set(aStructAny, aAny);
    TEST_ENSHURE(*(sal_Int32*)xRefl->forName(OUString::createFromAscii("ModuleA.StructB"))->getField(OUString( RTL_CONSTASCII_USTRINGPARAM("aLong") ))->get(aStructAny).getValue() == *(sal_Int32*)aAny.getValue(), "test_RegCoreReflection(): error 53");

    xRefl->forName( OUString::createFromAscii("[]ModuleA.StructA") )->createObject(aAny);

    TEST_ENSHURE(aAny.getValueTypeName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]ModuleA.StructA")), "test_RegCoreReflection(): error 54");
    xRefl->forName(OUString::createFromAscii("[][]ModuleA.StructA"))->createObject(aAny);

    TEST_ENSHURE(aAny.getValueTypeName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("[][]ModuleA.StructA") ), "test_RegCoreReflection(): error 56");

//      xClass = xRefl->forName(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.XIntroTest") ));

//      TEST_ENSHURE(xClass.is(), "test_RegCoreReflection(): error 58");
//      TEST_ENSHURE(xClass->getMethod(OUString::createFromAscii("getStrings"))->getReturnType()->getTypeClass() == TypeClass_SEQUENCE, "test_RegCoreReflection(): error 59");
//      TEST_ENSHURE(xClass->getMethod(OUString::createFromAscii("getStrings"))->getReturnType()->getComponentType()->getName() == OUString::createFromAscii("string"), "test_RegCoreReflection(): error 60");

//      xClass->getMethod(OUString::createFromAscii("getStrings"))->getReturnType()->createObject(aAny);
//      TEST_ENSHURE(aAny.getValueTypeName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("[]string") ),  "test_RegCoreReflection(): error 61");

    TEST_ENSHURE(xRefl->forName(OUString::createFromAscii("[][][]unsigned long"))->getComponentType()->getComponentType()->getComponentType()->getTypeClass() == TypeClass_UNSIGNED_LONG, "test_RegCoreReflection(): error 62");

    try
    {
        Any bla = xRefl->forName(OUString::createFromAscii("ModuleA.StructC"))->getField(OUString::createFromAscii("aString"))->get(Any());
        TEST_ENSHURE(sal_False, "test_RegCoreReflection(): error 63");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    try
    {
        Any blup;
        blup <<= aStructC;
        Any gulp;
        xRefl->forName(OUString::createFromAscii("ModuleA.StructC"))->getField(OUString::createFromAscii("aString"))->set(blup, gulp);
        TEST_ENSHURE(sal_False, "test_RegCoreReflection(): error 64");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    try
    {
        Any gulp;
        gulp <<= 3.14f;
        Any blup;
        blup <<= aStructC;
        xRefl->forName(OUString::createFromAscii("ModuleA.StructC"))->getField(OUString::createFromAscii("aString"))->set(blup, gulp);
        TEST_ENSHURE(sal_False, "test_RegCoreReflection(): error 65");
        return sal_False;
    }
    catch (IllegalArgumentException &)
    {
    }

    Any gulp;
    gulp <<= OUString(OUString::createFromAscii("Test"));
    Any blup;
    blup <<= aStructC;
    xRefl->forName(OUString::createFromAscii("ModuleA.StructC"))->getField(OUString::createFromAscii("aString"))->set(blup, gulp);

    Reference< XInterfaceA > xAI = new OInterfaceA();

    try
    {
        Sequence< Any > params;

        Any a;
        a <<= xAI;
        Any bla = xRefl->forName(OUString::createFromAscii("ModuleC.XInterfaceA"))->getMethod(OUString::createFromAscii("methodC"))->invoke(a, params);
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
        Any bla = xRefl->forName(OUString::createFromAscii("ModuleC.XInterfaceA"))->getMethod(OUString::createFromAscii("methodC"))->invoke(a, params);
    }
    try
    {
        Sequence< Any > params(2);

        params.getArray()[0].setValue(&aStructA, ::getCppuType( (const StructA *)0 ));
        params.getArray()[1].setValue(&aStructA, ::getCppuType( (const StructA *)0 ));

        Any a;
        a <<= xAI;
        Any bla = xRefl->forName(OUString::createFromAscii("ModuleC.XInterfaceA"))->getMethod(OUString::createFromAscii("methodC"))->invoke(a, params);
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
    TEST_ENSHURE(xRefl->forName(OUString::createFromAscii("ModuleC.XInterfaceA"))->getMethod(OUString::createFromAscii("methodC"))->invoke(a, params).getValueType()
        == ::getCppuType( (const Sequence<StructB> *)0 ), "test_RegCoreReflection(): error 68");

    return sal_True;
}

#ifdef UNX
#define REG_PREFIX      "lib"
#define DLL_POSTFIX     ".so"
#else
#define REG_PREFIX      ""
#define DLL_POSTFIX     ".dll"
#endif

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
    Reference< XMultiServiceFactory > xMgr( createRegistryServiceFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("stoctest.rdb") ) ) );

    sal_Bool bSucc = sal_False;
    try
    {
        Reference< XImplementationRegistration > xImplReg(
            xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration") ) ), UNO_QUERY );
        OSL_ENSHURE( xImplReg.is(), "### no impl reg!" );

        OUString aLibName( OUString::createFromAscii(REG_PREFIX) );
        aLibName += OUString::createFromAscii("corefl");
#ifndef OS2
        aLibName += OUString::createFromAscii(DLL_POSTFIX);
#endif
        xImplReg->registerImplementation(
            OUString::createFromAscii("com.sun.star.loader.SharedLibrary"), aLibName, Reference< XSimpleRegistry >() );

        Reference< XIdlReflection > xRefl( xMgr->createInstance( OUString::createFromAscii("com.sun.star.reflection.CoreReflection") ), UNO_QUERY );
        OSL_ENSHURE( xRefl.is(), "### no corereflection!" );

        bSucc = test_corefl( xRefl );
    }
    catch (Exception & rExc)
    {
        OSL_ENSHURE( sal_False, "### exception occured!" );
        OString aMsg( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### exception occured: " );
        OSL_TRACE( aMsg.getStr() );
        OSL_TRACE( "\n" );
    }

    Reference< XComponent >( xMgr, UNO_QUERY )->dispose();

    printf( "testcorefl %s !\n", (bSucc ? "succeeded" : "failed") );
    return (bSucc ? 0 : -1);
}
