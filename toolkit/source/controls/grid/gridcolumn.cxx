/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_toolkit.hxx"
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
        ,m_nColumnWidth(4)
        ,m_nPreferredWidth(0)
        ,m_nMaxWidth(0)
        ,m_nMinWidth(0)
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
        ,m_nColumnWidth( i_copySource.m_nColumnWidth )
        ,m_nPreferredWidth( i_copySource.m_nPreferredWidth )
        ,m_nMaxWidth( i_copySource.m_nMaxWidth )
        ,m_nMinWidth( i_copySource.m_nMinWidth )
        ,m_bResizeable( i_copySource.m_bResizeable )
        ,m_eHorizontalAlign( i_copySource.m_eHorizontalAlign )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    GridColumn::~GridColumn()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridColumn::broadcast_changed( sal_Char const * const i_asciiAttributeName, Any i_oldValue, Any i_newValue, ::osl::ClearableMutexGuard& i_Guard )
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
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_aIdentifier;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setIdentifier(const ::com::sun::star::uno::Any & value) throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aIdentifier = value;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getColumnWidth() throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_nColumnWidth;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setColumnWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_nColumnWidth, value, "ColumnWidth" );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getPreferredWidth() throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_nPreferredWidth;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setPreferredWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_nPreferredWidth, value, "PreferredWidth" );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getMaxWidth() throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
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
        ::osl::MutexGuard aGuard( m_aMutex );
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
        ::osl::MutexGuard aGuard( m_aMutex );
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
        ::osl::MutexGuard aGuard( m_aMutex );
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
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_bResizeable;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::setResizeable(sal_Bool value) throw (::com::sun::star::uno::RuntimeException)
    {
        impl_set( m_bResizeable, value, "Resizeable" );
    }

    //------------------------------------------------------------------------------------------------------------------
    HorizontalAlignment SAL_CALL GridColumn::getHorizontalAlign() throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
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
    void SAL_CALL GridColumn::dispose() throw (RuntimeException)
    {
        // simply disambiguate, the base class handles this
        GridColumn_Base::dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::addEventListener( const Reference< XEventListener >& i_listener ) throw (RuntimeException)
    {
        // simply disambiguate, the base class handles this
        GridColumn_Base::addEventListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridColumn::removeEventListener( const Reference< XEventListener >& i_listener ) throw (RuntimeException)
    {
        // simply disambiguate, the base class handles this
        GridColumn_Base::removeEventListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL GridColumn::getIndex() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_nIndex;
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridColumn::setIndex( sal_Int32 const i_index )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nIndex = i_index;
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

