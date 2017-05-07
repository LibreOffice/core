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


#include "dp_misc.h"
#include "dp_services.hxx"
#include <rtl/strbuf.hxx>
#include <osl/time.h>
#include <osl/thread.h>
#include <cppuhelper/compbase.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/servicedecl.hxx>
#include <comphelper/unwrapargs.hxx>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <stdio.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_log {

typedef ::cppu::WeakComponentImplHelper<ucb::XProgressHandler> t_log_helper;


class ProgressLogImpl : public ::dp_misc::MutexHolder, public t_log_helper
{
    Reference<io::XOutputStream> m_xLogFile;
    sal_Int32 m_log_level;
    void log_write( OString const & text );

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


ProgressLogImpl::~ProgressLogImpl()
{
}


void ProgressLogImpl::disposing()
{
    try {
        if (m_xLogFile.is()) {
            m_xLogFile->closeOutput();
            m_xLogFile.clear();
        }
    }
    catch (const Exception & exc) {
        (void) exc;
        OSL_FAIL( OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}


ProgressLogImpl::ProgressLogImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext )
    : t_log_helper( getMutex() ),
      m_log_level( 0 )
{
    OUString log_file;
    boost::optional< Reference<task::XInteractionHandler> > interactionHandler;
    comphelper::unwrapArgs( args, log_file, interactionHandler );

    Reference<ucb::XSimpleFileAccess3> xSimpleFileAccess( ucb::SimpleFileAccess::create(xContext) );
    // optional ia handler:
    if (interactionHandler)
        xSimpleFileAccess->setInteractionHandler( *interactionHandler );

    m_xLogFile.set(
        xSimpleFileAccess->openFileWrite( log_file ), UNO_QUERY_THROW );
    Reference<io::XSeekable> xSeekable( m_xLogFile, UNO_QUERY_THROW );
    xSeekable->seek( xSeekable->getLength() );

    // write log stamp
    OStringBuffer buf;
    buf.append( "###### Progress log entry " );
    TimeValue aStartTime, tLocal;
    oslDateTime date_time;
    if (osl_getSystemTime( &aStartTime ) &&
        osl_getLocalTimeFromSystemTime( &aStartTime, &tLocal ) &&
        osl_getDateTimeFromTimeValue( &tLocal, &date_time ))
    {
        char ar[ 128 ];
        snprintf(
            ar, sizeof (ar),
            "%04d-%02d-%02d %02d:%02d:%02d ",
            date_time.Year, date_time.Month, date_time.Day,
            date_time.Hours, date_time.Minutes, date_time.Seconds );
        buf.append( ar );
    }
    buf.append( "######\n" );
    log_write( buf.makeStringAndClear() );
}


void ProgressLogImpl::log_write( OString const & text )
{
    try {
        if (m_xLogFile.is()) {
            m_xLogFile->writeBytes(
                Sequence< sal_Int8 >(
                    reinterpret_cast< sal_Int8 const * >(text.getStr()),
                    text.getLength() ) );
        }
    }
    catch (const io::IOException & exc) {
        (void) exc;
        OSL_FAIL( OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

// XProgressHandler

void ProgressLogImpl::push( Any const & Status )
{
    update( Status );
    OSL_ASSERT( m_log_level >= 0 );
    ++m_log_level;
}


void ProgressLogImpl::update( Any const & Status )
{
    if (! Status.hasValue())
        return;

    OUStringBuffer buf;
    OSL_ASSERT( m_log_level >= 0 );
    for ( sal_Int32 n = 0; n < m_log_level; ++n )
        buf.append( ' ' );

    OUString msg;
    if (Status >>= msg) {
        buf.append( msg );
    }
    else {
        buf.append( "ERROR: " );
        buf.append( ::comphelper::anyToString(Status) );
    }
    buf.append( "\n" );
    log_write( OUStringToOString(
                   buf.makeStringAndClear(), osl_getThreadTextEncoding() ) );
}


void ProgressLogImpl::pop()
{
    OSL_ASSERT( m_log_level > 0 );
    --m_log_level;
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
