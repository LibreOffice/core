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

#include "unogridcolumnfacade.hxx"
#include "unocontroltablemodel.hxx"

#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/table/tablecontrol.hxx"

#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/awt/grid/XGridColumnListener.hpp>

#include <comphelper/stlunosequence.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

// .....................................................................................................................
namespace svt { namespace table
{
// .....................................................................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::awt::grid::XGridColumn;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::awt::grid::XGridColumnListener;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::awt::grid::GridColumnEvent;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::style::HorizontalAlignment_LEFT;
    using ::com::sun::star::style::HorizontalAlignment;

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        template< class ATTRIBUTE_TYPE >
        void lcl_set( Reference< XGridColumn > const & i_column, void ( SAL_CALL XGridColumn::*i_setter )( ATTRIBUTE_TYPE ),
            ATTRIBUTE_TYPE i_value )
        {
            try
            {
                (i_column.get()->*i_setter) ( i_value );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        template< class ATTRIBUTE_TYPE >
        ATTRIBUTE_TYPE lcl_get( Reference< XGridColumn > const & i_column, ATTRIBUTE_TYPE ( SAL_CALL XGridColumn::*i_getter )() )
        {
            ATTRIBUTE_TYPE value = ATTRIBUTE_TYPE();
            try
            {
                value = (i_column.get()->*i_getter)();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return value;
        }
    }

    //==================================================================================================================
    //= ColumnChangeMultiplexer
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper1 <   XGridColumnListener
                                    >   ColumnChangeMultiplexer_Base;
    class ColumnChangeMultiplexer   :public ColumnChangeMultiplexer_Base
                                    ,public ::boost::noncopyable
    {
    public:
        ColumnChangeMultiplexer( UnoGridColumnFacade& i_colImpl );

        void dispose();

    protected:
        ~ColumnChangeMultiplexer();

        // XGridColumnListener
        virtual void SAL_CALL columnChanged( const GridColumnEvent& i_event ) throw (RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& i_event ) throw (RuntimeException);

    private:
        UnoGridColumnFacade* m_pColumnImplementation;
    };

    //------------------------------------------------------------------------------------------------------------------
    ColumnChangeMultiplexer::ColumnChangeMultiplexer( UnoGridColumnFacade& i_colImpl )
        :m_pColumnImplementation( &i_colImpl )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ColumnChangeMultiplexer::~ColumnChangeMultiplexer()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void ColumnChangeMultiplexer::dispose()
    {
        DBG_TESTSOLARMUTEX();
        m_pColumnImplementation = NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ColumnChangeMultiplexer::columnChanged( const GridColumnEvent& i_event ) throw (RuntimeException)
    {
        if ( i_event.AttributeName == "DataColumnIndex" )
        {
            SolarMutexGuard aGuard;
            if ( m_pColumnImplementation != NULL )
                m_pColumnImplementation->dataColumnIndexChanged();
            return;
        }

        ColumnAttributeGroup nChangedAttributes( COL_ATTRS_NONE );

        if ( i_event.AttributeName == "HorizontalAlign" )
            nChangedAttributes |= COL_ATTRS_APPEARANCE;

        if  (   i_event.AttributeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ColumnWidth"))
            ||  i_event.AttributeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MaxWidth"))
            ||  i_event.AttributeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MinWidth"))
            ||  i_event.AttributeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("PreferredWidth"))
            ||  i_event.AttributeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Resizeable"))
            ||  i_event.AttributeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Flexibility"))
            )
            nChangedAttributes |= COL_ATTRS_WIDTH;

        OSL_ENSURE( nChangedAttributes != COL_ATTRS_NONE,
            "ColumnChangeMultiplexer::columnChanged: unknown column attributed changed!" );

        SolarMutexGuard aGuard;
        if ( m_pColumnImplementation != NULL )
            m_pColumnImplementation->columnChanged( nChangedAttributes );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ColumnChangeMultiplexer::disposing( const EventObject& i_event ) throw (RuntimeException)
    {
        OSL_UNUSED( i_event );
    }

    //==================================================================================================================
    //= UnoGridColumnFacade
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    UnoGridColumnFacade::UnoGridColumnFacade( UnoControlTableModel const & i_owner, Reference< XGridColumn > const & i_gridColumn )
        :m_pOwner( &i_owner )
        ,m_nDataColumnIndex( -1 )
        ,m_xGridColumn( i_gridColumn, UNO_QUERY_THROW )
        ,m_pChangeMultiplexer( new ColumnChangeMultiplexer( *this ) )
    {
        m_xGridColumn->addGridColumnListener( m_pChangeMultiplexer.get() );
        impl_updateDataColumnIndex_nothrow();
    }

    //------------------------------------------------------------------------------------------------------------------
    UnoGridColumnFacade::~UnoGridColumnFacade()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::dispose()
    {
        DBG_TESTSOLARMUTEX();
        ENSURE_OR_RETURN_VOID( m_pOwner != NULL, "UnoGridColumnFacade::dispose: already disposed!" );

        m_xGridColumn->removeGridColumnListener( m_pChangeMultiplexer.get() );
        m_pChangeMultiplexer->dispose();
        m_pChangeMultiplexer.clear();
        m_xGridColumn.clear();
        m_pOwner = NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::impl_updateDataColumnIndex_nothrow()
    {
        m_nDataColumnIndex = -1;
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        try
        {
            m_nDataColumnIndex = m_xGridColumn->getDataColumnIndex();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::dataColumnIndexChanged()
    {
        DBG_TESTSOLARMUTEX();
        impl_updateDataColumnIndex_nothrow();
        if ( m_pOwner != NULL )
            m_pOwner->notifyAllDataChanged();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::columnChanged( ColumnAttributeGroup const i_attributeGroup )
    {
        DBG_TESTSOLARMUTEX();
        if ( m_pOwner != NULL )
            m_pOwner->notifyColumnChange( m_pOwner->getColumnPos( *this ), i_attributeGroup );
    }

    //------------------------------------------------------------------------------------------------------------------
    Any UnoGridColumnFacade::getID() const
    {
        Any aID;
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", aID );
        try
        {
            aID = m_xGridColumn->getIdentifier();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return aID;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setID( const Any& i_ID )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        try
        {
            m_xGridColumn->setIdentifier( i_ID );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    OUString UnoGridColumnFacade::getName() const
    {
        OUString sName;
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", sName );
        try
        {
            sName = m_xGridColumn->getTitle();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return sName;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setName( const OUString& _rName )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        try
        {
            m_xGridColumn->setTitle( _rName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    OUString UnoGridColumnFacade::getHelpText() const
    {
        OUString sHelpText;
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", sHelpText );
        try
        {
            sHelpText = m_xGridColumn->getHelpText();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return sHelpText;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setHelpText( const OUString& i_helpText )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        try
        {
            m_xGridColumn->setHelpText( i_helpText );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    bool UnoGridColumnFacade::isResizable() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", false );
        return lcl_get( m_xGridColumn, &XGridColumn::getResizeable );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setResizable( bool i_resizable )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        lcl_set( m_xGridColumn, &XGridColumn::setResizeable, sal_Bool( i_resizable ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int32 UnoGridColumnFacade::getFlexibility() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", 1 );
        return lcl_get( m_xGridColumn, &XGridColumn::getFlexibility );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setFlexibility( sal_Int32 const i_flexibility )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        lcl_set( m_xGridColumn, &XGridColumn::setFlexibility, i_flexibility );
    }

    //------------------------------------------------------------------------------------------------------------------
    TableMetrics UnoGridColumnFacade::getWidth() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", 0 );
        return lcl_get( m_xGridColumn, &XGridColumn::getColumnWidth );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setWidth( TableMetrics _nWidth )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        lcl_set( m_xGridColumn, &XGridColumn::setColumnWidth, _nWidth );
    }

    //------------------------------------------------------------------------------------------------------------------
    TableMetrics UnoGridColumnFacade::getMinWidth() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", 0 );
        return lcl_get( m_xGridColumn, &XGridColumn::getMinWidth );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setMinWidth( TableMetrics _nMinWidth )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        lcl_set( m_xGridColumn, &XGridColumn::setMinWidth, _nMinWidth );
    }

    //------------------------------------------------------------------------------------------------------------------
    TableMetrics UnoGridColumnFacade::getMaxWidth() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", 0 );
        return lcl_get( m_xGridColumn, &XGridColumn::getMaxWidth );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setMaxWidth( TableMetrics _nMaxWidth )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        lcl_set( m_xGridColumn, &XGridColumn::setMinWidth, _nMaxWidth );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::style::HorizontalAlignment UnoGridColumnFacade::getHorizontalAlign()
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", HorizontalAlignment_LEFT );
        return lcl_get( m_xGridColumn, &XGridColumn::getHorizontalAlign );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoGridColumnFacade::setHorizontalAlign( com::sun::star::style::HorizontalAlignment _align )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        lcl_set( m_xGridColumn, &XGridColumn::setHorizontalAlign, _align );
    }

// .....................................................................................................................
} } // svt::table
// .....................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
