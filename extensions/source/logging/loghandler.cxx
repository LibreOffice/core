/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loghandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:59:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_LOGHANDLER_HXX
#include "loghandler.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LOGGING_LOGLEVEL_HPP_
#include <com/sun/star/logging/LogLevel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LOGGING_PLAINTEXTFORMATTER_HPP_
#include <com/sun/star/logging/PlainTextFormatter.hpp>
#endif
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>
#include <comphelper/componentcontext.hxx>
#include <rtl/tencinfo.h>

//........................................................................
namespace logging
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::logging::XLogFormatter;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::logging::PlainTextFormatter;
    /** === end UNO using === **/
    namespace LogLevel = ::com::sun::star::logging::LogLevel;

    //====================================================================
    //= LogHandlerHelper
    //====================================================================
    //--------------------------------------------------------------------
    LogHandlerHelper::LogHandlerHelper( const Reference< XComponentContext >& _rxContext, ::osl::Mutex& _rMutex, ::cppu::OBroadcastHelper& _rBHelper )
        :m_eEncoding( RTL_TEXTENCODING_UTF8 )
        ,m_nLevel( LogLevel::SEVERE )
        ,m_xFormatter( NULL )
        ,m_xContext( _rxContext )
        ,m_rMutex( _rMutex )
        ,m_rBHelper( _rBHelper )
        ,m_bInitialized( false )
    {
    }

    //--------------------------------------------------------------------
    void LogHandlerHelper::initFromSettings( const ::comphelper::NamedValueCollection& _rSettings )
    {
        ::rtl::OUString sEncoding;
        if ( _rSettings.get_ensureType( "Encoding", sEncoding ) )
        {
            if ( !setEncoding( sEncoding ) )
                throw IllegalArgumentException();
        }

        _rSettings.get_ensureType( "Formatter", m_xFormatter );
        _rSettings.get_ensureType( "Level", m_nLevel );
    }

    //--------------------------------------------------------------------
    void LogHandlerHelper::enterMethod()
    {
        m_rMutex.acquire();

        if ( !getIsInitialized() )
            throw DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "component not initialized" ) ), NULL );

        if ( m_rBHelper.bDisposed )
            throw DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "component already disposed" ) ), NULL );

        // fallback settings, in case they weren't passed at construction time
        if ( !getFormatter().is() )
        {
            try
            {
                Reference< XLogFormatter > xFormatter( PlainTextFormatter::create( m_xContext ), UNO_QUERY_THROW );
                setFormatter( xFormatter );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //--------------------------------------------------------------------
    bool LogHandlerHelper::getEncoding( ::rtl::OUString& _out_rEncoding ) const
    {
        const char* pMimeCharset = rtl_getMimeCharsetFromTextEncoding( m_eEncoding );
        if ( pMimeCharset )
        {
            _out_rEncoding = ::rtl::OUString::createFromAscii( pMimeCharset );
            return true;
        }
        _out_rEncoding = ::rtl::OUString();
        return false;
    }

    //--------------------------------------------------------------------
    bool LogHandlerHelper::setEncoding( const ::rtl::OUString& _rEncoding )
    {
        ::rtl::OString sAsciiEncoding( ::rtl::OUStringToOString( _rEncoding, RTL_TEXTENCODING_ASCII_US ) );
        rtl_TextEncoding eEncoding = rtl_getTextEncodingFromMimeCharset( sAsciiEncoding.getStr() );
        if ( eEncoding != RTL_TEXTENCODING_DONTKNOW )
        {
            m_eEncoding = eEncoding;
            return true;
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool LogHandlerHelper::formatForPublishing( const LogRecord& _rRecord, ::rtl::OString& _out_rEntry ) const
    {
        if ( _rRecord.Level < getLevel() )
            // not to be published due to low level
            return false;

        try
        {
            Reference< XLogFormatter > xFormatter( getFormatter(), UNO_QUERY_THROW );
            ::rtl::OUString sEntry( xFormatter->format( _rRecord ) );
            _out_rEntry = ::rtl::OUStringToOString( sEntry, getTextEncoding() );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool LogHandlerHelper::getEncodedHead( ::rtl::OString& _out_rHead ) const
    {
        try
        {
            Reference< XLogFormatter > xFormatter( getFormatter(), UNO_QUERY_THROW );
            ::rtl::OUString sHead( xFormatter->getHead() );
            _out_rHead = ::rtl::OUStringToOString( sHead, getTextEncoding() );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool LogHandlerHelper::getEncodedTail( ::rtl::OString& _out_rTail ) const
    {
        try
        {
            Reference< XLogFormatter > xFormatter( getFormatter(), UNO_QUERY_THROW );
            ::rtl::OUString sTail( xFormatter->getTail() );
            _out_rTail = ::rtl::OUStringToOString( sTail, getTextEncoding() );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

//........................................................................
} // namespace logging
//........................................................................

