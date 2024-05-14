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

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace toolkit
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::awt::grid;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::style;


    //= DefaultGridColumnModel


    GridColumn::GridColumn()
        :m_nIndex(-1)
        ,m_nDataColumnIndex(-1)
        ,m_nColumnWidth(4)
        ,m_nMaxWidth(0)
        ,m_nMinWidth(0)
        ,m_nFlexibility(1)
        ,m_bResizeable(true)
        ,m_eHorizontalAlign( HorizontalAlignment_LEFT )
    {
    }


    GridColumn::GridColumn( GridColumn const & i_copySource )
        :m_aIdentifier( i_copySource.m_aIdentifier )
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


    GridColumn::~GridColumn()
    {
    }


    void GridColumn::broadcast_changed( const OUString& i_asciiAttributeName, const Any& i_oldValue, const Any& i_newValue,
        std::unique_lock<std::mutex>& i_Guard )
    {
        Reference< XInterface > const xSource( getXWeak() );
        GridColumnEvent const aEvent(
            xSource, i_asciiAttributeName,
            i_oldValue, i_newValue, m_nIndex
        );

        maGridColumnListeners.notifyEach( i_Guard, &XGridColumnListener::columnChanged, aEvent );
    }


    css::uno::Any SAL_CALL GridColumn::getIdentifier()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_aIdentifier;
    }


    void SAL_CALL GridColumn::setIdentifier(const css::uno::Any & value)
    {
        std::unique_lock aGuard( m_aMutex );
        m_aIdentifier = value;
    }


    ::sal_Int32 SAL_CALL GridColumn::getColumnWidth()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_nColumnWidth;
    }


    void SAL_CALL GridColumn::setColumnWidth(::sal_Int32 value)
    {
        impl_set( m_nColumnWidth, value, u"ColumnWidth"_ustr );
    }


    ::sal_Int32 SAL_CALL GridColumn::getMaxWidth()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_nMaxWidth;
    }


    void SAL_CALL GridColumn::setMaxWidth(::sal_Int32 value)
    {
        impl_set( m_nMaxWidth, value, u"MaxWidth"_ustr );
    }


    ::sal_Int32 SAL_CALL GridColumn::getMinWidth()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_nMinWidth;
    }


    void SAL_CALL GridColumn::setMinWidth(::sal_Int32 value)
    {
        impl_set( m_nMinWidth, value, u"MinWidth"_ustr );
    }


    OUString SAL_CALL GridColumn::getTitle()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_sTitle;
    }


    void SAL_CALL GridColumn::setTitle(const OUString & value)
    {
        impl_set( m_sTitle, value, u"Title"_ustr );
    }


    OUString SAL_CALL GridColumn::getHelpText()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_sHelpText;
    }


    void SAL_CALL GridColumn::setHelpText( const OUString & value )
    {
        impl_set( m_sHelpText, value, u"HelpText"_ustr );
    }


    sal_Bool SAL_CALL GridColumn::getResizeable()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_bResizeable;
    }


    void SAL_CALL GridColumn::setResizeable(sal_Bool value)
    {
        impl_set( m_bResizeable, bool(value), u"Resizeable"_ustr );
    }


    ::sal_Int32 SAL_CALL GridColumn::getFlexibility()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_nFlexibility;
    }


    void SAL_CALL GridColumn::setFlexibility( ::sal_Int32 i_value )
    {
        if ( i_value < 0 )
            throw IllegalArgumentException( OUString(), *this, 1 );
        impl_set( m_nFlexibility, i_value, u"Flexibility"_ustr );
    }


    HorizontalAlignment SAL_CALL GridColumn::getHorizontalAlign()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_eHorizontalAlign;
    }


    void SAL_CALL GridColumn::setHorizontalAlign(HorizontalAlignment align)
    {
        impl_set( m_eHorizontalAlign, align, u"HorizontalAlign"_ustr );
    }


    void SAL_CALL GridColumn::addGridColumnListener( const Reference< XGridColumnListener >& xListener )
    {
        std::unique_lock aGuard( m_aMutex );
        maGridColumnListeners.addInterface( aGuard, xListener );
    }


    void SAL_CALL GridColumn::removeGridColumnListener( const Reference< XGridColumnListener >& xListener )
    {
        std::unique_lock aGuard( m_aMutex );
        maGridColumnListeners.removeInterface( aGuard, xListener );
    }


    void GridColumn::disposing(std::unique_lock<std::mutex>&)
    {
        m_aIdentifier.clear();
        m_sTitle.clear();
        m_sHelpText.clear();
    }


    ::sal_Int32 SAL_CALL GridColumn::getIndex()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_nIndex;
    }


    void GridColumn::setIndex( sal_Int32 const i_index )
    {
        std::unique_lock aGuard( m_aMutex );
        m_nIndex = i_index;
    }


    ::sal_Int32 SAL_CALL GridColumn::getDataColumnIndex()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_nDataColumnIndex;
    }


    void SAL_CALL GridColumn::setDataColumnIndex( ::sal_Int32 i_dataColumnIndex )
    {
        impl_set( m_nDataColumnIndex, i_dataColumnIndex, u"DataColumnIndex"_ustr );
    }


    OUString SAL_CALL GridColumn::getImplementationName(  )
    {
        return u"org.openoffice.comp.toolkit.GridColumn"_ustr;
    }

    sal_Bool SAL_CALL GridColumn::supportsService( const OUString& i_serviceName )
    {
        return cppu::supportsService(this, i_serviceName);
    }

    css::uno::Sequence< OUString > SAL_CALL GridColumn::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.awt.grid.GridColumn"_ustr };
    }


    Reference< XCloneable > SAL_CALL GridColumn::createClone(  )
    {
        return new GridColumn( *this );
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_openoffice_comp_toolkit_GridColumn_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::GridColumn());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
