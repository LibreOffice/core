/**************************************************************************
#*
#*    last change   $Author: jbu $ $Date: 2001-07-04 08:41:23 $
#*    $Revision: 1.2 $
#*
#*    $Logfile: $
#*
#*    Copyright (c) 1989 - 2000, Star Office GmbH
#*
#************************************************************************/

#include <stdio.h>
#include <osl/diagnose.h>

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

namespace bridge_test
{

//--------------------------------------------------------------------------------------------------
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
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
    OSL_ENSURE( rData1.Bool == rData2.Bool, "### bool does not match!" );
    OSL_ENSURE( rData1.Char == rData2.Char, "### char does not match!" );
    OSL_ENSURE( rData1.Byte == rData2.Byte, "### byte does not match!" );
    OSL_ENSURE( rData1.Short == rData2.Short, "### short does not match!" );
    OSL_ENSURE( rData1.UShort == rData2.UShort, "### unsigned short does not match!" );
    OSL_ENSURE( rData1.Long == rData2.Long, "### long does not match!" );
    OSL_ENSURE( rData1.ULong == rData2.ULong, "### unsigned long does not match!" );
    OSL_ENSURE( rData1.Hyper == rData2.Hyper, "### hyper does not match!" );
    OSL_ENSURE( rData1.UHyper == rData2.UHyper, "### unsigned hyper does not match!" );
    OSL_ENSURE( rData1.Float == rData2.Float, "### float does not match!" );
    OSL_ENSURE( rData1.Double == rData2.Double, "### double does not match!" );
    OSL_ENSURE( rData1.Enum == rData2.Enum, "### enum does not match!" );
    OSL_ENSURE( rData1.String == rData2.String, "### string does not match!" );
    OSL_ENSURE( rData1.Interface == rData2.Interface, "### interface does not match!" );
    OSL_ENSURE( rData1.Any == rData2.Any, "### any does not match!" );

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
                OSL_ENSURE( sal_False, "### sequence element did not match!" );
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

    OSL_ASSERT( any == any2 );
    return any == any2;
}



