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
#ifndef DBA_CONTAINERMEDIATOR_HXX
#define DBA_CONTAINERMEDIATOR_HXX

#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>

#include <map>

namespace dbaccess
{

    class OPropertyForward;

    class OContainerMediator :   public ::comphelper::OBaseMutex
                                ,public ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener >
    {
    private:
        typedef ::rtl::Reference< OPropertyForward >                TPropertyForward;
        typedef ::std::map< ::rtl::OUString, TPropertyForward >     PropertyForwardList;
        PropertyForwardList                                                             m_aForwardList;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xSettings;    // can not be weak
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >     m_xContainer;   // can not be weak
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection >     m_aConnection;

    protected:
        virtual ~OContainerMediator();

    public:
        OContainerMediator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xSettings,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
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

}   // namespace dbaccess

#endif // DBA_CONTAINERMEDIATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
