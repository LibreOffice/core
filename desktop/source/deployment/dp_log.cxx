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


#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/logging.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::logging;

namespace dp_log {

typedef ::cppu::WeakComponentImplHelper<ucb::XProgressHandler, lang::XServiceInfo> t_log_helper;

namespace {

class ProgressLogImpl : public cppu::BaseMutex, public t_log_helper
{
    comphelper::EventLogger m_logger;

protected:
    virtual void SAL_CALL disposing() override;
    virtual ~ProgressLogImpl() override;

public:
    ProgressLogImpl( Sequence<Any> const & args,
                     Reference<XComponentContext> const & xContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

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
    : t_log_helper( m_aMutex )
    // Use the logger created by unopkg app
    , m_logger(xContext, "unopkg")
{
}

// XServiceInfo
OUString ProgressLogImpl::getImplementationName()
{
    return u"com.sun.star.comp.deployment.ProgressLog"_ustr;
}

sal_Bool ProgressLogImpl::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > ProgressLogImpl::getSupportedServiceNames()
{
    // a private one
    return { u"com.sun.star.comp.deployment.ProgressLog"_ustr };
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
    m_logger.log(logLevel, buf.makeStringAndClear());
}


void ProgressLogImpl::pop()
{
}

} // namespace dp_log

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_deployment_ProgressLog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new dp_log::ProgressLogImpl(args, context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
