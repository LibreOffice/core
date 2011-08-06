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
#include "precompiled_extensions.hxx"
#include <stdio.h>
#include <stardiv/uno/repos/implementationregistration.hxx>
#include <stardiv/uno/script/script.hxx>
#include <stardiv/uno/beans/exactname.hxx>

#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <usr/services.hxx>
#include <vcl/svapp.hxx>
#include <usr/ustring.hxx>
#include <usr/weak.hxx>
#include <tools/string.hxx>
#include <osl/conditn.hxx>

using namespace usr;

using ::rtl::StringToOUString;
using ::rtl::OUStringToString;

#define PCHAR_TO_USTRING(x) StringToOUString(String(x),CHARSET_SYSTEM)
#define USTRING_TO_PCHAR(x) OUStringToString(x , CHARSET_DONTKNOW ).GetCharStr()





/*****
*
* A Test root object !
*
*****/
class MyPythonRoot :
        public XInvokation,
        public OWeakObject
{
public:
    MyPythonRoot() { m_iTestValue = 15; }
    BOOL                queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                acquire()                        { OWeakObject::acquire(); }
    void                release()                        { OWeakObject::release(); }
    void*               getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

public:
    XIntrospectionAccessRef getIntrospection(void) const        THROWS( (UsrSystemException) )
                { return XIntrospectionAccessRef();  }

    UsrAny                  invoke( const UString& FunctionName,
                                    const Sequence< UsrAny >& Params,
                                    Sequence< INT16 >& OutParamIndex,
                                    Sequence< UsrAny >& OutParam)
                                                                THROWS( (   IllegalArgumentException,
                                                                            CannotConvertException,
                                                                            InvocationTargetException,
                                                                            UsrSystemException) );
    void                    setValue(const UString& PropertyName, const UsrAny& Value)
                                                                THROWS( (   UnknownPropertyException,
                                                                            CannotConvertException,
                                                                            InvocationTargetException,
                                                                            UsrSystemException) );

    UsrAny                  getValue(const UString& PropertyName)
                                                                THROWS( (   UnknownPropertyException,
                                                                            UsrSystemException) );
    BOOL                    hasMethod(const UString& Name) const THROWS( (UsrSystemException) );
    BOOL                    hasProperty(const UString& Name) const THROWS( (UsrSystemException) );


    void getTestValueViaInout( int &inout )
        { inout = m_iTestValue; }

    INT32 getTestValue() const
        { return m_iTestValue; }

    void setTestValue( INT32 i )
        { m_iTestValue = i; }

    void printTestValue()
        { fprintf( stderr, "TestValue : %d\n" , getTestValue() ); }

    void addTestValue( INT32 i )
        { m_iTestValue += i; }

private:

    INT32 m_iTestValue;
};

BOOL MyPythonRoot::queryInterface( Uik aUik, XInterfaceRef &rOut )
{
    if( aUik == XInvokation::getSmartUik() ) {
        rOut = ( XInvokation * ) this;
    }
    else {
        return OWeakObject::queryInterface( aUik , rOut );
    }
    return TRUE;

}

UsrAny  MyPythonRoot::invoke(   const UString& FunctionName,
                                const Sequence< UsrAny >& Params,
                                Sequence< INT16 >& OutParamIndex,
                                Sequence< UsrAny >& OutParam)
                                                                THROWS( (   IllegalArgumentException,
                                                                            CannotConvertException,
                                                                            InvocationTargetException,
                                                                            UsrSystemException) )
{
    if( L"printTestValue" == FunctionName ) {
        printTestValue();
    }
    else if( L"addTestValue" == FunctionName ) {
        addTestValue( Params.getConstArray()[0].getINT32() );
    }
    else if( L"getTestValueViaInout" == FunctionName ) {
        int i = Params.getConstArray()[0].getINT32();
        getTestValueViaInout( i );
        OutParam.getArray()[0].setINT32( i );
    }
    else {
        THROW( InvocationTargetException() );
    }

    return UsrAny();
}

void    MyPythonRoot::setValue(const UString& PropertyName, const UsrAny& Value)
                                                                THROWS( (   UnknownPropertyException,
                                                                            CannotConvertException,
                                                                            InvocationTargetException,
                                                                            UsrSystemException) )
{
    if( L"TestValue" == PropertyName ) {
        setTestValue( Value.getINT32() );
    }
    else {
        THROW( UnknownPropertyException() );
    }
}

