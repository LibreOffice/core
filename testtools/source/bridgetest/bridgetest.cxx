/**************************************************************************
#*
#*    last change   $Author: vg $ $Date: 2003-04-15 16:42:21 $
#*    $Revision: 1.7 $
#*
#*    $Logfile: $
#*
#*    Copyright (c) 1989 - 2000, Star Office GmbH
#*
#************************************************************************/

#include <stdio.h>
#include <osl/diagnose.h>
#include <osl/time.h>

#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <uno/data.h>
#include <uno/environment.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>

#include <com/sun/star/test/bridge/XBridgeTest.hpp>

using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::bridge;
using namespace com::sun::star::test::bridge;

#define SERVICENAME     "com.sun.star.test.bridge.BridgeTest"
#define IMPLNAME        "com.sun.star.comp.bridge.BridgeTest"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define STRING_TEST_CONSTANT "\" paco\' chorizo\\\' \"\'"

namespace bridge_test
{

//--------------------------------------------------------------------------------------------------
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}

static sal_Bool check( sal_Bool b , char * message )
{
    if ( ! b )
        fprintf( stderr, "%s failed\n" , message );
    return b;
}

//==================================================================================================
class TestBridgeImpl : public WeakImplHelper2< XMain, XServiceInfo >
{
    Reference< XMultiServiceFactory > _xMgr;

public:
    TestBridgeImpl( const Reference< XMultiServiceFactory > & xMgr_ )
        : _xMgr( xMgr_ )
        {}

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

    // XMain
    virtual sal_Int32 SAL_CALL run( const Sequence< OUString > & rArgs ) throw (RuntimeException);
};

//==================================================================================================
static sal_Bool equals( const TestElement & rData1, const TestElement & rData2 )
{
    check( rData1.Bool == rData2.Bool, "### bool does not match!" );
    check( rData1.Char == rData2.Char, "### char does not match!" );
    check( rData1.Byte == rData2.Byte, "### byte does not match!" );
    check( rData1.Short == rData2.Short, "### short does not match!" );
    check( rData1.UShort == rData2.UShort, "### unsigned short does not match!" );
    check( rData1.Long == rData2.Long, "### long does not match!" );
    check( rData1.ULong == rData2.ULong, "### unsigned long does not match!" );
    check( rData1.Hyper == rData2.Hyper, "### hyper does not match!" );
    check( rData1.UHyper == rData2.UHyper, "### unsigned hyper does not match!" );
    check( rData1.Float == rData2.Float, "### float does not match!" );
    check( rData1.Double == rData2.Double, "### double does not match!" );
    check( rData1.Enum == rData2.Enum, "### enum does not match!" );
    check( rData1.String == rData2.String, "### string does not match!" );
    check( rData1.Interface == rData2.Interface, "### interface does not match!" );
    check( rData1.Any == rData2.Any, "### any does not match!" );

    return (rData1.Bool == rData2.Bool &&
            rData1.Char == rData2.Char &&
            rData1.Byte == rData2.Byte &&
            rData1.Short == rData2.Short &&
            rData1.UShort == rData2.UShort &&
            rData1.Long == rData2.Long &&
            rData1.ULong == rData2.ULong &&
            rData1.Hyper == rData2.Hyper &&
            rData1.UHyper == rData2.UHyper &&
            rData1.Float == rData2.Float &&
            rData1.Double == rData2.Double &&
            rData1.Enum == rData2.Enum &&
            rData1.String == rData2.String &&
            rData1.Interface == rData2.Interface &&
            rData1.Any == rData2.Any);
}
//==================================================================================================
static sal_Bool equals( const TestData & rData1, const TestData & rData2 )
{
    sal_Int32 nLen;

    if ((rData1.Sequence == rData2.Sequence) &&
        equals( (const TestElement &)rData1, (const TestElement &)rData2 ) &&
        (nLen = rData1.Sequence.getLength()) == rData2.Sequence.getLength())
    {
        // once again by hand sequence ==
        const TestElement * pElements1 = rData1.Sequence.getConstArray();
        const TestElement * pElements2 = rData2.Sequence.getConstArray();
        for ( ; nLen--; )
        {
            if (! equals( pElements1[nLen], pElements2[nLen] ))
            {
                check( sal_False, "### sequence element did not match!" );
                return sal_False;
            }
        }
        return sal_True;
    }
    return sal_False;
}
//==================================================================================================
static void assign( TestElement & rData,
                    sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                    sal_Int16 nShort, sal_uInt16 nUShort,
                    sal_Int32 nLong, sal_uInt32 nULong,
                    sal_Int64 nHyper, sal_uInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, const ::rtl::OUString& rStr,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                    const ::com::sun::star::uno::Any& rAny )
{
    rData.Bool = bBool;
    rData.Char = cChar;
    rData.Byte = nByte;
    rData.Short = nShort;
    rData.UShort = nUShort;
    rData.Long = nLong;
    rData.ULong = nULong;
    rData.Hyper = nHyper;
    rData.UHyper = nUHyper;
    rData.Float = fFloat;
    rData.Double = fDouble;
    rData.Enum = eEnum;
    rData.String = rStr;
    rData.Interface = xTest;
    rData.Any = rAny;
}
//==================================================================================================
static void assign( TestData & rData,
                    sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                    sal_Int16 nShort, sal_uInt16 nUShort,
                    sal_Int32 nLong, sal_uInt32 nULong,
                    sal_Int64 nHyper, sal_uInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, const ::rtl::OUString& rStr,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                    const ::com::sun::star::uno::Any& rAny,
                    const com::sun::star::uno::Sequence< TestElement >& rSequence )
{
    assign( (TestElement &)rData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny );
    rData.Sequence = rSequence;
}

