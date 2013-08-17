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

#include "defaultobjectnamecheck.hxx"

#include "dbu_misc.hrc"

#include "moduledbu.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/tools/XConnectionTools.hpp>

#include <connectivity/dbexception.hxx>
#include <connectivity/dbmetadata.hxx>

#include <rtl/ustrbuf.hxx>

#include <tools/diagnose_ex.h>
#include <tools/string.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <vector>
#include <boost/shared_ptr.hpp>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::container::XHierarchicalNameAccess;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbcx::XTablesSupplier;
    using ::com::sun::star::sdb::XQueriesSupplier;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::sdb::tools::XObjectNames;
    using ::com::sun::star::sdb::tools::XConnectionTools;
    using ::com::sun::star::uno::UNO_QUERY;

    using namespace dbtools;

    namespace CommandType = ::com::sun::star::sdb::CommandType;

    // helper
    namespace
    {
        void lcl_fillNameExistsError( const OUString& _rObjectName, SQLExceptionInfo& _out_rErrorToDisplay )
        {
            SQLException aError;
            OUString sErrorMessage = ModuleRes(STR_NAMED_OBJECT_ALREADY_EXISTS).toString();
            aError.Message = sErrorMessage.replaceAll("$#$", _rObjectName);
            _out_rErrorToDisplay = aError;
        }

    }

    // HierarchicalNameCheck_Impl
    struct HierarchicalNameCheck_Impl
    {
        Reference< XHierarchicalNameAccess >    xHierarchicalNames;
        OUString                         sRelativeRoot;
    };

    // HierarchicalNameCheck
    HierarchicalNameCheck::HierarchicalNameCheck( const Reference< XHierarchicalNameAccess >& _rxNames, const OUString& _rRelativeRoot )
        :m_pImpl( new HierarchicalNameCheck_Impl )
    {
        m_pImpl->xHierarchicalNames = _rxNames;
        m_pImpl->sRelativeRoot = _rRelativeRoot;

        if ( !m_pImpl->xHierarchicalNames.is() )
            throw IllegalArgumentException();
    }

    HierarchicalNameCheck::~HierarchicalNameCheck()
    {
    }

    bool HierarchicalNameCheck::isNameValid( const OUString& _rObjectName, SQLExceptionInfo& _out_rErrorToDisplay ) const
    {
        try
        {
            OUStringBuffer aCompleteName;
            if ( !m_pImpl->sRelativeRoot.isEmpty() )
            {
                aCompleteName.append( m_pImpl->sRelativeRoot );
                aCompleteName.appendAscii( "/" );
            }
            aCompleteName.append( _rObjectName );

            OUString sCompleteName( aCompleteName.makeStringAndClear() );
            if ( !m_pImpl->xHierarchicalNames->hasByHierarchicalName( sCompleteName ) )
                return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        lcl_fillNameExistsError( _rObjectName, _out_rErrorToDisplay );
        return false;
    }

    // DynamicTableOrQueryNameCheck_Impl
    struct DynamicTableOrQueryNameCheck_Impl
    {
        sal_Int32                   nCommandType;
        Reference< XObjectNames >   xObjectNames;
    };

    // DynamicTableOrQueryNameCheck
    DynamicTableOrQueryNameCheck::DynamicTableOrQueryNameCheck( const Reference< XConnection >& _rxSdbLevelConnection, sal_Int32 _nCommandType )
        :m_pImpl( new DynamicTableOrQueryNameCheck_Impl )
    {
        Reference< XConnectionTools > xConnTools( _rxSdbLevelConnection, UNO_QUERY );
        if ( xConnTools.is() )
            m_pImpl->xObjectNames.set( xConnTools->getObjectNames() );
        if ( !m_pImpl->xObjectNames.is() )
            throw IllegalArgumentException();

        if ( ( _nCommandType != CommandType::QUERY ) && ( _nCommandType != CommandType::TABLE ) )
            throw IllegalArgumentException();
        m_pImpl->nCommandType = _nCommandType;
    }

    DynamicTableOrQueryNameCheck::~DynamicTableOrQueryNameCheck()
    {
    }

    bool DynamicTableOrQueryNameCheck::isNameValid( const OUString& _rObjectName, ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay ) const
    {
        try
        {
            m_pImpl->xObjectNames->checkNameForCreate( m_pImpl->nCommandType, _rObjectName );
            return true;
        }
        catch( const SQLException& )
        {
            _out_rErrorToDisplay = ::dbtools::SQLExceptionInfo( ::cppu::getCaughtException() );
        }
        return false;
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
