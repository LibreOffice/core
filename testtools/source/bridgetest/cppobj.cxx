/**************************************************************************
#*
#*    last change   $Author: obo $ $Date: 2003-09-04 09:16:29 $
#*    $Revision: 1.3 $
#*
#*    $Logfile: $
#*
#*    Copyright (c) 1989 - 2000, Star Office GmbH
#*
#************************************************************************/
#include <stdio.h>

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/mutex.hxx>
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <com/sun/star/test/bridge/XBridgeTest2.hpp>

using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::test::bridge;

#define SERVICENAME     "com.sun.star.test.bridge.CppTestObject"
#define IMPLNAME        "com.sun.star.comp.bridge.CppTestObject"

namespace bridge_object
{

//--------------------------------------------------------------------------------------------------
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
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

//==================================================================================================
class Test_Impl : public WeakImplHelper3< XBridgeTest2, XServiceInfo , XRecursiveCall >
{
    TestData _aData, _aStructData;
    sal_Int32 m_nLastCallId;
    sal_Bool m_bFirstCall;
    sal_Bool m_bSequenceOfCallTestPassed;
    Mutex m_mutex;

    Sequence<sal_Bool> _arBool;
    Sequence<sal_Unicode> _arChar;
    Sequence<sal_Int8> _arByte;
    Sequence<sal_Int16> _arShort;
    Sequence<sal_uInt16> _arUShort;
    Sequence<sal_Int32> _arLong;
    Sequence<sal_uInt32> _arULong;
    Sequence<sal_Int64> _arHyper;
    Sequence<sal_uInt64> _arUHyper;
    Sequence<OUString> _arString;
    Sequence<float> _arFloat;
    Sequence<double> _arDouble;
    Sequence<TestEnum> _arEnum;
    Sequence<Reference<XInterface> > _arObject;
    Sequence<Sequence<sal_Int32> > _arLong2;
    Sequence<Sequence<Sequence<sal_Int32> > > _arLong3;
    Sequence<Any> _arAny;
    Sequence<TestElement> _arStruct;

public:
    Test_Impl() : m_nLastCallId( 0 ),
                  m_bFirstCall( sal_True ),
                  m_bSequenceOfCallTestPassed( sal_True )
        {}
    virtual ~Test_Impl()
        {
            OSL_TRACE( "> scalar Test_Impl dtor <\n" );
        }

    void SAL_CALL acquire() throw ()
    {
        OWeakObject::acquire();
    }
    void SAL_CALL release() throw ()
    {
        OWeakObject::release();
     }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

    // XLBTestBase
    virtual void SAL_CALL setValues( sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                                     sal_Int16 nShort, sal_uInt16 nUShort,
                                     sal_Int32 nLong, sal_uInt32 nULong,
                                     sal_Int64 nHyper, sal_uInt64 nUHyper,
                                     float fFloat, double fDouble,
                                     TestEnum eEnum, const ::rtl::OUString& rStr,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                     const ::com::sun::star::uno::Any& rAny,
                                     const ::com::sun::star::uno::Sequence<TestElement >& rSequence,
                                     const TestData& rStruct )
        throw(com::sun::star::uno::RuntimeException);

    virtual TestData SAL_CALL setValues2( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                                sal_Int16& nShort, sal_uInt16& nUShort,
                                                sal_Int32& nLong, sal_uInt32& nULong,
                                                sal_Int64& nHyper, sal_uInt64& nUHyper,
                                                float& fFloat, double& fDouble,
                                                TestEnum& eEnum, rtl::OUString& rStr,
                                                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                                ::com::sun::star::uno::Any& rAny,
                                                ::com::sun::star::uno::Sequence<TestElement >& rSequence,
                                                TestData& rStruct )
        throw(com::sun::star::uno::RuntimeException);

