/**************************************************************************
#*
#*    last change   $Author: vg $ $Date: 2003-10-06 12:58:44 $
#*    $Revision: 1.9 $
#*
#*    $Logfile: $
#*
#*    Copyright (c) 1989 - 2000, Star Office GmbH
#*
#************************************************************************/

#include <stdio.h>
#include <osl/diagnose.h>
#include <osl/time.h>
#include "typelib/typedescription.hxx"
#include <uno/dispatcher.hxx>
#include "uno/mapping.hxx"
#include <uno/data.h>
#include "uno/environment.hxx"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>

#include <com/sun/star/test/bridge/XBridgeTest.hpp>
#include <com/sun/star/test/bridge/XBridgeTest2.hpp>

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
template< class T>
Sequence<T> cloneSequence(const Sequence<T>& val);

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
    Reference< XComponentContext > m_xContext;

public:
    TestBridgeImpl( const Reference< XComponentContext > & xContext )
        : m_xContext( xContext )
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

class MyClass : public OWeakObject
{
public:
    static sal_Int32 s_instances;

    MyClass();
    virtual ~MyClass();
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();
};

sal_Int32 MyClass::s_instances = 0;

//______________________________________________________________________________
MyClass::MyClass()
{
    ++s_instances;
}
//______________________________________________________________________________
MyClass::~MyClass()
{
    --s_instances;
}
//______________________________________________________________________________
void MyClass::acquire() throw ()
{
    OWeakObject::acquire();
}
//______________________________________________________________________________
void MyClass::release() throw ()
{
    OWeakObject::release();
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

        Reference<XInterface > xI( new MyClass );

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
        // check inout sequence order
        // => inout sequence parameter was switched by test objects
        TestElement temp = aRet.Sequence[ 0 ];
        aRet.Sequence[ 0 ] = aRet.Sequence[ 1 ];
        aRet.Sequence[ 1 ] = temp;

        bRet = check(
            equals( aData, aSV2ret ) && equals( aData, aRet2 ),
            "getValues2 test") && bRet;
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
    {
    Reference<XBridgeTest2> xBT2(xLBT, UNO_QUERY);
    if ( ! xBT2.is())
        return bRet;

    // perform sequence tests (XBridgeTest2)
    // create the sequence which are compared with the results
    sal_Bool _arBool[] = {sal_True, sal_False, sal_True};
    sal_Unicode _arChar[] = {0x0065, 0x0066, 0x0067};
    sal_Int8 _arByte[] = { (sal_Int8)1, (sal_Int8) 2, (sal_Int8) 0xff};
    sal_Int16 _arShort[] = {(sal_Int16) 0x8000,(sal_Int16) 1, (sal_Int16) 0xefff};
    sal_uInt16 _arUShort[] = {0 , 1, 0xffff};
    sal_Int32 _arLong[] = {0x80000000, 1, 0x7fffffff};
    sal_uInt32 _arULong[] = {0, 1, 0xffffffff};
    sal_Int64 _arHyper[] = {0x8000000000000000, 1, 0x7fffffffffffffff};
    sal_uInt64 _arUHyper[] = {0, 1, 0xffffffffffffffff};
    float _arFloat[] = {1.1f, 2.2f, 3.3f};
    double _arDouble[] = {1.11, 2.22, 3.33};
    OUString _arString[] = {
        OUString(RTL_CONSTASCII_USTRINGPARAM("String 1")),
        OUString(RTL_CONSTASCII_USTRINGPARAM("String 2")),
        OUString(RTL_CONSTASCII_USTRINGPARAM("String 3"))
    };

    sal_Bool _aBool = sal_True;
    sal_Int32 _aInt = 0xbabebabe;
    float _aFloat = 3.14f;
    Any _any1(&_aBool, getCppuBooleanType());
    Any _any2(&_aInt, getCppuType((sal_Int32*) 0));
    Any _any3(&_aFloat, getCppuType((float*) 0));
    Any _arAny[] = { _any1, _any2, _any3};

    Reference<XInterface> _arObj[3];
    _arObj[0] = new OWeakObject();
    _arObj[1] = new OWeakObject();
    _arObj[2] = new OWeakObject();

    TestEnum _arEnum[] = {TestEnum_ONE, TestEnum_TWO, TestEnum_CHECK};

    TestElement _arStruct[3];
    assign( _arStruct[0], sal_True, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
            0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum_LOLA, OUSTR(STRING_TEST_CONSTANT), _arObj[0],
            Any( &_arObj[0], ::getCppuType( (const Reference<XInterface > *)0 ) ) );
    assign( _arStruct[1], sal_True, 'A', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
            0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum_TWO, OUSTR(STRING_TEST_CONSTANT), _arObj[1],
            Any( &_arObj[1], ::getCppuType( (const Reference<XInterface > *)0 ) ) );
    assign( _arStruct[2], sal_True, 'B', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
            0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum_CHECK, OUSTR(STRING_TEST_CONSTANT), _arObj[2],
            Any( &_arObj[2], ::getCppuType( (const Reference<XInterface > *)0 ) ) );

    Sequence<sal_Bool> arBool(_arBool, 3);
    Sequence<sal_Unicode> arChar( _arChar, 3);
    Sequence<sal_Int8> arByte(_arByte, 3);
    Sequence<sal_Int16> arShort(_arShort, 3);
    Sequence<sal_uInt16> arUShort(_arUShort, 3);
    Sequence<sal_Int32> arLong(_arLong, 3);
    Sequence<sal_uInt32> arULong(_arULong, 3);
    Sequence<sal_Int64> arHyper(_arHyper, 3);
    Sequence<sal_uInt64> arUHyper(_arUHyper, 3);
    Sequence<float> arFloat(_arFloat, 3);
    Sequence<double> arDouble(_arDouble, 3);
    Sequence<OUString> arString(_arString, 3);
    Sequence<Any> arAny(_arAny, 3);
    Sequence<Reference<XInterface> > arObject(_arObj, 3);
    Sequence<TestEnum> arEnum(_arEnum, 3);
    Sequence<TestElement> arStruct(_arStruct, 3);
    Sequence<Sequence<sal_Int32> > _arSeqLong2[3];
    for (int j = 0; j < 3; j++)
    {
        Sequence<sal_Int32> _arSeqLong[3];
        for (int i = 0; i < 3; i++)
        {
            _arSeqLong[i] = Sequence<sal_Int32>(_arLong, 3);
        }

        _arSeqLong2[j] = Sequence< Sequence<sal_Int32> > (_arSeqLong, 3);
    }
    Sequence<Sequence<Sequence<sal_Int32> > > arLong3( _arSeqLong2, 3);
    {
    Sequence<Sequence<sal_Int32> > seqSeqRet = xBT2->setDim2(arLong3[0]);
    bRet = check( seqSeqRet == arLong3[0], "sequence test") && bRet;
    Sequence<Sequence<Sequence<sal_Int32> > > seqSeqRet2 = xBT2->setDim3(arLong3);
    bRet = check( seqSeqRet2 == arLong3, "sequence test") && bRet;
    Sequence<Any> seqAnyRet = xBT2->setSequenceAny(arAny);
    bRet = check( seqAnyRet == arAny, "sequence test") && bRet;
    Sequence<sal_Bool> seqBoolRet = xBT2->setSequenceBool(arBool);
    bRet = check( seqBoolRet == arBool, "sequence test") && bRet;
    Sequence<sal_Int8> seqByteRet = xBT2->setSequenceByte(arByte);
    bRet = check( seqByteRet == arByte, "sequence test") && bRet;
    Sequence<sal_Unicode> seqCharRet = xBT2->setSequenceChar(arChar);
    bRet = check( seqCharRet == arChar, "sequence test") && bRet;
    Sequence<sal_Int16> seqShortRet = xBT2->setSequenceShort(arShort);
    bRet = check( seqShortRet == arShort, "sequence test") && bRet;
    Sequence<sal_Int32> seqLongRet = xBT2->setSequenceLong(arLong);
    bRet = check( seqLongRet == arLong, "sequence test") && bRet;
    Sequence<sal_Int64> seqHyperRet = xBT2->setSequenceHyper(arHyper);
    bRet = check( seqHyperRet == arHyper, "sequence test") && bRet;
    Sequence<float> seqFloatRet = xBT2->setSequenceFloat(arFloat);
    bRet = check( seqFloatRet == arFloat, "sequence test") && bRet;
    Sequence<double> seqDoubleRet = xBT2->setSequenceDouble(arDouble);
    bRet = check( seqDoubleRet == arDouble, "sequence test") && bRet;
    Sequence<TestEnum> seqEnumRet = xBT2->setSequenceEnum(arEnum);
    bRet = check( seqEnumRet == arEnum, "sequence test") && bRet;
    Sequence<sal_uInt16> seqUShortRet = xBT2->setSequenceUShort(arUShort);
    bRet = check( seqUShortRet == arUShort, "sequence test") && bRet;
    Sequence<sal_uInt32> seqULongRet = xBT2->setSequenceULong(arULong);
    bRet = check( seqULongRet == arULong, "sequence test") && bRet;
    Sequence<sal_uInt64> seqUHyperRet = xBT2->setSequenceUHyper(arUHyper);
    bRet = check( seqUHyperRet == arUHyper, "sequence test") && bRet;
    Sequence<Reference<XInterface> > seqObjectRet = xBT2->setSequenceXInterface(arObject);
    bRet = check( seqObjectRet == arObject, "sequence test") && bRet;
    Sequence<OUString> seqStringRet = xBT2->setSequenceString(arString);
    bRet = check( seqStringRet == arString, "sequence test") && bRet;
    Sequence<TestElement> seqStructRet = xBT2->setSequenceStruct(arStruct);
    bRet = check( seqStructRet == arStruct, "sequence test") && bRet;
    }
    {
    Sequence<sal_Bool> arBoolTemp = cloneSequence(arBool);
    Sequence<sal_Unicode> arCharTemp = cloneSequence(arChar);
    Sequence<sal_Int8> arByteTemp = cloneSequence(arByte);
    Sequence<sal_Int16> arShortTemp = cloneSequence(arShort);
    Sequence<sal_uInt16> arUShortTemp = cloneSequence(arUShort);
    Sequence<sal_Int32> arLongTemp = cloneSequence(arLong);
    Sequence<sal_uInt32> arULongTemp = cloneSequence(arULong);
    Sequence<sal_Int64> arHyperTemp = cloneSequence(arHyper);
    Sequence<sal_uInt64> arUHyperTemp = cloneSequence(arUHyper);
    Sequence<float> arFloatTemp = cloneSequence(arFloat);
    Sequence<double> arDoubleTemp = cloneSequence(arDouble);
    Sequence<TestEnum> arEnumTemp = cloneSequence(arEnum);
    Sequence<OUString> arStringTemp = cloneSequence(arString);
    Reference<XInterface> _xint;
    Sequence<Reference<XInterface> > arObjectTemp = cloneSequence(arObject);
    Sequence<Any> arAnyTemp = cloneSequence(arAny);
    Sequence<Sequence<sal_Int32> > arLong2Temp(arLong3[0]);
    Sequence<Sequence<Sequence<sal_Int32> > > arLong3Temp(arLong3);

    xBT2->setSequencesInOut(arBoolTemp, arCharTemp, arByteTemp, arShortTemp,
                            arUShortTemp, arLongTemp,arULongTemp, arHyperTemp,
                            arUHyperTemp, arFloatTemp, arDoubleTemp,
                            arEnumTemp, arStringTemp, arObjectTemp, arAnyTemp,
                            arLong2Temp, arLong3Temp);
    bRet = check(
        arBoolTemp == arBool &&
        arCharTemp == arChar &&
        arByteTemp == arByte &&
        arShortTemp == arShort &&
        arUShortTemp == arUShort &&
        arLongTemp == arLong &&
        arULongTemp == arULong &&
        arHyperTemp == arHyper &&
        arUHyperTemp == arUHyper &&
        arFloatTemp == arFloat &&
        arDoubleTemp == arDouble &&
        arEnumTemp == arEnum &&
        arStringTemp == arString &&
        arObjectTemp == arObject &&
        arAnyTemp == arAny &&
        arLong2Temp == arLong3[0] &&
        arLong3Temp == arLong3, "sequence test") && bRet;

    Sequence<sal_Bool> arBoolOut;
    Sequence<sal_Unicode> arCharOut;
    Sequence<sal_Int8> arByteOut;
    Sequence<sal_Int16> arShortOut;
    Sequence<sal_uInt16> arUShortOut;
    Sequence<sal_Int32> arLongOut;
    Sequence<sal_uInt32> arULongOut;
    Sequence<sal_Int64> arHyperOut;
    Sequence<sal_uInt64> arUHyperOut;
    Sequence<float> arFloatOut;
    Sequence<double> arDoubleOut;
    Sequence<TestEnum> arEnumOut;
    Sequence<OUString> arStringOut;
    Sequence<Reference<XInterface> > arObjectOut;
    Sequence<Any> arAnyOut;
    Sequence<Sequence<sal_Int32> > arLong2Out;
    Sequence<Sequence<Sequence<sal_Int32> > > arLong3Out;
    xBT2->setSequencesOut(arBoolOut, arCharOut, arByteOut, arShortOut,
                          arUShortOut, arLongOut,arULongOut, arHyperOut,
                          arUHyperOut, arFloatOut, arDoubleOut,
                          arEnumOut, arStringOut, arObjectOut, arAnyOut,
                          arLong2Out, arLong3Out);
    bRet = check(
        arBoolOut == arBool &&
        arCharOut == arChar &&
        arByteOut == arByte &&
        arShortOut == arShort &&
        arUShortOut == arUShort &&
        arLongOut == arLong &&
        arULongOut == arULong &&
        arHyperOut == arHyper &&
        arUHyperOut == arUHyper &&
        arFloatOut == arFloat &&
        arDoubleOut == arDouble &&
        arEnumOut == arEnum &&
        arStringOut == arString &&
        arObjectOut == arObject &&
        arAnyOut == arAny &&
        arLong2Out == arLong3[0] &&
        arLong3Out == arLong3, "sequence test") && bRet;
    }
    {
    //test with empty sequences
        Sequence<Sequence<sal_Int32> > arLong2;
        Sequence<Sequence<sal_Int32> > seqSeqRet = xBT2->setDim2(arLong2);
        bRet = check( seqSeqRet == arLong2, "sequence test") && bRet;
        Sequence<Sequence<Sequence<sal_Int32> > > arLong3;
        Sequence<Sequence<Sequence<sal_Int32> > > seqSeqRet2 = xBT2->setDim3(
            arLong3);
        bRet = check( seqSeqRet2 == arLong3, "sequence test") && bRet;
        Sequence<Any> arAny;
        Sequence<Any> seqAnyRet = xBT2->setSequenceAny(arAny);
        bRet = check( seqAnyRet == arAny, "sequence test") && bRet;
        Sequence<sal_Bool> arBool;
        Sequence<sal_Bool> seqBoolRet = xBT2->setSequenceBool(arBool);
        bRet = check( seqBoolRet == arBool, "sequence test") && bRet;
        Sequence<sal_Int8> arByte;
        Sequence<sal_Int8> seqByteRet = xBT2->setSequenceByte(arByte);
        bRet = check( seqByteRet == arByte, "sequence test") && bRet;
        Sequence<sal_Unicode> arChar;
        Sequence<sal_Unicode> seqCharRet = xBT2->setSequenceChar(arChar);
        bRet = check( seqCharRet == arChar, "sequence test") && bRet;
        Sequence<sal_Int16> arShort;
        Sequence<sal_Int16> seqShortRet = xBT2->setSequenceShort(arShort);
        bRet = check( seqShortRet == arShort, "sequence test") && bRet;
        Sequence<sal_Int32> arLong;
        Sequence<sal_Int32> seqLongRet = xBT2->setSequenceLong(arLong);
        bRet = check( seqLongRet == arLong, "sequence test") && bRet;
        Sequence<sal_Int64> arHyper;
        Sequence<sal_Int64> seqHyperRet = xBT2->setSequenceHyper(arHyper);
        bRet = check( seqHyperRet == arHyper, "sequence test") && bRet;
        Sequence<float> arFloat;
        Sequence<float> seqFloatRet = xBT2->setSequenceFloat(arFloat);
        bRet = check( seqFloatRet == arFloat, "sequence test") && bRet;
        Sequence<double> arDouble;
        Sequence<double> seqDoubleRet = xBT2->setSequenceDouble(arDouble);
        bRet = check( seqDoubleRet == arDouble, "sequence test") && bRet;
        Sequence<TestEnum> arEnum;
        Sequence<TestEnum> seqEnumRet = xBT2->setSequenceEnum(arEnum);
        bRet = check( seqEnumRet == arEnum, "sequence test") && bRet;
        Sequence<sal_uInt16> arUShort;
        Sequence<sal_uInt16> seqUShortRet = xBT2->setSequenceUShort(arUShort);
        bRet = check( seqUShortRet == arUShort, "sequence test") && bRet;
        Sequence<sal_uInt32> arULong;
        Sequence<sal_uInt32> seqULongRet = xBT2->setSequenceULong(arULong);
        bRet = check( seqULongRet == arULong, "sequence test") && bRet;
        Sequence<sal_uInt64> arUHyper;
        Sequence<sal_uInt64> seqUHyperRet = xBT2->setSequenceUHyper(arUHyper);
        bRet = check( seqUHyperRet == arUHyper, "sequence test") && bRet;
        Sequence<Reference<XInterface> > arObject;
        Sequence<Reference<XInterface> > seqObjectRet =
            xBT2->setSequenceXInterface(arObject);
        bRet = check( seqObjectRet == arObject, "sequence test") && bRet;
        Sequence<OUString> arString;
        Sequence<OUString> seqStringRet = xBT2->setSequenceString(arString);
        bRet = check( seqStringRet == arString, "sequence test") && bRet;
        Sequence<TestElement> arStruct;
        Sequence<TestElement> seqStructRet = xBT2->setSequenceStruct(arStruct);
        bRet = check( seqStructRet == arStruct, "sequence test") && bRet;
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

/* Returns an acquired sequence
 */
uno_Sequence* cloneSequence(const uno_Sequence* val, const Type& type)
{
    TypeDescription td(type);
    td.makeComplete();
    typelib_TypeDescription* pTdRaw = td.get();
    typelib_IndirectTypeDescription* pIndirectTd =
        (typelib_IndirectTypeDescription*) pTdRaw;

    typelib_TypeDescription* pTdElem = pIndirectTd->pType->pType;
    sal_Int8* buf = new sal_Int8[pTdElem->nSize * val->nElements];
    sal_Int8* pBufCur = buf;

    uno_Sequence* retSeq = NULL;
    switch (pTdElem->eTypeClass)
    {
    case TypeClass_SEQUENCE:
    {
        Type _tElem(pTdElem->pWeakRef);
        for (int i = 0; i < val->nElements; i++)
        {
            uno_Sequence* seq = cloneSequence(
                *(uno_Sequence**) (&val->elements + i * pTdElem->nSize),
                _tElem);
            *((uno_Sequence**) pBufCur) = seq;
            pBufCur += pTdElem->nSize;
        }
        break;
    }
    default:
        uno_type_sequence_construct( & retSeq, type.getTypeLibType(),
                                     (void*) val->elements, val->nElements, cpp_acquire);
        break;
    }
    delete[] buf;
    return retSeq;
}

template< class T>
Sequence<T> cloneSequence(const Sequence<T>& val)
{
    Sequence<T> seq( cloneSequence(val.get(), getCppuType(&val)), SAL_NO_ACQUIRE);
    return seq;
}

template< class T >
static inline bool makeSurrogate(
    Reference< T > & rOut, Reference< T > const & rOriginal )
{
    rOut.clear();
    if (! rOriginal.is())
        return false;

    Environment aCppEnv_official;
    Environment aUnoEnv_ano;
    Environment aCppEnv_ano;

    OUString aCppEnvTypeName(
        RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    OUString aUnoEnvTypeName(
        RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );
    // official:
    uno_getEnvironment(
        reinterpret_cast< uno_Environment ** >( &aCppEnv_official ),
        aCppEnvTypeName.pData, 0 );
    // anonymous:
    uno_createEnvironment(
        reinterpret_cast< uno_Environment ** >( &aCppEnv_ano ),
        aCppEnvTypeName.pData, 0 );
    uno_createEnvironment(
        reinterpret_cast< uno_Environment ** >( &aUnoEnv_ano ),
        aUnoEnvTypeName.pData, 0 );

    UnoInterfaceReference unoI;
    Mapping cpp2uno( aCppEnv_official.get(), aUnoEnv_ano.get() );
    Mapping uno2cpp( aUnoEnv_ano.get(), aCppEnv_ano.get() );
    if (!cpp2uno.is() || !uno2cpp.is())
    {
        throw RuntimeException(
            OUSTR("cannot get C++-UNO mappings!"),
            Reference< XInterface >() );
    }
    cpp2uno.mapInterface(
        reinterpret_cast< void ** >( &unoI.m_pUnoI ),
        rOriginal.get(), ::getCppuType( &rOriginal ) );
    if (! unoI.is())
    {
        throw RuntimeException(
            OUSTR("mapping C++ to binary UNO failed!"),
            Reference< XInterface >() );
    }
    uno2cpp.mapInterface(
        reinterpret_cast< void ** >( &rOut ),
        unoI.get(), ::getCppuType( &rOriginal ) );
    if (! rOut.is())
    {
        throw RuntimeException(
            OUSTR("mapping binary UNO to C++ failed!"),
            Reference< XInterface >() );
    }

    return rOut.is();
}

//==================================================================================================
sal_Int32 TestBridgeImpl::run( const Sequence< OUString > & rArgs )
    throw (RuntimeException)
{
    bool bRet = false;
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
            Reference <XInterface > r =
                m_xContext->getServiceManager()->createInstanceWithContext(
                    OUString::createFromAscii(
                        "com.sun.star.bridge.UnoUrlResolver" ), m_xContext );
            Reference <XUnoUrlResolver> rResolver( r , UNO_QUERY );
            xOriginal = rResolver->resolve( rArgs[1] );
        }
        else
        {
            // local test
            xOriginal =
                m_xContext->getServiceManager()->createInstanceWithContext(
                    rArgs[0], m_xContext );
        }

        if (! xOriginal.is())
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                              "cannot get test object!") ),
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
        bRet = check( makeSurrogate( xLBT, xTest ), "makeSurrogate" );
        bRet = check( performTest( xLBT ), "standard test" ) && bRet;
        bRet = check( raiseException( xLBT ) , "exception test" )&& bRet;
        bRet = check( raiseOnewayException( xLBT ),
                      "oneway exception test" ) && bRet;
        bRet = performQueryForUnknownType( xLBT ) && bRet;
        if (! bRet)
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("error: test failed!") ),
                Reference< XInterface >() );
        }
    }
    catch (Exception & exc)
    {
        OString cstr( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "exception occured: %s\n", cstr.getStr() );
        throw;
    }

    if (MyClass::s_instances != 0)
    {
        OSL_ENSURE( 0, "leaking object instance!" );
        printf( "\n\n ### leaking object instance!\n" );
    }

    if( bRet )
    {
        printf( "\n\n ### test succeeded!\n" );
    }
    else
    {
        printf( "\n> ### test failed!\n" );
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
    const Reference< XComponentContext > & xContext )
{
    return Reference< XInterface >(
        (XBridgeTest *) new TestBridgeImpl( xContext ) );
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
        Reference< XInterface > xFactory(
            createSingleComponentFactory(
                bridge_test::TestBridgeImpl_create,
                OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
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
