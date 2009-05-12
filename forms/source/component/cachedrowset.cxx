/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedrowset.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "cachedrowset.hxx"
#include "services.hxx"
#include "frm_strings.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//........................................................................
namespace frm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::sdb::XQueriesSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::sdbc::XStatement;
    /** === end UNO using === **/
    namespace ResultSetType = ::com::sun::star::sdbc::ResultSetType;

    //====================================================================
    //= CachedRowSet_Data
    //====================================================================
    struct CachedRowSet_Data
    {
        ::comphelper::ComponentContext  aContext;
        ::rtl::OUString                 sCommand;
        sal_Bool                        bEscapeProcessing;
        Reference< XConnection >        xConnection;

        bool                            bStatementDirty;

        CachedRowSet_Data( const ::comphelper::ComponentContext& _rContext )
            :aContext( _rContext )
            ,sCommand()
            ,bEscapeProcessing( sal_False )
            ,xConnection()
            ,bStatementDirty( true )
        {
        }
    };

    //====================================================================
    //= CachedRowSet
    //====================================================================
    //--------------------------------------------------------------------
    CachedRowSet::CachedRowSet( const ::comphelper::ComponentContext& _rContext )
        :m_pData( new CachedRowSet_Data( _rContext ) )
    {
    }

    //--------------------------------------------------------------------
    CachedRowSet::~CachedRowSet()
    {
        dispose();
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setCommand( const ::rtl::OUString& _rCommand )
    {
        if ( m_pData->sCommand == _rCommand )
            return;

        m_pData->sCommand = _rCommand;
        m_pData->bStatementDirty = true;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setCommandFromQuery( const ::rtl::OUString& _rQueryName )
    {
        Reference< XQueriesSupplier > xSupplyQueries( m_pData->xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess >      xQueries      ( xSupplyQueries->getQueries(), UNO_QUERY_THROW );
        Reference< XPropertySet >     xQuery        ( xQueries->getByName( _rQueryName ), UNO_QUERY_THROW );

        sal_Bool bEscapeProcessing( sal_False );
        OSL_VERIFY( xQuery->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bEscapeProcessing );
        setEscapeProcessing( bEscapeProcessing );

        ::rtl::OUString sCommand;
        OSL_VERIFY( xQuery->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand );
        setCommand( sCommand );
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setEscapeProcessing ( const sal_Bool _bEscapeProcessing )
    {
        if ( m_pData->bEscapeProcessing == _bEscapeProcessing )
            return;

        m_pData->bEscapeProcessing = _bEscapeProcessing;
        m_pData->bStatementDirty = true;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setConnection( const Reference< XConnection >& _rxConnection )
    {
        if ( m_pData->xConnection == _rxConnection )
            return;

        m_pData->xConnection = _rxConnection;
        m_pData->bStatementDirty = true;
    }

    //--------------------------------------------------------------------
    Reference< XResultSet > CachedRowSet::execute()
    {
        Reference< XResultSet > xResult;
        try
        {
            OSL_PRECOND( m_pData->xConnection.is(), "CachedRowSet::execute: how am I expected to do this without a connection?" );
            if ( !m_pData->xConnection.is() )
                return xResult;

            Reference< XStatement > xStatement( m_pData->xConnection->createStatement(), UNO_SET_THROW );
            Reference< XPropertySet > xStatementProps( xStatement, UNO_QUERY_THROW );
            xStatementProps->setPropertyValue( PROPERTY_ESCAPE_PROCESSING, makeAny( m_pData->bEscapeProcessing ) );
            xStatementProps->setPropertyValue( PROPERTY_RESULTSET_TYPE, makeAny( ResultSetType::FORWARD_ONLY ) );

            xResult.set( xStatement->executeQuery( m_pData->sCommand ), UNO_SET_THROW );
            m_pData->bStatementDirty = false;
        }
        catch( const SQLException& )
        {
            throw;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xResult;
    }

    //--------------------------------------------------------------------
    bool CachedRowSet::isDirty() const
    {
        return m_pData->bStatementDirty;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::dispose()
    {
        try
        {
            m_pData.reset( new CachedRowSet_Data( m_pData->aContext ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
} // namespace frm
//........................................................................
