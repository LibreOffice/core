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


#include <dp_misc.h>
#include <dp_services.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <osl/time.h>
#include <osl/thread.h>
#include <cppuhelper/compbase.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/servicedecl.hxx>
#include <comphelper/unwrapargs.hxx>
#include <comphelper/logging.hxx>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <stdio.h>
#include <boost/optional.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::logging;

namespace dp_log {

typedef ::cppu::WeakComponentImplHelper<ucb::XProgressHandler> t_log_helper;

namespace {

class ProgressLogImpl : public ::dp_misc::MutexHolder, public t_log_helper
{
    std::unique_ptr<comphelper::EventLogger> m_logger;

protected:
    virtual void SAL_CALL disposing() override;
    virtual ~ProgressLogImpl() override;

public:
    ProgressLogImpl( Sequence<Any> const & args,
                     Reference<XComponentContext> const & xContext );

    // XProgressHandler
    virtual void SAL_CALL push( Any const & Status ) override;
    virtual void SAL_CALL update( Any const & Status ) override;
    virtual void SAL_CALL pop() override;
};

}

ProgressLogImpl::~ProgressLogImpl()
{
}


void ProgressLogImpl::disposing()
{
}


ProgressLogImpl::ProgressLogImpl(
    Sequence<Any> const & /* args */,
    Reference<XComponentContext> const & xContext )
    : t_log_helper( getMutex() )
{
    // Use the logger created by unopkg app
    m_logger.reset(new comphelper::EventLogger(xContext, "unopkg"));
}

// XProgressHandler

void ProgressLogImpl::push( Any const & Status )
{
    update( Status );
}

void ProgressLogImpl::update( Any const & Status )
{
    if (! Status.hasValue())
        return;

    OUStringBuffer buf;

    OUString msg;
    sal_Int32 logLevel = LogLevel::INFO;
    if (Status >>= msg) {
        buf.append( msg );
    }
    else {
        logLevel = LogLevel::SEVERE;
        buf.append( ::comphelper::anyToString(Status) );
    }
    m_logger->log(logLevel, buf.makeStringAndClear());
}


void ProgressLogImpl::pop()
{
}

namespace sdecl = comphelper::service_decl;
sdecl::class_<ProgressLogImpl, sdecl::with_args<true> > const servicePLI;
sdecl::ServiceDecl const serviceDecl(
    servicePLI,
    // a private one:
    "com.sun.star.comp.deployment.ProgressLog",
    "com.sun.star.comp.deployment.ProgressLog" );

} // namespace dp_log

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
