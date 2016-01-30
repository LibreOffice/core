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

#include <windows.h>
#include <atlbase.h>
#include <stdio.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <osl/diagnose.h>
#include <uno/environment.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
// OPTIONAL is a constant in com.sun.star.beans.PropertyAttributes but it must be
// undef'd in some header files
#define OPTIONAL OPTIONAL
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.h>
#include <com/sun/star/uno/Reference.hxx>
#include <oletest/XTestSequence.hpp>
#include <oletest/XTestStruct.hpp>
#include <oletest/XTestOther.hpp>
#include <oletest/XTestInterfaces.hpp>
#include <oletest/XSimple.hpp>
#include <oletest/XSimple2.hpp>
#include <oletest/XSimple3.hpp>
#include <oletest/XTestInParameters.hpp>
#include <oletest/XIdentity.hpp>
#include <com/sun/star/beans/Property.hpp>
using namespace cppu;
using namespace osl;
using namespace oletest;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;


#define IMPL_NAME L"oletest.OleTestImpl"  // oletest.OleTestImpl in applicat.rdb
#define SERVICE_NAME L"oletest.OleTest"
#define KEY1 L"/oletest.OleTestImpl/UNO/SERVICES"
#define KEY2 L"oletest.OleTest"

class OComponent : public WeakImplHelper<
         XTestSequence, XTestStruct, XTestOther, XTestInterfaces,
                   XSimple, XTestInParameters, XIdentity >
{
    Reference<XInterface> m_xIntIdentity;
    sal_Int32 m_arrayConstructor;
    Reference<XMultiServiceFactory> m_rFactory;

    Sequence<sal_Int8> m_seqByte;
    Sequence<float> m_seqFloat;
    Sequence<double> m_seqDouble;
    Sequence<sal_Bool> m_seqBool;
    Sequence<sal_Int16> m_seqShort;
    Sequence<sal_uInt16> m_seqUShort;
    Sequence<sal_Int32> m_seqLong;
    Sequence<sal_uInt32> m_seqULong;
    Sequence<sal_Unicode> m_seqChar;
    Sequence<OUString> m_seqString;
    Sequence<Any> m_seqAny;
    Sequence<Type> m_seqType;
    Sequence<Sequence< sal_Int32> > m_seq1;
    Sequence<Sequence< Sequence< sal_Int32> > > m_seq2;
    Any m_any;
    Type m_type;
    Sequence<Reference< XInterface > > m_seqxInterface;

    sal_Int8 m_int8;
    sal_uInt8 m_uint8;
    sal_Int16 m_int16;
    sal_uInt16 m_uint16;
    sal_Int32 m_int32;
    sal_uInt32 m_uint32;
    sal_Int64 m_int64;
    sal_uInt64 m_uint64;
    float m_float;
    double m_double;
    OUString m_string;
    sal_Unicode m_char;
    sal_Bool m_bool;
    Reference<XInterface> m_xinterface;

    sal_Int8 m_attr_int8;
    sal_uInt8 m_attr_uint8;
    sal_Int16 m_attr_int16;
    sal_uInt16 m_attr_uint16;
    sal_Int32 m_attr_int32;
    sal_uInt32 m_attr_uint32;
    sal_Int64 m_attr_int64;
    sal_uInt64 m_attr_uint64;
    float m_attr_float;
    double m_attr_double;
    OUString m_attr_string;
    sal_Unicode m_attr_char;
    sal_Bool m_attr_bool;
    Any m_attr_any;
    Type m_attr_type;
    Reference<XInterface> m_attr_xinterface;
    Reference<XInvocation> m_attr_xinvocation;

public:
    OComponent( const Reference<XMultiServiceFactory> & rFactory ) :
      m_rFactory( rFactory ), m_arrayConstructor(0) {}
    ~OComponent();
public: // XTestSequence
    virtual Sequence<sal_Int8> SAL_CALL methodByte(const Sequence< sal_Int8 >& aSeq) throw( RuntimeException );
    virtual Sequence<float> SAL_CALL methodFloat(const Sequence< float >& aSeq) throw( RuntimeException );
    virtual Sequence< double > SAL_CALL methodDouble(const Sequence< double >& aSeq) throw( RuntimeException);
    virtual Sequence< sal_Bool > SAL_CALL methodBool(const Sequence< sal_Bool >& aSeq) throw( RuntimeException );
    virtual Sequence< sal_Int16 > SAL_CALL methodShort(const Sequence< sal_Int16 >& aSeq) throw( RuntimeException );
    virtual Sequence< sal_uInt16 > SAL_CALL methodUShort(const Sequence< sal_uInt16 >& aSeq) throw( RuntimeException );
    virtual Sequence< sal_Int32 > SAL_CALL methodLong(const Sequence< sal_Int32 >& aSeq) throw( RuntimeException) ;
    virtual Sequence< sal_uInt32 > SAL_CALL methodULong(const Sequence< sal_uInt32 >& aSeq) throw( RuntimeException );
    virtual Sequence< OUString > SAL_CALL methodString(const Sequence< OUString >& aSeq) throw( RuntimeException );
    virtual Sequence< sal_Unicode > SAL_CALL methodChar(const Sequence< sal_Unicode >& aSeq) throw( RuntimeException );
    virtual Sequence< Any > SAL_CALL methodAny(const Sequence< Any >& aSeq) throw( RuntimeException );
    virtual Sequence< Type > SAL_CALL methodType(const Sequence< Type >& aSeq) throw( RuntimeException );
    virtual Sequence< Reference< XInterface > > SAL_CALL methodXInterface( const Sequence< Reference< XInterface > >& aSeq ) throw(RuntimeException) ;
    virtual Sequence< Sequence< sal_Int32 > > SAL_CALL methodSequence(const Sequence< Sequence< sal_Int32 > >& aSeq) throw( RuntimeException );
    virtual Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL methodSequence2(const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq) throw( RuntimeException );
    virtual Sequence< Reference<XEventListener> > SAL_CALL methodXEventListeners( const Sequence<Reference<XEventListener> >& aSeq) throw( RuntimeException);
    virtual Sequence< Sequence<Reference<XEventListener > > > SAL_CALL methodXEventListenersMul( const Sequence<Sequence<Reference<XEventListener > > >& aSeq ) throw (RuntimeException);

    virtual Sequence< sal_Int8 > SAL_CALL getAttrByte() throw( RuntimeException );
    virtual void SAL_CALL setAttrByte(const Sequence< sal_Int8 >& AttrByte_) throw( RuntimeException );
    virtual Sequence< float > SAL_CALL getAttrFloat()  throw( RuntimeException) ;
    virtual void SAL_CALL setAttrFloat(const Sequence< float >& AttrFloat_) throw( RuntimeException );
    virtual Sequence< double > SAL_CALL getAttrDouble()  throw( RuntimeException) ;
    virtual void SAL_CALL setAttrDouble(const Sequence< double >& AttrDouble_) throw( RuntimeException );
    virtual Sequence< sal_Bool > SAL_CALL getAttrBool()  throw( RuntimeException );
    virtual void SAL_CALL setAttrBool(const Sequence< sal_Bool >& AttrBool_) throw( RuntimeException );
    virtual Sequence< sal_Int16 > SAL_CALL getAttrShort()  throw( RuntimeException );
    virtual void SAL_CALL setAttrShort(const Sequence< sal_Int16 >& AttrShort_) throw( RuntimeException );
    virtual Sequence< sal_uInt16 > SAL_CALL getAttrUShort()  throw( RuntimeException );
    virtual void SAL_CALL setAttrUShort(const Sequence< sal_uInt16 >& AttrUShort_) throw( RuntimeException );
    virtual Sequence< sal_Int32 > SAL_CALL getAttrLong()  throw( RuntimeException );
    virtual void SAL_CALL setAttrLong(const Sequence< sal_Int32 >& AttrLong_) throw( RuntimeException );
    virtual Sequence< sal_uInt32 > SAL_CALL getAttrULong()  throw( RuntimeException );
    virtual void SAL_CALL setAttrULong(const Sequence< sal_uInt32 >& AttrULong_) throw( RuntimeException );
    virtual Sequence< OUString > SAL_CALL getAttrString()  throw(RuntimeException );
    virtual void SAL_CALL setAttrString(const Sequence< OUString >& AttrString_) throw( RuntimeException );
    virtual Sequence< sal_Unicode > SAL_CALL getAttrChar()  throw( RuntimeException );
    virtual void SAL_CALL setAttrChar(const Sequence< sal_Unicode >& AttrChar_) throw( RuntimeException );
    virtual Sequence< Any > SAL_CALL getAttrAny()  throw( RuntimeException );
    virtual void SAL_CALL setAttrAny(const Sequence< Any >& AttrAny_) throw( RuntimeException );
    virtual Sequence< Type > SAL_CALL getAttrType()  throw( RuntimeException );
    virtual void SAL_CALL setAttrType( const Sequence< Type >& _attrtype ) throw (RuntimeException);
    virtual Sequence< Sequence< sal_Int32 > > SAL_CALL getAttrSequence() throw( RuntimeException );
    virtual void SAL_CALL  setAttrSequence(const Sequence< Sequence< sal_Int32 > >& AttrSequence_) throw( RuntimeException );
    virtual Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL getAttrSequence2()  throw( RuntimeException );
    virtual void SAL_CALL setAttrSequence2(const Sequence< Sequence< Sequence< sal_Int32 > > >& AttrSequence2_) throw ( RuntimeException );
    virtual Sequence< Reference< XInterface > > SAL_CALL getAttrXInterface() throw(RuntimeException);
    virtual void SAL_CALL setAttrXInterface( const Sequence< Reference< XInterface > >& _attrxinterface ) throw(RuntimeException);

    virtual ::sal_Int8 SAL_CALL getAByte() throw (RuntimeException);
    virtual void SAL_CALL setAByte( ::sal_Int8 _abyte ) throw (RuntimeException);
    virtual float SAL_CALL getAFloat() throw (RuntimeException);
    virtual void SAL_CALL setAFloat( float _afloat ) throw (RuntimeException);
    virtual double SAL_CALL getADouble() throw (RuntimeException);
    virtual void SAL_CALL setADouble( double _adouble ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL getABool() throw (RuntimeException);
    virtual void SAL_CALL setABool( sal_Bool _abool ) throw (RuntimeException);
    virtual ::sal_Int16 SAL_CALL getAShort() throw (RuntimeException);
    virtual void SAL_CALL setAShort( ::sal_Int16 _ashort ) throw (RuntimeException);
    virtual ::sal_uInt16 SAL_CALL getAUShort() throw (RuntimeException);
    virtual void SAL_CALL setAUShort( ::sal_uInt16 _aushort ) throw (RuntimeException);
    virtual ::sal_Int32 SAL_CALL getALong() throw (RuntimeException);
    virtual void SAL_CALL setALong( ::sal_Int32 _along ) throw (RuntimeException);
    virtual ::sal_uInt32 SAL_CALL getAULong() throw (RuntimeException);
    virtual void SAL_CALL setAULong( ::sal_uInt32 _aulong ) throw (RuntimeException);
    virtual OUString SAL_CALL getAString() throw (RuntimeException);
    virtual void SAL_CALL setAString( const OUString& _astring ) throw (RuntimeException);
    virtual ::sal_Unicode SAL_CALL getAChar() throw (RuntimeException);
    virtual void SAL_CALL setAChar( ::sal_Unicode _achar ) throw (RuntimeException);
    virtual Any SAL_CALL getAAny() throw (RuntimeException);
    virtual void SAL_CALL setAAny( const Any& _aany ) throw (RuntimeException);
    virtual Type SAL_CALL getAType() throw (RuntimeException);
    virtual void SAL_CALL setAType( const Type& _atype ) throw (RuntimeException);
    virtual Reference< XInterface > SAL_CALL getAXInterface() throw (RuntimeException);
    virtual void SAL_CALL setAXInterface( const Reference<XInterface >& _axinterface ) throw (RuntimeException);
    virtual Reference<XInvocation > SAL_CALL getAXInvocation() throw (RuntimeException);
    virtual void SAL_CALL setAXInvocation( const Reference< XInvocation >& _axinvocation ) throw (RuntimeException);

    virtual void SAL_CALL testout_methodByte(sal_Int8& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodFloat(float& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodDouble(double& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodBool(sal_Bool& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodShort(sal_Int16& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodUShort(sal_uInt16& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodLong(sal_Int32& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodULong(sal_uInt32& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodHyper(sal_Int64& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodUHyper(sal_uInt64& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodString(OUString& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodChar(sal_Unicode& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodAny(Any& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodType(Type& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodSequence(Sequence< sal_Int32 >& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodSequence2(Sequence< Sequence< sal_Int32 > >& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodMulParams1(sal_Int32& rout1, sal_Int32& rout2) throw( RuntimeException );
    virtual void SAL_CALL testout_methodMulParams2(sal_Int32& rout1, sal_Int32& rout2, OUString& rout3) throw( RuntimeException );
    virtual void SAL_CALL testout_methodMulParams3(const OUString& sin, OUString& sout) throw( RuntimeException );
    virtual void SAL_CALL testout_methodMulParams4( float in1, float& out1, sal_Int32 in2, sal_Int32& out2, sal_Int32 in3 ) throw(RuntimeException);
    virtual void SAL_CALL testout_methodXInterface( Reference< XInterface >& rOut ) throw(RuntimeException);

    virtual void SAL_CALL testinout_methodByte(sal_Int8& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodFloat(float& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodDouble(double& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodBool(sal_Bool& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodShort(sal_Int16& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodUShort(sal_uInt16& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodLong(sal_Int32& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodULong(sal_uInt32& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodHyper(sal_Int64& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodUHyper(sal_uInt64& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodString(OUString& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodChar(sal_Unicode& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodAny(Any& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodType(Type& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodSequence(Sequence< sal_Int32 >& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodSequence2(Sequence< Sequence< sal_Int32 > >& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodXInterface( Reference< XInvocation >& rOut ) throw(RuntimeException);
    virtual void SAL_CALL testinout_methodXInterface2( Reference< XInterface > & rOut) throw( RuntimeException);
    virtual Any SAL_CALL methodAnyTest1(const Any& rIn) throw( RuntimeException ) ;
    virtual Any SAL_CALL getAttrAny2() throw( RuntimeException ) ;
    virtual void SAL_CALL setAttrAny2(const Any& AttrAny2_) throw( RuntimeException ) ;


    // XTestStruct
    virtual void SAL_CALL methodStruct(const Property& aProp) throw( RuntimeException );

    virtual Property SAL_CALL retMethodStruct() throw( RuntimeException );

    virtual Property SAL_CALL  getAttrStruct() throw( RuntimeException );
    virtual void SAL_CALL setAttrStruct(const Property& AttrStruct_) throw( RuntimeException );
    virtual Property SAL_CALL methodStruct2( const Property& aProp ) throw (RuntimeException);

    // XTestOther
    virtual void SAL_CALL other_methodAnyIn(const Any& rAny) throw( RuntimeException );
    virtual void SAL_CALL other_methodAnyOut(Any& rAny) throw( RuntimeException );
    virtual Any SAL_CALL other_methodAnyRet() throw( RuntimeException );
    virtual void SAL_CALL in_float( float val) throw ( RuntimeException);
    virtual Any SAL_CALL other_methodAny( const Any& rAny, const OUString& typeInAny )
        throw (RuntimeException);

    // XTestInParameters
    virtual sal_Int8 SAL_CALL in_methodByte( sal_Int8 rIn ) throw (RuntimeException);
    virtual float SAL_CALL in_methodFloat( float rIn ) throw (RuntimeException);
    virtual double SAL_CALL in_methodDouble( double rIn ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL in_methodBool( sal_Bool rIn ) throw (RuntimeException);
    virtual sal_Int16 SAL_CALL in_methodShort( sal_Int16 rIn ) throw (RuntimeException);
    virtual sal_uInt16 SAL_CALL in_methodUShort( sal_uInt16 rIn ) throw (RuntimeException);
    virtual sal_Int32 SAL_CALL in_methodLong( sal_Int32 rIn ) throw (RuntimeException);
    virtual sal_uInt32 SAL_CALL in_methodULong( sal_uInt32 rIn ) throw (RuntimeException);
    virtual sal_Int64 SAL_CALL in_methodHyper( sal_Int64 rIn ) throw (RuntimeException);
    virtual sal_uInt64 SAL_CALL in_methodUHyper( sal_uInt64 rIn ) throw (RuntimeException);
    virtual OUString SAL_CALL in_methodString( const OUString& rIn ) throw (RuntimeException);
    virtual sal_Unicode SAL_CALL in_methodChar( sal_Unicode rIn ) throw (RuntimeException);
    virtual Any SAL_CALL in_methodAny( const Any& rIn ) throw (RuntimeException);
    virtual Type SAL_CALL in_methodType( const Type& rIn ) throw (RuntimeException);
    virtual Reference<XInterface> SAL_CALL in_methodXInterface( const Reference< XInterface >& rIn ) throw (RuntimeException);
    virtual Reference<XInvocation > SAL_CALL in_methodInvocation( const Reference< XInvocation >& inv ) throw (RuntimeException);
    virtual SimpleStruct SAL_CALL in_methodStruct( const SimpleStruct& aStruct ) throw (RuntimeException);
    virtual void SAL_CALL in_methodAll( sal_Int8 b, float f, double d, sal_Bool boo, sal_Int16 sh, sal_uInt16 us, sal_Int32 l, sal_uInt32 ul, const OUString& s, sal_Unicode c, const Any& a, const Type& t, const Reference<XInvocation>& inv ) throw (RuntimeException);

    // XTestInterfaces --------------------------------------------------------------------------
    virtual void SAL_CALL testInterface(  const Reference< XCallback >& xCallback, sal_Int32 mode ) throw(RuntimeException);
    virtual void SAL_CALL testInterface2(  const Reference< XSimple >& xSimple, sal_Int32 mode ) throw(RuntimeException);
    // XSimple --------------------------------------------------------------------------
    void SAL_CALL func( const OUString &message) throw(css::uno::RuntimeException);
    OUString SAL_CALL getName() throw(css::uno::RuntimeException);

    // XIdentity
    virtual void SAL_CALL setObject( const Reference< XInterface >& val ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL isSame( const Reference< XInterface >& val ) throw (RuntimeException);
    virtual Reference< XInterface > SAL_CALL getThis(  ) throw (RuntimeException);
};

class EventListener: public WeakImplHelper<XEventListener>
{
public:
    EventListener(): bCalled( sal_False)
        {}
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (RuntimeException);

    sal_Bool bCalled;
};


OComponent::~OComponent()
{
}


// Funktions ==============================================================================

Reference<XInterface> SAL_CALL OComponent_CreateInstance( const Reference<XMultiServiceFactory> & rSMgr ) throw(RuntimeException)
{
//  Reference<XInterface> xService(static_cast<XWeak*>(new OComponent( rSMgr )), UNO_QUERY);
    OComponent* o= new OComponent( rSMgr );
    Reference<XInterface> xService(static_cast<XIdentity*>(o), UNO_QUERY);
    return xService;
}

Sequence<OUString> OComponent_getSupportedServiceNames()
{
    Sequence<OUString> aRet { SERVICE_NAME;//ODataInputStream_getImplementationName() };

    return aRet;
}


extern "C" sal_Bool SAL_CALL component_writeInfo(   void * /*pServiceManager*/, void * pRegistryKey )
{
    if(pRegistryKey)
    {
        try
        {
            Reference<XRegistryKey> xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(KEY1);
            xNewKey->createKey( KEY2);
            return sal_True;

        }
        catch(InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!\n");
        }
    }
    return sal_False;
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    static void * pRet= NULL;
    if( ! pRet)
    {
        OUString aImplName( OUString::createFromAscii( pImplName ) );
        if (pServiceManager && aImplName.equals(  IMPL_NAME  ))
        {
            Reference<XMultiServiceFactory> xMulFac(
                reinterpret_cast< XMultiServiceFactory*>(pServiceManager));

            Sequence<OUString> seqServiceNames;
            Reference<XSingleServiceFactory> xFactory = createOneInstanceFactory( xMulFac, SERVICE_NAME,
                                        OComponent_CreateInstance, seqServiceNames);

            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
    }
    return  pRet;
}


// XTestSequence ============================================================================
Sequence<sal_Int8> SAL_CALL OComponent::methodByte(const Sequence< sal_Int8 >& aSeq) throw( RuntimeException )
{
    sal_Int8 _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence<float> SAL_CALL OComponent::methodFloat(const Sequence< float>& aSeq) throw( RuntimeException )
{
    float _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence<double> SAL_CALL OComponent::methodDouble(const Sequence< double >& aSeq) throw( RuntimeException)
{
    double _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence< sal_Bool > SAL_CALL OComponent::methodBool(const Sequence< sal_Bool >& aSeq) throw( RuntimeException)
{
    sal_Bool _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence< sal_Int16 > SAL_CALL OComponent::methodShort(const Sequence< sal_Int16 >& aSeq) throw( RuntimeException )
{
    sal_Int16 _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence< sal_uInt16 > SAL_CALL OComponent::methodUShort(const Sequence< sal_uInt16 >& aSeq) throw( RuntimeException )
{
    sal_uInt16 _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence< sal_Int32 > SAL_CALL OComponent::methodLong(const Sequence< sal_Int32 >& aSeq) throw( RuntimeException)
{
    sal_Int32 _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++)  {
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence< sal_uInt32 > SAL_CALL OComponent::methodULong(const Sequence< sal_uInt32 >& aSeq) throw( RuntimeException)
{
    sal_uInt32 _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence< OUString > SAL_CALL OComponent::methodString(const Sequence< OUString >& aSeq) throw( RuntimeException)
{
    OUString _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++) {
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence< sal_Unicode > SAL_CALL OComponent::methodChar(const Sequence< sal_Unicode >& aSeq) throw( RuntimeException)
{
    sal_Unicode _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}
Sequence< Any > SAL_CALL OComponent::methodAny(const Sequence< Any >& aSeq) throw( RuntimeException)
{
    Any _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
        TypeClass _t= _x.getValueTypeClass();
        if( _t== TypeClass_STRING)
            OUString s(* (rtl_uString**)_x.getValue());
    }
    return aSeq;

}

Sequence< Type > SAL_CALL OComponent::methodType(const Sequence< Type >& aSeq) throw( RuntimeException )
{
    Type _x;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        _x= aSeq.getConstArray()[i];
    }
    return aSeq;
}

Sequence< Reference< XInterface > > SAL_CALL OComponent::methodXInterface( const Sequence< Reference< XInterface > >& aSeq ) throw(RuntimeException)
{
    for( sal_Int32 i= 0; i < aSeq.getLength(); i++)
    {
        Reference<XInterface> xInt= aSeq[i];
        Reference<XEventListener> xList( xInt, UNO_QUERY);
        if( xList.is())
            xList->disposing( EventObject());
    }
    return aSeq;
}

Sequence< Sequence< sal_Int32 > > SAL_CALL OComponent::methodSequence(const Sequence< Sequence< sal_Int32 > >& aSeq) throw( RuntimeException)
{
    sal_Int32 value;
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        const Sequence<sal_Int32>& rseq2= aSeq.getConstArray()[i];
        for (sal_Int16 j= 0; j < rseq2.getLength(); j++){
            value= rseq2.getConstArray()[j];
        }
    }
    return aSeq;
}
Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL OComponent::methodSequence2(const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq)
    throw( RuntimeException )
{
    sal_Int32 value;
    sal_Int32 len= aSeq.getLength();
    for( sal_Int16 i= 0; i < aSeq.getLength(); i++){
        const Sequence< Sequence<sal_Int32> >& rseq2= aSeq.getConstArray()[i];
        len= rseq2.getLength();

        for (sal_Int16 j= 0; j < rseq2.getLength(); j++){
            const Sequence<sal_Int32> & rseq3= rseq2.getConstArray()[j];
            len= rseq3.getLength();

            for (sal_Int16 k= 0; k < rseq3.getLength(); k++)
                value= rseq3.getConstArray()[k];
        }
    }
    return aSeq;
}

Sequence< Reference< XEventListener> > SAL_CALL OComponent::methodXEventListeners( const Sequence< Reference <XEventListener> >& aSeq) throw( RuntimeException)
{
    Reference<XEventListener> listener;
    for( int i= 0; i < aSeq.getLength(); i++)
    {
        listener= aSeq[i];

        listener->disposing( EventObject() );
    }

    return aSeq;
}

Sequence< Sequence<Reference<XEventListener > > > SAL_CALL OComponent::methodXEventListenersMul( const Sequence<Sequence<Reference<XEventListener > > >& aSeq ) throw (RuntimeException)
{
    Reference<XEventListener> listener;
    for( int i= 0; i < aSeq.getLength(); i++)
    {
        Sequence<Reference<XEventListener> > seqInner= aSeq[i];
        for( int j= 0; j < seqInner.getLength(); j++)
        {
            listener= seqInner[j];
            listener->disposing( EventObject() );
        }
    }
    return aSeq;
}


Sequence< sal_Int8 > SAL_CALL OComponent::getAttrByte() throw( RuntimeException)
{
    return m_seqByte;
}
void SAL_CALL OComponent::setAttrByte(const Sequence< sal_Int8 >& AttrByte_) throw( RuntimeException )
{
    m_seqByte= AttrByte_;
}
Sequence< float > SAL_CALL OComponent::getAttrFloat()  throw( RuntimeException )
{
    return m_seqFloat;
}
void SAL_CALL OComponent::setAttrFloat(const Sequence< float >& AttrFloat_) throw( RuntimeException )
{
    m_seqFloat= AttrFloat_;
}

Sequence< double > SAL_CALL OComponent::getAttrDouble()  throw( RuntimeException )
{
    return m_seqDouble;
}
void SAL_CALL OComponent::setAttrDouble(const Sequence< double >& AttrDouble_) throw( RuntimeException )
{
    m_seqDouble= AttrDouble_;
}

Sequence< sal_Bool > SAL_CALL OComponent::getAttrBool()  throw( RuntimeException)
{
    return m_seqBool;
}

void SAL_CALL OComponent::setAttrBool(const Sequence< sal_Bool >& AttrBool_) throw (RuntimeException )
{
    m_seqBool= AttrBool_;
}

Sequence< sal_Int16 > SAL_CALL OComponent::getAttrShort()  throw( RuntimeException)
{
    return m_seqShort;
}
void SAL_CALL OComponent::setAttrShort(const Sequence< sal_Int16 >& AttrShort_) throw( RuntimeException )
{
    m_seqShort= AttrShort_;
}

Sequence< sal_uInt16 > SAL_CALL OComponent::getAttrUShort() throw( RuntimeException )
{
    return m_seqUShort;
}
void SAL_CALL OComponent::setAttrUShort(const Sequence< sal_uInt16 >& AttrUShort_) throw( RuntimeException )
{
    m_seqUShort= AttrUShort_;
}

Sequence< sal_Int32 > SAL_CALL OComponent::getAttrLong()  throw( RuntimeException)
{
    return m_seqLong;
}
void SAL_CALL OComponent::setAttrLong(const Sequence< sal_Int32 >& AttrLong_) throw( RuntimeException )
{
    m_seqLong= AttrLong_;
}

Sequence< sal_uInt32 > SAL_CALL OComponent::getAttrULong()  throw( RuntimeException )
{
    return m_seqULong;
}
void SAL_CALL OComponent::setAttrULong(const Sequence< sal_uInt32 >& AttrULong_) throw( RuntimeException )
{
    m_seqULong= AttrULong_;
}

Sequence< OUString > SAL_CALL OComponent::getAttrString()  throw( RuntimeException )
{
    return m_seqString;
}
void SAL_CALL OComponent::setAttrString(const Sequence< OUString >& AttrString_) throw( RuntimeException )
{
    m_seqString= AttrString_;
}

Sequence< sal_Unicode > SAL_CALL OComponent::getAttrChar()  throw( RuntimeException )
{
    return m_seqChar;
}
void SAL_CALL OComponent::setAttrChar(const Sequence< sal_Unicode >& AttrChar_) throw( RuntimeException)
{
    m_seqChar= AttrChar_;
}

Sequence< Any > SAL_CALL OComponent::getAttrAny() throw( RuntimeException)
{
    return m_seqAny;
}
void SAL_CALL OComponent::setAttrAny(const Sequence< Any >& AttrAny_) throw( RuntimeException )
{
    m_seqAny= AttrAny_;
}

Sequence< Type > SAL_CALL OComponent::getAttrType()  throw( RuntimeException )
{
    return m_seqType;
}

void SAL_CALL OComponent::setAttrType( const Sequence< Type >& AttrType_) throw( RuntimeException )
{
    m_seqType = AttrType_;
}


Sequence< Sequence< sal_Int32 > > SAL_CALL OComponent::getAttrSequence()  throw( RuntimeException)
{
    return m_seq1;
}
void SAL_CALL OComponent::setAttrSequence(const Sequence< Sequence< sal_Int32 > >& AttrSequence) throw(RuntimeException )
{
    sal_Int32 x= 0;
    for( sal_Int32 i=0; i < AttrSequence.getLength(); i++)
    {
         Sequence< sal_Int32 > seq= AttrSequence[i];

        for ( sal_Int32 j=0; j < seq.getLength(); j++)
        {
            x= seq[j];
        }
    }

    m_seq1= AttrSequence;
}

Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL OComponent::getAttrSequence2()  throw( RuntimeException )
{
    return m_seq2;
}
void SAL_CALL OComponent::setAttrSequence2(const Sequence< Sequence< Sequence< sal_Int32 > > >& AttrSequence2_)
    throw( RuntimeException )
{
    m_seq2= AttrSequence2_;
}

Sequence< Reference< XInterface > > SAL_CALL OComponent::getAttrXInterface() throw(RuntimeException)
{
    return m_seqxInterface;
}
void SAL_CALL OComponent::setAttrXInterface( const Sequence< Reference< XInterface > >& _attrxinterface ) throw(RuntimeException)
{
    m_seqxInterface= _attrxinterface;
}


::sal_Int8 SAL_CALL OComponent::getAByte() throw (RuntimeException)
{
    return m_attr_int8;
}

void SAL_CALL OComponent::setAByte( ::sal_Int8 _abyte ) throw (RuntimeException)
{
    m_attr_int8 = _abyte;
}

float SAL_CALL OComponent::getAFloat() throw (RuntimeException)
{
    return m_attr_float;
}

void SAL_CALL OComponent::setAFloat( float _afloat ) throw (RuntimeException)
{
    m_attr_float = _afloat;
}

double SAL_CALL OComponent::getADouble() throw (RuntimeException)
{
    return m_attr_double;
}

void SAL_CALL OComponent::setADouble( double _adouble ) throw (RuntimeException)
{
    m_attr_double = _adouble;
}

sal_Bool SAL_CALL OComponent::getABool() throw (RuntimeException)
{
    return m_attr_bool;
}

void SAL_CALL OComponent::setABool( sal_Bool _abool ) throw (RuntimeException)
{
    m_attr_bool = _abool;
}

::sal_Int16 SAL_CALL OComponent::getAShort() throw (RuntimeException)
{
    return m_attr_int16;
}

void SAL_CALL OComponent::setAShort( ::sal_Int16 _ashort ) throw (RuntimeException)
{
    m_attr_int16 = _ashort;
}

::sal_uInt16 SAL_CALL OComponent::getAUShort() throw (RuntimeException)
{
    return m_attr_uint16;
}

void SAL_CALL OComponent::setAUShort( ::sal_uInt16 _aushort ) throw (RuntimeException)
{
    m_attr_uint16 = _aushort;
}

::sal_Int32 SAL_CALL OComponent::getALong() throw (RuntimeException)
{
    return m_attr_int32;
}

void SAL_CALL OComponent::setALong( ::sal_Int32 _along ) throw (RuntimeException)
{
    m_attr_int32 = _along;
}

::sal_uInt32 SAL_CALL OComponent::getAULong() throw (RuntimeException)
{
    return m_attr_uint32;
}

void SAL_CALL OComponent::setAULong( ::sal_uInt32 _aulong ) throw (RuntimeException)
{
    m_attr_uint32 = _aulong;
}

OUString SAL_CALL OComponent::getAString() throw (RuntimeException)
{
    return m_attr_string;
}

void SAL_CALL OComponent::setAString( const OUString& _astring ) throw (RuntimeException)
{
    m_attr_string = _astring;
}

::sal_Unicode SAL_CALL OComponent::getAChar() throw (RuntimeException)
{
    return m_attr_char;
}

void SAL_CALL OComponent::setAChar( ::sal_Unicode _achar ) throw (RuntimeException)
{
    m_attr_char = _achar;
}

Any SAL_CALL OComponent::getAAny() throw (RuntimeException)
{
    return m_attr_any;
}

void SAL_CALL OComponent::setAAny( const Any& _aany ) throw (RuntimeException)
{
    m_attr_any = _aany;
}

Type SAL_CALL OComponent::getAType() throw (RuntimeException)
{
    return m_attr_type;
}

void SAL_CALL OComponent::setAType( const Type& _atype ) throw (RuntimeException)
{
    m_attr_type = _atype;
}

Reference< XInterface > SAL_CALL OComponent::getAXInterface() throw (RuntimeException)
{
    return m_attr_xinterface;
}

void SAL_CALL OComponent::setAXInterface( const Reference<XInterface >& _axinterface ) throw (RuntimeException)
{
    m_attr_xinterface = _axinterface;
}

Reference<XInvocation > SAL_CALL OComponent::getAXInvocation() throw (RuntimeException)
{
    return m_attr_xinvocation;
}

void SAL_CALL OComponent::setAXInvocation( const Reference< XInvocation >& _axinvocation ) throw (RuntimeException)
{
    m_attr_xinvocation = _axinvocation;
}

void SAL_CALL OComponent::testout_methodByte(sal_Int8& rOut) throw( RuntimeException )
{
    rOut= m_int8;
}
void SAL_CALL OComponent::testout_methodFloat(float& rOut) throw( RuntimeException )
{
    rOut= m_float;
}
void SAL_CALL OComponent::testout_methodDouble(double& rOut) throw( RuntimeException )
{
    rOut= m_double;
}

void SAL_CALL OComponent::testout_methodBool(sal_Bool& rOut) throw( RuntimeException )
{
    rOut= m_bool;
}
void SAL_CALL OComponent::testout_methodShort(sal_Int16& rOut) throw( RuntimeException )
{
    rOut= m_int16;
}
void SAL_CALL OComponent::testout_methodUShort(sal_uInt16& rOut) throw( RuntimeException )
{
    rOut= m_uint16;
}
void SAL_CALL OComponent::testout_methodLong(sal_Int32& rOut) throw( RuntimeException )
{
    rOut = m_int32;
}
void SAL_CALL OComponent::testout_methodULong(sal_uInt32& rOut) throw( RuntimeException )
{
    rOut= m_uint32;
}
void SAL_CALL OComponent::testout_methodHyper(sal_Int64& rOut) throw( RuntimeException )
{
    rOut = m_int64;
}

void SAL_CALL OComponent::testout_methodUHyper(sal_uInt64& rOut) throw( RuntimeException )
{
    rOut = m_uint64;
}

void SAL_CALL OComponent::testout_methodString(OUString& rOut) throw( RuntimeException )
{
    rOut= m_string;
}
void SAL_CALL OComponent::testout_methodChar(sal_Unicode& rOut) throw( RuntimeException )
{
    rOut= m_char;
}
void SAL_CALL OComponent::testout_methodAny(Any& rOut) throw( RuntimeException)
{
    rOut = m_any;
}

void SAL_CALL OComponent::testout_methodType(Type& rOut) throw( RuntimeException )
{
    rOut = m_type;
}

void SAL_CALL OComponent::testout_methodSequence(Sequence< sal_Int32 >& rOut) throw( RuntimeException)
{
    rOut.realloc(10);
    for( sal_Int16 i= 0; i < rOut.getLength(); i++) rOut.getArray()[i]= i;
}
void SAL_CALL OComponent::testout_methodSequence2(Sequence< Sequence< sal_Int32 > >& rOut) throw( RuntimeException )
{
    rOut.realloc( 10);
    for( sal_Int16 i= 0; i < rOut.getLength(); i++){
        Sequence<sal_Int32>& rseq2= rOut.getArray()[i];
        rseq2.realloc( 10);
        for (sal_Int16 j= 0; j < rseq2.getLength(); j++){
            rseq2.getArray()[j]= j;
        }
    }
}
void SAL_CALL OComponent::testout_methodMulParams1(sal_Int32& rout1, sal_Int32& rout2) throw( RuntimeException )
{
    rout1= 999;
    rout2= 1111;
}
void SAL_CALL OComponent::testout_methodMulParams2(sal_Int32& rout1, sal_Int32& rout2, OUString& rout3) throw( RuntimeException )
{
    rout1= 1111;
    rout2= 1222;
    rout3= L" another string";
}
void SAL_CALL OComponent::testout_methodMulParams3(const OUString&, OUString& sout) throw( RuntimeException )
{
    sout= L"Out Hallo!";
}
void SAL_CALL OComponent::testout_methodMulParams4( float in1, float& out1, sal_Int32 in2, sal_Int32& out2, sal_Int32 ) throw(RuntimeException)
{
    out1= in1 + 1;
    out2= in2 + 1;
}

void SAL_CALL OComponent::testout_methodXInterface( Reference< XInterface >& rOut ) throw(RuntimeException)
{
    rOut = m_xinterface;
}

// XTestInParameters ------------------------------------------------------------
sal_Int8 SAL_CALL OComponent::in_methodByte( sal_Int8 rIn ) throw (RuntimeException)
{
    m_int8 = rIn;
    return rIn;
}
float SAL_CALL OComponent::in_methodFloat( float rIn ) throw (RuntimeException)
{
    m_float = rIn;
    return rIn;
}
double SAL_CALL OComponent::in_methodDouble( double rIn ) throw (RuntimeException)
{
    m_double = rIn;
    return rIn;
}
sal_Bool SAL_CALL OComponent::in_methodBool( sal_Bool rIn ) throw (RuntimeException)
{
    m_bool = rIn;
    return rIn;
}
sal_Int16 SAL_CALL OComponent::in_methodShort( sal_Int16 rIn ) throw (RuntimeException)
{
    m_int16 = rIn;
    return rIn;
}
sal_uInt16 SAL_CALL OComponent::in_methodUShort( sal_uInt16 rIn ) throw (RuntimeException)
{
    m_uint16 = rIn;
    return rIn;
}
sal_Int32 SAL_CALL OComponent::in_methodLong( sal_Int32 rIn ) throw (RuntimeException)
{
    m_int32 = rIn;
    return rIn;
}
sal_uInt32 SAL_CALL OComponent::in_methodULong( sal_uInt32 rIn ) throw (RuntimeException)
{
    m_uint32 = rIn;
    return rIn;
}
sal_Int64 SAL_CALL OComponent::in_methodHyper( sal_Int64 rIn ) throw (RuntimeException)
{
    m_int64 = rIn;
    return rIn;
}

sal_uInt64 SAL_CALL OComponent::in_methodUHyper( sal_uInt64 rIn ) throw (RuntimeException)
{
    m_uint64 = rIn;
    return rIn;
}

OUString SAL_CALL OComponent::in_methodString( const OUString& rIn ) throw (RuntimeException)
{
    m_string = rIn;
    return rIn;
}
sal_Unicode SAL_CALL OComponent::in_methodChar( sal_Unicode rIn ) throw (RuntimeException)
{
    m_char = rIn;
    return rIn;
}
Any SAL_CALL OComponent::in_methodAny( const Any& rIn ) throw (RuntimeException)
{
    m_any = rIn;
    return rIn;
}

Type SAL_CALL OComponent::in_methodType( const Type& rIn ) throw (RuntimeException)
{
    m_type = rIn;
    return rIn;
}

Reference<XInvocation > SAL_CALL OComponent::in_methodInvocation( const Reference< XInvocation >& inv )
        throw (RuntimeException)
{
    //We expect the invocation results from a conversion of VBasicEventListener.VBEventListener
//which implements XEventListener
    // extensions/test/ole/EventListenerSample
    EventObject event( Reference<XInterface>(static_cast<XTestInParameters*>(this),UNO_QUERY));
    Any anyParam;
    anyParam <<= event;
    Sequence<Any> params( &anyParam, 1);
    Sequence<sal_Int16> outIndex;
    Sequence<Any> outParams;
    try{
    inv->invoke( OUString("disposing"),
                params, outIndex, outParams);
    }catch(IllegalArgumentException &) {
    }
    catch(CannotConvertException &){
    }
    catch(InvocationTargetException&) {
    }
    return inv;
}
Reference<XInterface> SAL_CALL OComponent::in_methodXInterface( const Reference<XInterface >& rIn ) throw (RuntimeException)
{
    m_xinterface = rIn;
    return rIn;
}

SimpleStruct SAL_CALL OComponent::in_methodStruct( const SimpleStruct& aStruct )
        throw (RuntimeException)
{
    SimpleStruct& s= const_cast<SimpleStruct&>(aStruct);
    s.message= s.message + OUString(
        "This string was set in OleTest");
    return aStruct;
}
void SAL_CALL OComponent::in_methodAll(
    sal_Int8, float, double, sal_Bool, sal_Int16, sal_uInt16,
    sal_Int32, sal_uInt32, const OUString&, sal_Unicode,
    const Any&, const Type&, const Reference<XInvocation>&) throw (RuntimeException)
{
}

// INOUT -----------------------------------------------------------------------------------
void SAL_CALL OComponent::testinout_methodByte(sal_Int8& rOut) throw( RuntimeException )
{
    sal_Int8 tmp = rOut;
    rOut = m_int8;
    m_int8 = tmp;
}
void SAL_CALL OComponent::testinout_methodFloat(float& rOut) throw( RuntimeException )
{
    float tmp = rOut;
    rOut = m_float;
    m_float = tmp;
}

void SAL_CALL OComponent::testinout_methodDouble(double& rOut) throw( RuntimeException )
{
    double tmp = rOut;
    rOut = m_double;
    m_double = tmp;
}
void SAL_CALL OComponent::testinout_methodBool(sal_Bool& rOut) throw( RuntimeException )
{
    sal_Bool tmp = rOut;
    rOut = m_bool;
    m_bool = tmp;
}
void SAL_CALL OComponent::testinout_methodShort(sal_Int16& rOut) throw( RuntimeException )
{
    sal_Int16 tmp= rOut;
    rOut = m_int16;
    m_int16 = tmp;
}
void SAL_CALL OComponent::testinout_methodUShort(sal_uInt16& rOut) throw( RuntimeException )
{
    sal_uInt16 tmp = rOut;
    rOut = m_uint16;
    m_uint16 = tmp;
}
void SAL_CALL OComponent::testinout_methodLong(sal_Int32& rOut) throw( RuntimeException )
{
    sal_Int32 tmp = rOut;
    rOut = m_int32;
    m_int32 = tmp;
}
void SAL_CALL OComponent::testinout_methodULong(sal_uInt32& rOut) throw( RuntimeException )
{
    sal_uInt32 tmp = rOut;
    rOut = m_uint32;
    m_uint32 = tmp;
}
void SAL_CALL OComponent::testinout_methodHyper(sal_Int64& rOut) throw( RuntimeException )
{
    sal_Int64 tmp = rOut;
    rOut = m_int64;
    m_int64 = tmp;
}

void SAL_CALL OComponent::testinout_methodUHyper(sal_uInt64& rOut) throw( RuntimeException )
{
    sal_uInt64 tmp = rOut;
    rOut = m_uint64;
    m_uint64 = tmp;
}

void SAL_CALL OComponent::testinout_methodString(OUString& rOut) throw( RuntimeException )
{
    OUString tmp = rOut;
    rOut = m_string;
    m_string = tmp;
}
void SAL_CALL OComponent::testinout_methodChar(sal_Unicode& rOut) throw( RuntimeException)
{
    sal_Unicode tmp = rOut;
    rOut = m_char;
    m_char = tmp;
}
void SAL_CALL OComponent::testinout_methodAny(Any& rOut) throw( RuntimeException)
{
    Any tmp = rOut;
    rOut = m_any;
    m_any = tmp;
}
void SAL_CALL OComponent::testinout_methodType(Type& rOut) throw( RuntimeException)
{
    Type tmp = rOut;
    rOut = m_type;
    m_type = tmp;
}


void SAL_CALL OComponent::testinout_methodSequence(Sequence< sal_Int32 >& rOut) throw( RuntimeException)
{

    sal_Int32* arr= rOut.getArray();
    for ( sal_Int32 i=0; i < rOut.getLength(); i++)
    {
        rOut.getArray()[i] += 1;
    }
}
void SAL_CALL OComponent::testinout_methodSequence2(Sequence< Sequence< sal_Int32 > >& rOut) throw( RuntimeException )
{
    for( sal_Int32 i=0; i < rOut.getLength(); i++)
    {
        Sequence< sal_Int32 >& seq= rOut.getArray()[i];

        for ( sal_Int32 j=0; j < seq.getLength(); j++)
        {
            seq.getArray()[j] += seq.getArray()[j];
        }
    }
}

// The parameter should implement XInvocation and a Property "value"
void SAL_CALL OComponent::testinout_methodXInterface( Reference< XInvocation >& rOut ) throw(RuntimeException)
{
        Any any;
        any= rOut->getValue( OUString( L"value"));
        OUString _s;
        any >>= _s;
        OUString string(L"out");
        any <<= string;
        rOut->setValue( OUString(L"value"), any);

        any= rOut->getValue( OUString( L"value"));
        any >>= _s;

}

void SAL_CALL OComponent::testinout_methodXInterface2( Reference< XInterface > & rOut) throw( RuntimeException)
{
    Reference<XInterface> tmp = rOut;
    rOut = m_xinterface;
    m_xinterface = tmp;
}
Any SAL_CALL OComponent::methodAnyTest1(const Any& rIn) throw( RuntimeException )
{
    return rIn;
}
Any SAL_CALL OComponent::getAttrAny2()  throw( RuntimeException )
{
    return m_any;
}
void SAL_CALL OComponent::setAttrAny2(const Any& AttrAny2_) throw( RuntimeException )
{
    m_any= AttrAny2_;
}


// XTestStruct =======================================================================================

void SAL_CALL OComponent::methodStruct(const Property& aProp) throw( RuntimeException )
{
    char buff[1024];
    buff[0]= 0;
    sprintf( buff,"Property::Attribute : %d \n Property::Handle : %d \n Property::Name : %S",
        aProp.Attributes, aProp.Handle, (const sal_Unicode*)aProp.Name);
    MessageBox( NULL, A2T(buff), _T("OleTest: methodStruct"), MB_OK);
}

Property SAL_CALL OComponent::retMethodStruct() throw( RuntimeException )
{
    Property a(L"OleTest_Property", 255, cppu::UnoType<XInterface>::get(), PropertyAttribute::MAYBEVOID |
        PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED |
        PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY |
        PropertyAttribute::MAYBEAMBIGUOUS | PropertyAttribute::MAYBEDEFAULT );
    return a;
}

Property SAL_CALL OComponent::getAttrStruct() throw( RuntimeException)
{
    Property a(L"OleTest_Property", 255, cppu::UnoType<XInterface>::get(), PropertyAttribute::MAYBEVOID |
        PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED |
        PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY |
        PropertyAttribute::MAYBEAMBIGUOUS | PropertyAttribute::MAYBEDEFAULT );
    return a;

}

void SAL_CALL OComponent::setAttrStruct(const Property& AttrStruct_) throw( RuntimeException )
{
    char buff[1024];
    buff[0]= 0;
    sprintf( buff,"Property::Attribute : %d \n Property::Handle : %d \n Property::Name : %S",
        AttrStruct_.Attributes, AttrStruct_.Handle, (const sal_Unicode*)AttrStruct_.Name);
//  MessageBox( NULL, A2T(buff), _T("OleTest: setAttrStruct"), MB_OK);
}

Property SAL_CALL OComponent::methodStruct2( const Property& aProp ) throw (RuntimeException)
{
    return aProp;
}

// XTestOther ==================================================================================
void SAL_CALL OComponent::other_methodAnyIn(const Any& ) throw( RuntimeException )
{
}
void SAL_CALL OComponent::other_methodAnyOut(Any& rAny) throw( RuntimeException )
{
    rAny <<= OUString(L"Ein Any");
}

Any SAL_CALL OComponent::other_methodAnyRet() throw(RuntimeException )
{
    Any a;
    a <<= OUString(L"Ein Any");
    return a;
}
void SAL_CALL OComponent::in_float( float val) throw ( RuntimeException)
{
    USES_CONVERSION;
    char buff[256];
    sprintf( buff, "parameter : %f", val);
    MessageBox( NULL, A2T(buff), _T("OleTest"), MB_OK);
}
Any SAL_CALL OComponent::other_methodAny( const Any& rAny, const OUString& typeInAny )
        throw (RuntimeException)
{
    Type expectedType;
    typelib_TypeDescription * pDesc= NULL;
    typelib_typedescription_getByName( &pDesc, typeInAny.pData );
    if( pDesc)
    {
        expectedType = Type( pDesc->pWeakRef );
        typelib_typedescription_release( pDesc);
    }
    if (rAny.getValueType() != expectedType)
        throw RuntimeException();

    return rAny;
}

// XTestInterfaces -------------------------------------------------------------------------------------
void SAL_CALL OComponent::testInterface(  const Reference< XCallback >& xCallback, sal_Int32 mode ) throw(RuntimeException)
{
    USES_CONVERSION;
    sal_Int32 i=0;
    OUString aString;
    char buff[1024];

    Reference<XSimple> xSimple;
    SimpleStruct aSimpleStruct;
    SimpleEnum aSimpleEnum;
    Sequence<Any> seqAny;
    Sequence< sal_Int8 > seqByte;
    Any outAny;
    sal_Bool aBool;
    sal_Unicode aChar;
    float aFloat;
    double aDouble;
    sal_Int8 aByte;
    sal_Int16 aShort;
    sal_Int32 aLong;

    switch( mode)
    {
    case 1:
         xCallback->func1(); break;
    case 2:
        xSimple=    xCallback->returnInterface();
        xSimple->func(L"XCallback::returnInterface");
        break;


    case 3:
        xCallback->outInterface( xSimple);
        sprintf( buff, "XCallback::outInterface, value: %x", xSimple.get());
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        xSimple->func(L"XCallback::outInterface works");
        break;
    case 4:
         xCallback->outStruct( aSimpleStruct);
         sprintf( buff,"XCallback::outStruct, SimpleStruct::message: %s", OLE2A( aSimpleStruct.message));
         MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
         break;
    case 5:
        xCallback->outEnum( aSimpleEnum);
        sprintf( buff,"XCallback::outEnum, SimpleEnum: %d", aSimpleEnum);
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 6:
        xCallback->outSeqAny( seqAny);
        sprintf( buff,"XCallback::outSeqAny, length: %d )", seqAny.getLength());
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        for( i=0; i < seqAny.getLength(); i++)
        {
            Any any= seqAny[i];
            if( any.getValueTypeClass() == TypeClass_STRING)
            {
                any >>= aString;
                OutputDebugStringW( aString);
            }

        }
        break;
    case 7:
        xCallback->outAny( outAny);
        if( outAny.getValueTypeClass() == TypeClass_STRING)
            outAny >>= aString;
        sprintf( buff,"XCallback::outAny, Any : %s", W2A( aString));
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 8:
        xCallback->outBool( aBool);
        sprintf( buff,"XCallback::outBool, value: %d", aBool);
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 9:
        xCallback->outChar( aChar);
        sprintf( buff,"XCallback::outChar, value: %C", aChar);
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 10:
        xCallback->outString( aString);
        sprintf( buff,"XCallback::outString, value: %s", W2A( aString));
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 11:
        xCallback->outFloat( aFloat);
        sprintf( buff,"XCallback::outFloat, value: %f", aFloat);
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 12:
        xCallback->outDouble( aDouble);
        sprintf( buff,"XCallback::outDouble, value: %f", aDouble);
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 13:
        xCallback->outByte( aByte);
        sprintf( buff,"XCallback::outByte, value: %d", aByte);
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 14:
        xCallback->outShort( aShort);
        sprintf( buff,"XCallback::outShort, value: %d", aShort);
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 15:
        xCallback->outLong( aLong);
        sprintf( buff,"XCallback::outLong, value: %d", aLong);
        MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
        break;
    case 18:
    case 19:
    case 30:
        {

            long outLong= 0;
            xCallback->outValuesMixed( 1111, outLong, OUString( L"in string") );

            sprintf( buff, "oletest.testInterface: outValue: %d", outLong);
            MessageBox( NULL, A2T(buff), _T("OleTest"), MB_OK);
            break;
        }

    case 31:
        {
            Reference< XSimple > xSimple;
            SimpleStruct aSimpleStruct;
            SimpleEnum aSimpleEnum;
            Sequence<Any> seqAny;
            Any         aAny;
            sal_Bool    aBool;
            sal_Unicode aChar;
            OUString    aString;
            float       aFloat;
            double      aDouble;
            sal_Int8    aByte;
            sal_Int16   aShort;
            sal_Int32   aLong;
            xCallback->outValuesAll( xSimple, aSimpleStruct, aSimpleEnum, seqAny, aAny, aBool,
                aChar, aString,
            aFloat, aDouble,
            aByte,
            aShort, aLong);

            MessageBox( NULL, _T("XCallback::outValuesAll returned"), _T("OleTest::testInterface"), MB_OK);
            break;
        }
    case 32:
        {

            xCallback->outSeqByte( seqByte);
            sprintf( buff,"XCallback::outSeqAny, length: %d )", seqAny.getLength());
            MessageBox( NULL, A2T(buff), _T("OleTest out parameter"), MB_OK);
            for( i=0; i < seqAny.getLength(); i++)
            {
                Any any= seqAny[i];
                if( any.getValueTypeClass() == TypeClass_STRING)
                {
                    any >>= aString;
                    OutputDebugStringW( aString);
                }
            }
            break;
        }
        // ############################################################################
        // IN OUT parameter
        // ############################################################################
    case 100:
        {
            Reference<XSimple> xSimple= static_cast<XSimple*>(this);
            xCallback->inoutInterface( xSimple);
            xSimple->func(L"XSimple called from OleTest");
            break;
        }
    case 101:
        {
        Reference<XIdlReflection> xRefl( theCoreReflection::get(comphelper::getComponentContext(m_rFactory)) );
        Reference<XIdlClass> xClass= xRefl->forName(L"oletest.SimpleStruct");
        Any any;
        if( xClass.is())
            xClass->createObject( any);

        if( any.getValueTypeClass() == TypeClass_STRUCT)
        {
            SimpleStruct* pStruct= ( SimpleStruct*) any.getValue();
            pStruct->message= "This struct was created in OleTest";

            SimpleStruct aStruct;
            any >>= aStruct;
            xCallback->inoutStruct( aStruct);
            // a Struct should now contain a different message
            MessageBox( NULL, W2T(aStruct.message), _T("OleTest in out parameter"), MB_OK);
        }

        break;
        }
    case 102:
        {
        SimpleEnum aEnum= SimpleEnum_B;
        xCallback->inoutEnum( aEnum);
        char buff[1024];
        sprintf( buff, "Enum: %d", aEnum);
        MessageBox( NULL, A2T(buff), _T("OleTest in out parameter"), MB_OK);
        break;
        }
    case 103:
        {
            Any arAny[3];
            arAny[0] <<= OUString( L"string 0");
            arAny[1] <<= OUString( L"string 1");
            arAny[2] <<= OUString( L"string 2");

            Sequence< Any >seqAny( arAny, 3);
            xCallback->inoutSeqAny( seqAny);
            char buff[1023];
            sprintf( buff, "Sequence length: %d", seqAny.getLength());
            MessageBox( NULL,A2T(buff) , _T("OleTest in out parameter"), MB_OK);

            for( int i=0; i < seqAny.getLength(); i++)
            {
                Any any;
                any <<= seqAny[i];
                if(any.getValueTypeClass() == TypeClass_STRING)
                {
                    OUString str;
                    any >>= str;

                }
            }

        break;
        }
    case 104:
        {
            Any any;
            OUString s(L" OleTest");
            any <<= s;
            xCallback->inoutAny(any);
            if( any.getValueTypeClass() == TypeClass_STRING)
            {
                OUString s= *( rtl_uString**) any.getValue();
                MessageBox( NULL, W2T( s), _T("OleTest: inout value any"), MB_OK);
            }
        break;
        }
    case 105:
        {
        sal_Bool b= sal_True;
        xCallback->inoutBool( b);
        char buff[1024];
        sprintf( buff, "out value bool: %d", b);
        MessageBox( NULL, A2T( buff), _T("OleTest in out parameter"), MB_OK);
            break;
        }
    case 106:
        {
            sal_Unicode uc= L'A';
            xCallback->inoutChar( uc);
            char buff[1024];
            sprintf( buff, "out value sal_Unicode: %C", uc);
            MessageBox( NULL, A2T( buff), _T("OleTest in out parameter"), MB_OK);
            break;
        }
    case 107:
        {
            OUString s(L"OleTest");
            xCallback->inoutString( s);
            char buff[1024];
            sprintf( buff, "out value string: %S", s.getStr());
            MessageBox( NULL, A2T( buff), _T("OleTest in out parameter"), MB_OK);
            break;
        }
    case 108:
        {
            float f= 3.14f;
            xCallback->inoutFloat(f);
            char buff[1024];
            sprintf( buff, "out value float: %f", f);
            MessageBox( NULL, A2T( buff), _T("OleTest in out parameter"), MB_OK);
            break;
        }
    case 109:
        {
            double f= 3.145;
            xCallback->inoutDouble( f);
            char buff[1024];
            sprintf( buff, "out value double: %g", f);
            MessageBox( NULL, A2T( buff), _T("OleTest in out parameter"), MB_OK);
            break;
        }
    case 110:
        {
            sal_Int8 aByte= 0xf;
            xCallback->inoutByte( aByte);
            char buff[1024];
            sprintf( buff, "out value sal_Int8: %d", aByte);
            MessageBox( NULL, A2T( buff), _T("OleTest in out parameter"), MB_OK);
            break;
        }
    case 111:
        {
            sal_Int16 aShort= 0xff;
            xCallback->inoutShort( aShort);
            char buff[1024];
            sprintf( buff, "out value sal_Int16: %d", aShort);
            MessageBox( NULL, A2T( buff), _T("OleTest in out parameter"), MB_OK);
            break;
        }
    case 112:
        {
            sal_Int32 aLong= 0xfffe;
            xCallback->inoutLong( aLong);
            char buff[1024];
            sprintf( buff, "out value sal_Int32: %d", aLong);
            MessageBox( NULL, A2T( buff), _T("OleTest in out parameter"), MB_OK);
            break;
        }
    case 120:
        {
        Reference<XSimple> aXSimple= static_cast<XSimple*>(this);

        SimpleStruct aStruct;
        Reference<XIdlReflection> xRefl( m_rFactory->createInstance(L"com.sun.star.reflection.CoreReflection"), UNO_QUERY);
        if( xRefl.is())
        {
            Reference<XIdlClass> xClass= xRefl->forName(L"oletest.SimpleStruct");
            Any any;
            if( xClass.is())
                xClass->createObject( any);

            if( any.getValueTypeClass() == TypeClass_STRUCT)
            {
                SimpleStruct* pStruct= ( SimpleStruct*) any.getValue();
                pStruct->message= "This struct was created in OleTest";
                any >>= aStruct;
            }
        }

        SimpleEnum aEnum= SimpleEnum_B;

        Sequence< Any > aSeq;
        Any arAny[3];
        arAny[0] <<= OUString( L"string 0");
        arAny[1] <<= OUString( L"string 1");
        arAny[2] <<= OUString( L"string 2");
        aSeq = Sequence< Any >( arAny, 3);

        Any aAny;
        OUString s(L" OleTest");
        aAny <<= s;

        sal_Bool aBool= sal_True;
        sal_Unicode aChar= L'A';
        OUString aString( L"OleTest");
        float aFloat=3.14f;
        double aDouble= 3.145;
        sal_Int8 aByte= 0xf;
        sal_Int16 aShort= 0xff;
        sal_Int32 aLong= 0xffe;

        xCallback->inoutValuesAll( aXSimple, aStruct, aEnum, aSeq,
            aAny, aBool, aChar, aString,  aFloat, aDouble,
            aByte, aShort,  aLong );

        aXSimple->func(L"XSimple called from OleTest");
        MessageBox( NULL, W2T(aStruct.message), _T("OleTest"), MB_OK);

        for( int i=0; i < aSeq.getLength(); i++)
        {
            Any any;
            any <<= aSeq[i];
            if(any.getValueTypeClass() == TypeClass_STRING)
            {
                OUString str;
                any >>= str;

            }
        }
        break;
        }

    // ############################################################################
    // IN parameter
    // ############################################################################

    case 200:

        xCallback->inValues( L'a', 0xffffL, OUString(L" a string from OleTest"));
        break;
    case 201:
    {
        sal_Int8 arbyte[3]= { 1,2,3};
        Sequence< sal_Int8 > seq( arbyte, 3);
        xCallback->inSeqByte( seq);
        break;
    }
    case 202:
    {
        const int LISTENERS= 3;
        Reference<XEventListener> arListeners[LISTENERS];
        EventObject arEvents[LISTENERS];

        for( int i= 0; i < LISTENERS; i++)
        {
            Reference<XInterface> aList= static_cast<XWeak*>( new EventListener());
            arListeners[i].set( aList, UNO_QUERY);
        }

        xCallback->inSeqXEventListener(Sequence<Reference<XEventListener> > (arListeners, LISTENERS),
                                       Sequence<EventObject>(arEvents, LISTENERS));
        break;
    }

    // ############################################################################
    // Call a COM object that has not been passed as parameter to a UNO component and
    // hence no type information are available in the COM wrapper
    // ############################################################################
    case 300:
        {
            Reference<XInterface> xIntFact = m_rFactory->createInstance(L"com.sun.star.bridge.oleautomation.Factory");

            Reference<XMultiServiceFactory> oleFact(xIntFact, UNO_QUERY);

            Reference<XInterface> xIntCallback= oleFact->createInstance(L"XCallback_Impl.Callback");
            Reference<XInvocation> xInv( xIntCallback, UNO_QUERY);
            Sequence<sal_Int16> seqIndices;
            Sequence<Any> seqOutParams;
            xInv->invoke( OUString( L"outValuesAll"), Sequence<Any>(), seqIndices, seqOutParams);

            if( seqOutParams.getLength() == 12)
            {
                Reference<XSimple> xSimple= *(XSimple**)seqOutParams[0].getValue();
                xSimple->func( L"Call from OleTest on XSimple");
                SimpleStruct aStruct;
                seqOutParams[1] >>= aStruct;
                SimpleEnum aEnum= *(SimpleEnum*)seqOutParams[2].getValue();

                Sequence<Any> seqAny;
                seqOutParams[3] >>= seqAny;
                for( int i=0; i<seqAny.getLength(); i++)
                {
                    OUString _s;
                    seqAny[i] >>= _s;
                }

                Any _any= *(Any*)seqOutParams[4].getValue();
                sal_Bool _bool= *(sal_Bool*)seqOutParams[5].getValue();
                sal_Unicode _char= *( sal_Unicode*) seqOutParams[6].getValue();
                OUString _str= *( rtl_uString**)seqOutParams[7].getValue();

                float _f= *( float*)seqOutParams[8].getValue();
                double _d= *( double*) seqOutParams[9].getValue();
                sal_Int8 _byte= *( sal_Int8*) seqOutParams[10].getValue();
                sal_Int16 _short= *( sal_Int16*) seqOutParams[11].getValue();

                sal_Int32 _long= *( sal_Int32*) seqOutParams[12].getValue();

            }
            break;
        }
    case 301:
        // in / out parameter
        {
            Reference<XInterface> xIntFact = m_rFactory->createInstance(L"com.sun.star.bridge.oleautomation.Factory");

            Reference<XMultiServiceFactory> oleFact(xIntFact, UNO_QUERY);

            Reference<XInterface> xIntCallback= oleFact->createInstance(L"XCallback_Impl.Callback");
            Reference<XInvocation> xInv( xIntCallback, UNO_QUERY);
            Sequence<sal_Int16> seqIndices;
            Sequence<Any> seqOutParams;


            Any arAny[13];
            Reference<XSimple> xSimple= static_cast<XSimple*>( this);

            arAny[0] <<=  xSimple;
            SimpleStruct aStruct;
            Reference<XIdlReflection> xRefl( m_rFactory->createInstance(L"com.sun.star.reflection.CoreReflection"), UNO_QUERY);
            if( xRefl.is())
            {
                Reference<XIdlClass> xClass= xRefl->forName(L"oletest.SimpleStruct");
                Any any;
                if( xClass.is())
                    xClass->createObject( any);

                if( any.getValueTypeClass() == TypeClass_STRUCT)
                {
                    SimpleStruct* pStruct= ( SimpleStruct*) any.getValue();
                    pStruct->message = "This struct was created in OleTest";
                    any >>= aStruct;
                }
            }
            arAny[1] <<= aStruct;
            arAny[2] <<= SimpleEnum_C;

            Any arSeqAny[3];
            arSeqAny[0] <<= OUString( L"string 0");
            arSeqAny[1] <<= OUString( L"string 1");
            arSeqAny[2] <<= OUString( L"string 2");

            arAny[3] <<=  Sequence< Any >( arAny, 3);

            OUString str(L" Ein Any param");
            arAny[4] <<= str;
            arAny[5] <<= sal_False;
            arAny[6] <<= L'B';
            OUString stringParam(L" a string parameter");
            arAny[7] <<= stringParam;
            float _float= 3.14f;
            arAny[8] <<= _float;
            double _double= 3.145;
            arAny[9] <<= _double;
            sal_Int8 _byte= -1;
            arAny[10] <<= _byte;
            sal_Int16 _short= -1;
            arAny[11] <<= _short;
            sal_Int32 _long= -1;
            arAny[12] <<= _long;

            Sequence<Any> params( arAny, 13);

            xInv->invoke( OUString( L"inoutValuesAll"), params, seqIndices, seqOutParams);

            if( seqOutParams.getLength() == 12)
            {
                Reference<XSimple> xSimple= *(XSimple**)seqOutParams[0].getValue();
                xSimple->func( L"Call from OleTest on XSimple");
                SimpleStruct aStruct;
                seqOutParams[1] >>= aStruct;
                SimpleEnum aEnum= *(SimpleEnum*)seqOutParams[2].getValue();

                Sequence<Any> seqAny;
                seqOutParams[3] >>= seqAny;
                for( int i=0; i<seqAny.getLength(); i++)
                {
                    OUString _s;
                    seqAny[i] >>= _s;
                }

                Any _any= *(Any*)seqOutParams[4].getValue();
                sal_Bool _bool= *(sal_Bool*)seqOutParams[5].getValue();
                sal_Unicode _char= *( sal_Unicode*) seqOutParams[6].getValue();
                OUString _str= *( rtl_uString**)seqOutParams[7].getValue();

                float _f= *( float*)seqOutParams[8].getValue();
                double _d= *( double*) seqOutParams[9].getValue();
                sal_Int8 _byte= *( sal_Int8*) seqOutParams[10].getValue();
                sal_Int16 _short= *( sal_Int16*) seqOutParams[11].getValue();

                sal_Int32 _long= *( sal_Int32*) seqOutParams[12].getValue();

            }
            break;
        }
    case 303:
        {
            Reference<XInterface> xIntFact = m_rFactory->createInstance(
                L"com.sun.star.bridge.oleautomation.Factory");

            Reference<XMultiServiceFactory> oleFact(xIntFact, UNO_QUERY);

            Reference<XInterface> xIntCallback= oleFact->createInstance(L"XCallback_Impl.Callback");
            Reference<XInvocation> xInv( xIntCallback, UNO_QUERY);
            Sequence<sal_Int16> seqIndices;
            Sequence<Any> seqOutParams;

            Any arAny[3];
            sal_Unicode aChar=L'a';
            arAny[0] <<= aChar;
            sal_Int32 aLong= 0xffffffff;
            arAny[1] <<= aLong;
            OUString aString(L" a string parameter");
            arAny[2] <<= aString;

            xInv->invoke( OUString( L"inValues"), Sequence<Any>(arAny,3), seqIndices, seqOutParams);

            break;
        }
    // ############################################################################
    // Attributes
    // ############################################################################

    case 400:
        Reference<XSimple> simple=  xCallback->getsimple();
        simple->func(L"OleTest calls on XSimple");
        break;


    }

}

void SAL_CALL OComponent::setObject( const Reference< XInterface >& val ) throw (RuntimeException)
{
    m_xIntIdentity= val;
}

sal_Bool SAL_CALL OComponent::isSame( const Reference< XInterface >& val ) throw (RuntimeException)
{
    if( m_xIntIdentity == val)
        return sal_True;
    else
        return sal_False;
}

Reference< XInterface > SAL_CALL OComponent::getThis(  ) throw (RuntimeException)
{
    Reference<XInterface> ret(static_cast<XIdentity*>(this), UNO_QUERY);
    return ret;
}

void SAL_CALL EventListener::disposing( const css::lang::EventObject& ) throw (RuntimeException)
{
    bCalled= sal_True;
}

// XSimple --------------------------------------------------------------------------
void SAL_CALL OComponent::func( const OUString &message)throw(css::uno::RuntimeException)
{
    USES_CONVERSION;

    MessageBox( NULL, W2T( message.getStr()), _T("OleTest: XSimple::func"), MB_OK);
}
// XSimple --------------------------------------------------------------------------
OUString SAL_CALL OComponent::getName()throw(css::uno::RuntimeException)
{
    return L"XSimple";
}
void SAL_CALL OComponent::testInterface2(  const Reference< XSimple >& xSimple, sal_Int32 mode ) throw(RuntimeException)
{
    switch( mode)
    {
    case 0:
        {
        xSimple->func( L"OleTest is calling XSimple");
        Reference<XSimple2> xSimple2( xSimple, UNO_QUERY);
        if( xSimple2.is())
            xSimple2->func2( L"OleTest is calling XSimple2");
        Reference<XSimple3> xSimple3( xSimple, UNO_QUERY);
        if( xSimple3.is())
            xSimple3->func3( L"OleTest is calling XSimple3");

        break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
