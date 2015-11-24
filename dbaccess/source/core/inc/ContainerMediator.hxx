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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_CONTAINERMEDIATOR_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_CONTAINERMEDIATOR_HXX

#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <map>

namespace dbaccess
{

    class OPropertyForward;

    class OContainerMediator :   public ::comphelper::OBaseMutex
                                ,public ::cppu::WeakImplHelper< css::container::XContainerListener >
    {
    private:
        typedef ::rtl::Reference< OPropertyForward >          TPropertyForward;
        typedef ::std::map< OUString, TPropertyForward >      PropertyForwardList;
        PropertyForwardList                                   m_aForwardList;
        css::uno::Reference< css::container::XNameAccess >    m_xSettings;    // can not be weak
        css::uno::Reference< css::container::XContainer >     m_xContainer;   // can not be weak

    protected:
        virtual ~OContainerMediator();

    public:
        OContainerMediator(
            const css::uno::Reference< css::container::XContainer >& _xContainer,
            const css::uno::Reference< css::container::XNameAccess >& _xSettings
       );

        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

        void notifyElementCreated(const OUString& _sElementName
                                ,const css::uno::Reference< css::beans::XPropertySet>& _xElement);

    private:
        /** cleans up the instance, by deregistering as listener at the containers,
            and resetting them to <NULL/>
        */
        void    impl_cleanup_nothrow();

        /** initializes the properties of the given object from its counterpart in our settings container
        */
        void    impl_initSettings_nothrow(
                    const OUString& _rName,
                    const css::uno::Reference< css::beans::XPropertySet >& _rxDestination
                );
    };

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_CONTAINERMEDIATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
