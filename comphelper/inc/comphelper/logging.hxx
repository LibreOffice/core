/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: logging.hxx,v $
 * $Revision: 1.3 $
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

#ifndef COMPHELPER_LOGGING_HXX
#define COMPHELPER_LOGGING_HXX

#include <comphelper/comphelperdllapi.h>

/** === begin UNO includes === **/
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/logging/XLogHandler.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
/** === end UNO includes === **/

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

//........................................................................
namespace comphelper
{
//........................................................................

    //====================================================================
    //= string conversions, employed by the templatized log* members of
    //= EventLogger
    //====================================================================

    namespace log { namespace convert
    {
        inline const ::rtl::OUString& convertLogArgToString( const ::rtl::OUString& _rValue )
        {
            return _rValue;
        }

        inline ::rtl::OUString convertLogArgToString( const sal_Char* _pAsciiValue )
        {
            return ::rtl::OUString::createFromAscii( _pAsciiValue );
        }

        inline ::rtl::OUString convertLogArgToString( double      _nValue ) { return ::rtl::OUString::valueOf( _nValue ); }
        inline ::rtl::OUString convertLogArgToString( float       _nValue ) { return ::rtl::OUString::valueOf( _nValue ); }
        inline ::rtl::OUString convertLogArgToString( sal_Int64   _nValue ) { return ::rtl::OUString::valueOf( _nValue ); }
        inline ::rtl::OUString convertLogArgToString( sal_Int32   _nValue ) { return ::rtl::OUString::valueOf( _nValue ); }
        inline ::rtl::OUString convertLogArgToString( sal_Int16   _nValue ) { return ::rtl::OUString::valueOf( (sal_Int32)_nValue ); }
        inline ::rtl::OUString convertLogArgToString( sal_Unicode _nValue ) { return ::rtl::OUString::valueOf( _nValue ); }
        inline ::rtl::OUString convertLogArgToString( sal_Bool    _nValue ) { return ::rtl::OUString::valueOf( _nValue ); }

    } } // namespace log::convert

    //====================================================================
    //= EventLogger
    //====================================================================
    class EventLogger_Impl;
    typedef ::boost::optional< ::rtl::OUString >    OptionalString;

    /** encapsulates an <type scope="com::sun::star::logging">XLogger</type>

        The class silences several (unlikely) errors which could potentially happen
        when working with a logger. Additionally, it provides some convenience methods
        for logging events.

        You can use this class as follows
<pre>
    EventLogger aLogger( xContext, sLoggerName );
    ....
    aLogger.log( LogLevel::SEVERE, sSomeMessage );
    aLogger.logp( LogLevel::CONFIG, "MyClass", "MyMethod", sSomeMessage, SomeParameter1, SomeParameter2, SomeParameter3 );
</pre>

        The <code>log</code> and <code>logp</code> calls support up to 6 parameters, which can be of
        arbitrary type. For every parameter, there must exist a function <code>convertLogArgToString</code>
        which takes an argument of the respective type, and returns a string.

        After a parameter has been converted to a string using the above mentioned
        <code>convertLogArgToString</code> function, a placeholder $1$ (resp. $2$ resp. $4$ ...)
        in the message will be replaced with this string, and the resulting message will be logged.
    */
    class COMPHELPER_DLLPUBLIC EventLogger
    {
    protected:
        ::boost::shared_ptr< EventLogger_Impl > m_pImpl;

    public:
        /** creates an <code>EventLogger</code> instance working with a css.logging.XLogger
            instance given by name.

            @param _rxContext
                the component context to create services
            @param _rLoggerName
                the name of the logger to create. If empty, the office-wide default logger will be used.
        */
        EventLogger(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::rtl::OUString& _rLoggerName = ::rtl::OUString()
        );

        /** creates an <code>EventLogger</code> instance working with a css.logging.XLogger
            instance given by ASCII name.

            @param _rxContext
                the component context to create services

            @param _rLoggerName
                the ASCII name of the logger to create.
        */
        EventLogger(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const sal_Char* _pAsciiLoggerName
        );

        ~EventLogger();

    public:
        /** returns the name of the logger
        */
        const ::rtl::OUString&  getName() const;

        /// returns the current log level threshold of the logger
        sal_Int32   getLogLevel() const;

        /// sets a new log level threshold of the logger
        void        setLogLevel( const sal_Int32 _nLogLevel ) const;

        /// determines whether an event with the given level would be logged
        bool        isLoggable( const sal_Int32 _nLogLevel ) const;

