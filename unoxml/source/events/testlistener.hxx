/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef EVENT_TESTLISTENER_HXX
#define EVENT_TESTLISTENER_HXX

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


using ::rtl::OUString;
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
            throw (RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException);


        // XEventListener
        virtual void SAL_CALL initialize(const Sequence< Any >& args) throw (RuntimeException);

        virtual void SAL_CALL handleEvent(const Reference< XEvent >& evt) throw (RuntimeException);


    };
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
