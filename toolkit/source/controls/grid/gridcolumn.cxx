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

#include "gridcolumn.hxx"

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/servicenames.hxx>

namespace toolkit
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::awt::grid;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::style;

    //==================================================================================================================
    //= DefaultGridColumnModel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    GridColumn::GridColumn()
        :GridColumn_Base( m_aMutex )
        ,m_aIdentifier()
        ,m_nIndex(-1)
        ,m_nDataColumnIndex(-1)
        ,m_nColumnWidth(4)
        ,m_nMaxWidth(0)
        ,m_nMinWidth(0)
        ,m_nFlexibility(1)
        ,m_bResizeable(true)
        ,m_eHorizontalAlign( HorizontalAlignment_LEFT )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    GridColumn::GridColumn( GridColumn const & i_copySource )
        :cppu::BaseMutex()
        ,GridColumn_Base( m_aMutex )
        ,m_aIdentifier( i_copySource.m_aIdentifier )
        ,m_nIndex( -1 )
        ,m_nDataColumnIndex( i_copySource.m_nDataColumnIndex )
        ,m_nColumnWidth( i_copySource.m_nColumnWidth )
        ,m_nMaxWidth( i_copySource.m_nMaxWidth )
        ,m_nMinWidth( i_copySource.m_nMinWidth )
        ,m_nFlexibility( i_copySource.m_nFlexibility )
        ,m_bResizeable( i_copySource.m_bResizeable )
        ,m_sTitle( i_copySource.m_sTitle )
        ,m_sHelpText( i_copySource.m_sHelpText )
        ,m_eHorizontalAlign( i_copySource.m_eHorizontalAlign )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    GridColumn::~GridColumn()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridColumn::broadcast_changed( sal_Char const * const i_asciiAttributeName, Any i_oldValue, Any i_newValue,
        ::comphelper::ComponentGuard& i_Guard )
    {
        Reference< XInterface > const xSource( static_cast< ::cppu::OWeakObject* >( this ) );
        GridColumnEvent const aEvent(
            xSource, ::rtl::OUString::createFromAscii( i_asciiAttributeName ),
            i_oldValue, i_newValue, m_nIndex
        );

        ::cppu::OInterfaceContainerHelper* pIter = rBHelper.getContainer( XGridColumnListener::static_type() );

        i_Guard.clear();
        if( pIter )
            pIter->notifyEach( &XGridColumnListener::columnChanged, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::uno::Any SAL_CALL GridColumn::getIdentifier() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_aIdentifier;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setIdentifier(const ::com::sun::star::uno::Any & value) throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        m_aIdentifier = value;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getColumnWidth() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_nColumnWidth;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setColumnWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_nColumnWidth, value, "ColumnWidth" );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getMaxWidth() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_nMaxWidth;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setMaxWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_nMaxWidth, value, "MaxWidth" );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getMinWidth() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_nMinWidth;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setMinWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_nMinWidth, value, "MinWidth" );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL GridColumn::getTitle() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_sTitle;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setTitle(const ::rtl::OUString & value) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_sTitle, value, "Title" );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL GridColumn::getHelpText() throw (RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_sHelpText;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setHelpText( const ::rtl::OUString & value ) throw (RuntimeException)
    {
        impl_set( m_sHelpText, value, "HelpText" );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool SAL_CALL GridColumn::getResizeable() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_bResizeable;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setResizeable(sal_Bool value) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_bResizeable, value, "Resizeable" );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getFlexibility() throw (RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_nFlexibility;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setFlexibility( ::sal_Int32 i_value ) throw (IllegalArgumentException, RuntimeException)
    {
        if ( i_value < 0 )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );
        impl_set( m_nFlexibility, i_value, "Flexibility" );
    }

    //------------------------------------------------------------------------------------------------------------------
    HorizontalAlignment SAL_CALL GridColumn::getHorizontalAlign() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_eHorizontalAlign;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setHorizontalAlign(HorizontalAlignment align) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_eHorizontalAlign, align, "HorizontalAlign" );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::addGridColumnListener( const Reference< XGridColumnListener >& xListener ) throw (RuntimeException)
    {
        rBHelper.addListener( XGridColumnListener::static_type(), xListener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::removeGridColumnListener( const Reference< XGridColumnListener >& xListener ) throw (RuntimeException)
    {
        rBHelper.removeListener( XGridColumnListener::static_type(), xListener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aIdentifier.clear();
        m_sTitle = m_sHelpText = ::rtl::OUString();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getIndex() throw (RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_nIndex;
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridColumn::setIndex( sal_Int32 const i_index )
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        m_nIndex = i_index;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getDataColumnIndex() throw(RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_nDataColumnIndex;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setDataColumnIndex( ::sal_Int32 i_dataColumnIndex ) throw(RuntimeException)
    {
        impl_set( m_nDataColumnIndex, i_dataColumnIndex, "DataColumnIndex" );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL GridColumn::getImplementationName(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.toolkit.GridColumn" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool SAL_CALL GridColumn::supportsService( const ::rtl::OUString& i_serviceName ) throw (RuntimeException)
    {
        const Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() );
        for ( sal_Int32 i=0; i<aServiceNames.getLength(); ++i )
            if ( aServiceNames[i] == i_serviceName )
                return sal_True;
        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL GridColumn::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        const ::rtl::OUString aServiceName( ::rtl::OUString::createFromAscii( szServiceName_GridColumn ) );
        const Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
        return aSeq;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XCloneable > SAL_CALL GridColumn::createClone(  ) throw (RuntimeException)
    {
        return new GridColumn( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int64 SAL_CALL GridColumn::getSomething( const Sequence< sal_Int8 >& i_identifier ) throw(RuntimeException)
    {
        if ( ( i_identifier.getLength() == 16 ) && ( i_identifier == getUnoTunnelId() ) )
            return ::sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ) );
        return 0;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< sal_Int8 > GridColumn::getUnoTunnelId() throw()
    {
        static ::cppu::OImplementationId const aId;
        return aId.getImplementationId();
    }

    //------------------------------------------------------------------------------------------------------------------
    GridColumn* GridColumn::getImplementation( const Reference< XInterface >& i_component )
    {
        Reference< XUnoTunnel > const xTunnel( i_component, UNO_QUERY );
        if ( xTunnel.is() )
            return reinterpret_cast< GridColumn* >( ::sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething( getUnoTunnelId() ) ) );
        return NULL;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GridColumn_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& )
{
    return *( new ::toolkit::GridColumn );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
