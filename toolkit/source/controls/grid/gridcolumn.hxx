/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <comphelper/componentguard.hxx>
#include <rtl/ref.hxx>
#include <toolkit/helper/mutexandbroadcasthelper.hxx>

#include <vector>

namespace toolkit
{

typedef ::cppu::WeakComponentImplHelper3    <   ::com::sun::star::awt::grid::XGridColumn
                                            ,   ::com::sun::star::lang::XServiceInfo
                                            ,   ::com::sun::star::lang::XUnoTunnel
                                            >   GridColumn_Base;
class GridColumn    :public ::cppu::BaseMutex
                    ,public GridColumn_Base
{
public:
    GridColumn();
    GridColumn( GridColumn const & i_copySource );
    virtual ~GridColumn();

    // ::com::sun::star::awt::grid::XGridColumn
    virtual ::com::sun::star::uno::Any SAL_CALL getIdentifier() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setIdentifier(const ::com::sun::star::uno::Any & value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getColumnWidth() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumnWidth(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getMaxWidth() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMaxWidth(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getMinWidth() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMinWidth(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getResizeable() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setResizeable(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getFlexibility() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFlexibility( ::sal_Int32 _flexibility ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitle() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle(const ::rtl::OUString & value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getHelpText() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHelpText(const ::rtl::OUString & value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getIndex() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getDataColumnIndex() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDataColumnIndex( ::sal_Int32 i_dataColumnIndex ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::style::HorizontalAlignment SAL_CALL getHorizontalAlign() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHorizontalAlign(::com::sun::star::style::HorizontalAlignment align) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addGridColumnListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeGridColumnListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // XCloneable (base of XGridColumn)
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XUnoTunnel and friends
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& i_identifier ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelId() throw();
    static GridColumn* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& i_component );

    // attribute access
    void setIndex( sal_Int32 const i_index );

private:
    void broadcast_changed(
            sal_Char const * const i_asciiAttributeName,
            ::com::sun::star::uno::Any i_oldValue,
            ::com::sun::star::uno::Any i_newValue,
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
        broadcast_changed( i_attributeName, ::com::sun::star::uno::makeAny( aOldValue ), ::com::sun::star::uno::makeAny( io_attribute ), aGuard );
    }

    ::com::sun::star::uno::Any                      m_aIdentifier;
    sal_Int32                                       m_nIndex;
    sal_Int32                                       m_nDataColumnIndex;
    sal_Int32                                       m_nColumnWidth;
    sal_Int32                                       m_nMaxWidth;
    sal_Int32                                       m_nMinWidth;
    sal_Int32                                       m_nFlexibility;
    sal_Bool                                        m_bResizeable;
    ::rtl::OUString                                 m_sTitle;
    ::rtl::OUString                                 m_sHelpText;
    ::com::sun::star::style::HorizontalAlignment    m_eHorizontalAlign;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
