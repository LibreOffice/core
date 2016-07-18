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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_COMPOSEDUIUPDATE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_COMPOSEDUIUPDATE_HXX

#include "propertyhandler.hxx"

#include <com/sun/star/inspection/XObjectInspectorUI.hpp>

#include <map>
#include <set>
#include <memory>


namespace pcr
{

    struct MapHandlerToUI;

    /** callback for an ComposedPropertyUIUpdate checking a given property for existence
    */
    class SAL_NO_VTABLE IPropertyExistenceCheck
    {
    public:
        virtual bool SAL_CALL hasPropertyByName( const OUString& _rName ) throw (css::uno::RuntimeException) = 0;

    protected:
        ~IPropertyExistenceCheck() {}
    };

    /** helper class composing requests to a ->XObjectInspectorUI interface, coming
        from multiple sources

        Usually, a handler tells the browser UI to enable to disable, or show or hide, certain
        elements. Now when multiple handlers do this, their instructions must be combined:
        If one handler disables a certain element, but others enable it, it must in the
        result still be disabled. Similar for showing/hiding elements.

        ->ComposedPropertyUIUpdate implements this combination. It does so by providing a dedicated
        ->XObjectInspectorUI instance for every participating handler, and remembering the UI
        state on a per-handler basis. Upon request (->fire), the combined UI state is
        forwarded to another ->XObjectInspectorUI instance, the so-called delegator UI.
    */
    class ComposedPropertyUIUpdate
    {
    private:
        ::std::unique_ptr< MapHandlerToUI >     m_pCollectedUIs;
        css::uno::Reference< css::inspection::XObjectInspectorUI >
                                                m_xDelegatorUI;
        oslInterlockedCount                     m_nSuspendCounter;
        IPropertyExistenceCheck*                m_pPropertyCheck;

    public:
        /** constructs a ->ComposedPropertyUIUpdate instance
            @param _rxDelegatorUI
                a ->XObjectInspectorUI instance to which composed UI requests should be forwarded. Must
                not be <NULL/>.
            @param _pPropertyCheck
                an instance checking properties for existence. If this is not <NULL/>, it will be invoked
                whenever one of the ->XObjectInspectorUI methods is called, to check the passed property
                name.<br/>
                Beware of lifetime issues. The instance pointed to by <arg>_pPropertyCheck</arg> must
                live at least as long as the ->ComposedPropertyUIUpdate instance you're going to create.
            @throws css::lang::NullPointerException
                if ->_rxDelegatorUI is <NULL/>
        */
        ComposedPropertyUIUpdate(
            const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxDelegatorUI,
            IPropertyExistenceCheck* _pPropertyCheck );
        ~ComposedPropertyUIUpdate();

        /** returns the delegator UI
            @throw css::lang::DisposedException
        */
        css::uno::Reference< css::inspection::XObjectInspectorUI > const & getDelegatorUI() const;

        /** returns a ->XObjectInspectorUI instance belonging to a given property handler

            In every call to an ->XPropertyHandler method which requires a ->XObjectInspectorUI,
            the same UI instance should be used. The instance here will cache all requests passed
            to it, and ->ComposedPropertyUIUpdate::fire will use the combination of all
            cached UI states of all handlers to update the delegator UI.
        */
        css::uno::Reference< css::inspection::XObjectInspectorUI >
            getUIForPropertyHandler( const css::uno::Reference< css::inspection::XPropertyHandler >& _rxHandler );

        /** Suspends automatic firing of UI changes

            normally, as soon as any of the property handlers does a request for an
            arbitrary UI change, the set of collected UI changes is evaluated, and the combined
            UI state is fired to the delegator UI.

            You can disable this automatic firing by calling ->suspendAutoFire. As longs as auto
            firing is suspended, only explicit ->fire calls trigger the notification to the
            delegator UI.

            Note that calls to ->suspendAutoFire are culmulative, that is, if you make multiple calls
            they must be accompanied by an equal number of calls to ->resumeAutoFire, to enable
            auto-firing again.

            @seealso resumeAutoFire
        */
        void SAL_CALL suspendAutoFire();

        /** Suspends automatic firing of UI changes

            @seealso suspendAutoFire
        */
        void SAL_CALL resumeAutoFire();

        /** disposes the instance, so it becomes non-functional.

            All cached handlers and cached ->XObjectInspectorUI instances will be released,
            the latter will also be disposed, so that if anybody still holds a reference to them
            and tries to operate them will get a DisposedException.
        */
        void SAL_CALL dispose();

        /** invokes m_pPropertyCheck to check whether a given property should be handled
        */
        bool shouldContinuePropertyHandling( const OUString& _rName ) const;

    private:
        /// determines whether the instance is already disposed
        inline  bool impl_isDisposed() const { return m_pCollectedUIs.get() == nullptr; }

        /// throws an exception if the component is already disposed
                void impl_checkDisposed() const;

        /** fires the collected UI changes to our delegator UI

            All operations for any elements are forwarded:
            <ul><li>If an element has been hidden at least once, it's also hidden at the delegator UI.</li>
                <li>If an element has been shown at least once, and never been hidden, it's also
                    shown at the delegator UI.</li>
                <li>If an element has never been shown or hidden, it's also not touched at the delegator UI.</li>
                <li>The same holds if you replace "hidden" in the last three items with "disabled",
                    and "shown" with "enabled".</li>
                <li>If an element should have been rebuilt (->XObjectInspectorUI::rebuiltPropertyUI)
                    at least once, it's rebuilt at the delegator UI, too.<br/>
                    After that, the request to rebuild the UI for this property is cleared, so subsequent
                    calls to ->fire will not trigger an new rebuilt request.
            </ul>

            @precond
                instance is not disposed
        */
        void    impl_fireAll_throw();

        /// fires the combination of ->XObjectInspectorUI::enablePropertyUI calls
        void    impl_fireEnablePropertyUI_throw();

        /// fires the combination of ->XObjectInspectorUI::enablePropertyUIElements calls
        void    impl_fireEnablePropertyUIElements_throw();

        /// fires the combination of ->XObjectInspectorUI::rebuildPropertyUI calls
        void    impl_fireRebuildPropertyUI_throw();

        /// fires the combination of ->XObjectInspectorUI::showPropertyUI and ->XObjectInspectorUI::hidePropertyUI calls
        void    impl_fireShowHidePropertyUI_throw();

        /// fires the combination of ->XObjectInspectorUI::showCategory calls
        void    impl_fireShowCategory_throw();

        /** callback for when a single property handler requested any change in the inspector UI
        */
        void    callback_inspectorUIChanged_throw();

    private:
        ComposedPropertyUIUpdate( const ComposedPropertyUIUpdate& ) = delete;
        ComposedPropertyUIUpdate& operator=( const ComposedPropertyUIUpdate& ) = delete;
    };

    class ComposedUIAutoFireGuard
    {
    private:
        ComposedPropertyUIUpdate&   m_rUIUpdate;
    public:
        explicit ComposedUIAutoFireGuard( ComposedPropertyUIUpdate& _rUIUpdate )
            :m_rUIUpdate( _rUIUpdate )
        {
            m_rUIUpdate.suspendAutoFire();
        }
        ~ComposedUIAutoFireGuard()
        {
            m_rUIUpdate.resumeAutoFire();
        }
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_COMPOSEDUIUPDATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
