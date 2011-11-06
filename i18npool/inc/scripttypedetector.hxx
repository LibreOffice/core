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


#ifndef _I18N_SCRIPTTYPEDETECTOR_HXX_
#define _I18N_SCRIPTTYPEDETECTOR_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/i18n/XScriptTypeDetector.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations

//  ----------------------------------------------------
//  class ScriptTypeDetector
//  ----------------------------------------------------
class ScriptTypeDetector : public cppu::WeakImplHelper2
<
    ::com::sun::star::i18n::XScriptTypeDetector,
    ::com::sun::star::lang::XServiceInfo
>
{
public:
    ScriptTypeDetector();
    virtual ~ScriptTypeDetector();

    // Methods
    virtual sal_Int32 SAL_CALL beginOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL beginOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);


    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( ::com::sun::star::uno::RuntimeException );
};

#endif