static sal_Bool performAnyTest( const Reference< XBridgeTest > &xLBT, const TestData &data)
{
    sal_Bool bReturn = sal_True;
    bReturn = testAny( data.Byte ,xLBT ) && bReturn;
    bReturn = testAny( data.Short,xLBT ) && bReturn;
    bReturn = testAny( data.UShort,xLBT ) && bReturn;
    bReturn = testAny( data.Long,xLBT ) && bReturn;
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


//==================================================================================================
static sal_Bool performTest( const Reference<XBridgeTest > & xLBT )
{
    OSL_ENSURE( xLBT.is(), "### no test interface!" );
    sal_Bool bRet = sal_False;
    if (xLBT.is())
    {
        // this data is never ever granted access to by calls other than equals(), assign()!
        TestData aData; // test against this data

        Reference<XInterface > xI( *new OWeakObject() );

        assign( (TestElement &)aData,
                sal_True, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
                0x123456789abcdef0, 0xfedcba9876543210,
                (float)17.0815, 3.1415926359, TestEnum_LOLA,
                OUString::createFromAscii("dum dum dum ich tanz im kreis herum..."), xI,
                Any( &xI, ::getCppuType( (const Reference<XInterface > *)0 ) ) );

        OSL_ENSURE( aData.Any == xI, "### unexpected any!" );
        OSL_ENSURE( !(aData.Any != xI), "### unexpected any!" );

        aData.Sequence = Sequence<TestElement >( (const TestElement *)&aData, 1 );
        // aData complete
        //================================================================================

        // this is a manually copy of aData for first setting...
        TestData aSetData;

        assign( (TestElement &)aSetData,
                aData.Bool, aData.Char, aData.Byte, aData.Short, aData.UShort,
                aData.Long, aData.ULong, aData.Hyper, aData.UHyper, aData.Float, aData.Double,
                aData.Enum, aData.String, xI,
                Any( &xI, ::getCppuType( (const Reference<XInterface > *)0 ) ) );

        aSetData.Sequence = Sequence<TestElement >( (const TestElement *)&aSetData, 1 );

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

        OSL_ASSERT( equals( aData, aRet ) && equals( aData, aRet2 ) );

        // set last retrieved values
        TestData aSV2ret = xLBT->setValues2(
            aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
            aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
            aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );

        OSL_ASSERT( equals( aData, aSV2ret ) && equals( aData, aRet2 ) );
        }
        {
        TestData aRet, aRet2;
        TestData aGVret = xLBT->getValues(
            aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
            aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
            aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );

        OSL_ASSERT( equals( aData, aRet ) && equals( aData, aRet2 ) && equals( aData, aGVret ) );

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

        OSL_ASSERT( equals( aData, aRet ) && equals( aData, aRet2 ) );

        // any test
        OSL_ASSERT( performAnyTest( xLBT , aData ) );

        // sequence of call test
        OSL_ASSERT( performSequenceOfCallTest( xLBT ) );

        // recursive call test
        OSL_ASSERT( performRecursiveCallTest( xLBT ) );

        bRet = (equals( aData, aRet ) && equals( aData, aRet2 ));
        }
        {
            // test queryInterface for an unknown type
            typelib_TypeDescriptionReference *pTypeRef = 0;
            OUString aName( RTL_CONSTASCII_USTRINGPARAM( "foo.MyInterface" ) );
            typelib_typedescriptionreference_new(
                &pTypeRef, typelib_TypeClass_INTERFACE,  aName.pData);
            Any a = xLBT->queryInterface( Type( pTypeRef ) );
            typelib_typedescriptionreference_release( pTypeRef );
            bRet = bRet && ( a  == Any( ));
        }
    }
    return bRet;
}
static sal_Bool raiseOnewayException( const Reference < XBridgeTest > & xLBT )
{
    sal_Bool bReturn = sal_True;
    OUString sCompare = OUString( RTL_CONSTASCII_USTRINGPARAM("dum dum dum ich tanz im kreis herum...") );
    try
    {
        // Note : the exception may fly or not (e.g. remote scenario).
        //        When it flies, it must contain the correct elements.
        xLBT->raiseRuntimeExceptionOneway(  sCompare, xLBT->getInterface() );
    }
    catch( RuntimeException & e )
    {
        bReturn = ( e.Message == sCompare && xLBT->getInterface() == e.Context );
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
                    5, OUString( RTL_CONSTASCII_USTRINGPARAM("dum dum dum ich tanz im kreis herum...") ),
                    xLBT->getInterface() );
            }
            catch (IllegalArgumentException aExc)
            {
                if (aExc.ArgumentPosition == 5 &&
                    aExc.Context == xLBT->getInterface() &&
                    aExc.Message.compareToAscii( "dum dum dum ich tanz im kreis herum..." ) == 0)
                {
                    ++nCount;
                }
                else
                {
                    OSL_ENSURE( sal_False, "### unexpected exception content!" );
                }

                /** it is certain, that the RuntimeException testing will fail, if no */
                xLBT->getRuntimeException();
            }
        }
        catch (const RuntimeException & rExc)
        {
            if (rExc.Context == xLBT->getInterface() &&
                rExc.Message.compareToAscii( "dum dum dum ich tanz im kreis herum..." ) == 0)
            {
                ++nCount;
            }
            else
            {
                OSL_ENSURE( sal_False, "### unexpected exception content!" );
            }

            /** it is certain, that the RuntimeException testing will fail, if no */
            xLBT->setRuntimeException( 0xcafebabe );
        }
    }
    catch (Exception & rExc)
    {
        if (rExc.Context == xLBT->getInterface() &&
            rExc.Message.compareToAscii( "dum dum dum ich tanz im kreis herum..." ) == 0)
        {
            ++nCount;
        }
        else
        {
            OSL_ENSURE( sal_False, "### unexpected exception content!" );
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
    OSL_ENSURE( pTD, "### cannot get typedescription!" );
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
    if (! rArgs.getLength())
    {
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "no test object specified!\n"
            "usage : ServiceName of test object | -u unourl of test object\n" ) ),
                                Reference< XInterface >() );
    }

    Reference<XBridgeTest > xOriginal;
    if( rArgs.getLength() > 1 && 0 == rArgs[0].compareToAscii( "-u" ) )
    {
        Reference <XInterface > r = _xMgr->createInstance(
            OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ) );
        Reference <XUnoUrlResolver> rResolver( r , UNO_QUERY );
        r =  rResolver->resolve( rArgs[1] );
        xOriginal = Reference < XBridgeTest > ( r , UNO_QUERY );
    }
    else
    {
        // local test
        xOriginal = Reference<XBridgeTest > ( _xMgr->createInstance( rArgs[0] ), UNO_QUERY );
    }

    if (! xOriginal.is())
    {
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("cannot instantiate object!") ),
                                Reference< XInterface >() );
    }

    Reference<XBridgeTest > xLBT;
    if (makeSurrogate( xLBT, xOriginal ) &&
        performTest( xLBT ) &&
        raiseException( xLBT ) &&
        raiseOnewayException( xLBT ) )
    {
        printf( "> dynamic invocation test succeeded!\n" );
    }
    else
    {
        printf( "> dynamic invocation test failed!\n" );
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

/**************************************************************************
    $Log: not supported by cvs2svn $
    Revision 1.1  2001/05/04 07:05:17  kr
    moved from grande to openoffice

    Revision 1.3  2001/03/12 16:22:44  jl
    OSL_ENSHURE replaced by OSL_ENSURE

    Revision 1.2  2000/08/14 07:12:50  jbu
    added remote tests

    Revision 1.1  2000/05/26 14:20:26  dbo
    new


**************************************************************************/
