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


#include "loghandler.hxx"

#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/logging/PlainTextFormatter.hpp>

#include <comphelper/diagnose_ex.hxx>
#include <rtl/tencinfo.h>


namespace logging
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::logging::XLogFormatter;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::logging::PlainTextFormatter;

    namespace LogLevel = ::com::sun::star::logging::LogLevel;

    LogHandlerHelper::LogHandlerHelper( const Reference< XComponentContext >& _rxContext, ::osl::Mutex& _rMutex, ::cppu::OBroadcastHelper& _rBHelper )
        :m_eEncoding( RTL_TEXTENCODING_UTF8 )
        ,m_nLevel( LogLevel::SEVERE )
        ,m_xContext( _rxContext )
        ,m_rMutex( _rMutex )
        ,m_rBHelper( _rBHelper )
        ,m_bInitialized( false )
    {
    }


    void LogHandlerHelper::initFromSettings( const ::comphelper::NamedValueCollection& _rSettings )
    {
        OUString sEncoding;
        if ( _rSettings.get_ensureType( u"Encoding"_ustr, sEncoding ) )
        {
            if ( !setEncoding( sEncoding ) )
                throw IllegalArgumentException();
        }

        _rSettings.get_ensureType( u"Formatter"_ustr, m_xFormatter );
        _rSettings.get_ensureType( u"Level"_ustr, m_nLevel );
    }


    void LogHandlerHelper::enterMethod()
    {
        m_rMutex.acquire();

        if ( !m_bInitialized )
            throw DisposedException(u"component not initialized"_ustr );

        if ( m_rBHelper.bDisposed )
            throw DisposedException(u"component already disposed"_ustr );

        // fallback settings, in case they weren't passed at construction time
        if ( !getFormatter().is() )
        {
            try
            {
                Reference< XLogFormatter > xFormatter( PlainTextFormatter::create( m_xContext ), css::uno::UNO_SET_THROW );
                setFormatter( xFormatter );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("extensions.logging");
            }
        }
    }


    bool LogHandlerHelper::getEncoding( OUString& _out_rEncoding ) const
    {
        const char* pMimeCharset = rtl_getMimeCharsetFromTextEncoding( m_eEncoding );
        if ( pMimeCharset )
        {
            _out_rEncoding = OUString::createFromAscii( pMimeCharset );
            return true;
        }
        _out_rEncoding.clear();
        return false;
    }


    bool LogHandlerHelper::setEncoding( std::u16string_view _rEncoding )
    {
        OString sAsciiEncoding( OUStringToOString( _rEncoding, RTL_TEXTENCODING_ASCII_US ) );
        rtl_TextEncoding eEncoding = rtl_getTextEncodingFromMimeCharset( sAsciiEncoding.getStr() );
        if ( eEncoding != RTL_TEXTENCODING_DONTKNOW )
        {
            m_eEncoding = eEncoding;
            return true;
        }
        return false;
    }


    bool LogHandlerHelper::formatForPublishing( const LogRecord& _rRecord, OString& _out_rEntry ) const
    {
        if ( _rRecord.Level < getLevel() )
            // not to be published due to low level
            return false;

        try
        {
            Reference< XLogFormatter > xFormatter( getFormatter(), css::uno::UNO_SET_THROW );
            OUString sEntry( xFormatter->format( _rRecord ) );
            _out_rEntry = OUStringToOString( sEntry, getTextEncoding() );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.logging");
        }
        return false;
    }


    bool LogHandlerHelper::getEncodedHead( OString& _out_rHead ) const
    {
        try
        {
            Reference< XLogFormatter > xFormatter( getFormatter(), css::uno::UNO_SET_THROW );
            OUString sHead( xFormatter->getHead() );
            _out_rHead = OUStringToOString( sHead, getTextEncoding() );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.logging");
        }
        return false;
    }


    bool LogHandlerHelper::getEncodedTail( OString& _out_rTail ) const
    {
        try
        {
            Reference< XLogFormatter > xFormatter( getFormatter(), css::uno::UNO_SET_THROW );
            OUString sTail( xFormatter->getTail() );
            _out_rTail = OUStringToOString( sTail, getTextEncoding() );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.logging");
        }
        return false;
    }


} // namespace logging


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
