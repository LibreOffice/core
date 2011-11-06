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



#ifndef _BASIC_MODSIZEEXCEEDED_HXX
#define _BASIC_MODSIZEEXCEEDED_HXX

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <cppuhelper/implbase1.hxx>

class ModuleSizeExceeded : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    // C++ interface
    public:
    ModuleSizeExceeded( const com::sun::star::uno::Sequence< ::rtl::OUString>& sModules );

    sal_Bool isAbort() const;
    sal_Bool isApprove() const;

    // UNO interface
    public:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException ) { return m_lContinuations; }
    com::sun::star::uno::Any SAL_CALL getRequest() throw( com::sun::star::uno::RuntimeException )
    {
        return m_aRequest;
    }

    // member
    private:
    rtl::OUString m_sMods;
    com::sun::star::uno::Any m_aRequest;
    com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > > m_lContinuations;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > m_xAbort;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation> m_xApprove;
};

#endif

