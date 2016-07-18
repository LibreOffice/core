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
#ifndef INCLUDED_SVX_SOURCE_INC_FORMCONTROLLER_HXX
#define INCLUDED_SVX_SOURCE_INC_FORMCONTROLLER_HXX

#include <sal/config.h>

#include <vector>

#include "delayedevent.hxx"
#include "formdispatchinterceptor.hxx"
#include "sqlparserclient.hxx"

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XTabController.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/DatabaseDeleteEvent.hpp>
#include <com/sun/star/form/DatabaseParameterEvent.hpp>
#include <com/sun/star/form/ErrorEvent.hpp>
#include <com/sun/star/form/validation/XFormComponentValidityListener.hpp>
#include <com/sun/star/form/XConfirmDeleteBroadcaster.hpp>
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#include <com/sun/star/form/XDatabaseParameterBroadcaster2.hpp>
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/form/runtime/XFilterController.hpp>
#include <com/sun/star/form/XFormControllerListener.hpp>
#include <com/sun/star/form/XGridControlListener.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/form/XResetListener.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#include <com/sun/star/sdb/XRowSetApproveListener.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/util/XModeSelector.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <comphelper/proparrhlp.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <rtl/ref.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>

#include <cppuhelper/compbase.hxx>

struct FmXTextComponentLess : public ::std::binary_function< css::uno::Reference< css::awt::XTextComponent >, css::uno::Reference< css::awt::XTextComponent> , bool>
{
    bool operator() (const css::uno::Reference< css::awt::XTextComponent >& x, const css::uno::Reference< css::awt::XTextComponent >& y) const
    {
        return reinterpret_cast<sal_Int64>(x.get()) < reinterpret_cast<sal_Int64>(y.get());
    }
};

typedef ::std::map< css::uno::Reference< css::awt::XTextComponent >, OUString, FmXTextComponentLess> FmFilterRow;
typedef ::std::vector< FmFilterRow > FmFilterRows;
typedef ::std::vector< css::uno::Reference< css::form::runtime::XFormController > > FmFormControllers;

namespace vcl { class Window; }

namespace svxform
{
    typedef ::std::vector< css::uno::Reference< css::awt::XTextComponent > >    FilterComponents;
    class ControlBorderManager;
    struct FmFieldInfo;

    typedef cppu::WeakComponentImplHelper           <   css::form::runtime::XFormController
                                                    ,   css::form::runtime::XFilterController
                                                    ,   css::awt::XFocusListener
                                                    ,   css::form::XLoadListener
                                                    ,   css::beans::XPropertyChangeListener
                                                    ,   css::awt::XTextListener
                                                    ,   css::awt::XItemListener
                                                    ,   css::container::XContainerListener
                                                    ,   css::util::XModifyListener
                                                    ,   css::form::XConfirmDeleteListener
                                                    ,   css::sdb::XSQLErrorListener
                                                    ,   css::sdbc::XRowSetListener
                                                    ,   css::sdb::XRowSetApproveListener
                                                    ,   css::form::XDatabaseParameterListener
                                                    ,   css::lang::XServiceInfo
                                                    ,   css::form::XResetListener
                                                    ,   css::frame::XDispatch
                                                    ,   css::awt::XMouseListener
                                                    ,   css::form::validation::XFormComponentValidityListener
                                                    ,   css::task::XInteractionHandler
                                                    ,   css::form::XGridControlListener
                                                    ,   css::form::runtime::XFeatureInvalidation
                                                    >   FormController_BASE;

