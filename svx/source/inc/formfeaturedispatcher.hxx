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

#ifndef INCLUDED_SVX_SOURCE_INC_FORMFEATUREDISPATCHER_HXX
#define INCLUDED_SVX_SOURCE_INC_FORMFEATUREDISPATCHER_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/form/runtime/XFormOperations.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>


namespace svx
{

    typedef ::cppu::WeakImplHelper <   css::frame::XDispatch
                                    >   OSingleFeatureDispatcher_Base;

    class OSingleFeatureDispatcher final : public OSingleFeatureDispatcher_Base
    {
    public:
        /** constructs the dispatcher

            @param _rFeatureURL
                the URL of the feature which this instance is responsible for

            @param _nFeatureId
                the feature which this instance is responsible for

            @param _rController
                the controller which is responsible for providing the state of feature of this instance,
                and for executing it. After disposing the dispatcher instance, the controller will
                not be accessed anymore

            @see dispose
        */
        OSingleFeatureDispatcher(
            const css::util::URL& _rFeatureURL,
            const sal_Int16 _nFormFeature,
            const css::uno::Reference< css::form::runtime::XFormOperations >& _rxFormOperations,
            ::osl::Mutex& _rMutex
        );

        /** notifies all our listeners of the current state
        */
        void    updateAllListeners();

    private:
        // XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL& _rURL, const css::uno::Sequence< css::beans::PropertyValue >& _rArguments ) override;
        virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& _rxControl, const css::util::URL& _rURL ) override;
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& _rxControl, const css::util::URL& _rURL ) override;

        /** notifies our current state to one or all listeners

            @param _rxListener
                the listener to notify. May be NULL, in this case all our listeners will be
                notified with the current state

            @param _rFreeForNotification
                a guard which currently locks our mutex, and which is to be cleared
                for actually doing the notification(s)
        */
        void    notifyStatus(
                    const css::uno::Reference< css::frame::XStatusListener >& _rxListener,
                    ::osl::ClearableMutexGuard& _rFreeForNotification
                );

        /** retrieves the current status of our feature, in a format which can be used
            for UNO notifications

            @precond
                our mutex is locked
        */
        void    getUnoState( css::frame::FeatureStateEvent& /* [out] */ _rState ) const;

        ::osl::Mutex&                       m_rMutex;
        ::comphelper::OInterfaceContainerHelper2   m_aStatusListeners;
        css::uno::Reference< css::form::runtime::XFormOperations >
                                            m_xFormOperations;
        const css::util::URL                m_aFeatureURL;
        css::uno::Any                       m_aLastKnownState;
        const sal_Int16                     m_nFormFeature;
        bool                                m_bLastKnownEnabled;

    };


}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