UsrAny  MyPythonRoot::getValue(const UString& PropertyName)
                                                                THROWS( (   UnknownPropertyException,
                                                                            UsrSystemException) )
{
    UsrAny aRet;

    if( L"TestValue" == PropertyName ) {
        aRet.setINT32( getTestValue() );
    }
    else {
        THROW( UnknownPropertyException() );
    }

    return aRet;
}


BOOL    MyPythonRoot::hasMethod(const UString& Name) const      THROWS( (UsrSystemException) )
{
    if( L"printTestValue" == Name ) {
        return TRUE;
    }
    else if( L"addTestValue" == Name ) {
        return TRUE;
    }
    else if( L"getTestValueViaInout" == Name ) {
        return TRUE;
    }
    return FALSE;
}


BOOL    MyPythonRoot::hasProperty(const UString& Name) const THROWS( (UsrSystemException) )
{
    if( L"TestValue" == Name ) {
        return TRUE;
    }

    return FALSE;
}


/*****
*
* A test engine listener to check the debug interface
*
*****/
class TestListener :
    public XEngineListener,
    public OWeakObject
{
public:

    TestListener()
    {
        m_pDebuggingRef = 0;
    }


    TestListener( XDebuggingRef *p )
    {
        attach( p  );
    }

    ~TestListener()
    {
        if( m_pDebuggingRef ) {
            detach();
        }
    }

    BOOL                queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                acquire()                        { OWeakObject::acquire(); }
    void                release()                        { OWeakObject::release(); }
    void*               getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }


    void attach( XDebuggingRef *p )
    {
        m_pDebuggingRef = p;
    }

    void detach( );


    virtual void disposing( const EventObject &o )
    {
        if( m_pDebuggingRef ) {
            detach();
        }
    }
    virtual void interrupt(const InterruptEngineEvent& Evt) THROWS( (UsrSystemException) )
    {
    }

    virtual void running(const EventObject& Evt) THROWS( (UsrSystemException) )
       {
        (*m_pDebuggingRef)->stop();

           m_aDebugCondition.set();
    }

    virtual void finished(const FinishEngineEvent& Evt) THROWS( (UsrSystemException) )
    {
           m_aDebugCondition.set();
    }


    void cmdLine();
protected:

    osl::Condition m_aDebugCondition;
    XDebuggingRef *m_pDebuggingRef;
};



void TestListener::cmdLine()
{
    // Condition is set by running listener
    m_aDebugCondition.wait();
    m_aDebugCondition.reset();
    (*m_pDebuggingRef)->doContinue();
    m_aDebugCondition.wait();
}

void TestListener::detach()
{
    OSL_ASSERT( m_pDebuggingRef );

       m_pDebuggingRef = 0;
}

BOOL TestListener::queryInterface( Uik aUik, XInterfaceRef & rOut )
{
    if( aUik == XEngineListener::getSmartUik() )
        rOut = (XEngineListener*)this;
    else
        return OWeakObject::queryInterface( aUik, rOut );
    return TRUE;
}


void checkInvokation( const XInvokationRef &xInvoke )
{
    UsrAny anyList;

    // check exporting an object as an invokation
    OSL_ASSERT( xInvoke->hasProperty( L"list" ) );
    anyList = xInvoke->getValue( L"list" );

    OSL_ASSERT( anyList.getReflection() == XInvokation_getReflection() );
    XInvokationRef *pRef = ( XInvokationRef * ) anyList.get();
    OSL_ASSERT( (*pRef).is() );

    OSL_ASSERT( (*pRef)->hasMethod( L"append"  ) );
    OSL_ASSERT( (*pRef)->hasMethod( L"count" ) );

    Sequence<UsrAny> seq(1);
    UsrAny any( (INT32) 1);
    (seq.getArray())[0] = any;

    any = (*pRef)->invoke( L"count" , seq , Sequence<INT16>(), Sequence<UsrAny>() );
    int nOldSize = any.getINT32();

    any = (*pRef)->invoke( L"append" , seq  , Sequence<INT16>(), Sequence<UsrAny>() );
    any = (*pRef)->invoke( L"count" , seq , Sequence<INT16>(), Sequence<UsrAny>() );

    OSL_ASSERT( nOldSize + 1 == any.getINT32() );
}

