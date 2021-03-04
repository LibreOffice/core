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
#pragma once

#include <com/sun/star/uno/XComponentContext.hpp>
#include <jvmaccess/virtualmachine.hxx>

#ifdef HAVE_64BIT_POINTERS
#error "no 64 bit pointer"
#endif //HAVE_64BIT_POINTERS

namespace comphelper
{
    class EventLogger;
}

namespace connectivity
{
    class SDBThreadAttach
    {
        jvmaccess::VirtualMachine::AttachGuard m_aGuard;
        SDBThreadAttach(SDBThreadAttach const &) = delete;
        SDBThreadAttach& operator= (SDBThreadAttach const &) = delete;
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


    class  java_lang_Object
    {
        java_lang_Object& operator= (java_lang_Object const &) = delete;
        java_lang_Object(java_lang_Object const &) = delete;

    protected:
        // The Java handle to this class
        jobject object;

        // Class definition
        // New in SJ2:
        static jclass theClass; // The class needs to be requested only once!

        virtual jclass getMyClass() const;

    public:
        // Ctor that should be used for the derived classes
        java_lang_Object( JNIEnv * pEnv, jobject myObj );

        // The actual ctor
        java_lang_Object();

        virtual ~java_lang_Object() COVERITY_NOEXCEPT_FALSE;

        void                saveRef( JNIEnv * pEnv, jobject myObj );
        jobject             getJavaObject() const { return object; }
        void clearObject(JNIEnv& rEnv);
        void clearObject();

        OUString toString() const;

        static void ThrowSQLException(JNIEnv * pEnv,const css::uno::Reference< css::uno::XInterface> & _rContext);
        static void ThrowLoggedSQLException(
            const ::comphelper::EventLogger& _rLogger,
            JNIEnv* pEnvironment,
            const css::uno::Reference< css::uno::XInterface >& _rxContext
        );
        static void ThrowRuntimeException(JNIEnv * pEnv,const css::uno::Reference< css::uno::XInterface> & _rContext);

        static ::rtl::Reference< jvmaccess::VirtualMachine > getVM(const css::uno::Reference< css::uno::XComponentContext >& _rxContext=nullptr);

        static jclass   findMyClass(const char* _pClassName);
        void            obtainMethodId_throwSQL(JNIEnv* _pEnv, const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID) const;
        void            obtainMethodId_throwRuntime(JNIEnv* _pEnv, const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID) const;

        bool        callBooleanMethod( const char* _pMethodName, jmethodID& _inout_MethodID ) const;
        bool        callBooleanMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const;
        jobject         callResultSetMethod( JNIEnv& _rEnv, const char* _pMethodName, jmethodID& _inout_MethodID ) const;
        sal_Int32       callIntMethod_ThrowSQL(const char* _pMethodName, jmethodID& _inout_MethodID) const;
        sal_Int32       callIntMethod_ThrowRuntime(const char* _pMethodName, jmethodID& _inout_MethodID) const;
        sal_Int32       callIntMethodWithIntArg_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const;
        sal_Int32       callIntMethodWithIntArg_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const;
        sal_Int32       callIntMethodWithStringArg( const char* _pMethodName, jmethodID& _inout_MethodID,const OUString& _nArgument ) const;
        OUString callStringMethod( const char* _pMethodName, jmethodID& _inout_MethodID ) const;
        OUString callStringMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID , sal_Int32 _nArgument) const;
        void            callVoidMethod_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID) const;
        void            callVoidMethod_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID) const;
        void            callVoidMethodWithIntArg_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const;
        void            callVoidMethodWithIntArg_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const;
        void            callVoidMethodWithBoolArg_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID, bool _nArgument ) const;
        void            callVoidMethodWithBoolArg_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID, bool _nArgument ) const;
        void            callVoidMethodWithStringArg( const char* _pMethodName, jmethodID& _inout_MethodID, const OUString& _nArgument ) const;
        jobject         callObjectMethod( JNIEnv * pEnv, const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID ) const;
        jobject         callObjectMethodWithIntArg( JNIEnv * pEnv, const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID , sal_Int32 _nArgument) const;

        template< typename T >
                        T callMethodWithIntArg(T (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) ,const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID , sal_Int32 _nArgument) const
        {
            SDBThreadAttach t;
            obtainMethodId_throwSQL(t.pEnv, _pMethodName,_pSignature, _inout_MethodID);
            T out = (t.pEnv->*pCallMethod)( object, _inout_MethodID,_nArgument);
            ThrowSQLException( t.pEnv, nullptr );
            return out;
        }

        template< typename T >
                        void callVoidMethod_ThrowSQL(const char* _pMethodName, const char* _pSignature, jmethodID& _inout_MethodID,sal_Int32 _nArgument, const T& _aValue) const
        {
            SDBThreadAttach t;
            obtainMethodId_throwSQL(t.pEnv, _pMethodName,_pSignature, _inout_MethodID);
            t.pEnv->CallVoidMethod( object, _inout_MethodID,_nArgument,_aValue);
            ThrowSQLException( t.pEnv, nullptr );
        }


    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
