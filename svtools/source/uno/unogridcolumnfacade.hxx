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

#ifndef SVTOOLS_GRIDCOLUMNFACADE_HXX
#define SVTOOLS_GRIDCOLUMNFACADE_HXX

#include "svtools/table/tablemodel.hxx"
#include "svtools/table/tablecontrol.hxx"

#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>

#include <cppuhelper/implbase2.hxx>
#include <rtl/ref.hxx>

#include <boost/noncopyable.hpp>

// .....................................................................................................................
namespace svt { namespace table
{
// .....................................................................................................................

    //==================================================================================================================
    //= UnoGridColumnFacade
    //==================================================================================================================
    class ColumnChangeMultiplexer;
    class UnoControlTableModel;
    class UnoGridColumnFacade   :public IColumnModel
                                ,public ::boost::noncopyable
    {
    public:
        UnoGridColumnFacade(
            UnoControlTableModel const & i_owner,
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > const & i_gridColumn
        );
        ~UnoGridColumnFacade();

        // IColumnModel overridables
        virtual ::com::sun::star::uno::Any
                                getID() const;
        virtual void            setID( const ::com::sun::star::uno::Any& i_ID );
        virtual OUString        getName() const;
        virtual void            setName( const OUString& _rName );
        virtual OUString        getHelpText() const;
        virtual void            setHelpText( const OUString& i_helpText );
        virtual bool            isResizable() const;
        virtual void            setResizable( bool _bResizable );
        virtual sal_Int32       getFlexibility() const;
        virtual void            setFlexibility( sal_Int32 const i_flexibility );
        virtual TableMetrics    getWidth() const;
        virtual void            setWidth( TableMetrics _nWidth );
        virtual TableMetrics    getMinWidth() const;
        virtual void            setMinWidth( TableMetrics _nMinWidth );
        virtual TableMetrics    getMaxWidth() const;
        virtual void            setMaxWidth( TableMetrics _nMaxWidth );
        virtual ::com::sun::star::style::HorizontalAlignment getHorizontalAlign();
        virtual void setHorizontalAlign(::com::sun::star::style::HorizontalAlignment _xAlign);

        /** disposes the column wrapper

            Note that the XGridColumn which is wrapped by the instance is <strong>not</strong> disposed, as we
            do not own it.
        */
        void    dispose();

        sal_Int32
                getDataColumnIndex() const { return m_nDataColumnIndex; }

        // callbacks for the XGridColumnListener
        void    columnChanged( ColumnAttributeGroup const i_attributeGroup );
        void    dataColumnIndexChanged();

    private:
        void    impl_updateDataColumnIndex_nothrow();

    private:
        UnoControlTableModel const *                                                    m_pOwner;
        sal_Int32                                                                       m_nDataColumnIndex;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn >    m_xGridColumn;
        ::rtl::Reference< ColumnChangeMultiplexer >                                     m_pChangeMultiplexer;
    };

// .....................................................................................................................
} } // svt::table
// .....................................................................................................................

#endif // SVTOOLS_GRIDCOLUMNFACADE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