    virtual TestData SAL_CALL getValues( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                               sal_Int16& nShort, sal_uInt16& nUShort,
                                               sal_Int32& nLong, sal_uInt32& nULong,
                                               sal_Int64& nHyper, sal_uInt64& nUHyper,
                                               float& fFloat, double& fDouble,
                                               TestEnum& eEnum, rtl::OUString& rStr,
                                               ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                               ::com::sun::star::uno::Any& rAny,
                                               ::com::sun::star::uno::Sequence< TestElement >& rSequence,
                                               TestData& rStruct )
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getBool() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Bool; }
    virtual sal_Int8 SAL_CALL getByte() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Byte; }
    virtual sal_Unicode SAL_CALL getChar() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Char; }
    virtual sal_Int16 SAL_CALL getShort() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Short; }
    virtual sal_uInt16 SAL_CALL getUShort() throw(com::sun::star::uno::RuntimeException)
        { return _aData.UShort; }
    virtual sal_Int32 SAL_CALL getLong() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Long; }
    virtual sal_uInt32 SAL_CALL getULong() throw(com::sun::star::uno::RuntimeException)
        { return _aData.ULong; }
    virtual sal_Int64 SAL_CALL getHyper() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Hyper; }
    virtual sal_uInt64 SAL_CALL getUHyper() throw(com::sun::star::uno::RuntimeException)
        { return _aData.UHyper; }
    virtual float SAL_CALL getFloat() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Float; }
    virtual double SAL_CALL getDouble() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Double; }
    virtual TestEnum SAL_CALL getEnum() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Enum; }
    virtual rtl::OUString SAL_CALL getString() throw(com::sun::star::uno::RuntimeException)
        { return _aData.String; }
    virtual com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getInterface(  ) throw(com::sun::star::uno::RuntimeException)
        { return _aData.Interface; }
    virtual com::sun::star::uno::Any SAL_CALL getAny() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Any; }
    virtual com::sun::star::uno::Sequence< TestElement > SAL_CALL getSequence() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Sequence; }
    virtual TestData SAL_CALL getStruct() throw(com::sun::star::uno::RuntimeException)
        { return _aStructData; }

    virtual void SAL_CALL setBool( sal_Bool _bool ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Bool = _bool; }
    virtual void SAL_CALL setByte( sal_Int8 _byte ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Byte = _byte; }
    virtual void SAL_CALL setChar( sal_Unicode _char ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Char = _char; }
    virtual void SAL_CALL setShort( sal_Int16 _short ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Short = _short; }
    virtual void SAL_CALL setUShort( sal_uInt16 _ushort ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.UShort = _ushort; }
    virtual void SAL_CALL setLong( sal_Int32 _long ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Long = _long; }
    virtual void SAL_CALL setULong( sal_uInt32 _ulong ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.ULong = _ulong; }
    virtual void SAL_CALL setHyper( sal_Int64 _hyper ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Hyper = _hyper; }
    virtual void SAL_CALL setUHyper( sal_uInt64 _uhyper ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.UHyper = _uhyper; }
    virtual void SAL_CALL setFloat( float _float ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Float = _float; }
    virtual void SAL_CALL setDouble( double _double ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Double = _double; }
    virtual void SAL_CALL setEnum( TestEnum _enum ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Enum = _enum; }
    virtual void SAL_CALL setString( const ::rtl::OUString& _string ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.String = _string; }
    virtual void SAL_CALL setInterface( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _interface ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Interface = _interface; }
    virtual void SAL_CALL setAny( const ::com::sun::star::uno::Any& _any ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Any = _any; }
    virtual void SAL_CALL setSequence( const ::com::sun::star::uno::Sequence<TestElement >& _sequence ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Sequence = _sequence; }
    virtual void SAL_CALL setStruct( const TestData& _struct ) throw(::com::sun::star::uno::RuntimeException)
        { _aStructData = _struct; }

    virtual ::com::sun::star::uno::Any SAL_CALL transportAny(
        const ::com::sun::star::uno::Any& value )
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL call( sal_Int32 nCallId, sal_Int32 nWaitMUSEC )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL callOneway( sal_Int32 nCallId, sal_Int32 nWaitMUSEC )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL sequenceOfCallTestPassed(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startRecursiveCall(
        const ::com::sun::star::uno::Reference< ::com::sun::star::test::bridge::XRecursiveCall >& xCall, sal_Int32 nToCall )
        throw(::com::sun::star::uno::RuntimeException);

public: // XBridgeTest
    virtual TestData SAL_CALL raiseException( sal_Int16 nArgumentPos, const OUString & rMsg, const Reference< XInterface > & xCOntext )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL raiseRuntimeExceptionOneway(
        const ::rtl::OUString& Message, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Context )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getRuntimeException() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRuntimeException( sal_Int32 _runtimeexception ) throw(::com::sun::star::uno::RuntimeException);

    // XBridgeTest2
    virtual Sequence< sal_Bool > SAL_CALL setSequenceBool(
        const Sequence< sal_Bool >& aSeq ) throw (RuntimeException);
    virtual Sequence< sal_Unicode > SAL_CALL setSequenceChar(
        const Sequence< sal_Unicode >& aSeq ) throw (RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL setSequenceByte(
        const Sequence< sal_Int8 >& aSeq ) throw (RuntimeException);
    virtual Sequence< sal_Int16 > SAL_CALL setSequenceShort(
        const Sequence< sal_Int16 >& aSeq ) throw (RuntimeException);
    virtual Sequence< sal_uInt16 > SAL_CALL setSequenceUShort(
        const Sequence< sal_uInt16 >& aSeq ) throw (RuntimeException);
    virtual Sequence< sal_Int32 > SAL_CALL setSequenceLong(
        const Sequence< sal_Int32 >& aSeq ) throw (RuntimeException);
    virtual Sequence< sal_uInt32 > SAL_CALL setSequenceULong(
        const Sequence< sal_uInt32 >& aSeq ) throw (RuntimeException);
    virtual Sequence< sal_Int64 > SAL_CALL setSequenceHyper(
        const Sequence< sal_Int64 >& aSeq ) throw (RuntimeException);
    virtual Sequence< sal_uInt64 > SAL_CALL setSequenceUHyper(
        const Sequence< sal_uInt64 >& aSeq ) throw (RuntimeException);
    virtual Sequence< float > SAL_CALL setSequenceFloat(
        const Sequence< float >& aSeq ) throw (RuntimeException);
    virtual Sequence< double > SAL_CALL setSequenceDouble(
        const Sequence< double >& aSeq ) throw (RuntimeException);
    virtual Sequence< TestEnum > SAL_CALL setSequenceEnum(
        const Sequence< TestEnum >& aSeq ) throw (RuntimeException) ;
    virtual Sequence< OUString > SAL_CALL setSequenceString(
        const Sequence< OUString >& aString ) throw (RuntimeException);
    virtual Sequence< Reference< XInterface > > SAL_CALL setSequenceXInterface(
        const Sequence< Reference< XInterface > >& aSeq )
        throw (RuntimeException);
    virtual Sequence<Any > SAL_CALL setSequenceAny(
        const Sequence<Any >& aSeq ) throw (RuntimeException);
    virtual Sequence<TestElement > SAL_CALL setSequenceStruct(
        const Sequence< TestElement >& aSeq ) throw (RuntimeException);
    virtual Sequence< Sequence< sal_Int32 > > SAL_CALL setDim2(
        const Sequence<Sequence< sal_Int32 > >& aSeq ) throw (RuntimeException);
    virtual Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL setDim3(
        const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq )
        throw (RuntimeException);
    virtual void SAL_CALL setSequencesInOut(Sequence< sal_Bool >& aSeqBoolean,
                                Sequence< sal_Unicode >& aSeqChar,
                                Sequence< sal_Int8 >& aSeqByte,
                                Sequence< sal_Int16 >& aSeqShort,
                                Sequence< sal_uInt16 >& aSeqUShort,
                                Sequence< sal_Int32 >& aSeqLong,
                                Sequence< sal_uInt32 >& aSeqULong,
                                Sequence< sal_Int64 >& aSeqHyper,
                                Sequence< sal_uInt64 >& aSeqUHyper,
                                Sequence< float >& aSeqFloat,
                                Sequence< double >& aSeqDouble,
                                Sequence< TestEnum >& aSeqTestEnum,
                                Sequence< OUString >& aSeqString,
                                Sequence<Reference<XInterface > >& aSeqXInterface,
                                Sequence< Any >& aSeqAny,
                                Sequence< Sequence< sal_Int32 > >& aSeqDim2,
                                Sequence< Sequence< Sequence< sal_Int32 > > >& aSeqDim3 )
        throw (RuntimeException);
    virtual void SAL_CALL setSequencesOut( Sequence< sal_Bool >& aSeqBoolean,
                             Sequence< sal_Unicode >& aSeqChar,
                             Sequence< sal_Int8 >& aSeqByte,
                             Sequence< sal_Int16 >& aSeqShort,
                             Sequence< sal_uInt16 >& aSeqUShort,
                             Sequence< sal_Int32 >& aSeqLong,
                             Sequence< sal_uInt32 >& aSeqULong,
                             Sequence< sal_Int64 >& aSeqHyper,
                             Sequence< sal_uInt64 >& aSeqUHyper,
                             Sequence< float >& aSeqFloat,
                             Sequence< double >& aSeqDouble,
                             Sequence< TestEnum >& aSeqEnum,
                             Sequence< OUString >& aSeqString,
                             Sequence< Reference< XInterface > >& aSeqXInterface,
                             Sequence< Any >& aSeqAny,
                             Sequence< Sequence< sal_Int32 > >& aSeqDim2,
                             Sequence< Sequence< Sequence< sal_Int32 > > >& aSeqDim3 )
        throw (RuntimeException);

public:
    virtual void SAL_CALL callRecursivly( const ::com::sun::star::uno::Reference< ::com::sun::star::test::bridge::XRecursiveCall >& xCall, sal_Int32 nToCall ) throw(::com::sun::star::uno::RuntimeException);
};

//__________________________________________________________________________________________________
Any Test_Impl::transportAny( const Any & value ) throw ( ::com::sun::star::uno::RuntimeException)
{
    return value;
}

//__________________________________________________________________________________________________
void Test_Impl::call( sal_Int32 nCallId , sal_Int32 nWaitMUSEC ) throw(::com::sun::star::uno::RuntimeException)
{
    TimeValue value = { nWaitMUSEC / 1000000 , nWaitMUSEC * 1000 };
    osl_waitThread( &value );
    if( m_bFirstCall )
    {
        m_bFirstCall = sal_False;
    }
    else
    {
        m_bSequenceOfCallTestPassed = m_bSequenceOfCallTestPassed && (nCallId > m_nLastCallId);
    }
    m_nLastCallId = nCallId;
}

//__________________________________________________________________________________________________
void Test_Impl::callOneway( sal_Int32 nCallId , sal_Int32 nWaitMUSEC ) throw (::com::sun::star::uno::RuntimeException)
{
    TimeValue value = { nWaitMUSEC / 1000000 , nWaitMUSEC * 1000 };
    osl_waitThread( &value );
    m_bSequenceOfCallTestPassed = m_bSequenceOfCallTestPassed && (nCallId > m_nLastCallId);
    m_nLastCallId = nCallId;
}

//__________________________________________________________________________________________________
sal_Bool Test_Impl::sequenceOfCallTestPassed() throw (::com::sun::star::uno::RuntimeException)
{
    return m_bSequenceOfCallTestPassed;
}

//__________________________________________________________________________________________________
void SAL_CALL Test_Impl::startRecursiveCall(
    const ::com::sun::star::uno::Reference< ::com::sun::star::test::bridge::XRecursiveCall >& xCall, sal_Int32 nToCall )
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard guard( m_mutex );
    if( nToCall )
    {
        nToCall --;
        xCall->callRecursivly( this , nToCall );
    }
}