template < class type >
static sal_Bool testAny( const type & value , const Reference< XBridgeTest > &xLBT )
{
    Any any;
    any <<=  value;

    Any any2 = xLBT->transportAny( any );

    if( ! ( any == any2 ) )
    {
        fprintf( stderr, "any is different after roundtrip: in %s, out %s\n",
                 OUStringToOString( any.getValueType().getTypeName(), RTL_TEXTENCODING_ASCII_US ).getStr(),
                 OUStringToOString( any2.getValueType().getTypeName(), RTL_TEXTENCODING_ASCII_US ).getStr());
    }
    return any == any2;
}



static sal_Bool performAnyTest( const Reference< XBridgeTest > &xLBT, const TestData &data)
{
    sal_Bool bReturn = sal_True;
    bReturn = testAny( data.Byte ,xLBT ) && bReturn;
    bReturn = testAny( data.Short,xLBT ) && bReturn;
    bReturn = testAny( data.UShort,xLBT ) && bReturn;
    bReturn = testAny( data.Long,xLBT ) && bReturn;
    bReturn = testAny( data.ULong,xLBT ) && bReturn;
    bReturn = testAny( data.Hyper,xLBT ) && bReturn;
    bReturn = testAny( data.UHyper,xLBT ) && bReturn;
    bReturn = testAny( data.Float,xLBT ) && bReturn;
    bReturn = testAny( data.Double,xLBT ) && bReturn;
    bReturn = testAny( data.Enum,xLBT ) && bReturn;
    bReturn = testAny( data.String,xLBT ) && bReturn;
    bReturn = testAny( data.Interface,xLBT ) && bReturn;
    bReturn = testAny( data, xLBT ) && bReturn;

    Any a;
    {
        a.setValue( &(data.Bool) , getCppuBooleanType() );
        Any a2 = xLBT->transportAny( a );
        OSL_ASSERT( a2 == a );
    }

    {
        a.setValue( &(data.Char) , getCppuCharType() );
        Any a2 = xLBT->transportAny( a );
        OSL_ASSERT( a2 == a );
    }

    return bReturn;
}

//_______________________________________________________________________________________
static sal_Bool performSequenceOfCallTest( const Reference < XBridgeTest > &xLBT )
{
    sal_Int32 i,nRounds;
    sal_Int32 nGlobalIndex = 0;
    const sal_Int32 nWaitTimeSpanMUSec = 10000;
    for( nRounds = 0 ; nRounds < 10 ; nRounds ++ )
    {
        for( i = 0 ; i < nRounds ; i ++ )
        {
            // fire oneways
            xLBT->callOneway( nGlobalIndex , nWaitTimeSpanMUSec );
            nGlobalIndex ++;
        }

        // call synchron
        xLBT->call( nGlobalIndex , nWaitTimeSpanMUSec );
        nGlobalIndex ++;
    }

    return xLBT->sequenceOfCallTestPassed();
}

class ORecursiveCall : public WeakImplHelper1< XRecursiveCall >
{
private:
    Mutex m_mutex;

public:
    void SAL_CALL callRecursivly(
        const ::com::sun::star::uno::Reference< ::com::sun::star::test::bridge::XRecursiveCall >& xCall,
        sal_Int32 nToCall )
        throw(::com::sun::star::uno::RuntimeException)
        {
            MutexGuard guard( m_mutex );
            if( nToCall )
            {
                nToCall --;
                xCall->callRecursivly( this , nToCall );
            }

        }
};


