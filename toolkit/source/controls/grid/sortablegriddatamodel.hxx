/*************************************************************************
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

#ifndef TOOLKIT_SORTABLEGRIDDATAMODEL_HXX
#define TOOLKIT_SORTABLEGRIDDATAMODEL_HXX

/** === begin UNO includes === **/
#include <com/sun/star/awt/grid/XSortableMutableGridDataModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    //==================================================================================================================
    //= SortableGridDataModel
    //==================================================================================================================
    typedef ::cppu::WeakComponentImplHelper3    <   ::com::sun::star::awt::grid::XSortableMutableGridDataModel
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   SortableGridDataModel_Base;
    class SortableGridDataModel :public ::cppu::BaseMutex
                                ,public SortableGridDataModel_Base
    {
    public:
        SortableGridDataModel( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & i_factory );
        SortableGridDataModel( SortableGridDataModel const & i_copySource );

    public:
        bool    isInitialized() const { return m_isInitialized; }

    protected:
        ~SortableGridDataModel();

    public:
        // XSortableGridData
        virtual void SAL_CALL sortByColumn( ::sal_Int32 ColumnIndex, ::sal_Bool SortAscending ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::Pair< ::sal_Int32, ::sal_Bool > SAL_CALL getCurrentSortOrder(  ) throw (::com::sun::star::uno::RuntimeException);

        // XMutableGridDataModel
        virtual void SAL_CALL addRow( const ::com::sun::star::uno::Any& Heading, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Data ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Headings, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& Data ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRow( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeAllRows(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateCellData( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRowData( const ::com::sun::star::uno::Sequence< ::sal_Int32 >& ColumnIndexes, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setRowHeading( ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Heading ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateCellToolTip( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRowToolTip( ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addGridDataListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeGridDataListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

        // XGridDataModel
        virtual ::sal_Int32 SAL_CALL getRowCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getColumnCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getCellData( ::sal_Int32 Column, ::sal_Int32 Row ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getCellToolTip( ::sal_Int32 Column, ::sal_Int32 Row ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getRowHeading( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing();

        // XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    private:
        /** translates the given public index into one to be passed to our delegator
            @throws ::com::sun::star::lang::IndexOutOfBoundsException
                if the given index does not denote a valid row
        */
        ::sal_Int32 impl_translateRowIndex_throw( ::sal_Int32 const i_publicRowIndex ) const;

        inline bool impl_isSorted_nothrow() const
        {
            return m_currentSortColumn >= 0;
        }

        /** rebuilds the index translation structure.

            Neither <member>m_currentSortColumn</member> nor <member>m_sortAscending</member> are touched by this method.
            Also, the given column index is not checked, this is the responsibility of the caller.
        */
        void    impl_reIndex_nothrow( ::sal_Int32 const i_columnIndex, sal_Bool const i_sortAscending );

    private:
        ::comphelper::ComponentContext                                                          m_context;
        bool                                                                                    m_isInitialized;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XMutableGridDataModel >  m_delegator;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >                   m_collator;
        ::sal_Int32                                                                             m_currentSortColumn;
        ::sal_Bool                                                                              m_sortAscending;
        ::std::vector< ::sal_Int32 >                                                            m_rowIndexTranslation;
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_SORTABLEGRIDDATAMODEL_HXX
