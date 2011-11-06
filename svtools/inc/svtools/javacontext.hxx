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



#ifndef _SVTOOLS_JAVACONTEXT_HXX_
#define _SVTOOLS_JAVACONTEXT_HXX_

#include "svtools/svtdllapi.h"
#include <osl/mutex.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>


namespace svt
{
// We cannot derive from  cppu::WeakImplHelper because we would export the inline
//generated class. This conflicts with other libraries if they use the same inline
//class.
    class SVT_DLLPUBLIC JavaContext :
        public com::sun::star::uno::XCurrentContext
    {

    public:
        JavaContext( const com::sun::star::uno::Reference<
                     com::sun::star::uno::XCurrentContext> & ctx);

        /** The parameter bShowErrorsOnce controls whether a message box is
            only displayed once for a reocurring Java error. That is only
            the first time JavaInteractionHandler.handle is called with a
            particular Request then the message box is shown. Afterwards
            nothing happens.
         */
        JavaContext( const com::sun::star::uno::Reference<
                     com::sun::star::uno::XCurrentContext> & ctx,
                     bool bReportErrorOnce);
        virtual ~JavaContext();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL acquire() throw ();

        virtual void SAL_CALL release() throw ();

        // XCurrentContext
        virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
            throw (com::sun::star::uno::RuntimeException);

    private:
        SVT_DLLPRIVATE JavaContext(); //not implemented
        SVT_DLLPRIVATE JavaContext(JavaContext&); //not implemented
        SVT_DLLPRIVATE JavaContext& operator = (JavaContext&); //not implemented

        oslInterlockedCount m_aRefCount;

        com::sun::star::uno::Reference<
            com::sun::star::uno::XCurrentContext > m_xNextContext;
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionHandler> m_xHandler;
        bool m_bShowErrorsOnce;
    };
}

#endif
