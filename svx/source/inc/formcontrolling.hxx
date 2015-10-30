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

#ifndef INCLUDED_SVX_SOURCE_INC_FORMCONTROLLING_HXX
#define INCLUDED_SVX_SOURCE_INC_FORMCONTROLLING_HXX

#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/runtime/FeatureState.hpp>
#include <com/sun/star/form/runtime/XFormOperations.hpp>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>

#include <cppuhelper/implbase.hxx>

#include <vector>


namespace svx
{

    class FeatureSlotTranslation
    {
    public:
        /// retrieves the feature id for a given feature URL
        static  sal_Int32       getControllerFeatureSlotIdForURL( const OUString& _rMainURL );

        /// retrieves the css.form.runtime.FormFeature ID for a given slot ID
        static  sal_Int16       getFormFeatureForSlotId( sal_Int32 _nSlotId );

        /// retrieves the slot id for a given css.form.runtime.FormFeature ID
        static  sal_Int32       getSlotIdForFormFeature( sal_Int16 _nFormFeature );
    };

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

    protected:
        ~IControllerFeatureInvalidation() {}
    };

    class FormControllerHelper;
    /** easier access to an FormControllerHelper instance
    */
    class ControllerFeatures
    {
    protected:
        IControllerFeatureInvalidation* m_pInvalidationCallback;    // necessary as long as m_pImpl is not yet constructed
        FormControllerHelper*           m_pImpl;

    public:
        /** standard ctor

            The instance is not functional until <method>assign</method> is used.

            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        ControllerFeatures(
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /** constructs the instance from a <type scope="css::form::runtime">XFormController<type> instance

            @param _rxController
                The form controller which the helper should be responsible for. Must not
                be <NULL/>, and must have a valid model (form).

            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        ControllerFeatures(
            const css::uno::Reference< css::form::runtime::XFormController >& _rxController,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /// dtor
        ~ControllerFeatures();

        /// checks whether the instance is properly assigned to a form and/or controller
        inline bool isAssigned( ) const { return m_pImpl != NULL; }

        /** assign to a controller
        */
        void assign(
            const css::uno::Reference< css::form::runtime::XFormController >& _rxController
        );

        /// clears the instance so that it cannot be used afterwards
        void dispose();

        // access to the instance which implements the functionality. Not to be used when not assigned
        inline const FormControllerHelper* operator->() const { return m_pImpl; }
        inline       FormControllerHelper* operator->()       { return m_pImpl; }
        inline const FormControllerHelper& operator*() const  { return *m_pImpl; }
        inline       FormControllerHelper& operator*()        { return *m_pImpl; }
    };


    //= FormControllerHelper

    typedef ::cppu::WeakImplHelper <   css::form::runtime::XFeatureInvalidation
                                    ,   css::sdb::XSQLErrorListener
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
        IControllerFeatureInvalidation* m_pInvalidationCallback;
        css::uno::Reference< css::form::runtime::XFormOperations >
                                        m_xFormOperations;

        css::uno::Any      m_aOperationError;

    public:
        /** constructs the helper from a <type scope="css::form::runtime">XFormController<type> instance

            @param _rxController
                The form controller which the helper should be responsible for. Must not
                be <NULL/>, and must have a valid model (form).
            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        FormControllerHelper(
            const css::uno::Reference< css::form::runtime::XFormController >& _rxController,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        // forwards to the XFormOperations implementation
        css::uno::Reference< css::sdbc::XRowSet >
                    getCursor() const;
        void        getState(
                        sal_Int32 _nSlotId,
                        css::form::runtime::FeatureState& _out_rState
                    ) const;
        bool        isEnabled( sal_Int32 _nSlotId ) const;
        void        execute( sal_Int32 _nSlotId ) const;
        void        execute( sal_Int32 _nSlotId, const OUString& _rParamName, const css::uno::Any& _rParamValue ) const;
        bool        commitCurrentRecord() const;
        bool        commitCurrentControl( ) const;
        bool        isInsertionRow() const;
        bool        isModifiedRow() const;

        bool        canDoFormFilter() const;

        /** disposes this instance.

            After this method has been called, the instance is not functional anymore
        */
        void        dispose();

    protected:
        /// dtor
        virtual ~FormControllerHelper();

        // XFeatureInvalidation
        virtual void SAL_CALL invalidateFeatures( const css::uno::Sequence< ::sal_Int16 >& Features ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL invalidateAllFeatures() throw (css::uno::RuntimeException, std::exception) override;

        // XSQLErrorListener
        virtual void SAL_CALL errorOccured( const css::sdb::SQLErrorEvent& _Event ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        enum FormOperation { EXECUTE, EXECUTE_ARGS, COMMIT_CONTROL, COMMIT_RECORD };

        bool    impl_operateForm_nothrow(
                    const FormOperation _eWhat,
                    const sal_Int16 _nFeature,  /* ignore for COMMIT_* */
                    const css::uno::Sequence< css::beans::NamedValue >& _rArguments /* ignore except for EXECUTE_ARGS */
                ) const;
        bool    impl_operateForm_nothrow( const FormOperation _eWhat ) const
        {
            return impl_operateForm_nothrow( _eWhat, 0, css::uno::Sequence< css::beans::NamedValue >() );
        }

    private:
        FormControllerHelper( const FormControllerHelper& ) = delete;
        FormControllerHelper& operator=( const FormControllerHelper& ) = delete;
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_FORMCONTROLLING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
