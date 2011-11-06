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


#ifndef CONNECTIVITY_HSQLDB_TERMINATELISTENER_HXX
#define CONNECTIVITY_HSQLDB_TERMINATELISTENER_HXX

#include <cppuhelper/compbase1.hxx>
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    namespace hsqldb
    {
        class ODriverDelegator;
        class OConnectionController : public ::cppu::WeakImplHelper1<
                                                    ::com::sun::star::frame::XTerminateListener >
        {
            ODriverDelegator* m_pDriver;
            protected:
                virtual ~OConnectionController() {m_pDriver = NULL;}
            public:
                OConnectionController(ODriverDelegator* _pDriver) : m_pDriver(_pDriver){}

                // XEventListener
                virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
                    throw( ::com::sun::star::uno::RuntimeException );

                // XTerminateListener
                virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
                    throw( ::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
                    throw( ::com::sun::star::uno::RuntimeException );
        };
    }
//........................................................................
}   // namespace connectivity
//........................................................................
#endif // CONNECTIVITY_HSQLDB_TERMINATELISTENER_HXX
