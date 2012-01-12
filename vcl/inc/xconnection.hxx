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



#ifndef _VCL_XCONNECTION_HXX
#define _VCL_XCONNECTION_HXX

#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include "vcl/displayconnectiondispatch.hxx"

#ifndef _STLP_LIST
#include <list>
#endif

namespace vcl {

    class DisplayConnection :
        public DisplayConnectionDispatch
    {
        ::osl::Mutex                    m_aMutex;
        ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler > >
                                        m_aHandlers;
        ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler > >
                                        m_aErrorHandlers;
        ::com::sun::star::uno::Any      m_aAny;
    public:
        DisplayConnection();
        virtual ~DisplayConnection();

        void start();
        void terminate();

        virtual bool dispatchEvent( void* pData, int nBytes );
        virtual bool dispatchErrorEvent( void* pData, int nBytes );

        // XDisplayConnection
        virtual void SAL_CALL addEventHandler( const ::com::sun::star::uno::Any& window, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler, sal_Int32 eventMask ) throw();
        virtual void SAL_CALL removeEventHandler( const ::com::sun::star::uno::Any& window, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual void SAL_CALL addErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual void SAL_CALL removeErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual ::com::sun::star::uno::Any SAL_CALL getIdentifier() throw();

    };

}

#endif // _VCL_XCONNECTION_HXX
