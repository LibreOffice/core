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

#ifndef INCLUDED_UNOXML_SOURCE_EVENTS_TESTLISTENER_HXX
#define INCLUDED_UNOXML_SOURCE_EVENTS_TESTLISTENER_HXX

#include <sal/types.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>

#include <cppuhelper/implbase3.hxx>

namespace DOM { namespace events
{

    typedef ::cppu::WeakImplHelper3
        < css::xml::dom::events::XEventListener
        , css::lang::XInitialization
        , css::lang::XServiceInfo
        > CTestListener_Base;

    class  CTestListener
        : public CTestListener_Base
    {

    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_factory;
        css::uno::Reference <css::xml::dom::events::XEventTarget> m_target;
        OUString m_type;
        bool m_capture;
        OUString m_name;

    public:

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static css::uno::Sequence< OUString > _getSupportedServiceNames();
        static css::uno::Reference< XInterface > _getInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory >&
                rSMgr);

        CTestListener(
                const css::uno::Reference< css::lang::XMultiServiceFactory >&
                    rSMgr)
            : m_factory(rSMgr)
            , m_capture(false)
        {
        }

        virtual ~CTestListener();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // XEventListener
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& args) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL handleEvent(const css::uno::Reference< css::xml::dom::events::XEvent >& evt) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    };
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