void SAL_CALL Test_Impl::callRecursivly(
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

//__________________________________________________________________________________________________
void Test_Impl::setValues( sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                           sal_Int16 nShort, sal_uInt16 nUShort,
                           sal_Int32 nLong, sal_uInt32 nULong,
                           sal_Int64 nHyper, sal_uInt64 nUHyper,
                           float fFloat, double fDouble,
                           TestEnum eEnum, const ::rtl::OUString& rStr,
                           const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                           const ::com::sun::star::uno::Any& rAny,
                           const ::com::sun::star::uno::Sequence<TestElement >& rSequence,
                           const TestData& rStruct )
    throw(com::sun::star::uno::RuntimeException)
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny, rSequence );
    _aStructData = rStruct;
}
//__________________________________________________________________________________________________
TestData Test_Impl::setValues2( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                sal_Int16& nShort, sal_uInt16& nUShort,
                                sal_Int32& nLong, sal_uInt32& nULong,
                                sal_Int64& nHyper, sal_uInt64& nUHyper,
                                float& fFloat, double& fDouble,
                                TestEnum& eEnum, rtl::OUString& rStr,
                                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                ::com::sun::star::uno::Any& rAny,
                                ::com::sun::star::uno::Sequence<TestElement >& rSequence,
                                TestData& rStruct )
    throw(com::sun::star::uno::RuntimeException)
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny, rSequence );
    _aStructData = rStruct;

    TestElement elem = rSequence[ 0 ];
    rSequence[ 0 ] = rSequence[ 1 ];
    rSequence[ 1 ] = elem;

    return _aStructData;
}
//__________________________________________________________________________________________________
TestData Test_Impl::getValues( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                               sal_Int16& nShort, sal_uInt16& nUShort,
                               sal_Int32& nLong, sal_uInt32& nULong,
                               sal_Int64& nHyper, sal_uInt64& nUHyper,
                               float& fFloat, double& fDouble,
                               TestEnum& eEnum, rtl::OUString& rStr,
                               ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                               ::com::sun::star::uno::Any& rAny,
                               ::com::sun::star::uno::Sequence<TestElement >& rSequence,
                               TestData& rStruct )
    throw(com::sun::star::uno::RuntimeException)
{
    bBool = _aData.Bool;
    cChar = _aData.Char;
    nByte = _aData.Byte;
    nShort = _aData.Short;
    nUShort = _aData.UShort;
    nLong = _aData.Long;
    nULong = _aData.ULong;
    nHyper = _aData.Hyper;
    nUHyper = _aData.UHyper;
    fFloat = _aData.Float;
    fDouble = _aData.Double;
    eEnum = _aData.Enum;
    rStr = _aData.String;
    xTest = _aData.Interface;
    rAny = _aData.Any;
    rSequence = _aData.Sequence;
    rStruct = _aStructData;
    return _aStructData;
}
//__________________________________________________________________________________________________
TestData Test_Impl::raiseException( sal_Int16 nArgumentPos, const OUString & rMsg, const Reference< XInterface > & xContext )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    IllegalArgumentException aExc;
    aExc.ArgumentPosition = nArgumentPos;
    aExc.Message          = _aData.String = rMsg;
    aExc.Context          = _aData.Interface = xContext;
    throw aExc;
    return TestData(); // for dummy
}

