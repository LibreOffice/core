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


#include "connectivity/sqlerror.hxx"

#include <com/sun/star/sdbc/SQLException.hpp>

#include <comphelper/officeresourcebundle.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>

#include <string.h>

//........................................................................
namespace connectivity
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::uno::Type;

    //using SQLError::ParamValue; // GCC (unxlngi6) does not like this
    namespace
    {
        typedef SQLError::ParamValue ParamValue;
    }

    //====================================================================
    //= SQLError_Impl - declaration
    //====================================================================
    class SQLError_Impl
    {
    public:
        SQLError_Impl( const Reference<XComponentContext> & _rxContext );
        ~SQLError_Impl();

        // versions of the public SQLError methods which are just delegated to this impl-class
        static const OUString& getMessagePrefix();
        OUString     getErrorMessage( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
        OUString     getSQLState( const ErrorCondition _eCondition );
        static ErrorCode    getErrorCode( const ErrorCondition _eCondition );
        void                raiseException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
        void                raiseException( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
        void                raiseTypedException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const Type& _rExceptionType, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
        SQLException        getSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );

    private:
        /// returns the basic error message associated with the given error condition, without any parameter replacements
        OUString
                impl_getErrorMessage( const ErrorCondition& _eCondition );

        /// returns the SQLState associated with the given error condition
        OUString
                impl_getSQLState( const ErrorCondition& _eCondition );

        /// returns an SQLException describing the given error condition
        SQLException
                impl_buildSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
                    const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );

        /// initializes our resource bundle
        bool    impl_initResources();

    private:
        ::osl::Mutex                                            m_aMutex;
        Reference<XComponentContext>                            m_aContext;
        ::std::auto_ptr< ::comphelper::OfficeResourceBundle >   m_pResources;
        bool                                                    m_bAttemptedInit;
    };

    //====================================================================
    //= SQLError_Impl - implementation
    //====================================================================
    //--------------------------------------------------------------------
    SQLError_Impl::SQLError_Impl( const Reference<XComponentContext> & _rxContext )
        :m_aContext( _rxContext )
        ,m_pResources( )
        ,m_bAttemptedInit( false )
    {
    }

    //--------------------------------------------------------------------
    SQLError_Impl::~SQLError_Impl()
    {
    }

    //--------------------------------------------------------------------
    const OUString& SQLError_Impl::getMessagePrefix()
    {
        static const OUString s_sMessagePrefix( "[OOoBase]" );
        return s_sMessagePrefix;
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        /** substitutes a given placeholder in the given message with the given value
        */
        void    lcl_substitutePlaceholder( OUString& _rMessage, const sal_Char* _pPlaceholder, ParamValue _rParamValue )
        {
            size_t nPlaceholderLen( strlen( _pPlaceholder ) );
            sal_Int32 nIndex = _rMessage.indexOfAsciiL( _pPlaceholder, nPlaceholderLen );

            bool bHasPlaceholder = ( nIndex != -1 );
            bool bWantsPlaceholder = _rParamValue.is();
            OSL_ENSURE( bHasPlaceholder == bWantsPlaceholder, "lcl_substitutePlaceholder: placeholder where none is expected, or no placeholder where one is needed!" );

            if ( bHasPlaceholder && bWantsPlaceholder )
                _rMessage = _rMessage.replaceAt( nIndex, nPlaceholderLen, *_rParamValue );
        }

        //................................................................
        sal_Int32   lcl_getResourceID( const ErrorCondition _eCondition, bool _bSQLState )
        {
            return  256
                +   2 * ::sal::static_int_cast< sal_Int32, ErrorCondition >( _eCondition )
                +   ( _bSQLState ? 1 : 0 );
        }
    }

    //--------------------------------------------------------------------
    OUString SQLError_Impl::getErrorMessage( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        OUString sErrorMessage( impl_getErrorMessage( _eCondition ) );

        lcl_substitutePlaceholder( sErrorMessage, "$1$", _rParamValue1 );
        lcl_substitutePlaceholder( sErrorMessage, "$2$", _rParamValue2 );
        lcl_substitutePlaceholder( sErrorMessage, "$3$", _rParamValue3 );

        return sErrorMessage;
    }

    //--------------------------------------------------------------------
    OUString SQLError_Impl::getSQLState( const ErrorCondition _eCondition )
    {
        return impl_getSQLState( _eCondition );
    }

    //--------------------------------------------------------------------
    ErrorCode SQLError_Impl::getErrorCode( const ErrorCondition _eCondition )
    {
        return 0 - ::sal::static_int_cast< ErrorCode, ErrorCondition >( _eCondition );
    }

    //--------------------------------------------------------------------
    void SQLError_Impl::raiseException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        raiseTypedException(
            _eCondition,
            _rxContext,
            ::cppu::UnoType< SQLException >::get(),
            _rParamValue1,
            _rParamValue2,
            _rParamValue3
        );
    }

    //--------------------------------------------------------------------
    void SQLError_Impl::raiseException( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        raiseTypedException(
            _eCondition,
            NULL,
            ::cppu::UnoType< SQLException >::get(),
            _rParamValue1,
            _rParamValue2,
            _rParamValue3
        );
    }

    //--------------------------------------------------------------------
    void SQLError_Impl::raiseTypedException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const Type& _rExceptionType, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        if ( !::cppu::UnoType< SQLException >::get().isAssignableFrom( _rExceptionType ) )
            throw ::std::bad_cast();

        // default-construct an exception of the desired type
        Any aException( NULL, _rExceptionType );

        // fill it
        SQLException* pException = static_cast< SQLException* >( aException.pData );
        *pException = impl_buildSQLException( _eCondition, _rxContext, _rParamValue1, _rParamValue2, _rParamValue3 );

        // throw it
        ::cppu::throwException( aException );
    }

    //--------------------------------------------------------------------
    SQLException SQLError_Impl::getSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        return impl_buildSQLException( _eCondition, _rxContext, _rParamValue1, _rParamValue2, _rParamValue3 );
    }

    //--------------------------------------------------------------------
    SQLException SQLError_Impl::impl_buildSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        return SQLException(
            getErrorMessage( _eCondition, _rParamValue1, _rParamValue2, _rParamValue3 ),
            _rxContext,
            getSQLState( _eCondition ),
            getErrorCode( _eCondition ),
            Any()
        );
    }

    //--------------------------------------------------------------------
    OUString SQLError_Impl::impl_getErrorMessage( const ErrorCondition& _eCondition )
    {
        OUStringBuffer aMessage;

        if ( impl_initResources() )
        {
            OUString sResMessage( m_pResources->loadString( lcl_getResourceID( _eCondition, false ) ) );
            OSL_ENSURE( !sResMessage.isEmpty(), "SQLError_Impl::impl_getErrorMessage: illegal error condition, or invalid resource!" );
            aMessage.append( getMessagePrefix() ).appendAscii( " " ).append( sResMessage );
        }

        return aMessage.makeStringAndClear();
    }

    //--------------------------------------------------------------------
    OUString SQLError_Impl::impl_getSQLState( const ErrorCondition& _eCondition )
    {
        OUString sState;

        if ( impl_initResources() )
        {
            sal_Int32 nResourceId( lcl_getResourceID( _eCondition, true ) );
            if ( m_pResources->hasString( nResourceId ) )
                sState = m_pResources->loadString( nResourceId );
        }

        if ( sState.isEmpty() )
            sState = OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "S1000" ) );

        return sState;
    }

    //--------------------------------------------------------------------
    bool SQLError_Impl::impl_initResources()
    {
        if ( m_pResources.get() )
            return true;
        if ( m_bAttemptedInit )
            return false;

        ::osl::MutexGuard aGuard( m_aMutex );
        m_bAttemptedInit = true;

        m_pResources.reset( new ::comphelper::OfficeResourceBundle( m_aContext, "sdberr" ) );
        return m_pResources.get() != NULL;
    }

    //====================================================================
    //= SQLError
    //====================================================================
    //--------------------------------------------------------------------
    SQLError::SQLError( const Reference<XComponentContext> & _rxContext )
        :m_pImpl( new SQLError_Impl( _rxContext ) )
    {
    }

    //--------------------------------------------------------------------
    SQLError::~SQLError()
    {
    }

    //--------------------------------------------------------------------
    const OUString& SQLError::getMessagePrefix()
    {
        return SQLError_Impl::getMessagePrefix();
    }

    //--------------------------------------------------------------------
    OUString SQLError::getErrorMessage( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 ) const
    {
        return m_pImpl->getErrorMessage( _eCondition, _rParamValue1, _rParamValue2, _rParamValue3 );
    }

    //--------------------------------------------------------------------
    ErrorCode SQLError::getErrorCode( const ErrorCondition _eCondition )
    {
        return SQLError_Impl::getErrorCode( _eCondition );
    }

    //--------------------------------------------------------------------
    void SQLError::raiseException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 ) const
    {
        m_pImpl->raiseException( _eCondition, _rxContext, _rParamValue1, _rParamValue2, _rParamValue3 );
    }

    //--------------------------------------------------------------------
    void SQLError::raiseException( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 ) const
    {
        m_pImpl->raiseException( _eCondition, _rParamValue1, _rParamValue2, _rParamValue3 );
    }

    //--------------------------------------------------------------------
    void SQLError::raiseTypedException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const Type& _rExceptionType, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 ) const
    {
        m_pImpl->raiseTypedException( _eCondition, _rxContext, _rExceptionType, _rParamValue1, _rParamValue2, _rParamValue3 );
    }

    //--------------------------------------------------------------------
    SQLException SQLError::getSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 ) const
    {
        return m_pImpl->getSQLException( _eCondition, _rxContext, _rParamValue1, _rParamValue2, _rParamValue3 );
    }

//........................................................................
} // namespace connectivity
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
