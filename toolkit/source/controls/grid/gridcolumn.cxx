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
#include <toolkit/helper/servicenames.hxx>

#define COLWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ColWidth" ))
#define MAXWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MaxWidth" ))
#define MINWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MinWidth" ))
#define PREFWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PrefWidth" ))
#define HALIGN ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HAlign" ))
#define TITLE ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Title" ))
#define COLRESIZE ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ColumnResize" ))

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
        ,m_nIndex(0)
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
        ,m_nIndex( i_copySource.m_nIndex )
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
    void GridColumn::broadcast_changed( ::rtl::OUString name, Any oldValue, Any newValue, ::osl::ClearableMutexGuard& i_Guard )
    {
        Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
        GridColumnEvent aEvent( xSource, name, oldValue, newValue, m_nIndex);

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
        value >>= m_aIdentifier;
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
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( m_nColumnWidth == value )
            return;

        m_nColumnWidth = value;
        broadcast_changed( COLWIDTH, Any( m_nColumnWidth ), Any( value ), aGuard );
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
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( m_nPreferredWidth == value )
            return;

        m_nPreferredWidth = value;
        broadcast_changed( PREFWIDTH, Any( m_nPreferredWidth ), Any( value ), aGuard );
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
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( m_nMaxWidth == value )
            return;

        m_nMaxWidth = value;
        broadcast_changed( MAXWIDTH, Any( m_nMaxWidth ), Any( value ), aGuard );
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
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( m_nMinWidth == value )
            return;

        m_nMinWidth = value;
        broadcast_changed( MINWIDTH, Any( m_nMinWidth ), Any( value ), aGuard );
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
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( m_sTitle == value )
            return;

        m_sTitle = value;
        broadcast_changed( TITLE, Any( m_sTitle ), Any( value ), aGuard );
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
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( m_bResizeable == value )
            return;

        m_bResizeable = value;
        broadcast_changed( COLRESIZE, Any( m_bResizeable ), Any( value ), aGuard );
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
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( m_eHorizontalAlign == align )
            return;

        m_eHorizontalAlign = align;
        broadcast_changed( HALIGN, Any( m_eHorizontalAlign ), Any( align ), aGuard );
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
    void SAL_CALL GridColumn::setIndex(sal_Int32 _nIndex) throw (::com::sun::star::uno::RuntimeException)
    {
        m_nIndex = _nIndex;
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
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GridColumn_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& )
{
    return *( new ::toolkit::GridColumn );
}