// just for testing !
class PythonCodeLibrary :
        public XLibraryAccess,
        public OWeakObject
{

    BOOL                queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                acquire()                        { OWeakObject::acquire(); }
    void                release()                        { OWeakObject::release(); }
    void*               getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }


    virtual BOOL isFunction(const UString& FunctionName)            THROWS( (UsrSystemException) )
      {
          return FALSE;
      }

    virtual BOOL isValidPath(const UString& PathName)               THROWS( (UsrSystemException) )
    {
        return FALSE;
    }

    virtual Sequence< UString > getModuleNames(void)                THROWS( (UsrSystemException) )
    {
        return Sequence<UString> ();
    }

    virtual UString getModuleSource(const UString& ModulName)       THROWS( (UsrSystemException) )
    {
        if( ModulName == L"testmodul" ) {
            return UString( L"def testmethod():\n"
                            L"  return 42\n");
        }
        return UString();
    }

    virtual Sequence< BYTE > getModuleCode(const UString& ModuleName) THROWS( (UsrSystemException) )
    {
        return Sequence< BYTE > ();
    }

    virtual UString getFunctionSource(const UString& FunctionName) THROWS( (UsrSystemException) )
    {
        return UString();
    }
    virtual Sequence< BYTE > getFunctionCode(const UString& FunctionName) THROWS( (UsrSystemException) )
    {
        return Sequence< BYTE > ();
    }
};

BOOL PythonCodeLibrary::queryInterface( Uik aUik, XInterfaceRef & rOut )
{
    if( XLibraryAccess::getSmartUik() == aUik ) {
        rOut = (XLibraryAccess* ) this;
    }
    else {
        return OWeakObject::queryInterface( aUik , rOut );
    }

    return TRUE;
}



/*
 * main.
 */
