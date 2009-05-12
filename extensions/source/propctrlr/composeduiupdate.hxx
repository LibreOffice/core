/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: composeduiupdate.hxx,v $
 * $Revision: 1.3 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_COMPOSEDUIUPDATE_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_COMPOSEDUIUPDATE_HXX

#include "propertyhandler.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
/** === end UNO includes === **/

#include <map>
#include <set>
#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= some helper types
    //====================================================================

    struct MapHandlerToUI;

    /** callback for an ComposedPropertyUIUpdate checking a given property for existence
    */
    class SAL_NO_VTABLE IPropertyExistenceCheck
    {
    public:
        virtual ::sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& _rName ) throw (::com::sun::star::uno::RuntimeException) = 0;
    };

    //====================================================================
    //= ComposedPropertyUIUpdate
    //====================================================================
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
        ::std::auto_ptr< MapHandlerToUI >       m_pCollectedUIs;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >
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
            @throws ::com::sun::star::lang::NullPointerException
                if ->_rxDelegatorUI is <NULL/>
        */
        ComposedPropertyUIUpdate(
            const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxDelegatorUI,
            IPropertyExistenceCheck* _pPropertyCheck );
        ~ComposedPropertyUIUpdate();

        /** returns the delegator UI
            @throw ::com::sun::star::lang::DisposedException
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI > getDelegatorUI() const;

        /** returns a ->XObjectInspectorUI instance belonging to a given property handler

            In every call to an ->XPropertyHandler method which requires a ->XObjectInspectorUI,
            the same UI instance should be used. The instance here will cache all requests passed
            to it, and ->ComposedPropertyUIUpdate::fire will use the combination of all
            cached UI states of all handlers to update the delegator UI.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >
            getUIForPropertyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >& _rxHandler );

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

            @throws ::com::sun::star::lang::DisposedException
                if ->dispose has been called previously
        */
        void SAL_CALL fire();

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
        bool shouldContinuePropertyHandling( const ::rtl::OUString& _rName ) const;

    private:
        /// determines whether the instance is already disposed
        inline  bool impl_isDisposed() const { return m_pCollectedUIs.get() == NULL; }

        /// throws an exception if the component is already disposed
                void impl_checkDisposed() const;

        /** fires all accumulated changes
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
        ComposedPropertyUIUpdate();                                             // never implemented
        ComposedPropertyUIUpdate( const ComposedPropertyUIUpdate& );            // never implemented
        ComposedPropertyUIUpdate& operator=( const ComposedPropertyUIUpdate& ); // never implemented
    };

    //====================================================================
    //= ComposedUIAutoFireGuard
    //====================================================================
    class ComposedUIAutoFireGuard
    {
    private:
        ComposedPropertyUIUpdate&   m_rUIUpdate;
    public:
        ComposedUIAutoFireGuard( ComposedPropertyUIUpdate& _rUIUpdate )
            :m_rUIUpdate( _rUIUpdate )
        {
            m_rUIUpdate.suspendAutoFire();
        }
        ~ComposedUIAutoFireGuard()
        {
            m_rUIUpdate.resumeAutoFire();
        }
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_COMPOSEDUIUPDATE_HXX

