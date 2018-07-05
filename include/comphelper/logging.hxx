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

#ifndef INCLUDED_COMPHELPER_LOGGING_HXX
#define INCLUDED_COMPHELPER_LOGGING_HXX

#include <comphelper/comphelperdllapi.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/logging/XLogHandler.hpp>
#include <com/sun/star/logging/LogLevel.hpp>

#include <boost/optional.hpp>
#include <memory>

namespace comphelper
{


    //= string conversions, employed by the templatized log* members of
    //= EventLogger


    namespace log { namespace convert
    {
        inline const OUString& convertLogArgToString( const OUString& _rValue )
        {
            return _rValue;
        }

        inline OUString convertLogArgToString( const sal_Char* _pAsciiValue )
        {
            return OUString::createFromAscii( _pAsciiValue );
        }

        inline OUString convertLogArgToString( double      _nValue ) { return OUString::number( _nValue ); }
        inline OUString convertLogArgToString( float       _nValue ) { return OUString::number( _nValue ); }
        inline OUString convertLogArgToString( sal_Int64   _nValue ) { return OUString::number( _nValue ); }
        inline OUString convertLogArgToString( sal_Int32   _nValue ) { return OUString::number( _nValue ); }
        inline OUString convertLogArgToString( sal_Int16   _nValue ) { return OUString::number( _nValue ); }
        inline OUString convertLogArgToString( sal_Unicode _nValue ) { return OUString( _nValue ); }
        inline OUString convertLogArgToString( bool    _bValue ) { return OUString::boolean( _bValue ); }
        void convertLogArgToString(sal_Bool) = delete;

    } } // namespace log::convert


    //= EventLogger

    class EventLogger_Impl;
    typedef ::boost::optional< OUString >    OptionalString;

    /** encapsulates an css::logging::XLogger

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
        std::shared_ptr< EventLogger_Impl > m_pImpl;

    public:
        /** creates an <code>EventLogger</code> instance working with a css.logging.XLogger
            instance given by ASCII name.

            @param _rxContext
                the component context to create services

            @param _rLoggerName
                the ASCII name of the logger to create.
        */
        EventLogger(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const sal_Char* _pAsciiLoggerName
        );

    public:
        /// determines whether an event with the given level would be logged
        bool        isLoggable( const sal_Int32 _nLogLevel ) const;


        //- XLogger::log equivalents/wrappers
        //- string messages

        /// logs a given message, without any arguments, or source class/method names
        bool        log( const sal_Int32 _nLogLevel, const OUString& rMessage ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log(_nLogLevel, nullptr, nullptr, rMessage);
            return false;
        }

        /** logs a given message, replacing a placeholder in the message with an argument

            The function takes, additionally to the log level and the message, an arbitrary
            argument. This argument is converted to a string using an overloaded function
            named <code>convertLogArgToString</code>. Then, a placeholder &quot;$1$&quot;
            is searched in the message string, and replaced with the argument string.
        */
        template< typename ARGTYPE1 >
        bool        log( const sal_Int32 _nLogLevel, const OUString& _rMessage, ARGTYPE1 _argument1 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, nullptr, nullptr, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        log( const sal_Int32 _nLogLevel, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, nullptr, nullptr, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        log( const sal_Int32 _nLogLevel, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, nullptr, nullptr, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ) );
            return false;
        }

        /// logs a given message, replacing 4 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        bool        log( const sal_Int32 _nLogLevel, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, nullptr, nullptr, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ) );
            return false;
        }

        /// logs a given message, replacing 5 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        bool        log( const sal_Int32 _nLogLevel, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, nullptr, nullptr, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ) );
            return false;
        }

        /// logs a given message, replacing 6 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5, typename ARGTYPE6 >
        bool        log( const sal_Int32 _nLogLevel, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5, ARGTYPE6 _argument6 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, nullptr, nullptr, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument6 ) ) );
            return false;
        }


        //- XLogger::log equivalents/wrappers
        //- ASCII messages

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
                return impl_log( _nLogLevel, nullptr, nullptr, OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, nullptr, nullptr, OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Char* _pMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, nullptr, nullptr, OUString::createFromAscii( _pMessage ),
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
                return impl_log( _nLogLevel, nullptr, nullptr, OUString::createFromAscii( _pMessage ),
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
                return impl_log( _nLogLevel, nullptr, nullptr, OUString::createFromAscii( _pMessage ),
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
                return impl_log( _nLogLevel, nullptr, nullptr, OUString::createFromAscii( _pMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument3 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument4 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument5 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument6 ) ) );
            return false;
        }


        //- XLogger::logp equivalents/wrappers
        //- string messages

        /** logs a given message, replacing a placeholder in the message with an argument

            The function takes, additionally to the logp level and the message, an arbitrary
            argument. This argument is converted to a string using an overloaded function
            named <code>convertLogArgToString</code>. Then, a placeholder &quot;$1$&quot;
            is searched in the message string, and replaced with the argument string.
        */
        template< typename ARGTYPE1 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const OUString& _rMessage, ARGTYPE1 _argument1 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, _rMessage,
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
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
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
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
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
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
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const OUString& _rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5, ARGTYPE6 _argument6 ) const
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


        //- XLogger::logp equivalents/wrappers
        //- ASCII messages

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
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, OUString::createFromAscii( _pAsciiMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 2 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, OUString::createFromAscii( _pAsciiMessage ),
                    OptionalString( log::convert::convertLogArgToString( _argument1 ) ),
                    OptionalString( log::convert::convertLogArgToString( _argument2 ) ) );
            return false;
        }

        /// logs a given ASCII message, replacing 3 placeholders in the message with respective values
        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        logp( const sal_Int32 _nLogLevel, const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const sal_Char* _pAsciiMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            if ( isLoggable( _nLogLevel ) )
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, OUString::createFromAscii( _pAsciiMessage ),
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
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, OUString::createFromAscii( _pAsciiMessage ),
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
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, OUString::createFromAscii( _pAsciiMessage ),
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
                return impl_log( _nLogLevel, _pSourceClass, _pSourceMethod, OUString::createFromAscii( _pAsciiMessage ),
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
                        const OUString& _rMessage,
                        const OptionalString& _rArgument1 = OptionalString(),
                        const OptionalString& _rArgument2 = OptionalString(),
                        const OptionalString& _rArgument3 = OptionalString(),
                        const OptionalString& _rArgument4 = OptionalString(),
                        const OptionalString& _rArgument5 = OptionalString(),
                        const OptionalString& _rArgument6 = OptionalString()
                    ) const;
    };
} // namespace comphelper


#endif // INCLUDED_COMPHELPER_LOGGING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
