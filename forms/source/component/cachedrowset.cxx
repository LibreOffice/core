/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedrowset.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:56:14 $
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
#include "precompiled_forms.hxx"

#include "cachedrowset.hxx"
#include "services.hxx"
#include "frm_strings.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
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
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::sdbc::XRowSet;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::uno::Any;
    /** === end UNO using === **/
    namespace CommandType = ::com::sun::star::sdb::CommandType;

    //====================================================================
    //= CachedRowSet_Data
    //====================================================================
    struct CachedRowSet_Data
    {
        ::comphelper::ComponentContext  aContext;
        ::rtl::OUString                 sDataSource;
        ::rtl::OUString                 sCommand;
        sal_Int32                       nCommandType;
        sal_Bool                        bEscapeProcessing;
        Reference< XConnection >        xConnection;

        Reference< XRowSet >            xRowSet;
        bool                            bRowSetDirty;

        CachedRowSet_Data( const ::comphelper::ComponentContext& _rContext )
            :aContext( _rContext )
            ,sDataSource()
            ,sCommand()
            ,nCommandType( CommandType::COMMAND )
            ,bEscapeProcessing( sal_True )
            ,xConnection()
            ,xRowSet()
            ,bRowSetDirty( true )
        {
        }
    };

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        void lcl_clearRowSet_throw( CachedRowSet_Data& _rData )
        {
            if ( !_rData.xRowSet.is() )
                return;

            Reference< XComponent > xRowSetComp( _rData.xRowSet, UNO_QUERY_THROW );
            xRowSetComp->dispose();

            _rData.xRowSet.clear();
            _rData.bRowSetDirty = true;
        }
    }

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
    void CachedRowSet::setDataSource( const ::rtl::OUString& _rDataSource )
    {
        if ( m_pData->sDataSource == _rDataSource )
            return;

        m_pData->sDataSource = _rDataSource;
        m_pData->bRowSetDirty = true;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setCommand( const ::rtl::OUString& _rCommand )
    {
        if ( m_pData->sCommand == _rCommand )
            return;

        m_pData->sCommand = _rCommand;
        m_pData->bRowSetDirty = true;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setCommandType( const sal_Int32 _nCommandType )
    {
        if ( m_pData->nCommandType == _nCommandType )
            return;

        m_pData->nCommandType = _nCommandType;
        m_pData->bRowSetDirty = true;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setEscapeProcessing ( const sal_Bool _bEscapeProcessing )
    {
        if ( m_pData->bEscapeProcessing == _bEscapeProcessing )
            return;

        m_pData->bEscapeProcessing = _bEscapeProcessing;
        m_pData->bRowSetDirty = true;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setConnection( const Reference< XConnection >& _rxConnection )
    {
        if ( m_pData->xConnection == _rxConnection )
            return;

        m_pData->xConnection = _rxConnection;
        m_pData->bRowSetDirty = true;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setDataSource( const Any& _rDataSourceValue )
    {
        ::rtl::OUString sDataSource;
        OSL_VERIFY( _rDataSourceValue >>= sDataSource );
        setDataSource( sDataSource );
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setCommand( const Any& _rCommandValue )
    {
        ::rtl::OUString sCommand;
        OSL_VERIFY( _rCommandValue >>= sCommand );
        setCommand( sCommand );
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setEscapeProcessing( const Any& _rEscapeProcessingValue )
    {
        sal_Bool bEscapeProcessing( sal_False );
        OSL_VERIFY( _rEscapeProcessingValue >>= bEscapeProcessing );
        setEscapeProcessing( bEscapeProcessing );
    }

    //--------------------------------------------------------------------
    void CachedRowSet::setConnection( const Any& _rConnectionValue )
    {
        Reference< XConnection > xConnection;
        OSL_VERIFY( _rConnectionValue >>= xConnection );
        setConnection( xConnection );
    }

    //--------------------------------------------------------------------
    void CachedRowSet::execute()
    {
        try
        {
            if ( m_pData->bRowSetDirty )
                lcl_clearRowSet_throw( *m_pData );

            if ( !m_pData->xRowSet.is() )
                m_pData->aContext.createComponent( "com.sun.star.sdb.RowSet", m_pData->xRowSet );

            Reference< XPropertySet > xRowSetProps( m_pData->xRowSet, UNO_QUERY_THROW );
            xRowSetProps->setPropertyValue( PROPERTY_DATASOURCE, makeAny( m_pData->sDataSource ) );
            xRowSetProps->setPropertyValue( PROPERTY_COMMAND, makeAny( m_pData->sCommand ) );
            xRowSetProps->setPropertyValue( PROPERTY_COMMANDTYPE, makeAny( m_pData->nCommandType ) );
            xRowSetProps->setPropertyValue( PROPERTY_ESCAPE_PROCESSING, makeAny( m_pData->bEscapeProcessing ) );
            xRowSetProps->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( m_pData->xConnection ) );

            m_pData->xRowSet->execute();
            m_pData->bRowSetDirty = false;
        }
        catch( const SQLException& )
        {
            throw;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    bool CachedRowSet::isDirty() const
    {
        return m_pData->bRowSetDirty;
    }

    //--------------------------------------------------------------------
    const Reference< XRowSet >& CachedRowSet::getRowSet() const
    {
        return m_pData->xRowSet;
    }

    //--------------------------------------------------------------------
    void CachedRowSet::dispose()
    {
        try
        {
            lcl_clearRowSet_throw( *m_pData );
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