//_______________________________________________________________________________________
static sal_Bool performRecursiveCallTest( const Reference < XBridgeTest > & xLBT )
{
    xLBT->startRecursiveCall( new ORecursiveCall , 50 );
    // on failure, the test would lock up or crash
    return sal_True;
}

static sal_Bool performQueryForUnknownType( const Reference< XBridgeTest > & xLBT )
{
    sal_Bool bRet = sal_True;
    // use this when you want to test querying for unknown types
    // currently (not supported by the java remote bridge )
    {
        // test queryInterface for an unknown type
        typelib_TypeDescriptionReference *pTypeRef = 0;
        OUString aName( RTL_CONSTASCII_USTRINGPARAM( "foo.MyInterface" ) );
        typelib_typedescriptionreference_new(
            &pTypeRef, typelib_TypeClass_INTERFACE,  aName.pData);
        try
        {
            Any a = xLBT->queryInterface( Type( pTypeRef ) );
            bRet = check( a == Any( ), "got an foo.MyInterface, but didn't expect to get one" );
        }
        catch( com::sun::star::uno::RuntimeException & )
        {
            fprintf(
                stderr,
                "tried to query for an interface reference of an unknown type "
                "but got a runtime exception. This should work for native bridges "
                "but isn't implemented for Java remote bridge\n"
                "Note: All subsequent tests may fail now as the remote bridge is broken\n"
                "QueryForUnknownType" );
        }
        typelib_typedescriptionreference_release( pTypeRef );
    }
    return bRet;
}