void Test_Impl::raiseRuntimeExceptionOneway( const OUString & rMsg, const Reference< XInterface > & xContext )
    throw(::com::sun::star::uno::RuntimeException)
{
    RuntimeException aExc;
    aExc.Message          = _aData.String = rMsg;
    aExc.Context          = _aData.Interface = xContext;
    throw aExc;
}

//__________________________________________________________________________________________________
sal_Int32 Test_Impl::getRuntimeException() throw(::com::sun::star::uno::RuntimeException)
{
    RuntimeException aExc;
    aExc.Message          = _aData.String;
    aExc.Context          = _aData.Interface;
    throw aExc;
    return 0; // for dummy
}
//__________________________________________________________________________________________________
void Test_Impl::setRuntimeException( sal_Int32 _runtimeexception ) throw(::com::sun::star::uno::RuntimeException)
{
    RuntimeException aExc;
    aExc.Message          = _aData.String;
    aExc.Context          = _aData.Interface;
    throw aExc;
}

// XBridgeTest2 -------------------------------------------------------------
Sequence< sal_Bool > SAL_CALL Test_Impl::setSequenceBool(
        const Sequence< sal_Bool >& aSeq ) throw (RuntimeException)
{
    _arBool = aSeq;
    return aSeq;
}

Sequence< sal_Unicode > SAL_CALL Test_Impl::setSequenceChar(
        const Sequence< sal_Unicode >& aSeq ) throw (RuntimeException)
{
    _arChar = aSeq;
    return aSeq;
}

