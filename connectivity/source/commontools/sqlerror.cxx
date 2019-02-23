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


#include <memory>
#include <connectivity/sqlerror.hxx>

#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <unotools/resmgr.hxx>
#include <osl/diagnose.h>

#include <strings.hrc>
#include <strings.hxx>
#include <string.h>

namespace connectivity
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::uno::Type;

    //using SQLError::ParamValue; // GCC (unxlngi6) does not like this
    namespace
    {
        typedef SQLError::ParamValue ParamValue;
    }


    class SQLError_Impl
    {
    public:
        explicit SQLError_Impl();

        // versions of the public SQLError methods which are just delegated to this impl-class
        static const OUString& getMessagePrefix();
        OUString     getErrorMessage( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
        static ErrorCode    getErrorCode( const ErrorCondition _eCondition );
        void                raiseException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
        void                raiseException( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
        void                raiseTypedException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const Type& _rExceptionType, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
        SQLException        getSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );

    private:
        /// returns the basic error message associated with the given error condition, without any parameter replacements
        OUString
                impl_getErrorMessage( ErrorCondition _eCondition );

        /// returns the SQLState associated with the given error condition
        static OUString
                impl_getSQLState( ErrorCondition _eCondition );

        /// returns an SQLException describing the given error condition
        SQLException
                impl_buildSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
                    const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 );
    private:
        std::locale                                             m_aResources;
    };

    SQLError_Impl::SQLError_Impl()
        : m_aResources(Translate::Create("cnr"))
    {
    }

    const OUString& SQLError_Impl::getMessagePrefix()
    {
        static const OUString s_sMessagePrefix( "[OOoBase]" );
        return s_sMessagePrefix;
    }

    namespace
    {

        /** substitutes a given placeholder in the given message with the given value
        */
        void lcl_substitutePlaceholder(OUString& _rMessage, const sal_Char* _pPlaceholder, const ParamValue& rParamValue)
        {
            size_t nPlaceholderLen( strlen( _pPlaceholder ) );
            sal_Int32 nIndex = _rMessage.indexOfAsciiL( _pPlaceholder, nPlaceholderLen );

            bool bHasPlaceholder = ( nIndex != -1 );
            bool bWantsPlaceholder = rParamValue.is();
            OSL_ENSURE( bHasPlaceholder == bWantsPlaceholder, "lcl_substitutePlaceholder: placeholder where none is expected, or no placeholder where one is needed!" );

            if ( bHasPlaceholder && bWantsPlaceholder )
                _rMessage = _rMessage.replaceAt( nIndex, nPlaceholderLen, *rParamValue );
        }

        const char* lcl_getResourceErrorID(const ErrorCondition _eCondition)
        {
            switch (_eCondition)
            {
                case css::sdb::ErrorCondition::ROW_SET_OPERATION_VETOED:
                    return STR_ROW_SET_OPERATION_VETOED;
                case css::sdb::ErrorCondition::PARSER_CYCLIC_SUB_QUERIES:
                    return STR_PARSER_CYCLIC_SUB_QUERIES;
                case css::sdb::ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES:
                    return STR_DB_OBJECT_NAME_WITH_SLASHES;
                case css::sdb::ErrorCondition::DB_INVALID_SQL_NAME:
                    return STR_DB_INVALID_SQL_NAME;
                case css::sdb::ErrorCondition::DB_QUERY_NAME_WITH_QUOTES:
                    return STR_DB_QUERY_NAME_WITH_QUOTES;
                case css::sdb::ErrorCondition::DB_OBJECT_NAME_IS_USED:
                    return STR_DB_OBJECT_NAME_IS_USED;
                case css::sdb::ErrorCondition::DB_NOT_CONNECTED:
                    return STR_DB_NOT_CONNECTED;
                case css::sdb::ErrorCondition::AB_ADDRESSBOOK_NOT_FOUND:
                    return STR_AB_ADDRESSBOOK_NOT_FOUND;
                case css::sdb::ErrorCondition::DATA_CANNOT_SELECT_UNFILTERED:
                    return STR_DATA_CANNOT_SELECT_UNFILTERED;
            }
            return nullptr;
        }

        OUString lcl_getResourceState(const ErrorCondition _eCondition)
        {
            switch (_eCondition)
            {
                case css::sdb::ErrorCondition::DB_NOT_CONNECTED:
                    return OUString(STR_DB_NOT_CONNECTED_STATE);
                case css::sdb::ErrorCondition::DATA_CANNOT_SELECT_UNFILTERED:
                    return OUString(STR_DATA_CANNOT_SELECT_UNFILTERED_STATE);
            }
            return OUString();
        }
    }

    OUString SQLError_Impl::getErrorMessage( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        OUString sErrorMessage( impl_getErrorMessage( _eCondition ) );

        lcl_substitutePlaceholder( sErrorMessage, "$1$", _rParamValue1 );
        lcl_substitutePlaceholder( sErrorMessage, "$2$", _rParamValue2 );
        lcl_substitutePlaceholder( sErrorMessage, "$3$", _rParamValue3 );

        return sErrorMessage;
    }


    ErrorCode SQLError_Impl::getErrorCode( const ErrorCondition _eCondition )
    {
        return 0 - ::sal::static_int_cast< ErrorCode, ErrorCondition >( _eCondition );
    }


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


    void SQLError_Impl::raiseException( const ErrorCondition _eCondition, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        raiseTypedException(
            _eCondition,
            nullptr,
            ::cppu::UnoType< SQLException >::get(),
            _rParamValue1,
            _rParamValue2,
            _rParamValue3
        );
    }

    void SQLError_Impl::raiseTypedException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const Type& _rExceptionType, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        if ( !::cppu::UnoType< SQLException >::get().isAssignableFrom( _rExceptionType ) )
            throw std::bad_cast();

        // default-construct an exception of the desired type
        Any aException( nullptr, _rExceptionType );

        // fill it
        SQLException* pException = static_cast< SQLException* >( aException.pData );
        *pException = impl_buildSQLException( _eCondition, _rxContext, _rParamValue1, _rParamValue2, _rParamValue3 );

        // throw it
        ::cppu::throwException( aException );
    }

    SQLException SQLError_Impl::getSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        return impl_buildSQLException( _eCondition, _rxContext, _rParamValue1, _rParamValue2, _rParamValue3 );
    }

    SQLException SQLError_Impl::impl_buildSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 )
    {
        return SQLException(
            getErrorMessage( _eCondition, _rParamValue1, _rParamValue2, _rParamValue3 ),
            _rxContext,
            impl_getSQLState( _eCondition ),
            getErrorCode( _eCondition ),
            Any()
        );
    }

    OUString SQLError_Impl::impl_getErrorMessage( ErrorCondition _eCondition )
    {
        OUStringBuffer aMessage;

        OUString sResMessage(Translate::get(lcl_getResourceErrorID(_eCondition), m_aResources));
        OSL_ENSURE( !sResMessage.isEmpty(), "SQLError_Impl::impl_getErrorMessage: illegal error condition, or invalid resource!" );
        aMessage.append( getMessagePrefix() ).append( " " ).append( sResMessage );

        return aMessage.makeStringAndClear();
    }

    OUString SQLError_Impl::impl_getSQLState( ErrorCondition _eCondition )
    {
        OUString sState = lcl_getResourceState(_eCondition);
        if (sState.isEmpty())
            sState = OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "S1000" ) );
        return sState;
    }

    SQLError::SQLError()
        :m_pImpl( new SQLError_Impl )
    {
    }


    SQLError::~SQLError()
    {
    }


    const OUString& SQLError::getMessagePrefix()
    {
        return SQLError_Impl::getMessagePrefix();
    }


    OUString SQLError::getErrorMessage( const ErrorCondition _eCondition ) const
    {
        return m_pImpl->getErrorMessage( _eCondition, ParamValue(), ParamValue(), ParamValue() );
    }


    ErrorCode SQLError::getErrorCode( const ErrorCondition _eCondition )
    {
        return SQLError_Impl::getErrorCode( _eCondition );
    }


    void SQLError::raiseException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext, const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 ) const
    {
        m_pImpl->raiseException( _eCondition, _rxContext, _rParamValue1, _rParamValue2, _rParamValue3 );
    }


    void SQLError::raiseException( const ErrorCondition _eCondition ) const
    {
        m_pImpl->raiseException( _eCondition, ParamValue(), ParamValue(), ParamValue() );
    }


    void SQLError::raiseTypedException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const Type& _rExceptionType ) const
    {
        m_pImpl->raiseTypedException( _eCondition, _rxContext, _rExceptionType, ParamValue(), ParamValue(), ParamValue() );
    }


    SQLException SQLError::getSQLException( const ErrorCondition _eCondition, const Reference< XInterface >& _rxContext,
        const ParamValue& _rParamValue1, const ParamValue& _rParamValue2, const ParamValue& _rParamValue3 ) const
    {
        return m_pImpl->getSQLException( _eCondition, _rxContext, _rParamValue1, _rParamValue2, _rParamValue3 );
    }


} // namespace connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
