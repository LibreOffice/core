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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX

#include "pcrcommon.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <comphelper/listenernotification.hxx>

#include <vector>
#include <set>
#include <map>


namespace pcr
{


    typedef ::std::map< OUString, css::uno::Reference< css::beans::XPropertySet >, ::std::less< OUString > >
            MapStringToPropertySet;


    //= EFormsHelper

    class EFormsHelper
    {
    protected:
        css::uno::Reference< css::beans::XPropertySet >
                        m_xControlModel;
        css::uno::Reference< css::form::binding::XBindableValue >
                        m_xBindableControl;
        css::uno::Reference< css::xforms::XFormsSupplier >
                        m_xDocument;
        PropertyChangeListeners
                        m_aPropertyListeners;
        MapStringToPropertySet
                        m_aSubmissionUINames;   // only filled upon request
        MapStringToPropertySet
                        m_aBindingUINames;      // only filled upon request

    public:
        EFormsHelper(
            ::osl::Mutex& _rMutex,
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const css::uno::Reference< css::frame::XModel >& _rxContextDocument
        );

        /** determines whether the given document is an eForm

            If this method returns <FALSE/>, you cannot instantiate a EFormsHelper with
            this document, since then no of it's functionality will be available.
        */
        static  bool
                isEForm(
                    const css::uno::Reference< css::frame::XModel >& _rxContextDocument
                );

        /** registers a listener to be notified when any aspect of the binding changes.

            The listener will be registered at the current binding of the control model. If the binding
            changes (see <method>setBinding</method>), the listener will be revoked from the old binding,
            registered at the new binding, and for all properties which differ between both bindings,
            the listener will be notified.
            @see revokeBindingListener
        */
        void    registerBindingListener(
                    const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxBindingListener
                );

        /** revokes the binding listener which has previously been registered
            @see registerBindingListener
        */
        void    revokeBindingListener(
                    const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxBindingListener
                );

        /** checks whether it's possible to bind the control model to a given XSD data type

            @param _nDataType
                the data type which should be bound. If this is -1, <TRUE/> is returned if the control model
                can be bound to <em>any</em> data type.
        */
        bool    canBindToDataType( sal_Int32 _nDataType = -1 ) const;

        /** checks whether the control model can be bound to any XSD data type
        */
        bool    canBindToAnyDataType() const { return canBindToDataType(); }

        /** checks whether the control model is a source for list entries, as supplied by XML data bindings
        */
        bool    isListEntrySink() const;

        /** retrieves the names of all XForms models in the document the control lives in
        */
        void    getFormModelNames( ::std::vector< OUString >& /* [out] */ _rModelNames ) const;

        /** retrieves the names of all bindings for a given model
            @see getFormModelNames
        */
        void    getBindingNames( const OUString& _rModelName, ::std::vector< OUString >& /* [out] */ _rBindingNames ) const;

        /// retrieves the XForms model (within the control model's document) with the given name
        css::uno::Reference< css::xforms::XModel >
                getFormModelByName( const OUString& _rModelName ) const;

        /** retrieves the model which the active binding of the control model belongs to
        */
        css::uno::Reference< css::xforms::XModel >
                getCurrentFormModel() const;

        /** retrieves the name of the model which the active binding of the control model belongs to
        */
        OUString
                getCurrentFormModelName() const;

        /** retrieves the binding instance which is currently attached to the control model
        */
        css::uno::Reference< css::beans::XPropertySet >
                getCurrentBinding() const;

        /** retrieves the name of the binding instance which is currently attached to the control model
        */
        OUString
                getCurrentBindingName() const;

        /** sets a new binding at the control model
        */
        void    setBinding( const css::uno::Reference< css::beans::XPropertySet >& _rxBinding );

        /** retrieves the binding instance which is currently used as list source for the control model
            @see isListEntrySink
        */
        css::uno::Reference< css::form::binding::XListEntrySource >
                getCurrentListSourceBinding() const;

        /** sets a new list source at the control model
            @see isListEntrySink
        */
        void    setListSourceBinding( const css::uno::Reference< css::form::binding::XListEntrySource >& _rxListSource );

        /** retrieves a given binding for a given model, or creates a new one

            @param _rTargetModel
                the name of the model to create a binding for. Must not be empty
            @param _rBindingName
                the name of the binding to retrieve. If the model denoted by <arg>_rTargetModel</arg> does not
                have a binding with this name, a new binding is created and returned.
        */
        css::uno::Reference< css::beans::XPropertySet >
            getOrCreateBindingForModel( const OUString& _rTargetModel, const OUString& _rBindingName ) const;

        /** types of sub-elements of a model
        */
        enum ModelElementType
        {
            Submission,
            Binding
        };

        /** retrieves the name of a model's sub-element, as to be shown in the UI
            @see getModelElementFromUIName
            @see getAllElementUINames
        */
        static OUString
                getModelElementUIName(
                    const ModelElementType _eType,
                    const css::uno::Reference< css::beans::XPropertySet >& _rxElement
                );

        /** retrieves the submission object for an UI name

            Note that <member>getAllElementUINames</member> must have been called before, for the given element type

            @see getModelElementUIName
            @see getAllElementUINames
        */
        css::uno::Reference< css::beans::XPropertySet >
                getModelElementFromUIName(
                    const ModelElementType _eType,
                    const OUString& _rUIName
                ) const;

        /** retrieves the UI names of all elements of all models in our document
            @param _eType
                the type of elements for which the names should be retrieved
            @param _rElementNames
                the array of element names
            @see getModelElementUIName
            @see getModelElementFromUIName
        */
        void    getAllElementUINames(
                    const ModelElementType _eType,
                    ::std::vector< OUString >& /* [out] */ _rElementNames,
                    bool _bPrepentEmptyEntry
                );

    protected:
        void    firePropertyChanges(
                    const css::uno::Reference< css::beans::XPropertySet >& _rxOldProps,
                    const css::uno::Reference< css::beans::XPropertySet >& _rxNewProps,
                    ::std::set< OUString >& _rFilter
                ) const;

        /** fires a change in a single property, if the property value changed, and if we have a listener
            interested in property changes
        */
        void    firePropertyChange(
                    const OUString& _rName,
                    const css::uno::Any& _rOldValue,
                    const css::uno::Any& _rNewValue
                ) const;

    private:
        void impl_switchBindingListening_throw( bool _bDoListening, const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener );

        /// implementation for both <member>getOrCreateBindingForModel</member>
        css::uno::Reference< css::beans::XPropertySet >
            implGetOrCreateBinding( const OUString& _rTargetModel, const OUString& _rBindingName ) const;

        void
            impl_toggleBindingPropertyListening_throw( bool _bDoListen, const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxConcreteListenerOrNull );

    private:
        EFormsHelper( const EFormsHelper& ) SAL_DELETED_FUNCTION;
        EFormsHelper& operator=( const EFormsHelper& ) SAL_DELETED_FUNCTION;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