        /** adds the given log handler to the logger's set of handlers.

            Note that normally, you would not use this method: The logger implementations
            initialize themselves from the configuration, where usually, a default log handler
            is specified. In this case, the logger will create and use this handler.

            @return
                <TRUE/> if and only if the addition was successful (as far as this can be detected
                from outside the <code>XLogger</code>'s implementation.
        */
        bool        addLogHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::logging::XLogHandler >& _rxLogHandler );

        /** removes the given log handler from the logger's set of handlers.

            @return
                <TRUE/> if and only if the addition was successful (as far as this can be detected
                from outside the <code>XLogger</code>'s implementation.
        */
        bool        removeLogHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::logging::XLogHandler >& _rxLogHandler );

        //----------------------------------------------------------------
        //- XLogger::log equivalents/wrappers
        //- string messages

        /// logs a given message, without any arguments, or source class/method names
        bool        log( const sal_Int32 _nLogLevel, const ::rtl::OUString& _rMessage ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, _rMessage );
            return false;
        }

        /** logs a given message, replacing a placeholder in the message with an argument

            The function takes, additionally to the log level and the message, an arbitrary
            argument. This argument is converted to a string using an overloaded function
            named <code>convertLogArgToString</code>. Then, a placeholder &quot;$1$&quot;
            is searched in the message string, and replaced with the argument string.
        */
        template< typename ARGTYPE1 >
        bool        log( const sal_Int32 _nLogLevel, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        log( const sal_Int32 _nLogLevel, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        log( const sal_Int32 _nLogLevel, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ) );
            return false;
        }

        /// logs a given message, replacing 4 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        bool        log( const sal_Int32 _nLogLevel, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ) );
            return false;
        }

        /// logs a given message, replacing 5 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        bool        log( const sal_Int32 _nLogLevel, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ) );
            return false;
        }

        /// logs a given message, replacing 6 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5, typename ARGTYPE6 >
        bool        log( const sal_Int32 _nLogLevel, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5, ARGTYPE6 _argument6 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument6 ) ) );
            return false;
        }

        //----------------------------------------------------------------
        //- XLogger::log equivalents/wrappers
        //- ASCII messages

        /// logs a given message, without any arguments, or source class/method names
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, ::rtl::OUString::createFromAscii( _pMessage ) );
            return false;
        }

        /** logs a given message, replacing a placeholder in the message with an argument

            The function takes, additionally to the log level and the message, an arbitrary
            argument. This argument is converted to a string using an overloaded function
            named <code>convertLogArgToString</code>. Then, a placeholder &quot;$1$&quot;
            is searched in the message string, and replaced with the argument string.
        */
        template< typename ARGTYPE1 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage, ARGTYPE1 _argument1 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, ::rtl::OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, ::rtl::OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, ::rtl::OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ) );
            return false;
        }

        /// logs a given message, replacing 4 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, ::rtl::OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ) );
            return false;
        }

        /// logs a given message, replacing 5 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, ::rtl::OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ) );
            return false;
        }

        /// logs a given message, replacing 6 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5, typename ARGTYPE6 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5, ARGTYPE6 _argument6 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, ::rtl::OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument6 ) ) );
            return false;
        }

        //----------------------------------------------------------------
        //- XLogger::logp equivalents/wrappers
        //- string messages

        /// logs a given message, without any arguments, or source class/method names
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const ::rtl::OUString& _rMessage ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage );
            return false;
        }

        /** logs a given message, replacing a placeholder in the message with an argument

            The function takes, additionally to the logp level and the message, an arbitrary
            argument. This argument is converted to a string using an overloaded function
            named <code>convertLogArgToString</code>. Then, a placeholder &quot;$1$&quot;
            is searched in the message string, and replaced with the argument string.
        */
        template< typename ARGTYPE1 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ) );
            return false;
        }

        /// logs a given message, replacing 4 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ) );
            return false;
        }

        /// logs a given message, replacing 5 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ) );
            return false;
        }

        /// logs a given message, replacing 6 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5, typename ARGTYPE6 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const ::rtl::OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5, ARGTYPE6 _argument6 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument6 ) ) );
            return false;
        }

        //----------------------------------------------------------------
        //- XLogger::logp equivalents/wrappers
        //- ASCII messages

        /// logs a given ASCII message, without any arguments, or source class/method names
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, ::rtl::OUString::createFromAscii( _pAsciiMessage ) );
            return false;
        }

        /** logs a given ASCII message, replacing a placeholder in the message with an argument

            The function takes, additionally to the logp level and the message, an arbitrary
            argument. This argument is converted to a string using an overloaded function
            named <code>convertLogArgToString</code>. Then, a placeholder &quot;$1$&quot;
            is searched in the message string, and replaced with the argument string.
        */
        template< typename ARGTYPE1 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage, ARGTYPE1 _argument1 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, ::rtl::OUString::createFromAscii( _pAsciiMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, ::rtl::OUString::createFromAscii( _pAsciiMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, ::rtl::OUString::createFromAscii( _pAsciiMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 4 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, ::rtl::OUString::createFromAscii( _pAsciiMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 5 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, ::rtl::OUString::createFromAscii( _pAsciiMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 6 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5, typename ARGTYPE6 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5, ARGTYPE6 _argument6 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, ::rtl::OUString::createFromAscii( _pAsciiMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument6 ) ) );
            return false;
        }

    protected:
        bool        impl_log(
                        const sal_Int32 _nLogLevel,
                        const sal_Char* _pSourceClass,
                        const sal_Char* _pSourceMethod,
                        const ::rtl::OUString& _rMessage,
                        const OptionalString& _rArgument1 = OptionalString(),
                        const OptionalString& _rArgument2 = OptionalString(),
                        const OptionalString& _rArgument3 = OptionalString(),
                        const OptionalString& _rArgument4 = OptionalString(),
                        const OptionalString& _rArgument5 = OptionalString(),
                        const OptionalString& _rArgument6 = OptionalString()
                    ) const;
    };

    //====================================================================
    //= ResourceBasedEventLogger
    //====================================================================
    struct ResourceBasedEventLogger_Data;
    /** extends the EventLogger class with functionality to load log messages from
        a resource bundle.
    */
    class COMPHELPER_DLLPUBLIC ResourceBasedEventLogger : public EventLogger
    {
    private:
        ::boost::shared_ptr< ResourceBasedEventLogger_Data >    m_pData;

    public:
        /** creates a resource based event logger
            @param _rxContext
                the component context for creating new components
            @param _rResourceBundleBaseName
                the base name of the resource bundle to use. Will be used
                in conjunction with XResourceBundleLoader::loadResource.
            @param _rLoggerName
                the name of the logger to work with. If empty, the office-wide
                default logger will be used.

        */
        ResourceBasedEventLogger(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::rtl::OUString& _rResourceBundleBaseName,
            const ::rtl::OUString& _rLoggerName = ::rtl::OUString()
        );

        /** creates a resource based event logger
            @param _rxContext
                the component context for creating new components
            @param _pResourceBundleBaseName
                the ASCII base name of the resource bundle to use. Will be used
                in conjunction with XResourceBundleLoader::loadResource.
            @param _pAsciiLoggerName
                the ASCII name of the logger to work with. If NULL, the office-wide
                default logger will be used.

        */
        ResourceBasedEventLogger(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const sal_Char* _pResourceBundleBaseName,
            const sal_Char* _pAsciiLoggerName = NULL
        );

        //----------------------------------------------------------------
        //- XLogger::log equivalents/wrappers
        //- resource IDs

        /// logs a given message, without any arguments, or source class/method names
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, impl_loadStringMessage_nothrow( _nMessageResID ) );
            return false;
        }

        /** logs a given message, replacing a placeholder in the message with an argument

            The function takes, additionally to the log level and the message, an arbitrary
            argument. This argument is converted to a string using an overloaded function
            named <code>convertLogArgToString</code>. Then, a placeholder &quot;$1$&quot;
            is searched in the message string, and replaced with the argument string.
        */
        template< typename ARGTYPE1 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ) );
            return false;
        }

        /// logs a given message, replacing 4 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ) );
            return false;
        }

        /// logs a given message, replacing 5 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ) );
            return false;
        }

        /// logs a given message, replacing 6 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5, typename ARGTYPE6 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5, ARGTYPE6 _argument6 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, NULL, NULL, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument6 ) ) );
            return false;
        }

        //----------------------------------------------------------------
        //- XLogger::logp equivalents/wrappers
        //- resource IDs

        /// logs a given ASCII message, without any arguments, or source class/method names
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Int32 _nMessageResID ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, impl_loadStringMessage_nothrow( _nMessageResID ) );
            return false;
        }

        /** logs a given ASCII message, replacing a placeholder in the message with an argument
        */
        template< typename ARGTYPE1 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 4 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 5 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 6 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5, typename ARGTYPE6 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5, ARGTYPE6 _argument6 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, impl_loadStringMessage_nothrow( _nMessageResID ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument6 ) ) );
            return false;
        }

    private:
        ::rtl::OUString impl_loadStringMessage_nothrow( const sal_Int32 _nMessageResID ) const;
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_LOGGING_HXX
