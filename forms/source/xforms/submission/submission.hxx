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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_SUBMISSION_SUBMISSION_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_SUBMISSION_SUBMISSION_HXX

#include <tools/urlobj.hxx>
#include <rtl/ustring.h>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

#include <com/sun/star/task/XInteractionHandler.hpp>

#include <com/sun/star/frame/XFrame.hpp>

#include <cppuhelper/implbase.hxx>

#include "serialization.hxx"

#include <memory>

class CSubmissionPut;
class CSubmissionPost;
class CSubmissionGet;

class CCommandEnvironmentHelper final : public cppu::WeakImplHelper< css::ucb::XCommandEnvironment >
{
    friend class CSubmissionPut;
    friend class CSubmissionPost;
    friend class CSubmissionGet;
    friend class CSubmission;

    css::uno::Reference< css::task::XInteractionHandler >   m_aInteractionHandler;
    css::uno::Reference< css::ucb::XProgressHandler >       m_aProgressHandler;

public:
    virtual css::uno::Reference< css::task::XInteractionHandler > SAL_CALL getInteractionHandler() override
    {
        return m_aInteractionHandler;
    }
    virtual css::uno::Reference< css::ucb::XProgressHandler > SAL_CALL getProgressHandler() override
    {
        return m_aProgressHandler;
    }
};

class CProgressHandlerHelper final : public cppu::WeakImplHelper< css::ucb::XProgressHandler >
{
    friend class CSubmissionPut;
    friend class CSubmissionPost;
    friend class CSubmissionGet;
    osl::Condition m_cFinished;
    osl::Mutex m_mLock;
    sal_Int32 m_count;
public:
    CProgressHandlerHelper()
        : m_count(0)
    {}
    virtual void SAL_CALL push( const css::uno::Any& /*aStatus*/) override
    {
        m_mLock.acquire();
        m_count++;
        m_mLock.release();
    }
    virtual void SAL_CALL update(const css::uno::Any& /*aStatus*/) override
    {
    }
    virtual void SAL_CALL pop() override
    {
        m_mLock.acquire();
        m_count--;
        if (m_count == 0)
            m_cFinished.set();
        m_mLock.release();
    }
};

class CSubmission
{

protected:
    INetURLObject m_aURLObj;
    css::uno::Reference< css::xml::dom::XDocumentFragment > m_aFragment;
    css::uno::Reference< css::io::XInputStream >            m_aResultStream;
    css::uno::Reference< css::uno::XComponentContext >      m_xContext;

    ::std::unique_ptr< CSerialization > createSerialization(const css::uno::Reference< css::task::XInteractionHandler >& aHandler
                                                  ,css::uno::Reference<css::ucb::XCommandEnvironment>& _rOutEnv);

public:
    enum SubmissionResult {
        SUCCESS,
        UNKNOWN_ERROR
    };

    CSubmission(const OUString& aURL, const css::uno::Reference< css::xml::dom::XDocumentFragment >& aFragment)
        : m_aURLObj(aURL)
        , m_aFragment(aFragment)
        , m_xContext(::comphelper::getProcessComponentContext())
    {}

    virtual ~CSubmission() {}

    virtual SubmissionResult submit(const css::uno::Reference< css::task::XInteractionHandler >& ) = 0;

    SubmissionResult replace(const OUString&, const css::uno::Reference< css::xml::dom::XDocument >&, const css::uno::Reference< css::frame::XFrame>&);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
