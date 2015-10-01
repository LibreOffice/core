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

#ifndef INCLUDED_FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX
#define INCLUDED_FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX

#include <com/sun/star/form/XResetListener.hpp>
#include <comphelper/listenernotification.hxx>
#include <cppuhelper/weak.hxx>


namespace frm
{

    template < class LISTENER >
    class EventListeners : public ::comphelper::OListenerContainerBase< LISTENER, css::lang::EventObject >
    {
    public:
        typedef LISTENER                            ListenerClass;
        typedef css::lang::EventObject              EventClass;
        typedef ::comphelper::OListenerContainerBase< ListenerClass, EventClass >
                                                    EventListeners_Base;

    private:
        ::cppu::OWeakObject&    m_rInstigator;

    protected:
        inline EventListeners( ::cppu::OWeakObject& _rInstigator, ::osl::Mutex& _rMutex )
            :EventListeners_Base( _rMutex )
            ,m_rInstigator( _rInstigator )
        {
        }

    public:
        inline bool notify()
        {
            css::lang::EventObject aEvent( m_rInstigator );
            return EventListeners_Base::notify( aEvent );
        }

        inline void disposing()
        {
            css::lang::EventObject aEvent( m_rInstigator );
            EventListeners_Base::disposing( aEvent );
        }
    protected:
        using EventListeners_Base::notify;
        using EventListeners_Base::disposing;
    };

    typedef EventListeners  <   css::form::XResetListener
                            >   ResetListeners_Base;
    class ResetListeners : public ResetListeners_Base
    {
    private:
        enum NotificationType
        {
            eApproval,
            eFinal
        };
        NotificationType        m_eCurrentNotificationType;

    public:
        inline ResetListeners( ::cppu::OWeakObject& _rInstigator, ::osl::Mutex& _rMutex )
            :ResetListeners_Base( _rInstigator, _rMutex )
            ,m_eCurrentNotificationType( eApproval )
        {
        }

        /** see whether all our listeners approve the reset
        */
        bool approveReset()
        {
            m_eCurrentNotificationType = eApproval;
            return notify();
        }

        /** tell all our listeners that the reset happened
        */
        void resetted()
        {
            m_eCurrentNotificationType = eFinal;
            notify();
        }

    protected:
        virtual bool    implTypedNotify(
                            const css::uno::Reference< css::form::XResetListener >& _rxListener,
                            const css::lang::EventObject& _rEvent
                        ) SAL_OVERRIDE;
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
