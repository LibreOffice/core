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

#ifndef INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDCOLUMN_HXX
#define INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDCOLUMN_HXX

#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/componentguard.hxx>
#include <rtl/ref.hxx>
#include <toolkit/helper/mutexandbroadcasthelper.hxx>

#include <vector>

namespace toolkit
{

typedef ::cppu::WeakComponentImplHelper    <   css::awt::grid::XGridColumn
                                            ,   css::lang::XServiceInfo
                                            ,   css::lang::XUnoTunnel
                                            >   GridColumn_Base;
class GridColumn    :public ::cppu::BaseMutex
                    ,public GridColumn_Base
{
public:
    GridColumn();
    GridColumn( GridColumn const & i_copySource );
    virtual ~GridColumn();

    // css::awt::grid::XGridColumn
    virtual css::uno::Any SAL_CALL getIdentifier() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setIdentifier(const css::uno::Any & value) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getColumnWidth() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColumnWidth(::sal_Int32 the_value) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getMaxWidth() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMaxWidth(::sal_Int32 the_value) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getMinWidth() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMinWidth(::sal_Int32 the_value) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getResizeable() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setResizeable(sal_Bool the_value) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getFlexibility() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFlexibility( ::sal_Int32 _flexibility ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTitle(const OUString & value) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getHelpText() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHelpText(const OUString & value) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getIndex() throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getDataColumnIndex() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDataColumnIndex( ::sal_Int32 i_dataColumnIndex ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::style::HorizontalAlignment SAL_CALL getHorizontalAlign() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHorizontalAlign(css::style::HorizontalAlignment align) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addGridColumnListener( const css::uno::Reference< css::awt::grid::XGridColumnListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeGridColumnListener( const css::uno::Reference< css::awt::grid::XGridColumnListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XCloneable (base of XGridColumn)
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XUnoTunnel and friends
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& i_identifier ) throw(css::uno::RuntimeException, std::exception) override;
    static css::uno::Sequence< sal_Int8 > getUnoTunnelId() throw();
    static GridColumn* getImplementation( const css::uno::Reference< css::uno::XInterface >& i_component );

    // attribute access
    void setIndex( sal_Int32 const i_index );

private:
    void broadcast_changed(
            sal_Char const * const i_asciiAttributeName,
            const css::uno::Any& i_oldValue,
            const css::uno::Any& i_newValue,
            ::comphelper::ComponentGuard& i_Guard
        );

    template< class TYPE >
    void impl_set( TYPE & io_attribute, TYPE const & i_newValue, sal_Char const * i_attributeName )
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        if ( io_attribute == i_newValue )
            return;

        TYPE const aOldValue( io_attribute );
        io_attribute = i_newValue;
        broadcast_changed( i_attributeName, css::uno::makeAny( aOldValue ), css::uno::makeAny( io_attribute ), aGuard );
    }

    css::uno::Any                      m_aIdentifier;
    sal_Int32                          m_nIndex;
    sal_Int32                          m_nDataColumnIndex;
    sal_Int32                          m_nColumnWidth;
    sal_Int32                          m_nMaxWidth;
    sal_Int32                          m_nMinWidth;
    sal_Int32                          m_nFlexibility;
    bool                               m_bResizeable;
    OUString                           m_sTitle;
    OUString                           m_sHelpText;
    css::style::HorizontalAlignment    m_eHorizontalAlign;
};

}

#endif // INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
