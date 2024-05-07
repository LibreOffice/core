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

#include <sal/config.h>

#include "methodguard.hxx"
#include "loghandler.hxx"

#include <com/sun/star/logging/XConsoleHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <stdio.h>

namespace logging
{
    using ::com::sun::star::logging::XConsoleHandler;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::logging::XLogFormatter;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::NamedValue;

    typedef ::cppu::WeakComponentImplHelper    <   XConsoleHandler
                                                ,   XServiceInfo
                                                >   ConsoleHandler_Base;

    namespace {

    class ConsoleHandler    :public ::cppu::BaseMutex
                            ,public ConsoleHandler_Base
    {
    private:
        LogHandlerHelper                m_aHandlerHelper;
        sal_Int32                       m_nThreshold;

    public:
        ConsoleHandler(const Reference<XComponentContext> &context,
            const css::uno::Sequence<css::uno::Any> &arguments);
        virtual ~ConsoleHandler() override;

    private:
        // XConsoleHandler
        virtual ::sal_Int32 SAL_CALL getThreshold() override;
        virtual void SAL_CALL setThreshold( ::sal_Int32 _threshold ) override;

        // XLogHandler
        virtual OUString SAL_CALL getEncoding() override;
        virtual void SAL_CALL setEncoding( const OUString& _encoding ) override;
        virtual Reference< XLogFormatter > SAL_CALL getFormatter() override;
        virtual void SAL_CALL setFormatter( const Reference< XLogFormatter >& _formatter ) override;
        virtual ::sal_Int32 SAL_CALL getLevel() override;
        virtual void SAL_CALL setLevel( ::sal_Int32 _level ) override;
        virtual void SAL_CALL flush(  ) override;
        virtual sal_Bool SAL_CALL publish( const LogRecord& Record ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& _rServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

    public:
        typedef ComponentMethodGuard< ConsoleHandler > MethodGuard;
        void    enterMethod( MethodGuard::Access );
        void    leaveMethod( MethodGuard::Access );
    };

    }

    ConsoleHandler::ConsoleHandler(const Reference<XComponentContext> &context,
            const css::uno::Sequence<css::uno::Any> &arguments)
        :ConsoleHandler_Base( m_aMutex )
        ,m_aHandlerHelper( context, m_aMutex, rBHelper )
        ,m_nThreshold( css::logging::LogLevel::SEVERE )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !arguments.hasElements() )
        {   // create() - nothing to init
            m_aHandlerHelper.setIsInitialized();
            return;
        }

        if ( arguments.getLength() != 1 )
            throw IllegalArgumentException( OUString(), *this, 1 );

        Sequence< NamedValue > aSettings;
        if ( !( arguments[0] >>= aSettings ) )
            throw IllegalArgumentException( OUString(), *this, 1 );

        // createWithSettings( [in] sequence< css::beans::NamedValue > Settings )
        ::comphelper::NamedValueCollection aTypedSettings( aSettings );
        m_aHandlerHelper.initFromSettings( aTypedSettings );

        aTypedSettings.get_ensureType( u"Threshold"_ustr, m_nThreshold );

        m_aHandlerHelper.setIsInitialized();
    }

    ConsoleHandler::~ConsoleHandler()
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }
    }


    void SAL_CALL ConsoleHandler::disposing()
    {
        m_aHandlerHelper.setFormatter( nullptr );
    }


    void ConsoleHandler::enterMethod( MethodGuard::Access )
    {
        m_aHandlerHelper.enterMethod();
    }


    void ConsoleHandler::leaveMethod( MethodGuard::Access )
    {
        m_aMutex.release();
    }


    ::sal_Int32 SAL_CALL ConsoleHandler::getThreshold()
    {
        MethodGuard aGuard( *this );
        return m_nThreshold;
    }


    void SAL_CALL ConsoleHandler::setThreshold( ::sal_Int32 _threshold )
    {
        MethodGuard aGuard( *this );
        m_nThreshold = _threshold;
    }


    OUString SAL_CALL ConsoleHandler::getEncoding()
    {
        MethodGuard aGuard( *this );
        OUString sEncoding;
        OSL_VERIFY( m_aHandlerHelper.getEncoding( sEncoding ) );
        return sEncoding;
    }


    void SAL_CALL ConsoleHandler::setEncoding( const OUString& _rEncoding )
    {
        MethodGuard aGuard( *this );
        OSL_VERIFY( m_aHandlerHelper.setEncoding( _rEncoding ) );
    }


    Reference< XLogFormatter > SAL_CALL ConsoleHandler::getFormatter()
    {
        MethodGuard aGuard( *this );
        return m_aHandlerHelper.getFormatter();
    }


    void SAL_CALL ConsoleHandler::setFormatter( const Reference< XLogFormatter >& _rxFormatter )
    {
        MethodGuard aGuard( *this );
        m_aHandlerHelper.setFormatter( _rxFormatter );
    }


    ::sal_Int32 SAL_CALL ConsoleHandler::getLevel()
    {
        MethodGuard aGuard( *this );
        return m_aHandlerHelper.getLevel();
    }


    void SAL_CALL ConsoleHandler::setLevel( ::sal_Int32 _nLevel )
    {
        MethodGuard aGuard( *this );
        m_aHandlerHelper.setLevel( _nLevel );
    }


    void SAL_CALL ConsoleHandler::flush(  )
    {
        MethodGuard aGuard( *this );
        fflush( stdout );
        fflush( stderr );
    }


    sal_Bool SAL_CALL ConsoleHandler::publish( const LogRecord& _rRecord )
    {
        MethodGuard aGuard( *this );

        OString sEntry;
        if ( !m_aHandlerHelper.formatForPublishing( _rRecord, sEntry ) )
            return false;

        if ( _rRecord.Level >= m_nThreshold )
            fprintf( stderr, "%s\n", sEntry.getStr() );
        else
            fprintf( stdout, "%s\n", sEntry.getStr() );

        return true;
    }

    OUString SAL_CALL ConsoleHandler::getImplementationName()
    {
        return u"com.sun.star.comp.extensions.ConsoleHandler"_ustr;
    }

    sal_Bool SAL_CALL ConsoleHandler::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL ConsoleHandler::getSupportedServiceNames()
    {
        return { u"com.sun.star.logging.ConsoleHandler"_ustr };
    }

} // namespace logging

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_extensions_ConsoleHandler(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new logging::ConsoleHandler(context, arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
