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

#ifndef INCLUDED_COMPHELPER_CONTAINERMULTIPLEXER_HXX
#define INCLUDED_COMPHELPER_CONTAINERMULTIPLEXER_HXX

#include <com/sun/star/container/XContainer.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{


    class OContainerListenerAdapter;

    //= OContainerListener

    /** a non-UNO container listener
        <p>Useful if you have a non-refcountable class which should act as container listener.<br/>
        In this case, derive this class from OContainerListener, and create an adapter
        OContainerListenerAdapter which multiplexes the changes.</p>
    */
    class COMPHELPER_DLLPUBLIC OContainerListener
    {
        friend class OContainerListenerAdapter;
    protected:
        OContainerListenerAdapter*  m_pAdapter;
        ::osl::Mutex&               m_rMutex;

    public:
        OContainerListener(::osl::Mutex& _rMutex);
        virtual ~OContainerListener();

        virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent )
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception);
        virtual void _elementRemoved( const ::com::sun::star::container::ContainerEvent& _Event )
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception);
        virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent )
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception);
        virtual void _disposing(const ::com::sun::star::lang::EventObject& _rSource)
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception);

    protected:
        void setAdapter(OContainerListenerAdapter* _pAdapter);
    };


    //= OContainerListenerAdapter

    class COMPHELPER_DLLPUBLIC OContainerListenerAdapter
            :public cppu::WeakImplHelper< ::com::sun::star::container::XContainerListener >
    {
        friend class OContainerListener;

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >
                                m_xContainer;
        OContainerListener*     m_pListener;
        sal_Int32               m_nLockCount;

        virtual ~OContainerListenerAdapter();

    public:
        OContainerListenerAdapter(OContainerListener* _pListener,
            const  ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _rxContainer);

        // XEventListener
        virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // locking the multiplexer
        sal_Int32   locked() const { return m_nLockCount; }

        /// dispose the object. No multiplexing anymore
        void        dispose();

    };


}   // namespace dbaui


#endif // INCLUDED_COMPHELPER_CONTAINERMULTIPLEXER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