Sequence< sal_Int8 > SAL_CALL Test_Impl::setSequenceByte(
        const Sequence< sal_Int8 >& aSeq ) throw (RuntimeException)
{
    _arByte = aSeq;
    return aSeq;
}

Sequence< sal_Int16 > SAL_CALL Test_Impl::setSequenceShort(
        const Sequence< sal_Int16 >& aSeq ) throw (RuntimeException)
{
    _arShort = aSeq;
    return aSeq;
}

Sequence< sal_uInt16 > SAL_CALL Test_Impl::setSequenceUShort(
        const Sequence< sal_uInt16 >& aSeq ) throw (RuntimeException)
{
    _arUShort = aSeq;
    return aSeq;
}

Sequence< sal_Int32 > SAL_CALL Test_Impl::setSequenceLong(
        const Sequence< sal_Int32 >& aSeq ) throw (RuntimeException)
{
    _arLong = aSeq;
    return aSeq;
}

Sequence< sal_uInt32 > SAL_CALL Test_Impl::setSequenceULong(
        const Sequence< sal_uInt32 >& aSeq ) throw (RuntimeException)
{
    _arULong = aSeq;
    return aSeq;
}

Sequence< sal_Int64 > SAL_CALL Test_Impl::setSequenceHyper(
        const Sequence< sal_Int64 >& aSeq ) throw (RuntimeException)
{
    _arHyper = aSeq;
    return aSeq;
}

