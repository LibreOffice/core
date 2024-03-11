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

#include <config_options.h>
#include <com/sun/star/container/XContainerListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ref.hxx>

namespace osl { class Mutex; }
namespace com::sun::star::container { class XContainer; }


namespace comphelper
{


    class OContainerListenerAdapter;

    //= OContainerListener

    /** a non-UNO container listener
        <p>Useful if you have a non-refcountable class which should act as container listener.<br/>
        In this case, derive this class from OContainerListener, and create an adapter
        OContainerListenerAdapter which multiplexes the changes.</p>
    */
    class UNLESS_MERGELIBS_MORE(COMPHELPER_DLLPUBLIC) OContainerListener
    {
        friend class OContainerListenerAdapter;
        rtl::Reference<OContainerListenerAdapter>  m_xAdapter;
        ::osl::Mutex&                              m_rMutex;

    public:
        OContainerListener(::osl::Mutex& _rMutex);
        virtual ~OContainerListener();

        /// @throws css::uno::RuntimeException
        virtual void _elementInserted( const css::container::ContainerEvent& _rEvent );
        /// @throws css::uno::RuntimeException
        virtual void _elementRemoved( const css::container::ContainerEvent& _rEvent );
        /// @throws css::uno::RuntimeException
        virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent );
        /// @throws css::uno::RuntimeException
        virtual void _disposing(const css::lang::EventObject& _rSource);

    protected:
        void setAdapter(OContainerListenerAdapter* _pAdapter);
    };

    // workaround for incremental linking bugs in MSVC2015
    class SAL_DLLPUBLIC_TEMPLATE OContainerListenerAdapter_Base : public cppu::WeakImplHelper< css::container::XContainerListener > {};

    class UNLESS_MERGELIBS_MORE(COMPHELPER_DLLPUBLIC) OContainerListenerAdapter final : public OContainerListenerAdapter_Base
    {
        friend class OContainerListener;

    private:
        css::uno::Reference< css::container::XContainer >
                                m_xContainer;
        OContainerListener*     m_pListener;

        virtual ~OContainerListenerAdapter() override;

    public:
        OContainerListenerAdapter(OContainerListener* _pListener,
            const  css::uno::Reference< css::container::XContainer >& _rxContainer);

        // XEventListener
        virtual void SAL_CALL disposing( const  css::lang::EventObject& Source ) override;

        // XContainerListener
        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
        virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;

        /// dispose the object. No multiplexing anymore
        void        dispose();

    };


}   // namespace dbaui


#endif // INCLUDED_COMPHELPER_CONTAINERMULTIPLEXER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
