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

#ifndef FORMS_FORMOPERATIONS_HXX
#define FORMS_FORMOPERATIONS_HXX

#include <com/sun/star/form/runtime/XFormOperations.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>

#include <comphelper/componentcontext.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase6.hxx>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= FormOperations
    //====================================================================
    typedef ::cppu::WeakComponentImplHelper6    <   ::com::sun::star::form::runtime::XFormOperations
                                                ,   ::com::sun::star::lang::XInitialization
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::beans::XPropertyChangeListener
                                                ,   ::com::sun::star::util::XModifyListener
                                                ,   ::com::sun::star::sdbc::XRowSetListener
                                                >   FormOperations_Base;

    class FormOperations    :public ::cppu::BaseMutex
                            ,public FormOperations_Base
    {
    public:
        class MethodGuard;

    private:
        ::comphelper::ComponentContext                                                          m_aContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >    m_xController;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >                     m_xCursor;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >            m_xUpdateCursor;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               m_xCursorProperties;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >                   m_xLoadableForm;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFeatureInvalidation >
                                                                                                m_xFeatureInvalidation;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >
                                                                                                m_xParser;

        bool    m_bInitializedParser;
        bool    m_bActiveControlModified;
        bool    m_bConstructed;
    #ifdef DBG_UTIL
        mutable long
                m_nMethodNestingLevel;
    #endif

    public:
        FormOperations( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxContext );

        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        struct MethodAccess { friend class MethodGuard; private: MethodAccess() { } };

        inline void enterMethod( MethodAccess ) const
        {
            m_aMutex.acquire();
            impl_checkDisposed_throw();
        #ifdef DBG_UTIL
            ++m_nMethodNestingLevel;
        #endif
        }

        inline void leaveMethod( MethodAccess ) const
        {
            m_aMutex.release();
        #ifdef DBG_UTIL
            --m_nMethodNestingLevel;
        #endif
        }

    protected:
        virtual ~FormOperations();

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XFormOperations
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > SAL_CALL getCursor() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate > SAL_CALL getUpdateCursor() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController > SAL_CALL getController() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFeatureInvalidation > SAL_CALL getFeatureInvalidation() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFeatureInvalidation(const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFeatureInvalidation > & the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::form::runtime::FeatureState SAL_CALL getState(::sal_Int16 Feature) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isEnabled(::sal_Int16 Feature) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL execute(::sal_Int16 Feature) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::WrappedTargetException);
        virtual void SAL_CALL executeWithArguments(::sal_Int16 Feature, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::WrappedTargetException);
        virtual ::sal_Bool SAL_CALL commitCurrentRecord(::sal_Bool & RecordInserted) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::sdbc::SQLException);
        virtual ::sal_Bool SAL_CALL commitCurrentControl() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::sdbc::SQLException);
        virtual ::sal_Bool SAL_CALL isInsertionRow() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::WrappedTargetException);
        virtual ::sal_Bool SAL_CALL isModifiedRow() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::WrappedTargetException);

        // XRowSetListener
        virtual void SAL_CALL cursorMoved( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowSetChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& _rSource ) throw( ::com::sun::star::uno::RuntimeException );

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent/OComponentHelper
        virtual void SAL_CALL disposing();

    private:
        // service constructors
        void    createWithFormController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >& _rxController );
        void    createWithForm( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm );

        /** determines whether or not we're already disposed
        */
        inline bool impl_isDisposed_nothrow() const { return !m_xCursor.is(); }

        /** checks whether the instance is already disposed, and throws an exception if so
        */
        void        impl_checkDisposed_throw() const;

        /** initializes the instance after m_xController has been set
            @precond
                m_xController is not <NULL/>
        */
        void        impl_initFromController_throw();

        /** initializes the instance after m_xCursor has been set
            @precond
                m_xCursor is not <NULL/>
        */
        void        impl_initFromForm_throw();

        /// invalidate the full palette of features which we know
        void        impl_invalidateAllSupportedFeatures_nothrow( MethodGuard& _rClearForCallback ) const;

        /** invalidate the features which depend on the "modified" state of the current control
            of our controller
        */
        void        impl_invalidateModifyDependentFeatures_nothrow( MethodGuard& _rClearForCallback ) const;

        /** ensures that our parse is initialized, or at least that we attempted to do so
            @precond
                we're not disposed
        */
        void        impl_ensureInitializedParser_nothrow();

        /// disposes our parser, if we have one
        void        impl_disposeParser_nothrow();

        /** determines whether our cursor can be moved left
            @precond hasCursor()
        */
        bool        impl_canMoveLeft_throw() const;

        /** determines whether our cursor can be moved right
            @precond hasCursor()
        */
        bool        impl_canMoveRight_throw() const;

        /// determines whether we're positioned on the insertion row
        bool        impl_isInsertionRow_throw() const;

        /// retrieves the RowCount property of the form
        sal_Int32   impl_getRowCount_throw() const;

        /// retrieves the RowCountFinal property of the form
        bool        impl_isRowCountFinal_throw() const;

        /// retrieves the IsModified property of the form
        bool        impl_isModifiedRow_throw() const;

        /// determines whether we can parse the query of our form
        bool        impl_isParseable_throw() const;

        /// determines if we have an active filter or order condition
        bool        impl_hasFilterOrOrder_throw() const;

        /// determines whether our form is in "insert-only" mode
        bool        impl_isInsertOnlyForm_throw() const;

        /** retrieces the column to which the current control of our controller is bound
            @precond
                m_xController.is()
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    impl_getCurrentBoundField_nothrow( ) const;

        /** returns the control model of the current control

            If the current control is a grid control, then the returned model is the
            model of the current <em>column</em> in the grid.

            @precond
                m_xController.is()
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                    impl_getCurrentControlModel_throw() const;

        /// determines if we have a valid cursor
        inline  bool    impl_hasCursor_nothrow() const { return m_xCursorProperties.is(); }

        /** determines the model position from a grid control column's view position

            A grid control can have columns which are currently hidden, so the index of a
            column in the view is not necessarily the same as its index in the model.
        */
        sal_Int16   impl_gridView2ModelPos_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxColumns, sal_Int16 _nViewPos ) const;

        /** moves our cursor one position to the left, caring for different possible
            cursor states.

            Before the movement is done, the current row is saved, if necessary.

            @precond
                canMoveLeft()
        */
        bool        impl_moveLeft_throw() const;

        /** moves our cursor one position to the right, caring for different possible
            cursor states.

            Before the movement is done, the current row is saved, if necessary.

            @precond
                canMoveRight()
        */
        bool        impl_moveRight_throw( ) const;

        /** impl-version of commitCurrentRecord, which can be called without caring for
            an output parameter, and within const-contexts

            @precond
                our mutex is locked
        */
        bool        impl_commitCurrentRecord_throw( sal_Bool* _pRecordInserted = NULL ) const;

        /** impl-version of commitCurrentControl, which can be called in const-contexts

            @precond
                our mutex is locked
        */
        bool        impl_commitCurrentControl_throw() const;

        /// resets all control models in our own form
        void        impl_resetAllControls_nothrow() const;

        /// executes the "auto sort ascending" and "auto sort descending" features
        void        impl_executeAutoSort_throw( bool _bUp ) const;

        /// executes the "auto filter" feature
        void        impl_executeAutoFilter_throw( ) const;

        /// executes the interactive sort resp. filter feature
        void        impl_executeFilterOrSort_throw( bool _bFilter ) const;

    private:
        /** calls a (member) function, catches SQLExceptions, extends them with additional context information,
            and rethrows them

            @param Action
                a fuctionoid with no arguments to do the work
            @param _nErrorResourceId
                the id of the resources string to use as error message
        */
        template < typename FunctObj >
        void        impl_doActionInSQLContext_throw( FunctObj Action, sal_uInt16 _nErrorResourceId ) const;

        // functionoid to call appendOrderByColumn
        class impl_appendOrderByColumn_throw
        {
        public:
            impl_appendOrderByColumn_throw(const FormOperations *pFO,
                                           ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xField,
                                           bool bUp)
                : m_pFO(pFO)
                , m_xField(xField)
                , m_bUp(bUp)
            {};

            void operator()() { m_pFO->m_xParser->appendOrderByColumn(m_xField, m_bUp); }
        private:
            const FormOperations *m_pFO;
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xField;
            bool m_bUp;
        };

        // functionoid to call appendFilterByColumn
        class impl_appendFilterByColumn_throw
        {
        public:
            impl_appendFilterByColumn_throw(const FormOperations *pFO,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xField)
                : m_pFO(pFO)
                , m_xField(xField)
            {};

            void operator()() { m_pFO->m_xParser->appendFilterByColumn( m_xField, sal_True, ::com::sun::star::sdb::SQLFilterOperator::EQUAL ); }
        private:
            const FormOperations *m_pFO;
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xField;
        };

    private:
        FormOperations();                                   // never implemented
        FormOperations( const FormOperations& );            // never implemented
        FormOperations& operator=( const FormOperations& ); // never implemented

    public:
        class MethodGuard
        {
            FormOperations& m_rOwner;
            bool            m_bCleared;

        public:
            inline MethodGuard( FormOperations& _rOwner )
                :m_rOwner( _rOwner )
                ,m_bCleared( false )
            {
                m_rOwner.enterMethod( FormOperations::MethodAccess() );
            }

            inline ~MethodGuard()
            {
                clear();
            }

            inline void clear()
            {
                if ( !m_bCleared )
                    m_rOwner.leaveMethod( FormOperations::MethodAccess() );
                m_bCleared = true;
            }
        };
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_FORMOPERATIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