Sequence< sal_uInt64 > SAL_CALL Test_Impl::setSequenceUHyper(
        const Sequence< sal_uInt64 >& aSeq ) throw (RuntimeException)
{
    _arUHyper = aSeq;
    return aSeq;
}

Sequence< float > SAL_CALL Test_Impl::setSequenceFloat(
        const Sequence< float >& aSeq ) throw (RuntimeException)
{
    _arFloat = aSeq;
    return aSeq;
}

Sequence< double > SAL_CALL Test_Impl::setSequenceDouble(
    const Sequence< double >& aSeq ) throw (RuntimeException)
{
    _arDouble = aSeq;
    return aSeq;
}

Sequence< TestEnum > SAL_CALL Test_Impl::setSequenceEnum(
    const Sequence< TestEnum >& aSeq ) throw (RuntimeException)
{
    _arEnum = aSeq;
    return aSeq;
}

Sequence< OUString > SAL_CALL Test_Impl::setSequenceString(
    const Sequence< OUString >& aSeq ) throw (RuntimeException)
{
    _arString = aSeq;
    return aSeq;
}

Sequence< Reference< XInterface > > SAL_CALL Test_Impl::setSequenceXInterface(
        const Sequence< Reference< XInterface > >& aSeq )
        throw (RuntimeException)
{
    _arObject = aSeq;
    return aSeq;
}

Sequence<Any > SAL_CALL Test_Impl::setSequenceAny(
    const Sequence<Any >& aSeq ) throw (RuntimeException)
{
    _arAny = aSeq;
    return aSeq;
}

Sequence<TestElement > SAL_CALL Test_Impl::setSequenceStruct(
    const Sequence< TestElement >& aSeq ) throw (RuntimeException)
{
    _arStruct = aSeq;
    return aSeq;
}

Sequence< Sequence< sal_Int32 > > SAL_CALL Test_Impl::setDim2(
        const Sequence<Sequence< sal_Int32 > >& aSeq ) throw (RuntimeException)
{
    _arLong2 = aSeq;
    return aSeq;
}

Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL Test_Impl::setDim3(
        const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq )
        throw (RuntimeException)
{
    _arLong3 = aSeq;
    return aSeq;
}

void SAL_CALL Test_Impl::setSequencesInOut(Sequence< sal_Bool >& aSeqBoolean,
                                Sequence< sal_Unicode >& aSeqChar,
                                Sequence< sal_Int8 >& aSeqByte,
                                Sequence< sal_Int16 >& aSeqShort,
                                Sequence< sal_uInt16 >& aSeqUShort,
                                Sequence< sal_Int32 >& aSeqLong,
                                Sequence< sal_uInt32 >& aSeqULong,
                                Sequence< sal_Int64 >& aSeqHyper,
                                Sequence< sal_uInt64 >& aSeqUHyper,
                                Sequence< float >& aSeqFloat,
                                Sequence< double >& aSeqDouble,
                                Sequence< TestEnum >& aSeqTestEnum,
                                Sequence< OUString >& aSeqString,
                                Sequence<Reference<XInterface > >& aSeqXInterface,
                                Sequence< Any >& aSeqAny,
                                Sequence< Sequence< sal_Int32 > >& aSeqDim2,
                                Sequence< Sequence< Sequence< sal_Int32 > > >& aSeqDim3 )
        throw (RuntimeException)
{
    _arBool = aSeqBoolean;
    _arChar = aSeqChar;
    _arByte = aSeqByte;
    _arShort = aSeqShort;
    _arUShort = aSeqUShort;
    _arLong = aSeqLong;
    _arULong = aSeqULong;
    _arHyper  = aSeqHyper;
    _arUHyper = aSeqUHyper;
    _arFloat = aSeqFloat;
    _arDouble = aSeqDouble;
    _arEnum = aSeqTestEnum;
    _arString = aSeqString;
    _arObject = aSeqXInterface;
    _arAny = aSeqAny;
    _arLong2 = aSeqDim2;
    _arLong3 = aSeqDim3;
}

