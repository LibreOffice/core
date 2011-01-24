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

#ifndef SVTOOLS_GRIDCOLUMNFACADE_HXX
#define SVTOOLS_GRIDCOLUMNFACADE_HXX

#include "svtools/table/tablemodel.hxx"
#include "svtools/table/tablecontrol.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>
/** === end UNO includes === **/

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
        virtual String          getName() const;
        virtual void            setName( const String& _rName );
        virtual String          getHelpText() const;
        virtual void            setHelpText( const String& i_helpText );
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