//==================================================================================================
static sal_Bool performTest( const Reference<XBridgeTest > & xLBT )
{
    check( xLBT.is(), "### no test interface!" );
    sal_Bool bRet = sal_True;
    if (xLBT.is())
    {
        // this data is never ever granted access to by calls other than equals(), assign()!
        TestData aData; // test against this data

        Reference<XInterface > xI( *new OWeakObject() );

        assign( (TestElement &)aData,
                sal_True, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
                0x123456789abcdef0, 0xfedcba9876543210,
                (float)17.0815, 3.1415926359, TestEnum_LOLA,
                OUSTR(STRING_TEST_CONSTANT), xI,
                Any( &xI, ::getCppuType( (const Reference<XInterface > *)0 ) ) );

        bRet = check( aData.Any == xI, "### unexpected any!" ) && bRet;
        bRet = check( !(aData.Any != xI), "### unexpected any!" ) && bRet;

        aData.Sequence.realloc( 2 );
        aData.Sequence[ 0 ] = *(const TestElement *)&aData;
        // aData.Sequence[ 1 ] is empty

        // aData complete
        //================================================================================

        // this is a manually copy of aData for first setting...
        TestData aSetData;

        assign( (TestElement &)aSetData,
                aData.Bool, aData.Char, aData.Byte, aData.Short, aData.UShort,
                aData.Long, aData.ULong, aData.Hyper, aData.UHyper, aData.Float, aData.Double,
                aData.Enum, aData.String, xI,
                Any( &xI, ::getCppuType( (const Reference<XInterface > *)0 ) ) );

        aSetData.Sequence.realloc( 2 );
        aSetData.Sequence[ 0 ] = *(const TestElement *)&aSetData;
        // aSetData.Sequence[ 1 ] is empty

        xLBT->setValues(
            aSetData.Bool, aSetData.Char, aSetData.Byte, aSetData.Short, aSetData.UShort,
            aSetData.Long, aSetData.ULong, aSetData.Hyper, aSetData.UHyper, aSetData.Float, aSetData.Double,
            aSetData.Enum, aSetData.String, aSetData.Interface, aSetData.Any, aSetData.Sequence, aSetData );

        {
        TestData aRet, aRet2;
        xLBT->getValues(
            aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
            aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
            aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );

        bRet = check( equals( aData, aRet ) && equals( aData, aRet2 ) , "getValues test") && bRet;

        // set last retrieved values
        TestData aSV2ret = xLBT->setValues2(
            aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
            aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
            aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );

        bRet = check(
            equals( aData, aSV2ret ) && equals( aData, aRet2 ) , "getValues2 test") && bRet;

        // check inout sequence order => inout sequence parameter was switched by test objects
        bRet = check(
            equals( aRet.Sequence[ 0 ], aData.Sequence[ 1 ] ) &&
            equals( aRet.Sequence[ 1 ], aData.Sequence[ 0 ] ), "sequence order test" );
        }
        {
        TestData aRet, aRet2;
        TestData aGVret = xLBT->getValues(
            aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
            aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
            aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );

        bRet = check( equals( aData, aRet ) && equals( aData, aRet2 ) && equals( aData, aGVret ), "getValues test" ) && bRet;

        // set last retrieved values
        xLBT->setBool( aRet.Bool );
        xLBT->setChar( aRet.Char );
        xLBT->setByte( aRet.Byte );
        xLBT->setShort( aRet.Short );
        xLBT->setUShort( aRet.UShort );
        xLBT->setLong( aRet.Long );
        xLBT->setULong( aRet.ULong );
        xLBT->setHyper( aRet.Hyper );
        xLBT->setUHyper( aRet.UHyper );
        xLBT->setFloat( aRet.Float );
        xLBT->setDouble( aRet.Double );
        xLBT->setEnum( aRet.Enum );
        xLBT->setString( aRet.String );
        xLBT->setInterface( aRet.Interface );
        xLBT->setAny( aRet.Any );
        xLBT->setSequence( aRet.Sequence );
        xLBT->setStruct( aRet2 );
        }
        {
        TestData aRet, aRet2;
        aRet.Hyper = xLBT->getHyper();
        aRet.UHyper = xLBT->getUHyper();
        aRet.Float = xLBT->getFloat();
        aRet.Double = xLBT->getDouble();
        aRet.Byte = xLBT->getByte();
        aRet.Char = xLBT->getChar();
        aRet.Bool = xLBT->getBool();
        aRet.Short = xLBT->getShort();
        aRet.UShort = xLBT->getUShort();
        aRet.Long = xLBT->getLong();
        aRet.ULong = xLBT->getULong();
        aRet.Enum = xLBT->getEnum();
        aRet.String = xLBT->getString();
        aRet.Interface = xLBT->getInterface();
        aRet.Any = xLBT->getAny();
        aRet.Sequence = xLBT->getSequence();
        aRet2 = xLBT->getStruct();

        bRet = check( equals( aData, aRet ) && equals( aData, aRet2 ) , "struct comparison test") && bRet;
        // any test
        bRet = check( performAnyTest( xLBT , aData ) , "any test" ) && bRet;

        // sequence of call test
        bRet = check( performSequenceOfCallTest( xLBT ) , "sequence of call test" ) && bRet;

        // recursive call test
        bRet = check( performRecursiveCallTest( xLBT ) , "recursive test" ) && bRet;

        bRet = (equals( aData, aRet ) && equals( aData, aRet2 )) && bRet ;
        }

    }
    return bRet;
}
static sal_Bool raiseOnewayException( const Reference < XBridgeTest > & xLBT )
{
    sal_Bool bReturn = sal_True;
    OUString sCompare = OUSTR(STRING_TEST_CONSTANT);
    try
    {
        // Note : the exception may fly or not (e.g. remote scenario).
        //        When it flies, it must contain the correct elements.
        xLBT->raiseRuntimeExceptionOneway(  sCompare, xLBT->getInterface() );
    }
    catch( RuntimeException & e )
    {
        bReturn = (
#if OSL_DEBUG_LEVEL == 0
            // java stack traces trash Message
            e.Message == sCompare &&
#endif
            xLBT->getInterface() == e.Context );
    }
    return bReturn;
}

//==================================================================================================
static sal_Bool raiseException( const Reference< XBridgeTest > & xLBT )
{
    sal_Int32 nCount = 0;
    try
    {
        try
        {
            try
            {
                TestData aRet, aRet2;
                xLBT->raiseException(
                    5, OUSTR(STRING_TEST_CONSTANT),
                    xLBT->getInterface() );
            }
            catch (IllegalArgumentException aExc)
            {
                if (aExc.ArgumentPosition == 5 &&
#if OSL_DEBUG_LEVEL == 0
                    // java stack traces trash Message
                    aExc.Message.compareToAscii( STRING_TEST_CONSTANT ) == 0 &&
#endif
                    aExc.Context == xLBT->getInterface())
                {
                    ++nCount;
                }
                else
                {
                    check( sal_False, "### unexpected exception content!" );
                }

                /** it is certain, that the RuntimeException testing will fail, if no */
                xLBT->getRuntimeException();
            }
        }
        catch (const RuntimeException & rExc)
        {
            if (rExc.Context == xLBT->getInterface()
#if OSL_DEBUG_LEVEL == 0
                    // java stack traces trash Message
                && rExc.Message.compareToAscii( STRING_TEST_CONSTANT ) == 0
#endif
                )
            {
                ++nCount;
            }
            else
            {
                check( sal_False, "### unexpected exception content!" );
            }

            /** it is certain, that the RuntimeException testing will fail, if no */
            xLBT->setRuntimeException( 0xcafebabe );
        }
    }
    catch (Exception & rExc)
    {
        if (rExc.Context == xLBT->getInterface()
#if OSL_DEBUG_LEVEL == 0
            // java stack traces trash Message
            && rExc.Message.compareToAscii( STRING_TEST_CONSTANT ) == 0
#endif
            )
        {
            ++nCount;
        }
        else
        {
            check( sal_False, "### unexpected exception content!" );
        }
        return (nCount == 3);
    }
    return sal_False;
}

