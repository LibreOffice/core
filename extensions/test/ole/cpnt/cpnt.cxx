/*************************************************************************
 *
 *  $RCSfile: cpnt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:55 $
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

#include <windows.h>
#include <atlbase.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <osl/diagnose.h>
#include <uno/environment.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>


#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring>
#include <com/sun/star/uno/Reference.hxx>
#include <oletest/XTestSequence.hpp>
#include <oletest/XTestStruct.hpp>
#include <oletest/XTestOther.hpp>
#include <oletest/XTestInterfaces.hpp>
#include <oletest/XSimple.hpp>
#include <oletest/XSimple2.hpp>
#include <oletest/XSimple3.hpp>

#include <com/sun/star/beans/Property.hpp>
using namespace cppu;
using namespace osl;
using namespace oletest;
using namespace rtl;
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

class OComponent : public WeakImplHelper5<
         XTestSequence, XTestStruct, XTestOther, XTestInterfaces, XSimple>
{
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
    Sequence<Sequence< sal_Int32> > m_seq1;
    Sequence<Sequence< Sequence< sal_Int32> > > m_seq2;
    Any m_any;
    Sequence<Reference< XInterface > > m_seqxInterface;
public:
    OComponent( const Reference<XMultiServiceFactory> & rFactory ) :
      m_rFactory( rFactory ), m_arrayConstructor(0) {}
    ~OComponent();

public: // refcounting
//      sal_Bool                        queryInterface( Uik aUik, XInterfaceRef & rOut );
//      void                        acquire()                        { OWeakObject::acquire(); }
//      void                        release()                        { OWeakObject::release(); }
//      void*                       getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

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
    virtual Sequence< Reference< XInterface > > SAL_CALL methodXInterface( const Sequence< Reference< XInterface > >& aSeq ) throw(RuntimeException) ;
    virtual Sequence< Sequence< sal_Int32 > > SAL_CALL methodSequence(const Sequence< Sequence< sal_Int32 > >& aSeq) throw( RuntimeException );
    virtual Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL methodSequence2(const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq) throw( RuntimeException );

    virtual Sequence< sal_Int8 > SAL_CALL getAttrByte(void) throw( RuntimeException );
    virtual void SAL_CALL setAttrByte(const Sequence< sal_Int8 >& AttrByte_) throw( RuntimeException );
    virtual Sequence< float > SAL_CALL getAttrFloat(void)  throw( RuntimeException) ;
    virtual void SAL_CALL setAttrFloat(const Sequence< float >& AttrFloat_) throw( RuntimeException );
    virtual Sequence< double > SAL_CALL getAttrDouble(void)  throw( RuntimeException) ;
    virtual void SAL_CALL setAttrDouble(const Sequence< double >& AttrDouble_) throw( RuntimeException );
    virtual Sequence< sal_Bool > SAL_CALL getAttrBool(void)  throw( RuntimeException );
    virtual void SAL_CALL setAttrBool(const Sequence< sal_Bool >& AttrBool_) throw( RuntimeException );
    virtual Sequence< sal_Int16 > SAL_CALL getAttrShort(void)  throw( RuntimeException );
    virtual void SAL_CALL setAttrShort(const Sequence< sal_Int16 >& AttrShort_) throw( RuntimeException );
    virtual Sequence< sal_uInt16 > SAL_CALL getAttrUShort(void)  throw( RuntimeException );
    virtual void SAL_CALL setAttrUShort(const Sequence< sal_uInt16 >& AttrUShort_) throw( RuntimeException );
    virtual Sequence< sal_Int32 > SAL_CALL getAttrLong(void)  throw( RuntimeException );
    virtual void SAL_CALL setAttrLong(const Sequence< sal_Int32 >& AttrLong_) throw( RuntimeException );
    virtual Sequence< sal_uInt32 > SAL_CALL getAttrULong(void)  throw( RuntimeException );
    virtual void SAL_CALL setAttrULong(const Sequence< sal_uInt32 >& AttrULong_) throw( RuntimeException );
    virtual Sequence< OUString > SAL_CALL getAttrString(void)  throw(RuntimeException );
    virtual void SAL_CALL setAttrString(const Sequence< OUString >& AttrString_) throw( RuntimeException );
    virtual Sequence< sal_Unicode > SAL_CALL getAttrChar(void)  throw( RuntimeException );
    virtual void SAL_CALL setAttrChar(const Sequence< sal_Unicode >& AttrChar_) throw( RuntimeException );
    virtual Sequence< Any > SAL_CALL getAttrAny(void)  throw( RuntimeException );
    virtual void SAL_CALL setAttrAny(const Sequence< Any >& AttrAny_) throw( RuntimeException );
    virtual Sequence< Sequence< sal_Int32 > > SAL_CALL getAttrSequence(void) throw( RuntimeException );
    virtual void SAL_CALL  setAttrSequence(const Sequence< Sequence< sal_Int32 > >& AttrSequence_) throw( RuntimeException );
    virtual Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL getAttrSequence2(void)  throw( RuntimeException );
    virtual void SAL_CALL setAttrSequence2(const Sequence< Sequence< Sequence< sal_Int32 > > >& AttrSequence2_) throw ( RuntimeException );
    virtual Sequence< Reference< XInterface > > SAL_CALL getAttrXInterface() throw(RuntimeException);
    virtual void SAL_CALL setAttrXInterface( const Sequence< Reference< XInterface > >& _attrxinterface ) throw(RuntimeException);

    virtual void SAL_CALL testout_methodByte(sal_Int8& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodFloat(float& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodDouble(double& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodBool(sal_Bool& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodShort(sal_Int16& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodUShort(sal_uInt16& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodLong(sal_Int32& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodULong(sal_uInt32& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodString(OUString& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodChar(sal_Unicode& rOut) throw( RuntimeException );
    virtual void SAL_CALL testout_methodAny(Any& rOut) throw( RuntimeException );
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
    virtual void SAL_CALL testinout_methodString(OUString& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodChar(sal_Unicode& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodAny(Any& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodSequence(Sequence< sal_Int32 >& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodSequence2(Sequence< Sequence< sal_Int32 > >& rOut) throw( RuntimeException );
    virtual void SAL_CALL testinout_methodXInterface( Reference< XInvocation >& rOut ) throw(RuntimeException);
    virtual Any SAL_CALL methodAnyTest1(const Any& rIn) throw( RuntimeException ) ;
    virtual Any SAL_CALL getAttrAny2(void) throw( RuntimeException ) ;
    virtual void SAL_CALL setAttrAny2(const Any& AttrAny2_) throw( RuntimeException ) ;


    // XTestStruct
    virtual void SAL_CALL methodStruct(const Property& aProp) throw( RuntimeException );

    virtual Property SAL_CALL retMethodStruct(void) throw( RuntimeException );

    virtual Property SAL_CALL  getAttrStruct(void) throw( RuntimeException );
    virtual void SAL_CALL setAttrStruct(const Property& AttrStruct_) throw( RuntimeException );

    // XTestOther
    virtual void SAL_CALL other_methodAnyIn(const Any& rAny) throw( RuntimeException );
    virtual void SAL_CALL other_methodAnyOut(Any& rAny) throw( RuntimeException );
    virtual Any SAL_CALL other_methodAnyRet(void) throw( RuntimeException );
    virtual void SAL_CALL in_float( float val) throw ( RuntimeException);

    // XTestOutParameters ------------------------------------------------------------------------
    virtual void SAL_CALL out_test(sal_Int8 rIn) throw( RuntimeException );

    virtual void SAL_CALL out_methodByte(sal_Int8& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodFloat(float& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodDouble(double& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodBool(sal_Bool& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodShort(sal_Int16& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodUShort(sal_uInt16& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodLong(sal_Int32& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodULong(sal_uInt32& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodString(OUString& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodChar(sal_Unicode& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodAny(Any& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodSequence(Sequence< sal_Int32 >& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodSequence2(Sequence< Sequence< sal_Int32 > >& rOut) throw( RuntimeException );
    virtual void SAL_CALL out_methodMulParams1(sal_Int32& rout1, sal_Int32& rout2) throw( RuntimeException );
    virtual void SAL_CALL out_methodMulParams2(sal_Int32& rout1, sal_Int32& rout2, OUString& rout3) throw( RuntimeException );
    virtual void SAL_CALL out_methodMulParams3(const OUString& sin, OUString& sout) throw( RuntimeException );

    // XTestInterfaces --------------------------------------------------------------------------
    virtual void SAL_CALL testInterface(  const Reference< XCallback >& xCallback, sal_Int32 mode ) throw(RuntimeException);
    virtual void SAL_CALL testInterface2(  const Reference< XSimple >& xSimple, sal_Int32 mode ) throw(RuntimeException);
    // XSimple --------------------------------------------------------------------------
    void SAL_CALL func( const OUString &message) throw(::com::sun::star::uno::RuntimeException);
    OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

};


OComponent::~OComponent()
{
}


// Funktions ==============================================================================

Reference<XInterface> SAL_CALL OComponent_CreateInstance( const Reference<XMultiServiceFactory> & rSMgr ) throw(RuntimeException)
{
    Reference<XInterface> xService = *new OComponent( rSMgr );
    return xService;
}

Sequence<OUString> OComponent_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = SERVICE_NAME;//ODataInputStream_getImplementationName();

    return aRet;
}


extern "C" sal_Bool SAL_CALL component_writeInfo(   void * pServiceManager, void * pRegistryKey )
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
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!\n");
        }
    }
    return sal_False;
}

extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
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

extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
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
//Sequence< Reference< XInterface> > SAL_CALL methodXInterface ( const Sequence< Reference < XInterface> >& aSeq)
//  throw (RuntimeException)
Sequence< Reference< XInterface > > SAL_CALL OComponent::methodXInterface( const Sequence< Reference< XInterface > >& aSeq ) throw(RuntimeException)
{
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



//---------------------------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OComponent::getAttrByte(void) throw( RuntimeException)
{
    return m_seqByte;
}
void SAL_CALL OComponent::setAttrByte(const Sequence< sal_Int8 >& AttrByte_) throw( RuntimeException )
{
    m_seqByte= AttrByte_;
}
Sequence< float > SAL_CALL OComponent::getAttrFloat(void)  throw( RuntimeException )
{
    return m_seqFloat;
}
void SAL_CALL OComponent::setAttrFloat(const Sequence< float >& AttrFloat_) throw( RuntimeException )
{
    m_seqFloat= AttrFloat_;
}

Sequence< double > SAL_CALL OComponent::getAttrDouble(void)  throw( RuntimeException )
{
    return m_seqDouble;
}
void SAL_CALL OComponent::setAttrDouble(const Sequence< double >& AttrDouble_) throw( RuntimeException )
{
    m_seqDouble= AttrDouble_;
}

Sequence< sal_Bool > SAL_CALL OComponent::getAttrBool(void)  throw( RuntimeException)
{
    return m_seqBool;
}

void SAL_CALL OComponent::setAttrBool(const Sequence< sal_Bool >& AttrBool_) throw (RuntimeException )
{
    m_seqBool= AttrBool_;
}

Sequence< sal_Int16 > SAL_CALL OComponent::getAttrShort(void)  throw( RuntimeException)
{
    return m_seqShort;
}
void SAL_CALL OComponent::setAttrShort(const Sequence< sal_Int16 >& AttrShort_) throw( RuntimeException )
{
    m_seqShort= AttrShort_;
}

Sequence< sal_uInt16 > SAL_CALL OComponent::getAttrUShort(void) throw( RuntimeException )
{
    return m_seqUShort;
}
void SAL_CALL OComponent::setAttrUShort(const Sequence< sal_uInt16 >& AttrUShort_) throw( RuntimeException )
{
    m_seqUShort= AttrUShort_;
}

Sequence< sal_Int32 > SAL_CALL OComponent::getAttrLong(void)  throw( RuntimeException)
{
    return m_seqLong;
}
void SAL_CALL OComponent::setAttrLong(const Sequence< sal_Int32 >& AttrLong_) throw( RuntimeException )
{
    m_seqLong= AttrLong_;
}

Sequence< sal_uInt32 > SAL_CALL OComponent::getAttrULong(void)  throw( RuntimeException )
{
    return m_seqULong;
}
void SAL_CALL OComponent::setAttrULong(const Sequence< sal_uInt32 >& AttrULong_) throw( RuntimeException )
{
    m_seqULong= AttrULong_;
}

Sequence< OUString > SAL_CALL OComponent::getAttrString(void)  throw( RuntimeException )
{
    return m_seqString;
}
void SAL_CALL OComponent::setAttrString(const Sequence< OUString >& AttrString_) throw( RuntimeException )
{
    m_seqString= AttrString_;
}

Sequence< sal_Unicode > SAL_CALL OComponent::getAttrChar(void)  throw( RuntimeException )
{
    return m_seqChar;
}
void SAL_CALL OComponent::setAttrChar(const Sequence< sal_Unicode >& AttrChar_) throw( RuntimeException)
{
    m_seqChar= AttrChar_;
}

Sequence< Any > SAL_CALL OComponent::getAttrAny(void) throw( RuntimeException)
{
    return m_seqAny;
}
void SAL_CALL OComponent::setAttrAny(const Sequence< Any >& AttrAny_) throw( RuntimeException )
{
    m_seqAny= AttrAny_;
}

Sequence< Sequence< sal_Int32 > > SAL_CALL OComponent::getAttrSequence(void)  throw( RuntimeException)
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

Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL OComponent::getAttrSequence2(void)  throw( RuntimeException )
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

// ----------------------------------------------------------------------------

void SAL_CALL OComponent::testout_methodByte(sal_Int8& rOut) throw( RuntimeException )
{
    rOut= 111;
}
void SAL_CALL OComponent::testout_methodFloat(float& rOut) throw( RuntimeException )
{
    rOut= 3.14;
}
void SAL_CALL OComponent::testout_methodDouble(double& rOut) throw( RuntimeException )
{
    rOut= 3.14;
}

void SAL_CALL OComponent::testout_methodBool(sal_Bool& rOut) throw( RuntimeException )
{
    rOut= sal_True;
}
void SAL_CALL OComponent::testout_methodShort(sal_Int16& rOut) throw( RuntimeException )
{
    rOut= 222;
}
void SAL_CALL OComponent::testout_methodUShort(sal_uInt16& rOut) throw( RuntimeException )
{
    rOut= 333;
}
void SAL_CALL OComponent::testout_methodLong(sal_Int32& rOut) throw( RuntimeException )
{
    rOut = 444;
}
void SAL_CALL OComponent::testout_methodULong(sal_uInt32& rOut) throw( RuntimeException )
{
    rOut= 555;
}
void SAL_CALL OComponent::testout_methodString(OUString& rOut) throw( RuntimeException )
{
    rOut= L"a little string";
}
void SAL_CALL OComponent::testout_methodChar(sal_Unicode& rOut) throw( RuntimeException )
{
    rOut= 'A';
}
void SAL_CALL OComponent::testout_methodAny(Any& rOut) throw( RuntimeException)
{
    rOut <<= OUString(L"I am a string in an any");
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
void SAL_CALL OComponent::testout_methodMulParams3(const OUString& sin, OUString& sout) throw( RuntimeException )
{
    sout= L"Out Hallo!";
}
void SAL_CALL OComponent::testout_methodMulParams4( float in1, float& out1, sal_Int32 in2, sal_Int32& out2, sal_Int32 in3 ) throw(RuntimeException)
{
    out1= in1 + 1;
    out2= in2 + 1;
}

void SAL_CALL OComponent::testout_methodXInterface( Reference< XInterface >& rOut ) throw(RuntimeException)
{
    OUString string( L"Property Any");
    m_any <<= string;
    rOut= (XInterface*) static_cast<XTestSequence*>(this);
}

// INOUT
void SAL_CALL OComponent::testinout_methodByte(sal_Int8& rOut) throw( RuntimeException )
{
    rOut += 1;
}
void SAL_CALL OComponent::testinout_methodFloat(float& rOut) throw( RuntimeException )
{
    rOut += 1;
}

void SAL_CALL OComponent::testinout_methodDouble(double& rOut) throw( RuntimeException )
{
    rOut += 1;
}
void SAL_CALL OComponent::testinout_methodBool(sal_Bool& rOut) throw( RuntimeException )
{
    rOut= rOut== sal_True ? sal_False : sal_True;
}
void SAL_CALL OComponent::testinout_methodShort(sal_Int16& rOut) throw( RuntimeException )
{
    rOut += 1;
}
void SAL_CALL OComponent::testinout_methodUShort(sal_uInt16& rOut) throw( RuntimeException )
{
    rOut += 1;
}
void SAL_CALL OComponent::testinout_methodLong(sal_Int32& rOut) throw( RuntimeException )
{
    rOut += 1;
}
void SAL_CALL OComponent::testinout_methodULong(sal_uInt32& rOut) throw( RuntimeException )
{
    rOut += 1;
}
void SAL_CALL OComponent::testinout_methodString(OUString& rOut) throw( RuntimeException )
{
    rOut= rOut + L" out string";
}
void SAL_CALL OComponent::testinout_methodChar(sal_Unicode& rOut) throw( RuntimeException)
{
    rOut += 1;
}
void SAL_CALL OComponent::testinout_methodAny(Any& rOut) throw( RuntimeException)
{
    TypeClass t= rOut.getValueTypeClass();
}
void SAL_CALL OComponent::testinout_methodSequence(Sequence< sal_Int32 >& rOut) throw( RuntimeException)
{
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
        OUString string(L" this string was written in the UNO component to the inout pararmeter");
        any <<= string;
        rOut->setValue( OUString(L"value"), any);
}
Any SAL_CALL OComponent::methodAnyTest1(const Any& rIn) throw( RuntimeException )
{
    return rIn;
}
Any SAL_CALL OComponent::getAttrAny2(void)  throw( RuntimeException )
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
}

Property SAL_CALL OComponent::retMethodStruct(void) throw( RuntimeException )
{
    Property a(L"OleTest_Property", 255, getCppuType( (Reference<XInterface>*)0), PropertyAttribute::MAYBEVOID |
        PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED |
        PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY |
        PropertyAttribute::MAYBEAMBIGUOUS | PropertyAttribute::MAYBEDEFAULT );
    return a;
}

Property SAL_CALL OComponent::getAttrStruct(void) throw( RuntimeException)
{
    Property a(L"OleTest_Property", 255, getCppuType( (Reference<XInterface>*)0), PropertyAttribute::MAYBEVOID |
        PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED |
        PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY |
        PropertyAttribute::MAYBEAMBIGUOUS | PropertyAttribute::MAYBEDEFAULT );
    return a;

}

void SAL_CALL OComponent::setAttrStruct(const Property& AttrStruct_) throw( RuntimeException )
{}

// XTestOther ==================================================================================
void SAL_CALL OComponent::other_methodAnyIn(const Any& rAny) throw( RuntimeException )
{
}
void SAL_CALL OComponent::other_methodAnyOut(Any& rAny) throw( RuntimeException )
{
    rAny <<= OUString(L"Ein Any");
}

Any SAL_CALL OComponent::other_methodAnyRet(void) throw(RuntimeException )
{
    Any a;
    a <<= OUString(L"Ein Any");
    return a;
}
void SAL_CALL OComponent::in_float( float val) throw ( RuntimeException)
{
    char buff[256];
    sprintf( buff, "parameter : %f", val);
    MessageBox( NULL, _T(buff), _T("OleTest"), MB_OK);
}

// XTestOutParameters ============================================================================
void SAL_CALL OComponent::out_test(sal_Int8 rIn) throw( RuntimeException )
{
}

void SAL_CALL OComponent::out_methodByte(sal_Int8& rOut) throw( RuntimeException )
{
    rOut= 100;
}
void SAL_CALL OComponent::out_methodDouble(double& rOut) throw( RuntimeException )
{
    rOut= 3.14;
}
void SAL_CALL OComponent::out_methodFloat(float& rOut) throw( RuntimeException )
{
    rOut= 3.14;
}
void SAL_CALL OComponent::out_methodBool(sal_Bool& rOut) throw( RuntimeException )
{
    rOut= sal_True;
}
void SAL_CALL OComponent::out_methodShort(sal_Int16& rOut) throw( RuntimeException )
{
    rOut= -100;
}
void SAL_CALL OComponent::out_methodUShort(sal_uInt16& rOut) throw( RuntimeException )
{
    rOut= 100;
}
void SAL_CALL OComponent::out_methodLong(sal_Int32& rOut) throw( RuntimeException )
{
    rOut= -100;
}

void SAL_CALL OComponent::out_methodULong(sal_uInt32& rOut) throw( RuntimeException )
{
    rOut= 100;
}
void SAL_CALL OComponent::out_methodString(OUString& rOut) throw( RuntimeException )
{
    rOut= L"I'm a string";
}
void SAL_CALL OComponent::out_methodChar(sal_Unicode& rOut) throw( RuntimeException )
{
    rOut= 'A';
}
void SAL_CALL OComponent::out_methodAny(Any& rOut) throw( RuntimeException)
{
    Any a;
    a <<= OUString( L"Hi");
    rOut= a;
}
void SAL_CALL OComponent::out_methodSequence(Sequence< sal_Int32 >& rOut) throw( RuntimeException )
{
    Sequence< sal_Int32 > aseq(10);
    for( sal_Int16 i= 0; i < aseq.getLength(); i++) aseq.getArray()[i]= i;
    rOut= aseq;
}
void SAL_CALL OComponent::out_methodSequence2(Sequence< Sequence< sal_Int32 > >& rOut) throw( RuntimeException )
{
    rOut= Sequence< Sequence< sal_Int32 > >();
}
void SAL_CALL OComponent::out_methodMulParams1(sal_Int32& rout1, sal_Int32& rout2) throw( RuntimeException )
{
    rout1= 111;
    rout2= 222;
}
void SAL_CALL OComponent::out_methodMulParams2(sal_Int32& rout1, sal_Int32& rout2, OUString& rout3) throw( RuntimeException )
{
    rout1= 111;
    rout2= 222;
    rout3= L"this is a neet little string";
}
void SAL_CALL OComponent::out_methodMulParams3(const OUString& sin, OUString& sout) throw( RuntimeException )
{
    sout= L"this is a neet little string";
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
    sal_uInt16 aUShort;
    sal_uInt32 aULong;

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
            sal_uInt16  aUShort;
            sal_uInt32  aULong;
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
                pStruct->message= OUString::createFromAscii("This struct was created in OleTest");

                SimpleStruct aStruct;
                any >>= aStruct;
                xCallback->inoutStruct( aStruct);
                // a Struct should now contain a different message
                MessageBox( NULL, W2T(aStruct.message), _T("OleTest in out parameter"), MB_OK);
            }

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
            float f= 3.14;
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
                pStruct->message= OUString::createFromAscii("This struct was created in OleTest");
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
        float aFloat=3.14;
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

    // ############################################################################
    // Call a COM object that has not been passed as parameter to a UNO component and
    // hence no type information are available in the COM wrapper
    // ############################################################################
    case 300:
        {
            Reference<XInterface> xIntFact = m_rFactory->createInstance(L"com.sun.star.bridge.OleObjectFactory");

            Reference<XMultiServiceFactory> oleFact(xIntFact, UNO_QUERY);

            Reference<XInterface> xIntCallback= oleFact->createInstance(L"XCallback_Impl.Callback");
            Reference<XInvocation> xInv( xIntCallback, UNO_QUERY);
//Any SAL_CALL invoke( const OUString& aFunctionName, const Sequence<Any >& aParams,Sequence< sal_Int16 >& aOutParamIndex,
//          Sequence<Any >& aOutParam );
            Sequence<sal_Int16> seqIndizes;
            Sequence<Any> seqOutParams;
            xInv->invoke( OUString( L"outValuesAll"), Sequence<Any>(), seqIndizes, seqOutParams);

//          void outValuesAll( [out] oletest::XSimple outInterface,
//                     [out] SimpleStruct outStruct ,
//                     [out] SimpleEnum outEnum,
//                     [out] sequence<any> outSeqAny,
//                     [out] any outAny,
//                     [out] boolean outBool,
//                     [out] char outChar,
//                     [out] string outString,
//                     [out] float outFloat,
//                     [out] double outDouble,
//                     [out] byte   outByte,
//                     [out] short outShort,
//                     [out] long outLong);
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
            Reference<XInterface> xIntFact = m_rFactory->createInstance(L"com.sun.star.bridge.OleObjectFactory");

            Reference<XMultiServiceFactory> oleFact(xIntFact, UNO_QUERY);

            Reference<XInterface> xIntCallback= oleFact->createInstance(L"XCallback_Impl.Callback");
            Reference<XInvocation> xInv( xIntCallback, UNO_QUERY);
            Sequence<sal_Int16> seqIndizes;
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
                    pStruct->message= OUString::createFromAscii("This struct was created in OleTest");
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
            float _float= 3.14;
            arAny[8] <<= _float;
            double _double= 3.145;
            arAny[9] <<= _double;
            sal_Int8 _byte= 0xff;
            arAny[10] <<= _byte;
            sal_Int16 _short= 0xffff;
            arAny[11] <<= _short;
            sal_Int32 _long= 0xffffffff;
            arAny[12] <<= _long;
//
            Sequence<Any> params( arAny, 13);

            xInv->invoke( OUString( L"inoutValuesAll"), params, seqIndizes, seqOutParams);

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
        // in parameter
//  void inValues( [in] char aChar, [in] long aLong, [in] string aString);
        {
            Reference<XInterface> xIntFact = m_rFactory->createInstance(L"com.sun.star.bridge.OleObjectFactory");

            Reference<XMultiServiceFactory> oleFact(xIntFact, UNO_QUERY);

            Reference<XInterface> xIntCallback= oleFact->createInstance(L"XCallback_Impl.Callback");
            Reference<XInvocation> xInv( xIntCallback, UNO_QUERY);
            Sequence<sal_Int16> seqIndizes;
            Sequence<Any> seqOutParams;

            Any arAny[3];
            sal_Unicode aChar=L'a';
            arAny[0] <<= aChar;
            sal_Int32 aLong= 0xffffffff;
            arAny[1] <<= aLong;
            OUString aString(L" a string parameter");
            arAny[2] <<= aString;

            xInv->invoke( OUString( L"inValues"), Sequence<Any>(arAny,3), seqIndizes, seqOutParams);

            break;
        }




    }

}

// XSimple --------------------------------------------------------------------------
void SAL_CALL OComponent::func( const OUString &message)throw(::com::sun::star::uno::RuntimeException)
{
    USES_CONVERSION;

    MessageBox( NULL, W2T( message.getStr()), _T("OleTest: XSimple::func"), MB_OK);
}
// XSimple --------------------------------------------------------------------------
OUString SAL_CALL OComponent::getName()throw(::com::sun::star::uno::RuntimeException)
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
