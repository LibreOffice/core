/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#define _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_

// jni.h needs cstdarg for std::va_list
#include <cstdarg>
#include <osl/thread.h>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <osl/diagnose.h>
#include <jvmaccess/virtualmachine.hxx>
#include <memory>
#include <functional>
#include <boost/shared_ptr.hpp>

//=====================================================================

#ifdef HAVE_64BIT_POINTERS
#error "no 64 bit pointer"
#else
#ifdef OS2
#define PVOID_TO_INT64(x) (jlong)(sal_Int32)x
#define INT64_TO_PVOID(x) (void *)x
#endif // OS2
#endif //HAVE_64BIT_POINTERS

namespace comphelper
{
    class ResourceBasedEventLogger;
}

namespace connectivity
{
    typedef ::boost::shared_ptr< jvmaccess::VirtualMachine::AttachGuard> TGuard;
    class SDBThreadAttach
    {
        jvmaccess::VirtualMachine::AttachGuard m_aGuard;
        SDBThreadAttach(SDBThreadAttach&);
        SDBThreadAttach& operator= (SDBThreadAttach&);
    public:
        SDBThreadAttach();
        ~SDBThreadAttach();

        JNIEnv* pEnv;
        static void addRef();
        static void releaseRef();

    public:
        JNIEnv& env() const
        {
            // according to the documentation of jvmaccess::VirtualMachine::AttachGuard, our env is never
            // NULL, so why bothering with pointer checks?
            return *pEnv;
        }
    };
    //=====================================================================
    //=====================================================================
    class java_lang_Class;
    class  java_lang_Object
    {
        // Zuweisungsoperator und Copy Konstruktor sind verboten
        java_lang_Object& operator= (java_lang_Object&);
        java_lang_Object(java_lang_Object&);

        // nur zum Zerstoeren des C++ Pointers in vom JSbxObject
        // abgeleiteten Java Objekten
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    protected:
        // der JAVA Handle zu dieser Klasse
        jobject object;
        // Klassendefinition

        // neu in SJ2:
        static jclass theClass;             // die Klasse braucht nur einmal angefordert werden !

        virtual jclass getMyClass() const;

    public:
        // der Konstruktor, der fuer die abgeleiteten Klassen verwendet
        // werden soll.
        java_lang_Object( JNIEnv * pEnv, jobject myObj );
        // der eigentliche Konstruktor
        java_lang_Object(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory=NULL);

        virtual ~java_lang_Object();

        void                saveRef( JNIEnv * pEnv, jobject myObj );
        jobject             getJavaObject() const { return object; }
        java_lang_Object *  GetWrapper() { return this; }
        void clearObject(JNIEnv& rEnv);
        void clearObject();

        virtual ::rtl::OUString toString() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() { return m_xFactory; }

        static void ThrowSQLException(JNIEnv * pEnv,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> & _rContext);
        static void ThrowLoggedSQLException(
            const ::comphelper::ResourceBasedEventLogger& _rLogger,
            JNIEnv* pEnvironment,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext
        );

        static ::rtl::Reference< jvmaccess::VirtualMachine > getVM(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory=NULL);

        static jclass   findMyClass(const char* _pClassName);
        void            obtainMethodId(JNIEnv* _pEnv, const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID) const;

        sal_Bool        callBooleanMethod( const char* _pMethodName, jmethodID& _inout_MethodID ) const;
        sal_Bool        callBooleanMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const;
        jobject         callResultSetMethod( JNIEnv& _rEnv, const char* _pMethodName, jmethodID& _inout_MethodID ) const;
        sal_Int32       callIntMethod( const char* _pMethodName, jmethodID& _inout_MethodID,bool _bIgnoreException = false ) const;
        sal_Int32       callIntMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const;
        sal_Int32       callIntMethodWithStringArg( const char* _pMethodName, jmethodID& _inout_MethodID,const ::rtl::OUString& _nArgument ) const;
        ::rtl::OUString callStringMethod( const char* _pMethodName, jmethodID& _inout_MethodID ) const;
        ::rtl::OUString callStringMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID , sal_Int32 _nArgument) const;
        void            callVoidMethod( const char* _pMethodName, jmethodID& _inout_MethodID) const;
        void            callVoidMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument,bool _bIgnoreException = false ) const;
        void            callVoidMethodWithBoolArg( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument,bool _bIgnoreException = false ) const;
        void            callVoidMethodWithStringArg( const char* _pMethodName, jmethodID& _inout_MethodID, const ::rtl::OUString& _nArgument ) const;
        jobject         callObjectMethod( JNIEnv * pEnv, const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID ) const;
        jobject         callObjectMethodWithIntArg( JNIEnv * pEnv, const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID , sal_Int32 _nArgument) const;

        template< typename T >
                        T callMethodWithIntArg(T (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) ,const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID , sal_Int32 _nArgument) const
        {
            SDBThreadAttach t;
            obtainMethodId(t.pEnv, _pMethodName,_pSignature, _inout_MethodID);
            T out = (t.pEnv->*pCallMethod)( object, _inout_MethodID,_nArgument);
            ThrowSQLException( t.pEnv, NULL );
            return out;
        }

        template< typename T >
                        void callVoidMethod(const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID,sal_Int32 _nArgument, const T& _aValue) const
        {
            SDBThreadAttach t;
            obtainMethodId(t.pEnv, _pMethodName,_pSignature, _inout_MethodID);
            t.pEnv->CallVoidMethod( object, _inout_MethodID,_nArgument,_aValue);
            ThrowSQLException( t.pEnv, NULL );
        }


    };
}
#endif //_CONNECTIVITY_JAVA_LANG_OBJJECT_HXX_