template< class T >
static inline sal_Bool makeSurrogate( com::sun::star::uno::Reference< T > & rOut,
                                      const com::sun::star::uno::Reference< T > & rOriginal )
{
    rOut.clear();

    typelib_TypeDescription * pTD = 0;
    const com::sun::star::uno::Type & rType = ::getCppuType( &rOriginal );
    TYPELIB_DANGER_GET( &pTD, rType.getTypeLibType() );
    check( pTD ? 1 : 0 , "### cannot get typedescription!" );
    if (pTD)
    {
        uno_Environment * pCppEnv1 = 0;
        uno_Environment * pCppEnv2 = 0;

        OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
        uno_getEnvironment( &pCppEnv1, aCppEnvTypeName.pData, 0 );
        uno_createEnvironment( &pCppEnv2, aCppEnvTypeName.pData, 0 ); // anonymous

        ::com::sun::star::uno::Mapping aMapping( pCppEnv1, pCppEnv2, OUString::createFromAscii("prot") );
        T * p = (T *)aMapping.mapInterface( rOriginal.get(), (typelib_InterfaceTypeDescription *)pTD );
        if (p)
        {
            rOut = p;
            p->release();
        }

        (*pCppEnv2->release)( pCppEnv2 );
        (*pCppEnv1->release)( pCppEnv1 );

        TYPELIB_DANGER_RELEASE( pTD );
    }
    return rOut.is();
}

//==================================================================================================
sal_Int32 TestBridgeImpl::run( const Sequence< OUString > & rArgs )
    throw (RuntimeException)
{
    try
    {
        if (! rArgs.getLength())
        {
            throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                  "no test object specified!\n"
                                                  "usage : ServiceName of test object | -u unourl of test object\n" ) ),
                                    Reference< XInterface >() );
        }

        Reference< XInterface > xOriginal;
        if( rArgs.getLength() > 1 && 0 == rArgs[0].compareToAscii( "-u" ) )
        {
            Reference <XInterface > r = _xMgr->createInstance(
                OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ) );
            Reference <XUnoUrlResolver> rResolver( r , UNO_QUERY );
            xOriginal = rResolver->resolve( rArgs[1] );
        }
        else
        {
            // local test
            xOriginal = _xMgr->createInstance( rArgs[0] );
        }

        if (! xOriginal.is())
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("cannot instantiate service!") ),
                Reference< XInterface >() );
        }
        Reference< XBridgeTest > xTest( xOriginal, UNO_QUERY );
        if (! xTest.is())
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("test object does not implement XBridgeTest!") ),
                Reference< XInterface >() );
        }

        Reference<XBridgeTest > xLBT;
        sal_Bool bRet;
        bRet = check( makeSurrogate( xLBT, xTest ), "makeSurrogate" );
        bRet = check( performTest( xLBT ), "standard test" ) && bRet;
        bRet = check( raiseException( xLBT ) , "exception test" )&& bRet;
        bRet = check( raiseOnewayException( xLBT ), "oneway exception test" ) && bRet;
        bRet = performQueryForUnknownType( xLBT ) && bRet;
        if (! bRet)
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("error: test failed!") ),
                Reference< XInterface >() );
        }

        if( bRet )
        {
            printf( "> dynamic invocation test succeeded!\n" );
        }
        else
        {
            printf( "> dynamic invocation test failed!\n" );
        }
    }
    catch (Exception & exc)
    {
        OString cstr( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "exception occured: %s\n", cstr.getStr() );
        throw;
    }

    return 0;
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString TestBridgeImpl::getImplementationName()
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}
//__________________________________________________________________________________________________
sal_Bool TestBridgeImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > TestBridgeImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return bridge_test::getSupportedServiceNames();
}

// ...

//==================================================================================================
static Reference< XInterface > SAL_CALL TestBridgeImpl_create(
    const Reference< XMultiServiceFactory > & xSMgr )
{
    return Reference< XInterface >( (XBridgeTest *)new TestBridgeImpl( xSMgr ) );
}

}

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );
            xNewKey->createKey( OUString( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) ) );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            bridge_test::TestBridgeImpl_create,
            bridge_test::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}
