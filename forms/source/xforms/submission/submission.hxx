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

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>

#include "serialization.hxx"

#include <memory>

namespace CSS = com::sun::star;

class CSubmissionPut;
class CSubmissionPost;
class CSubmissionGet;

class CCommandEnvironmentHelper : public cppu::WeakImplHelper1< CSS::ucb::XCommandEnvironment >
{
    friend class CSubmissionPut;
    friend class CSubmissionPost;
    friend class CSubmissionGet;
    friend class CSubmission;

protected:
    CSS::uno::Reference< CSS::task::XInteractionHandler >   m_aInteractionHandler;
    CSS::uno::Reference< CSS::ucb::XProgressHandler >       m_aProgressHandler;

public:
    virtual CSS::uno::Reference< CSS::task::XInteractionHandler > SAL_CALL getInteractionHandler() throw (CSS::uno::RuntimeException, std::exception)
    {
        return m_aInteractionHandler;
    }
    virtual CSS::uno::Reference< CSS::ucb::XProgressHandler > SAL_CALL getProgressHandler() throw (CSS::uno::RuntimeException, std::exception)
    {
        return m_aProgressHandler;
    }
};

class CProgressHandlerHelper : public cppu::WeakImplHelper1< CSS::ucb::XProgressHandler >
{
    friend class CSubmissionPut;
    friend class CSubmissionPost;
    friend class CSubmissionGet;
protected:
    osl::Condition m_cFinished;
    osl::Mutex m_mLock;
    sal_Int32 m_count;
public:
    CProgressHandlerHelper()
        : m_count(0)
    {}
    virtual void SAL_CALL push( const com::sun::star::uno::Any& /*aStatus*/) throw(com::sun::star::uno::RuntimeException, std::exception)
    {
        m_mLock.acquire();
        m_count++;
        m_mLock.release();
    }
    virtual void SAL_CALL update(const com::sun::star::uno::Any& /*aStatus*/) throw(com::sun::star::uno::RuntimeException, std::exception)
    {
    }
    virtual void SAL_CALL pop() throw(com::sun::star::uno::RuntimeException, std::exception)
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
    CSS::uno::Reference< CSS::xml::xpath::XXPathObject >    m_aXPathObject;
    CSS::uno::Reference< CSS::xml::dom::XDocumentFragment > m_aFragment;
    CSS::uno::Reference< CSS::io::XInputStream >            m_aResultStream;
    CSS::uno::Reference< CSS::uno::XComponentContext >      m_xContext;
    OUString m_aEncoding;

    ::std::auto_ptr< CSerialization > createSerialization(const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler
                                                  ,com::sun::star::uno::Reference<com::sun::star::ucb::XCommandEnvironment>& _rOutEnv);

public:
    enum SubmissionResult {
        SUCCESS,
        INVALID_METHOD,
        INVALID_URL,
        INVALID_ENCODING,
        E_TRANSMISSION,
        UNKNOWN_ERROR
    };

    CSubmission(const OUString& aURL, const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment)
        : m_aURLObj(aURL)
        , m_aFragment(aFragment)
        , m_xContext(::comphelper::getProcessComponentContext())
    {}

    virtual ~CSubmission() {}

    virtual void setEncoding(const OUString& aEncoding)
    {
        m_aEncoding = aEncoding;
    }
    virtual SubmissionResult submit(const CSS::uno::Reference< CSS::task::XInteractionHandler >& ) = 0;

    virtual SubmissionResult replace(const OUString&, const CSS::uno::Reference< CSS::xml::dom::XDocument >&, const CSS::uno::Reference< CSS::frame::XFrame>&);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
