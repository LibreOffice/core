/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