int __LOADONCALLAPI main (int argc, char **argv)
{
    // necessary startup code
    XMultiServiceFactoryRef xSMgr = createRegistryServiceManager();
    registerUsrServices( xSMgr );
    setProcessServiceManager( xSMgr );

    XInterfaceRef x = xSMgr->createInstance( L"stardiv.uno.repos.ImplementationRegistration" );
    XImplementationRegistrationRef xReg( x, USR_QUERY );
    sal_Char szBuf[1024];

    ORealDynamicLoader::computeModuleName( "pythonengine", szBuf, 1024 );
    UString aDllName( StringToOUString( szBuf, CHARSET_SYSTEM ) );
    xReg->registerImplementation( L"stardiv.loader.SharedLibrary", aDllName, XSimpleRegistryRef() );

    x = xSMgr->createInstance( L"stardiv.script.Python" );
    XEngineRef xEngine( x, USR_QUERY );
    XInvokationRef xInvoke(  x, USR_QUERY );
    XDebuggingRef xDebug( x , USR_QUERY );

    XInterfaceRef rRoot( (XInvokation * )new MyPythonRoot , USR_QUERY );
    xEngine->setRoot( rRoot );


    // execute a simple script
    xEngine->run(   L"nIntTest = 5\n"
                    L"list = [2,3,4]\n" , XInterfaceRef(), Sequence<UsrAny> () );

    /****
    *
    * Xinvokation - Test
    *
    *****/
    // get/set an int !
    {
        OSL_ASSERT( xInvoke->hasProperty( L"nIntTest" ) );
        UsrAny any = xInvoke->getValue( L"nIntTest" );

        OSL_ASSERT( any.getReflection()->getTypeClass() == TypeClass_LONG );
        OSL_ASSERT( any.getINT32() == 5 );

        // simple test: set an int !
        xInvoke->setValue( L"nIntTest" , UsrAny( (INT32) 10 ) );
        any = xInvoke->getValue( L"nIntTest" );

        OSL_ASSERT( any.getReflection()->getTypeClass() == TypeClass_LONG );
        OSL_ASSERT( any.getINT32() == 10 );
    }

    //  call a python method !
    {
        xEngine->run( L"def foo():\n"
                      L"    return 'this is foo'\n" , XInterfaceRef() , Sequence<UsrAny> () );
        OSL_ASSERT( xInvoke->hasMethod( L"foo" ) );
        UsrAny any = xInvoke->invoke(   L"foo" ,
                                        Sequence<UsrAny>(),
                                        Sequence<INT16>() ,
                                        Sequence<UsrAny> () );
        OSL_ASSERT( any.getString() == L"this is foo" );
    }


    // check exception handling !
    {
        try {
            xInvoke->invoke( L"foo" , Sequence<UsrAny>(1) , Sequence<INT16>(), Sequence<UsrAny> () );
            // wrong number of arguments
            OSL_ASSERT( 0 );
        }
        catch ( IllegalArgumentException& e ) {
        }
        catch ( InvocationTargetException& e ) {
        }
        catch ( CannotConvertException& e ) {
            // empty any cannot be converted
        }
    }

    // check InOut-Parameter
    checkInvokation( xInvoke );

    /*******
    *
    * Check Introspection Access
    *
    *******/
    {
        XIntrospectionAccessRef xIntrospection = xInvoke->getIntrospection();
        OSL_ASSERT( xIntrospection.is() );

        // no further test, simply call them
        xIntrospection->getMethods(0);
        xIntrospection->getProperties(0);

        OSL_ASSERT( xIntrospection->getSuppliedMethodConcepts() == 0 );
        OSL_ASSERT( xIntrospection->getSuppliedPropertyConcepts() == 0 );

        Property prop = xIntrospection->getProperty( L"nIntTest" ,0 );
        OSL_ASSERT( prop.Name == L"nIntTest" );
        OSL_ASSERT( prop.Type->getTypeClass() == TypeClass_LONG );

        XIdlMethodRef method = xIntrospection->getMethod( L"foo" , 0 );
        OSL_ASSERT( method->getName() == L"foo" );
    }


    /******
    *
    * Multithreading test
    *
    *******/

    /******
    *
    * XDebuggingTest
    *
    ******/
    // stop/doContinue + runAsync listener
    {
        // test hangs, if something is wrong

        TestListener *pListener = new TestListener( &xDebug );
        XEngineListenerRef ref( (XEngineListener * ) pListener , USR_QUERY );

        // single listener check !
        xEngine->runAsync( L"pass\n"
                            , XInterfaceRef() , Sequence<UsrAny> () , ref );
        pListener->cmdLine();
    }

    // ListenerAdministration check !
    {
        // test hangs, if something is wrong

        TestListener *pListener = new TestListener( &xDebug );
        XEngineListenerRef ref( (XEngineListener * ) pListener , USR_QUERY );

        // engine listener check !
        xEngine->addEngineListener( ref );
        xEngine->runAsync( L"pass\n"
                            , XInterfaceRef() , Sequence<UsrAny> () , XEngineListenerRef() );
        pListener->cmdLine();
        xEngine->removeEngineListener( ref);

    }

    // check the import mechanism
    {
        XLibraryAccessRef xLibrary( ( XLibraryAccess * ) new PythonCodeLibrary , USR_QUERY );
        xEngine->setLibraryAccess( xLibrary );

        xEngine->run(   L"import testmodul\n"
                        L"x = testmodul.testmethod()\n" , XInterfaceRef() , Sequence<UsrAny>() );
        UsrAny any = xInvoke->getValue( L"x" );
        OSL_ASSERT( any.getReflection()->getTypeClass() == TypeClass_LONG );
        OSL_ASSERT( any.getINT32() == 42 );
    }

    // check other imports
    {
        // Check, if the libraries are available at run time
        xEngine->run(   L"import math\n"
                        L"dMathTest = math.exp(0)\n"  , XInterfaceRef() , Sequence<UsrAny> () );

        OSL_ASSERT( xInvoke->hasProperty( L"dMathTest" ) );
        UsrAny any = xInvoke->getValue( L"dMathTest" );

        OSL_ASSERT( any.getReflection()->getTypeClass() == TypeClass_DOUBLE );
        OSL_ASSERT( any.getDouble() == 1. );
    }

    // Test connection to root object !
    {
        xEngine->run(   L"x = stardiv.root.TestValue\n"
                        L"y = stardiv.inout(5)\n"
                        L"stardiv.root.getTestValueViaInout(y)\n"
                        L"z = y.value\n" , XInterfaceRef() , Sequence<UsrAny> () );

        UsrAny any = xInvoke->getValue( L"x" );
        OSL_ASSERT( any.getReflection()->getTypeClass() == TypeClass_LONG );
        OSL_ASSERT( any.getINT32() == 15 );

        any = xInvoke->getValue( L"z" );
        OSL_ASSERT( any.getReflection()->getTypeClass() == TypeClass_LONG );
        OSL_ASSERT( any.getINT32() == 15 );
    }

    // Test exactName interface
    {
        UsrAny any = xInvoke->getValue( L"__builtins__" );
        OSL_ASSERT( any.getReflection()->getTypeClass() == TypeClass_INTERFACE );

        XInvokationRef rInv( *((XInterfaceRef *) any.get() ), USR_QUERY );
        OSL_ASSERT( rInv.is() );

        XExactNameRef rName( *((XInterfaceRef*) any.get() ), USR_QUERY );
        OSL_ASSERT( rName.is() );

        UString str = rName->getExactName( L"SYNTAXERROR" );
        OSL_ASSERT( str.len() );
    }


    // Test exactName interface of the engine itself
    {
        XExactNameRef rName( xInvoke , USR_QUERY );
        OSL_ASSERT( rName.is() );
        UString str = rName->getExactName( L"STARDIV" );
        OSL_ASSERT( str.len() );
    }


    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
