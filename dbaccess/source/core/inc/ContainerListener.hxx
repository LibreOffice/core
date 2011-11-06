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


#ifndef DBA_CONTAINERLISTENER_HXX
#define DBA_CONTAINERLISTENER_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERAPPROVELISTENER_HPP_
#include <com/sun/star/container/XContainerApproveListener.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    //==========================================================================
    //= OContainerListener
    //==========================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::container::XContainerListener
                                    ,   ::com::sun::star::container::XContainerApproveListener
                                    >   OContainerListener_BASE;

    /** is helper class to avoid a cycle in refcount
    */
    class OContainerListener : public OContainerListener_BASE
    {
        ::osl::Mutex&       m_rMutex;
        OWeakObject&        m_rDestination;
        bool                m_bDisposed;

    public:
        OContainerListener( OWeakObject& _rDestination, ::osl::Mutex& _rMutex )
            :m_rMutex( _rMutex )
            ,m_rDestination( _rDestination )
            ,m_bDisposed( false )
        {
        }

        // XContainerApproveListener
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveInsertElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveReplaceElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveRemoveElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        void SAL_CALL dispose()
        {
            m_bDisposed = true;
        }

    protected:
        virtual ~OContainerListener();
    };
//........................................................................
}   // namespace dbaccess
//........................................................................
#endif // DBA_CONTAINERLISTENER_HXX

