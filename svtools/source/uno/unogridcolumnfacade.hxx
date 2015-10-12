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

#ifndef INCLUDED_SVTOOLS_SOURCE_UNO_UNOGRIDCOLUMNFACADE_HXX
#define INCLUDED_SVTOOLS_SOURCE_UNO_UNOGRIDCOLUMNFACADE_HXX

#include <svtools/table/tablemodel.hxx>
#include "table/tablecontrol.hxx"

#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>

#include <rtl/ref.hxx>

#include <boost/noncopyable.hpp>


namespace svt { namespace table
{



    //= UnoGridColumnFacade

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
        virtual ~UnoGridColumnFacade();

        // IColumnModel overridables
        virtual OUString        getName() const override;
        virtual OUString        getHelpText() const override;
        virtual bool            isResizable() const override;
        virtual sal_Int32       getFlexibility() const override;
        virtual TableMetrics    getWidth() const override;
        virtual void            setWidth( TableMetrics _nWidth ) override;
        virtual TableMetrics    getMinWidth() const override;
        virtual TableMetrics    getMaxWidth() const override;
        virtual ::com::sun::star::style::HorizontalAlignment getHorizontalAlign() override;

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


} } // svt::table


#endif // INCLUDED_SVTOOLS_SOURCE_UNO_UNOGRIDCOLUMNFACADE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
