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

#ifndef TOOLKIT_GRIDEVENTFORWARDER_HXX
#define TOOLKIT_GRIDEVENTFORWARDER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/grid/XGridColumnListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase2.hxx>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    class UnoGridControl;

    //==================================================================================================================
    //= GridEventForwarder
    //==================================================================================================================
    typedef ::cppu::ImplHelper2 <   ::com::sun::star::awt::grid::XGridDataListener
                                ,   ::com::sun::star::container::XContainerListener
                                >   GridEventForwarder_Base;

    class GridEventForwarder : public GridEventForwarder_Base
    {
    public:
        GridEventForwarder( UnoGridControl& i_parent );
        virtual ~GridEventForwarder();

    public:
        // XInterface
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XGridDataListener
        virtual void SAL_CALL rowsInserted( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowsRemoved( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dataChanged( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowHeadingChanged( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& i_event ) throw (::com::sun::star::uno::RuntimeException);

    private:
        UnoGridControl& m_parent;
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_GRIDEVENTFORWARDER_HXX
