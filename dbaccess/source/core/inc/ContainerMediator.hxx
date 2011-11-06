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


#ifndef DBA_CONTAINERMEDIATOR_HXX
#define DBA_CONTAINERMEDIATOR_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include <map>

//........................................................................
namespace dbaccess
{
//........................................................................

    class OPropertyForward;

    class OContainerMediator :   public ::comphelper::OBaseMutex
                                ,public ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener >
    {
    public:
        enum ContainerType
        {
            eColumns,
            eTables
        };

    private:
        typedef ::rtl::Reference< OPropertyForward >                TPropertyForward;
        typedef ::std::map< ::rtl::OUString, TPropertyForward >     PropertyForwardList;
        PropertyForwardList                                                             m_aForwardList;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xSettings;    // can not be weak
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >     m_xContainer;   // can not be weak
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection >     m_aConnection;
        ContainerType                                                                   m_eType;

    protected:
        virtual ~OContainerMediator();

    public:
        OContainerMediator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xSettings,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            ContainerType _eType
       );

        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        void notifyElementCreated(const ::rtl::OUString& _sElementName
                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xElement);

    private:
        /** cleans up the instance, by deregistering as listener at the containers,
            and resetting them to <NULL/>
        */
        void    impl_cleanup_nothrow();

        /** initializes the properties of the given object from its counterpart in our settings container
        */
        void    impl_initSettings_nothrow(
                    const ::rtl::OUString& _rName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDestination
                );
    };
//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // DBA_CONTAINERMEDIATOR_HXX