    class ColumnInfoCache;
    class FormController :public ::cppu::BaseMutex
                                        ,public FormController_BASE
                                        ,public ::cppu::OPropertySetHelper
                                        ,public DispatchInterceptor
                                        ,public ::comphelper::OAggregationArrayUsageHelper< FormController >
                                        ,public ::svxform::OSQLParserClient
    {
        typedef ::std::map  <   sal_Int16,
                                css::uno::Reference< css::frame::XDispatch >
                            >   DispatcherContainer;

        css::uno::Reference< css::uno::XAggregation>              m_xAggregate;
        css::uno::Reference< css::awt::XTabController>            m_xTabController;
        css::uno::Reference< css::awt::XControl>                  m_xActiveControl, m_xCurrentControl;
        css::uno::Reference< css::container::XIndexAccess>        m_xModelAsIndex;
        css::uno::Reference< css::script::XEventAttacherManager>  m_xModelAsManager;
        css::uno::Reference< css::uno::XInterface>                m_xParent;
        css::uno::Reference< css::uno::XComponentContext>         m_xComponentContext;
        // Composer used for checking filter conditions
        css::uno::Reference< css::sdb::XSingleSelectQueryComposer >       m_xComposer;
        css::uno::Reference< css::task::XInteractionHandler >             m_xInteractionHandler;
        css::uno::Reference< css::form::runtime::XFormControllerContext > m_xFormControllerContext;

        css::uno::Sequence< css::uno::Reference< css::awt::XControl> >   m_aControls;
        ::comphelper::OInterfaceContainerHelper2
                                    m_aActivateListeners,
                                    m_aModifyListeners,
                                    m_aErrorListeners,
                                    m_aDeleteListeners,
                                    m_aRowSetApproveListeners,
                                    m_aParameterListeners,
                                    m_aFilterListeners;

        FmFormControllers           m_aChildren;
        FilterComponents            m_aFilterComponents;
        FmFilterRows                m_aFilterRows;

        Idle                        m_aTabActivationIdle;
        Timer                       m_aFeatureInvalidationTimer;

        ::svxform::ControlBorderManager*
                                    m_pControlBorderManager;

        css::uno::Reference< css::form::runtime::XFormOperations >
                                    m_xFormOperations;
        DispatcherContainer         m_aFeatureDispatchers;
        ::std::set< sal_Int16 >     m_aInvalidFeatures;     // for asynchronous feature invalidation

        OUString                    m_aMode;

        ::svxform::DelayedEvent     m_aLoadEvent;
        ::svxform::DelayedEvent     m_aToggleEvent;
        ::svxform::DelayedEvent     m_aActivationEvent;
        ::svxform::DelayedEvent     m_aDeactivationEvent;

        ::std::unique_ptr< ColumnInfoCache >
                                    m_pColumnInfoCache;

        sal_Int32                   m_nCurrentFilterPosition;   // current level for filtering (or-criteria)

        bool                        m_bCurrentRecordModified    : 1;
        bool                        m_bCurrentRecordNew         : 1;
        bool                        m_bLocked                   : 1;
        bool                        m_bDBConnection             : 1;    // Focuslistener nur fuer Datenbankformulare
        bool                        m_bCycle                    : 1;
        bool                        m_bCanInsert                : 1;
        bool                        m_bCanUpdate                : 1;
        bool                        m_bCommitLock               : 1;    // lock the committing of controls see focusGained
        bool                        m_bModified                 : 1;    // ist der Inhalt eines Controls modifiziert ?
        bool                        m_bControlsSorted           : 1;
        bool                        m_bFiltering                : 1;
        bool                        m_bAttachEvents             : 1;
        bool                        m_bDetachEvents             : 1;
        bool                        m_bAttemptedHandlerCreation : 1;
        bool                        m_bSuspendFilterTextListening;          // no bit field, passed around as reference

        // as we want to intercept dispatches of _all_ controls we're responsible for, and an object implementing
        // the css::frame::XDispatchProviderInterceptor interface can intercept only _one_ objects dispatches, we need a helper class
        std::vector<rtl::Reference<DispatchInterceptionMultiplexer>>  m_aControlDispatchInterceptors;

    public:
        FormController( const css::uno::Reference< css::uno::XComponentContext > & _rxORB );

    protected:
        virtual ~FormController();

    // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& type) throw ( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

    // XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL& _rURL, const css::uno::Sequence< css::beans::PropertyValue >& _rArgs ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& _rxListener, const css::util::URL& _rURL ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& _rxListener, const css::util::URL& _rURL ) throw (css::uno::RuntimeException, std::exception) override;

    // css::container::XChild
        virtual css::uno::Reference< css::uno::XInterface> SAL_CALL getParent() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL setParent(const css::uno::Reference< css::uno::XInterface>& Parent) throw( css::lang::NoSupportException, css::uno::RuntimeException, std::exception ) override;

    // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

    // OComponentHelper
        virtual void SAL_CALL disposing() override;

    // OPropertySetHelper
        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any & rConvertedValue, css::uno::Any & rOldValue,
                                                sal_Int32 nHandle, const css::uno::Any& rValue )
                    throw( css::lang::IllegalArgumentException ) override;

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw( css::uno::Exception, std::exception ) override;
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

