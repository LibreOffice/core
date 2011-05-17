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

#ifndef SVX_FORMCONTROLLING_HXX
#define SVX_FORMCONTROLLING_HXX

#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/form/runtime/FeatureState.hpp>
#include <com/sun/star/form/runtime/XFormOperations.hpp>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>

#include <cppuhelper/implbase2.hxx>
#include <comphelper/componentcontext.hxx>

#include <vector>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= FeatureSlotTranslation
    //====================================================================
    class FeatureSlotTranslation
    {
    public:
        /// retrieves the feature id for a given feature URL
        static  sal_Int32       getControllerFeatureSlotIdForURL( const ::rtl::OUString& _rMainURL );

        /// retrieves the feature URL for a given feature id
        static ::rtl::OUString  getControllerFeatureURLForSlotId( sal_Int32 _nSlotId );

        /// determines whether the given URL is a controller feature URL
        static sal_Bool         isFeatureURL( const ::rtl::OUString& _rMainURL );

        /// retrieves the css.form.runtime.FormFeature ID for a given slot ID
        static  sal_Int16       getFormFeatureForSlotId( sal_Int32 _nSlotId );

        /// retrieves the slot id for a given css.form.runtime.FormFeature ID
        static  sal_Int32       getSlotIdForFormFeature( sal_Int16 _nFormFeature );
    };

    //====================================================================
    //= IControllerFeatureInvalidation
    //====================================================================
    class IControllerFeatureInvalidation
    {
    public:
        /** invalidates the given features

            Invalidation means that any user interface representation (such as toolbox buttons), or
            any dispatches associated with the features in question are potentially out-of-date, and
            need to be updated

            @param _rFeatures
                Ids of the features to be invalidated.
        */
        virtual void invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures ) = 0;
    };

    //====================================================================
    //= ControllerFeatures
    //====================================================================
    class FormControllerHelper;
    /** easier access to an FormControllerHelper instance
    */
    class ControllerFeatures
    {
    protected:
        ::comphelper::ComponentContext  m_aContext;
        IControllerFeatureInvalidation* m_pInvalidationCallback;    // necessary as long as m_pImpl is not yet constructed
        FormControllerHelper*           m_pImpl;

    public:
        /** standard ctor

            The instance is not functional until <method>assign</method> is used.

            @param _rxORB
                a multi service factory for creating various needed components

            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        ControllerFeatures(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /** constructs the instance from a <type scope="com::sun::star::form::runtime">XFormController<type> instance

            @param _rxORB
                a multi service factory for creating various needed components

            @param _rxController
                The form controller which the helper should be responsible for. Must not
                be <NULL/>, and must have a valid model (form).

            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        ControllerFeatures(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >& _rxController,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /** constructs the helper form a <type scope="com::sun::star::form">XForm<type> instance

            Any functionality which depends on a controller will not be available.

            @param _rxORB
                a multi service factory for creating various needed components

            @param _rxForm
                The form which the helper should be responsible for. Must not be <NULL/>.

            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        ControllerFeatures(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /// dtor
        ~ControllerFeatures();

        /// checks whether the instance is properly assigned to a form and/or controller
        inline bool isAssigned( ) const { return m_pImpl != NULL; }

        /** assign to a controller
        */
        void assign(
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >& _rxController
        );

        /** assign to a controller
        */
        void assign(
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm
        );

        /// clears the instance so that it cannot be used afterwards
        void dispose();

        // access to the instance which implements the functionality. Not to be used when not assigned
        inline const FormControllerHelper* operator->() const { return m_pImpl; }
        inline       FormControllerHelper* operator->()       { return m_pImpl; }
        inline const FormControllerHelper& operator*() const  { return *m_pImpl; }
        inline       FormControllerHelper& operator*()        { return *m_pImpl; }
    };

    //====================================================================
    //= FormControllerHelper
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::form::runtime::XFeatureInvalidation
                                    ,   ::com::sun::star::sdb::XSQLErrorListener
                                    >   FormControllerHelper_Base;
    /** is a helper class which manages form controller functionality (such as moveNext etc.).

        <p>The class helps implementing form controller functionality, by providing
        methods to determine the state of, and execute, various common form features.<br/>
        A <em>feature</em> is for instance moving the form associated with the controller
        to a certain position, or reloading the form, and so on.</p>
    */
    class FormControllerHelper : public FormControllerHelper_Base
    {
    protected:
        ::comphelper::ComponentContext  m_aContext;
        IControllerFeatureInvalidation* m_pInvalidationCallback;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormOperations >
                                        m_xFormOperations;

        ::com::sun::star::uno::Any      m_aOperationError;

    public:
        /** constructs the helper from a <type scope="com::sun::star::form::runtime">XFormController<type> instance

            @param _rContext
                the context the component lives in
            @param _rxController
                The form controller which the helper should be responsible for. Must not
                be <NULL/>, and must have a valid model (form).
            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        FormControllerHelper(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >& _rxController,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /** constructs the helper form a <type scope="com::sun::star::form">XForm<type> instance

            Any functionality which depends on a controller will not be available.

            @param _rContext
                the context the component lives in
            @param _rxForm
                The form which the helper should be responsible for. Must not be <NULL/>.
            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        FormControllerHelper(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        // forwards to the XFormOperations implementation
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                    getCursor() const;
        void        getState(
                        sal_Int32 _nSlotId,
                        ::com::sun::star::form::runtime::FeatureState& _out_rState
                    ) const;
        sal_Bool    isEnabled( sal_Int32 _nSlotId ) const;
        void        execute( sal_Int32 _nSlotId ) const;
        void        execute( sal_Int32 _nSlotId, const ::rtl::OUString& _rParamName, const ::com::sun::star::uno::Any& _rParamValue ) const;
        sal_Bool    commitCurrentRecord() const;
        sal_Bool    commitCurrentControl( ) const;
        sal_Bool    isInsertionRow() const;
        sal_Bool    isModifiedRow() const;

        bool        moveLeft( ) const;
        bool        moveRight( ) const;

        bool        canDoFormFilter() const;

        /** disposes this instance.

            After this method has been called, the instance is not functional anymore
        */
        void        dispose();

        const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormOperations >&
                    getFormOperations() const { return m_xFormOperations; }
    protected:
        /// dtor
        ~FormControllerHelper();

        // XFeatureInvalidation
        virtual void SAL_CALL invalidateFeatures( const ::com::sun::star::uno::Sequence< ::sal_Int16 >& Features ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL invalidateAllFeatures() throw (::com::sun::star::uno::RuntimeException);

        // XSQLErrorListener
        virtual void SAL_CALL errorOccured( const ::com::sun::star::sdb::SQLErrorEvent& _Event ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        enum FormOperation { EXECUTE, EXECUTE_ARGS, COMMIT_CONTROL, COMMIT_RECORD };

        bool    impl_operateForm_nothrow(
                    const FormOperation _eWhat,
                    const sal_Int16 _nFeature,  /* ignore for COMMIT_* */
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rArguments /* ignore except for EXECUTE_ARGS */
                ) const;
        bool    impl_operateForm_nothrow( const FormOperation _eWhat ) const
        {
            return impl_operateForm_nothrow( _eWhat, 0, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >() );
        }
        bool    impl_operateForm_nothrow( const sal_Int16 _nFeature ) const
        {
            return impl_operateForm_nothrow( EXECUTE, _nFeature, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >() );
        }

    private:
        FormControllerHelper();                                         // never implemented
        FormControllerHelper( const FormControllerHelper& );            // never implemented
        FormControllerHelper& operator=( const FormControllerHelper& ); // never implemented
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // SVX_FORMCONTROLLING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
