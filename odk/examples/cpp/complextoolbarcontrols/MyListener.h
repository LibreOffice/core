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

#ifndef INCLUDED_EXAMPLES_COMPLEXTOOLBARCONTROLS_MYLISTENER_H
#define INCLUDED_EXAMPLES_COMPLEXTOOLBARCONTROLS_MYLISTENER_H

#include <sal/config.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#define MYLISTENER_IMPLEMENTATIONNAME  "vnd.My.impl.NewDocListener"
#define MYLISTENER_SERVICENAME         "vnd.My.NewDocListener"

/**
 * Register in the Office configuration as job.
 * This is called automatically for all new opened documents. You receive
 * a reference of the opened document and then you can check if it's a
 * supported format. (We are only interested in Writer/Calc documents.)
 *
 * @see CalcListener
 * @see WriterListener
 */
class MyListener : public cppu::WeakImplHelper2< css::task::XJob         ,
                                                 css::lang::XServiceInfo >
{
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
    public:
                 MyListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~MyListener();

    // XJob
    virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName();

    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& sServiceName);

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();

    public:

    static css::uno::Reference< css::uno::XInterface > st_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
};

class CalcListener : public cppu::WeakImplHelper1< css::document::XEventListener >
{
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;


    public:
        CalcListener(const css::uno::Reference< css::lang::XMultiServiceFactory > &rxMSF)
            : mxMSF( rxMSF )
        {}

        virtual ~CalcListener()
        {}

    // document.XEventListener
    virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent);
    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent);
};

class WriterListener : public cppu::WeakImplHelper1< css::document::XEventListener >
{
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;

    public:
        WriterListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMSF);

        virtual ~WriterListener()
        {}

        // document.XEventListener
    virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent);
    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
