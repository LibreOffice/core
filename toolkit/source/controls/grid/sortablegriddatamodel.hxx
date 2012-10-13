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

#ifndef TOOLKIT_SORTABLEGRIDDATAMODEL_HXX
#define TOOLKIT_SORTABLEGRIDDATAMODEL_HXX

#include "initguard.hxx"

#include <com/sun/star/awt/grid/XSortableMutableGridDataModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/awt/grid/XGridDataListener.hpp>

#include <comphelper/componentcontext.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/implbase1.hxx>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    class SortableGridDataModel;
    typedef InitGuard< SortableGridDataModel >  MethodGuard;

    //==================================================================================================================
    //= SortableGridDataModel
    //==================================================================================================================
    typedef ::cppu::WeakComponentImplHelper3    <   ::com::sun::star::awt::grid::XSortableMutableGridDataModel
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   SortableGridDataModel_Base;
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::grid::XGridDataListener
                                >   SortableGridDataModel_PrivateBase;
    class SortableGridDataModel :public ::cppu::BaseMutex
                                ,public SortableGridDataModel_Base
                                ,public SortableGridDataModel_PrivateBase
    {
    public:
        SortableGridDataModel( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & i_factory );
        SortableGridDataModel( SortableGridDataModel const & i_copySource );

        bool    isInitialized() const { return m_isInitialized; }

#ifdef DBG_UTIL
        const char* checkInvariants() const;
#endif

    protected:
        ~SortableGridDataModel();

    public:
        // XSortableGridData
        virtual void SAL_CALL sortByColumn( ::sal_Int32 ColumnIndex, ::sal_Bool SortAscending ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeColumnSort(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::Pair< ::sal_Int32, ::sal_Bool > SAL_CALL getCurrentSortOrder(  ) throw (::com::sun::star::uno::RuntimeException);

        // XMutableGridDataModel
        virtual void SAL_CALL addRow( const ::com::sun::star::uno::Any& Heading, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Data ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Headings, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& Data ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL insertRow( ::sal_Int32 i_index, const ::com::sun::star::uno::Any& i_heading, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Data ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);
        virtual void SAL_CALL insertRows( ::sal_Int32 i_index, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& Headings, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& Data ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRow( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeAllRows(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateCellData( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRowData( const ::com::sun::star::uno::Sequence< ::sal_Int32 >& ColumnIndexes, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRowHeading( ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Heading ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateCellToolTip( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRowToolTip( ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addGridDataListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeGridDataListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

        // XGridDataModel
        virtual ::sal_Int32 SAL_CALL getRowCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getColumnCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getCellData( ::sal_Int32 Column, ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getCellToolTip( ::sal_Int32 Column, ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getRowHeading( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getRowData( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

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

        // XGridDataListener
        virtual void SAL_CALL rowsInserted( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowsRemoved( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dataChanged( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowHeadingChanged( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& i_event ) throw (::com::sun::star::uno::RuntimeException);

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** translates the given public index into one to be passed to our delegator
            @throws ::com::sun::star::lang::IndexOutOfBoundsException
                if the given index does not denote a valid row
        */
        ::sal_Int32 impl_getPrivateRowIndex_throw( ::sal_Int32 const i_publicRowIndex ) const;

        /** translates the given private row index to a public one
        */
        ::sal_Int32 impl_getPublicRowIndex_nothrow( ::sal_Int32 const i_privateRowIndex ) const;

        inline bool impl_isSorted_nothrow() const
        {
            return m_currentSortColumn >= 0;
        }

        /** rebuilds the index translation structure.

            Neither <member>m_currentSortColumn</member> nor <member>m_sortAscending</member> are touched by this method.
            Also, the given column index is not checked, this is the responsibility of the caller.
        */
        bool    impl_reIndex_nothrow( ::sal_Int32 const i_columnIndex, sal_Bool const i_sortAscending );

        /** translates the given event, obtained from our delegator, to a version which can be broadcasted to our own
            clients.
        */
        ::com::sun::star::awt::grid::GridDataEvent
                impl_createPublicEvent( ::com::sun::star::awt::grid::GridDataEvent const & i_originalEvent ) const;

        /** broadcasts the given event to our registered XGridDataListeners
        */
        void    impl_broadcast(
                    void ( SAL_CALL ::com::sun::star::awt::grid::XGridDataListener::*i_listenerMethod )( const ::com::sun::star::awt::grid::GridDataEvent & ),
                    ::com::sun::star::awt::grid::GridDataEvent const & i_publicEvent,
                    MethodGuard& i_instanceLock
                );

        /** rebuilds our indexes, notifying row removal and row addition events

            First, a rowsRemoved event is notified to our registered listeners. Then, the index translation tables are
            rebuilt, and a rowsInserted event is notified.

            Only to be called when we're sorted.
        */
        void    impl_rebuildIndexesAndNotify( MethodGuard& i_instanceLock );

        /** removes the current sorting, and notifies a change of all data
        */
        void    impl_removeColumnSort( MethodGuard& i_instanceLock );

        /** removes the current sorting, without any broadcast
        */
        void    impl_removeColumnSort_noBroadcast();

    private:
        ::comphelper::ComponentContext                                                          m_context;
        bool                                                                                    m_isInitialized;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XMutableGridDataModel >  m_delegator;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >                   m_collator;
        ::sal_Int32                                                                             m_currentSortColumn;
        ::sal_Bool                                                                              m_sortAscending;
        ::std::vector< ::sal_Int32 >                                                            m_publicToPrivateRowIndex;
        ::std::vector< ::sal_Int32 >                                                            m_privateToPublicRowIndex;
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_SORTABLEGRIDDATAMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
