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

#include "table/defaultinputhandler.hxx"
#include "table/gridtablerenderer.hxx"
#include "table/tablecontrol.hxx"

#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/awt/grid/XGridColumnListener.hpp>

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <boost/noncopyable.hpp>


namespace svt { namespace table
{


    using css::uno::Reference;
    using css::uno::RuntimeException;
    using css::uno::Sequence;
    using css::uno::UNO_QUERY_THROW;
    using css::uno::UNO_QUERY;
    using css::awt::grid::XGridColumn;
    using css::uno::XInterface;
    using css::uno::Exception;
    using css::awt::grid::XGridColumnListener;
    using css::lang::EventObject;
    using css::awt::grid::GridColumnEvent;
    using css::uno::Any;
    using css::style::HorizontalAlignment_LEFT;
    using css::style::HorizontalAlignment;


    namespace
    {
        template< class T1, class T2 >
        void lcl_set( Reference< XGridColumn > const & i_column, void ( SAL_CALL XGridColumn::*i_setter )( T1 ),
            T2 i_value )
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


    //= ColumnChangeMultiplexer

    typedef ::cppu::WeakImplHelper <   XGridColumnListener
                                    >   ColumnChangeMultiplexer_Base;
    class ColumnChangeMultiplexer   :public ColumnChangeMultiplexer_Base
                                    ,public ::boost::noncopyable
    {
    public:
        explicit ColumnChangeMultiplexer( UnoGridColumnFacade& i_colImpl );

        void dispose();

    protected:
        virtual ~ColumnChangeMultiplexer();

        // XGridColumnListener
        virtual void SAL_CALL columnChanged( const GridColumnEvent& i_event ) throw (RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& i_event ) throw (RuntimeException, std::exception) override;

    private:
        UnoGridColumnFacade* m_pColumnImplementation;
    };


    ColumnChangeMultiplexer::ColumnChangeMultiplexer( UnoGridColumnFacade& i_colImpl )
        :m_pColumnImplementation( &i_colImpl )
    {
    }


    ColumnChangeMultiplexer::~ColumnChangeMultiplexer()
    {
    }


    void ColumnChangeMultiplexer::dispose()
    {
        DBG_TESTSOLARMUTEX();
        m_pColumnImplementation = nullptr;
    }


    void SAL_CALL ColumnChangeMultiplexer::columnChanged( const GridColumnEvent& i_event ) throw (RuntimeException, std::exception)
    {
        if ( i_event.AttributeName == "DataColumnIndex" )
        {
            SolarMutexGuard aGuard;
            if ( m_pColumnImplementation != nullptr )
                m_pColumnImplementation->dataColumnIndexChanged();
            return;
        }

        ColumnAttributeGroup nChangedAttributes( ColumnAttributeGroup::NONE );

        if ( i_event.AttributeName == "HorizontalAlign" )
            nChangedAttributes |= ColumnAttributeGroup::APPEARANCE;

        if  (   i_event.AttributeName == "ColumnWidth"
            ||  i_event.AttributeName == "MaxWidth"
            ||  i_event.AttributeName == "MinWidth"
            ||  i_event.AttributeName == "PreferredWidth"
            ||  i_event.AttributeName == "Resizeable"
            ||  i_event.AttributeName == "Flexibility"
            )
            nChangedAttributes |= ColumnAttributeGroup::WIDTH;

        OSL_ENSURE( nChangedAttributes != ColumnAttributeGroup::NONE,
            "ColumnChangeMultiplexer::columnChanged: unknown column attributed changed!" );

        SolarMutexGuard aGuard;
        if ( m_pColumnImplementation != nullptr )
            m_pColumnImplementation->columnChanged( nChangedAttributes );
    }


    void SAL_CALL ColumnChangeMultiplexer::disposing( const EventObject& ) throw (RuntimeException, std::exception)
    {
    }


    //= UnoGridColumnFacade


    UnoGridColumnFacade::UnoGridColumnFacade( UnoControlTableModel const & i_owner, Reference< XGridColumn > const & i_gridColumn )
        :m_pOwner( &i_owner )
        ,m_nDataColumnIndex( -1 )
        ,m_xGridColumn( i_gridColumn, UNO_QUERY_THROW )
        ,m_pChangeMultiplexer( new ColumnChangeMultiplexer( *this ) )
    {
        m_xGridColumn->addGridColumnListener( m_pChangeMultiplexer.get() );
        impl_updateDataColumnIndex_nothrow();
    }


    UnoGridColumnFacade::~UnoGridColumnFacade()
    {
    }


    void UnoGridColumnFacade::dispose()
    {
        DBG_TESTSOLARMUTEX();
        ENSURE_OR_RETURN_VOID( m_pOwner != nullptr, "UnoGridColumnFacade::dispose: already disposed!" );

        m_xGridColumn->removeGridColumnListener( m_pChangeMultiplexer.get() );
        m_pChangeMultiplexer->dispose();
        m_pChangeMultiplexer.clear();
        m_xGridColumn.clear();
        m_pOwner = nullptr;
    }


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


    void UnoGridColumnFacade::dataColumnIndexChanged()
    {
        DBG_TESTSOLARMUTEX();
        impl_updateDataColumnIndex_nothrow();
        if ( m_pOwner != nullptr )
            m_pOwner->notifyAllDataChanged();
    }


    void UnoGridColumnFacade::columnChanged( ColumnAttributeGroup const i_attributeGroup )
    {
        DBG_TESTSOLARMUTEX();
        if ( m_pOwner != nullptr )
            m_pOwner->notifyColumnChange( m_pOwner->getColumnPos( *this ), i_attributeGroup );
    }


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


    bool UnoGridColumnFacade::isResizable() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", false );
        return lcl_get( m_xGridColumn, &XGridColumn::getResizeable );
    }


    sal_Int32 UnoGridColumnFacade::getFlexibility() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", 1 );
        return lcl_get( m_xGridColumn, &XGridColumn::getFlexibility );
    }


    TableMetrics UnoGridColumnFacade::getWidth() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", 0 );
        return lcl_get( m_xGridColumn, &XGridColumn::getColumnWidth );
    }


    void UnoGridColumnFacade::setWidth( TableMetrics _nWidth )
    {
        ENSURE_OR_RETURN_VOID( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!" );
        lcl_set( m_xGridColumn, &XGridColumn::setColumnWidth, _nWidth );
    }


    TableMetrics UnoGridColumnFacade::getMinWidth() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", 0 );
        return lcl_get( m_xGridColumn, &XGridColumn::getMinWidth );
    }


    TableMetrics UnoGridColumnFacade::getMaxWidth() const
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", 0 );
        return lcl_get( m_xGridColumn, &XGridColumn::getMaxWidth );
    }


    css::style::HorizontalAlignment UnoGridColumnFacade::getHorizontalAlign()
    {
        ENSURE_OR_RETURN( m_xGridColumn.is(), "UnoGridColumnFacade: already disposed!", HorizontalAlignment_LEFT );
        return lcl_get( m_xGridColumn, &XGridColumn::getHorizontalAlign );
    }


} } // svt::table


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