void SAL_CALL Test_Impl::setSequencesOut( Sequence< sal_Bool >& aSeqBoolean,
                             Sequence< sal_Unicode >& aSeqChar,
                             Sequence< sal_Int8 >& aSeqByte,
                             Sequence< sal_Int16 >& aSeqShort,
                             Sequence< sal_uInt16 >& aSeqUShort,
                             Sequence< sal_Int32 >& aSeqLong,
                             Sequence< sal_uInt32 >& aSeqULong,
                             Sequence< sal_Int64 >& aSeqHyper,
                             Sequence< sal_uInt64 >& aSeqUHyper,
                             Sequence< float >& aSeqFloat,
                             Sequence< double >& aSeqDouble,
                             Sequence< TestEnum >& aSeqEnum,
                             Sequence< OUString >& aSeqString,
                             Sequence< Reference< XInterface > >& aSeqXInterface,
                             Sequence< Any >& aSeqAny,
                             Sequence< Sequence< sal_Int32 > >& aSeqDim2,
                             Sequence< Sequence< Sequence< sal_Int32 > > >& aSeqDim3 )
        throw (RuntimeException)
{
    aSeqBoolean = _arBool;
    aSeqChar = _arChar;
    aSeqByte = _arByte;
    aSeqShort = _arShort;
    aSeqUShort = _arUShort;
    aSeqLong = _arLong;
    aSeqULong = _arULong;
    aSeqHyper = _arHyper;
    aSeqUHyper = _arUHyper;
    aSeqFloat = _arFloat;
    aSeqDouble = _arDouble;
    aSeqEnum = _arEnum;
    aSeqString = _arString;
    aSeqXInterface = _arObject;
    aSeqAny = _arAny;
    aSeqDim2 = _arLong2;
    aSeqDim3 = _arLong3;
}





// XServiceInfo
//__________________________________________________________________________________________________
OUString Test_Impl::getImplementationName()
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}
//__________________________________________________________________________________________________
sal_Bool Test_Impl::supportsService( const OUString & rServiceName )
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
Sequence< OUString > Test_Impl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return bridge_object::getSupportedServiceNames();
}

// ...

//==================================================================================================
static Reference< XInterface > SAL_CALL Test_Impl_create( const Reference< XMultiServiceFactory > & xSMgr )
{
    return Reference< XInterface >( (XBridgeTest *)new Test_Impl() );
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
            bridge_object::Test_Impl_create,
            bridge_object::getSupportedServiceNames() ) );

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
    Revision 1.2.24.3  2003/07/04 06:44:55  jl
    *** empty log message ***

    Revision 1.2.24.2  2003/06/05 13:31:27  dbo
    #107130# fixing tests

    Revision 1.2.24.1  2003/04/11 17:15:20  dbo
    #107130# CLI tests

    Revision 1.2  2002/11/27 10:06:09  dbo
    #104312# extended tests

    Revision 1.1  2001/05/04 07:05:17  kr
    moved from grande to openoffice

    Revision 1.5  2001/03/14 09:55:11  jl
    #include <osl/time.h> added

    Revision 1.4  2001/03/12 16:22:44  jl
    OSL_ENSHURE replaced by OSL_ENSURE

    Revision 1.3  2000/08/30 13:13:59  jbu
    now passes cc50 compiler

    Revision 1.2  2000/08/14 07:12:50  jbu
    added remote tests

    Revision 1.1  2000/05/26 14:20:35  dbo
    new


**************************************************************************/