        virtual css::uno::Reference< css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw( css::uno::RuntimeException, std::exception ) override;
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

        using OPropertySetHelper::getFastPropertyValue;

        // XFilterController
        virtual ::sal_Int32 SAL_CALL getFilterComponents() throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getDisjunctiveTerms() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addFilterControllerListener( const css::uno::Reference< css::form::runtime::XFilterControllerListener >& Listener ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeFilterControllerListener( const css::uno::Reference< css::form::runtime::XFilterControllerListener >& Listener ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL setPredicateExpression( ::sal_Int32 Component, ::sal_Int32 Term, const OUString& PredicateExpression ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XControl > SAL_CALL getFilterComponent( ::sal_Int32 Component ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL getPredicateExpressions() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeDisjunctiveTerm( ::sal_Int32 Term ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL appendEmptyDisjunctiveTerm() throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getActiveTerm() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setActiveTerm( ::sal_Int32 ActiveTerm ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL hasElements() throw( css::uno::RuntimeException, std::exception ) override;

    // css::container::XEnumerationAccess
        virtual css::uno::Reference< css::container::XEnumeration> SAL_CALL createEnumeration() throw( css::uno::RuntimeException, std::exception ) override;

    // css::container::XContainerListener
        virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XLoadListener
        virtual void SAL_CALL loaded(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL unloaded(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL unloading(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL reloading(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL reloaded(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XModeSelector
        virtual void SAL_CALL setMode(const OUString& Mode) throw( css::lang::NoSupportException, css::uno::RuntimeException, std::exception ) override;
        virtual OUString SAL_CALL getMode() throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedModes() throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL supportsMode(const OUString& Mode) throw( css::uno::RuntimeException, std::exception ) override;

    // css::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 Index) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener(const css::uno::Reference< css::util::XModifyListener>& l) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeModifyListener(const css::uno::Reference< css::util::XModifyListener>& l) throw( css::uno::RuntimeException, std::exception ) override;

    // XFocusListener
        virtual void SAL_CALL focusGained(const  css::awt::FocusEvent& e) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL focusLost(const  css::awt::FocusEvent& e) throw( css::uno::RuntimeException, std::exception ) override;

    // XMouseListener
        virtual void SAL_CALL mousePressed( const css::awt::MouseEvent& _rEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL mouseReleased( const css::awt::MouseEvent& _rEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL mouseEntered( const css::awt::MouseEvent& _rEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL mouseExited( const css::awt::MouseEvent& _rEvent ) throw (css::uno::RuntimeException, std::exception) override;

    // XFormComponentValidityListener
        virtual void SAL_CALL componentValidityChanged( const css::lang::EventObject& _rSource ) throw (css::uno::RuntimeException, std::exception) override;

    // XInteractionHandler
        virtual void SAL_CALL handle( const css::uno::Reference< css::task::XInteractionRequest >& Request ) throw (css::uno::RuntimeException, std::exception) override;

    // XGridControlListener
        virtual void SAL_CALL columnChanged( const css::lang::EventObject& _event ) throw (css::uno::RuntimeException, std::exception) override;

    // css::beans::XPropertyChangeListener -> change of states
        virtual void SAL_CALL propertyChange(const  css::beans::PropertyChangeEvent& evt) throw( css::uno::RuntimeException, std::exception ) override;

    // XTextListener           -> modify setzen
        virtual void SAL_CALL textChanged(const  css::awt::TextEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XItemListener            -> modify setzen
        virtual void SAL_CALL itemStateChanged(const  css::awt::ItemEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XModifyListener   -> modify setzen
        virtual void SAL_CALL modified(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XFormController
        virtual css::uno::Reference< css::form::runtime::XFormOperations > SAL_CALL getFormOperations() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XControl> SAL_CALL getCurrentControl() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL addActivateListener(const css::uno::Reference< css::form::XFormControllerListener>& l) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeActivateListener(const css::uno::Reference< css::form::XFormControllerListener>& l) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL addChildController( const css::uno::Reference< css::form::runtime::XFormController >& ChildController ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception ) override;

        virtual css::uno::Reference< css::form::runtime::XFormControllerContext > SAL_CALL getContext() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setContext( const css::uno::Reference< css::form::runtime::XFormControllerContext >& _context ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::task::XInteractionHandler > SAL_CALL getInteractionHandler() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setInteractionHandler( const css::uno::Reference< css::task::XInteractionHandler >& _interactionHandler ) throw (css::uno::RuntimeException, std::exception) override;

    // XTabController
        virtual css::uno::Sequence< css::uno::Reference< css::awt::XControl> > SAL_CALL getControls() throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setModel(const css::uno::Reference< css::awt::XTabControllerModel>& Model) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Reference< css::awt::XTabControllerModel> SAL_CALL getModel() throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setContainer(const css::uno::Reference< css::awt::XControlContainer>& Container) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Reference< css::awt::XControlContainer> SAL_CALL getContainer() throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL autoTabOrder() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL activateTabOrder() throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL activateFirst() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL activateLast() throw( css::uno::RuntimeException, std::exception ) override;

    // css::sdbc::XRowSetListener
        virtual void SAL_CALL cursorMoved(const css::lang::EventObject& event) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL rowChanged(const css::lang::EventObject& event) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL rowSetChanged(const css::lang::EventObject& event) throw( css::uno::RuntimeException, std::exception ) override;

    // XRowSetApproveListener
        virtual sal_Bool SAL_CALL approveCursorMove(const css::lang::EventObject& event) throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL approveRowChange(const  css::sdb::RowChangeEvent& event) throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL approveRowSetChange(const css::lang::EventObject& event) throw( css::uno::RuntimeException, std::exception ) override;

    // XRowSetApproveBroadcaster
        virtual void SAL_CALL addRowSetApproveListener(const css::uno::Reference< css::sdb::XRowSetApproveListener>& listener) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeRowSetApproveListener(const css::uno::Reference< css::sdb::XRowSetApproveListener>& listener) throw( css::uno::RuntimeException, std::exception ) override;

    // XSQLErrorBroadcaster
        virtual void SAL_CALL errorOccured(const css::sdb::SQLErrorEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XSQLErrorListener
        virtual void SAL_CALL addSQLErrorListener(const css::uno::Reference< css::sdb::XSQLErrorListener>& _rListener) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeSQLErrorListener(const css::uno::Reference< css::sdb::XSQLErrorListener>& _rListener) throw( css::uno::RuntimeException, std::exception ) override;

    // XDatabaseParameterBroadcaster2
        virtual void SAL_CALL addDatabaseParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener>& aListener) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeDatabaseParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener>& aListener) throw( css::uno::RuntimeException, std::exception ) override;

    // XDatabaseParameterBroadcaster
        virtual void SAL_CALL addParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener>& aListener) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener>& aListener) throw( css::uno::RuntimeException, std::exception ) override;

    // XDatabaseParameterListener
        virtual sal_Bool SAL_CALL approveParameter(const css::form::DatabaseParameterEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XConfirmDeleteBroadcaster
        virtual void SAL_CALL addConfirmDeleteListener(const css::uno::Reference< css::form::XConfirmDeleteListener>& aListener) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeConfirmDeleteListener(const css::uno::Reference< css::form::XConfirmDeleteListener>& aListener) throw( css::uno::RuntimeException, std::exception ) override;

    // XConfirmDeleteListener
        virtual sal_Bool SAL_CALL confirmDelete(const  css::sdb::RowChangeEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString >  SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XResetListener
        virtual sal_Bool SAL_CALL approveReset(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL resetted(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

        // XFeatureInvalidation
        virtual void SAL_CALL invalidateFeatures( const css::uno::Sequence< ::sal_Int16 >& Features ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL invalidateAllFeatures(  ) throw (css::uno::RuntimeException, std::exception) override;

// method for registration
        static  css::uno::Sequence< OUString > const &  getSupportedServiceNames_Static();

        // comphelper::OPropertyArrayUsageHelper
        virtual void fillProperties(
            css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps,
            css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps
            ) const override;

    protected:
        // DispatchInterceptor
        virtual css::uno::Reference< css::frame::XDispatch>
            interceptedQueryDispatch(
                    const css::util::URL& aURL,
                    const OUString& aTargetFrameName,
                    sal_Int32 nSearchFlags
                )   throw( css::uno::RuntimeException ) override;

        virtual ::osl::Mutex* getInterceptorMutex() override { return &m_aMutex; }

        /// update all our dispatchers
        void    updateAllDispatchers() const;

        /** disposes all dispatchers in m_aFeatureDispatchers, and empties m_aFeatureDispatchers
        */
        void    disposeAllFeaturesAndDispatchers();

        void startFiltering();
        void stopFiltering();
        void setFilter(::std::vector<FmFieldInfo>&);
        void startListening();
        void stopListening();

        /** ensures that we have an interaction handler, if possible

            If an interaction handler was provided at creation time (<member>initialize</member>), this
            one will be used. Else, an attempt is made to create an <type scope="css::sdb">InteractionHandler</type>
            is made.

            @return <TRUE/>
                if and only if <member>m_xInteractionHandler</member> is valid when the method returns
        */
        bool ensureInteractionHandler();

        /** replaces one of our controls with another one

            Upon successful replacing, the old control will be disposed. Also, internal members pointing
            to the current or active control will be adjusted. Yet more, if the replaced control was
            the active control, the new control will be made active.

            @param _rxExistentControl
                The control to replace. Must be one of the controls in our ControlContainer.
            @param _rxNewControl
                The control which should replace the existent control.
            @return
                <TRUE/> if and only if the control was successfully replaced
        */
        bool    replaceControl(
            const css::uno::Reference< css::awt::XControl >& _rxExistentControl,
            const css::uno::Reference< css::awt::XControl >& _rxNewControl
        );

        // we're listening at all bound controls for modifications
        void startControlModifyListening(const css::uno::Reference< css::awt::XControl>& xControl);
        void stopControlModifyListening(const css::uno::Reference< css::awt::XControl>& xControl);

        void setLocks();
        void setControlLock(const css::uno::Reference< css::awt::XControl>& xControl);
        void addToEventAttacher(const css::uno::Reference< css::awt::XControl>& xControl);
        void removeFromEventAttacher(const css::uno::Reference< css::awt::XControl>& xControl);
        void toggleAutoFields(bool bAutoFields);
        void unload() throw( css::uno::RuntimeException );
        void removeBoundFieldListener();

        void startFormListening( const css::uno::Reference< css::beans::XPropertySet >& _rxForm, bool _bPropertiesOnly  );
        void stopFormListening( const css::uno::Reference< css::beans::XPropertySet >& _rxForm, bool _bPropertiesOnly );

        css::uno::Reference< css::awt::XControl> findControl( css::uno::Sequence< css::uno::Reference< css::awt::XControl> >& rCtrls, const css::uno::Reference< css::awt::XControlModel>& rxCtrlModel, bool _bRemove, bool _bOverWrite ) const;

        void insertControl(const css::uno::Reference< css::awt::XControl>& xControl);
        void removeControl(const css::uno::Reference< css::awt::XControl>& xControl);

        /// called when a new control is to be handled by the controller
        void implControlInserted( const css::uno::Reference< css::awt::XControl>& _rxControl, bool _bAddToEventAttacher );
        /// called when a control is not to be handled by the controller anymore
        void implControlRemoved( const css::uno::Reference< css::awt::XControl>& _rxControl, bool _bRemoveFromEventAttacher );

        /** sets m_xCurrentControl, plus does administrative tasks depending on it
        */
        void    implSetCurrentControl( const css::uno::Reference< css::awt::XControl >& _rxControl );

        /** invalidates the FormFeatures which depend on the current control
        */
        void    implInvalidateCurrentControlDependentFeatures();

        bool    impl_isDisposed_nofail() const { return FormController_BASE::rBHelper.bDisposed; }
        void    impl_checkDisposed_throw() const;

        void    impl_onModify();

        /** adds the given filter row to m_aFilterRows, setting m_nCurrentFilterPosition to 0 if the newly added
            row is the first one.

            @precond
                our mutex is locked
        */
        void    impl_addFilterRow( const FmFilterRow& _row );

        /** adds an empty filter row to m_aFilterRows, and notifies our listeners
        */
        void    impl_appendEmptyFilterRow( ::osl::ClearableMutexGuard& _rClearBeforeNotify );

        bool isLocked() const {return m_bLocked;}
        bool determineLockState() const;

        vcl::Window* getDialogParentWindow();
            // returns the window which should be used as parent window for dialogs

        css::uno::Reference< css::frame::XDispatchProviderInterceptor>    createInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterception>& _xInterception);
            // create a new interceptor, register it on the given object
        void                            deleteInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterception>& _xInterception);
            // if createInterceptor was called for the given object the according interceptor will be removed
            // from the objects interceptor chain and released

        /** checks all form controls belonging to our form for validity

            If a form control supports the XValidatableFormComponent interface, this is used to determine
            the validity of the control. If the interface is not supported, the control is supposed to be
            valid.

            @param _rFirstInvalidityExplanation
                if the method returns <FALSE/> (i.e. if there is an invalid control), this string contains
                the explanation for the invalidity, as obtained from the validator.

            @param _rxFirstInvalidModel
                if the method returns <FALSE/> (i.e. if there is an invalid control), this contains
                the control model

            @return
                <TRUE/> if and only if all controls belonging to our form are valid
        */
        bool    checkFormComponentValidity(
                    OUString& /* [out] */ _rFirstInvalidityExplanation,
                    css::uno::Reference< css::awt::XControlModel >& /* [out] */ _rxFirstInvalidModel
                );

        /** locates the control which belongs to a given model
        */
        css::uno::Reference< css::awt::XControl >
                locateControl( const css::uno::Reference< css::awt::XControlModel >& _rxModel );

        // set the text for all filters
        void impl_setTextOnAllFilter_throw();

        // in filter mode we do not listen for changes
        bool isListeningForChanges() const {return m_bDBConnection && !m_bFiltering && !isLocked();}
        css::uno::Reference< css::awt::XControl> isInList(const css::uno::Reference< css::awt::XWindowPeer>& xPeer) const;

        DECL_LINK_TYPED( OnActivateTabOrder, Idle*, void );
        DECL_LINK_TYPED( OnInvalidateFeatures, Timer*, void );
        DECL_LINK_TYPED( OnLoad, void*, void );
        DECL_LINK_TYPED( OnToggleAutoFields, void*, void );
        DECL_LINK_TYPED( OnActivated, void*, void );
        DECL_LINK_TYPED( OnDeactivated, void*, void );
    };

}

#endif // INCLUDED_SVX_SOURCE_INC_FORMCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
