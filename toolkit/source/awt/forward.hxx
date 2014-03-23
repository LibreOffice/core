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



#ifndef AWT_FORWARD_HXX
#define AWT_FORWARD_HXX

#include <comphelper/uno3.hxx>

#define IMPLEMENT_FORWARD_XTYPEPROVIDER1( classname, baseclass ) \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL classname::getTypes(  ) throw (::com::sun::star::uno::RuntimeException) \
    { return baseclass::getTypes(); } \
    IMPLEMENT_GET_IMPLEMENTATION_ID( classname )

#define IMPLEMENT_2_FORWARD_XINTERFACE1( classname, refcountbase1, refcountbase2 ) \
    void SAL_CALL classname::acquire() throw() { refcountbase1::acquire(); refcountbase2::acquire(); } \
    void SAL_CALL classname::release() throw() { refcountbase1::release(); refcountbase2::release(); } \
    ::com::sun::star::uno::Any SAL_CALL classname::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aReturn = refcountbase1::queryInterface( _rType ); \
        if ( !aReturn.hasValue() ) \
        { \
            aReturn = refcountbase2::queryInterface( _rType ); \
        } \
        return aReturn; \
    }

#define IMPLEMENT_2_FORWARD_XINTERFACE2( classname, refcountbase1, refcountbase2, baseclass3 ) \
    void SAL_CALL classname::acquire() throw() { refcountbase1::acquire(); refcountbase2::acquire(); } \
    void SAL_CALL classname::release() throw() { refcountbase1::release(); refcountbase2::release(); } \
    ::com::sun::star::uno::Any SAL_CALL classname::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aReturn = refcountbase1::queryInterface( _rType ); \
        if ( !aReturn.hasValue() ) \
        { \
            aReturn = refcountbase2::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
                aReturn = baseclass3::queryInterface( _rType ); \
        } \
        return aReturn; \
    }

// fix to remove a type ambiguity e.g.
//    class layoutimpl::VCLXDialog -> class VCLXWindow -> VCLXWindow_Base -> class VCLXDevice -> ::cppu::OWeakObject
//    class layoutimpl::VCLXDialog -> class layoutimpl::Bin -> class layoutimpl::Container -> Container_Base -> class cppu::OWeakObject
//    class layoutimpl::VCLXDialog -> class layoutimpl::Bin -> class layoutimpl::Container -> class layoutimpl::PropHelper -> cppu::OWeakObjec
#define W3K_EXPLICIT_CAST(x) static_cast <XWindow2*> (&x)

#endif /* AWT_FORWARD_HXX */
