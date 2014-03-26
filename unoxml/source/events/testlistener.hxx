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


using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM { namespace events
{

    typedef ::cppu::WeakImplHelper3
        < ::com::sun::star::xml::dom::events::XEventListener
        , ::com::sun::star::lang::XInitialization
        , ::com::sun::star::lang::XServiceInfo
        > CTestListener_Base;

    class  CTestListener
        : public CTestListener_Base
    {

    private:
        Reference< ::com::sun::star::lang::XMultiServiceFactory > m_factory;
        Reference <XEventTarget> m_target;
        OUString m_type;
        sal_Bool m_capture;
        OUString m_name;

    public:

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(
            const Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                rSMgr);

        CTestListener(
                const Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                    rSMgr)
            : m_factory(rSMgr){};

        virtual ~CTestListener();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException, std::exception) SAL_OVERRIDE;


        // XEventListener
        virtual void SAL_CALL initialize(const Sequence< Any >& args) throw (RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL handleEvent(const Reference< XEvent >& evt) throw (RuntimeException, std::exception) SAL_OVERRIDE;


    };
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
