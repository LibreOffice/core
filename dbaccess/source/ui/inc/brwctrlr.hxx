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

#ifndef _SBA_BWRCTRLR_HXX
#define _SBA_BWRCTRLR_HXX

#include "genericcontroller.hxx"
#include "moduledbu.hxx"
#include "brwview.hxx"
#include "sbagrid.hxx"

#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/form/XResetListener.hpp>
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XModule.hpp>

#include <vcl/timer.hxx>
#include <svtools/transfer.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <cppuhelper/implbase9.hxx>
#include <svtools/cliplistener.hxx>

struct FmFoundRecordInformation;
struct FmSearchContext;

namespace dbtools
{
    class SQLExceptionInfo;
}

namespace dbaui
{
    // =========================================================================

    typedef ::cppu::ImplInheritanceHelper9  <   OGenericUnoController
                                            ,   ::com::sun::star::sdb::XSQLErrorListener
                                            ,   ::com::sun::star::form::XDatabaseParameterListener
                                            ,   ::com::sun::star::form::XConfirmDeleteListener
                                            ,   ::com::sun::star::form::XLoadListener
                                            ,   ::com::sun::star::form::XResetListener
                                            ,   ::com::sun::star::awt::XFocusListener
                                            ,   ::com::sun::star::container::XContainerListener
                                            ,   ::com::sun::star::beans::XPropertyChangeListener
                                            ,   ::com::sun::star::frame::XModule
                                            >   SbaXDataBrowserController_Base;

    class SbaXDataBrowserController :public SbaXDataBrowserController_Base
                                    ,public SbaGridListener
    {
    // ==========
    // attributes
    private:
        // for implementing the XFormController
        class FormControllerImpl;
        friend class FormControllerImpl;
        OModuleClient                                                            m_aModuleClient;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >             m_xRowSet;      // our rowset
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >   m_xColumnsSupplier; // queried from the rowset member
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >           m_xLoadable;        // queried from the rowset member as well
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >      m_xGridModel;   // the model of our grid
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >         m_xFormControllerImpl;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >
                                                                                        m_xParser;      // for sorting 'n filtering

        sal_Int32               m_nRowSetPrivileges;    // cached Privileges property of m_xRowSet

        AutoTimer               m_aInvalidateClipboard;             // for testing the state of the CUT/COPY/PASTE-slots

        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        TransferableClipboardListener*
                                m_pClipbordNotifier;    // notifier for changes in the clipboard

        ::osl::Mutex            m_aAsyncLoadSafety;     // for multi-thread access to our members

        OAsyncronousLink        m_aAsyncGetCellFocus;
        OAsyncronousLink        m_aAsyncDisplayError;
        ::dbtools::SQLExceptionInfo m_aCurrentError;

        String                  m_sStateSaveRecord;
        String                  m_sStateUndoRecord;
        ::rtl::OUString         m_sModuleIdentifier;

        // members for asynchronous load operations
        FormControllerImpl*     m_pFormControllerImpl;  // implementing the XFormController

        sal_uLong                   m_nPendingLoadFinished;         // the event used to tell ourself that the load is finished
        sal_uInt16              m_nFormActionNestingLevel;      // see enter-/leaveFormAction

        sal_Bool                m_bLoadCanceled : 1;            // the load was canceled somehow
        bool                    m_bCannotSelectUnfiltered : 1;  // recieved an DATA_CANNOT_SELECT_UNFILTERED error

    protected:
        class FormErrorHelper
        {
            SbaXDataBrowserController*  m_pOwner;
        public:
            FormErrorHelper(SbaXDataBrowserController* pOwner) : m_pOwner(pOwner) { m_pOwner->enterFormAction(); }
            virtual ~FormErrorHelper() { m_pOwner->leaveFormAction(); }
        };
        friend class FormErrorHelper;

    // ================
    // attribute access
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >             getRowSet()         const   { return m_xRowSet; }
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >   getColumnsSupplier()const   { return m_xColumnsSupplier; }
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >           getLoadable()       const   { return m_xLoadable; }

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >      getFormComponent()  const   { return m_xGridModel; }
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >        getControlModel()   const   { return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > (m_xGridModel, ::com::sun::star::uno::UNO_QUERY); }
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    getNumberFormatter()const   { return m_xFormatter; }

        sal_Bool    isValid() const         { return m_xRowSet.is() && m_xGridModel.is(); }
        sal_Bool    isValidCursor() const;  // checks the ::com::sun::star::data::XDatabaseCursor-interface of m_xRowSet
        sal_Bool    isLoaded() const;
        sal_Bool    loadingCancelled() const { return m_bLoadCanceled; }
        void        onStartLoading( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >& _rxLoadable );
        void        setLoadingCancelled()   { m_bLoadCanceled = sal_True; }

        const TransferableDataHelper&
            getViewClipboard() const { return m_aSystemClipboard; }

    public:
        SbaXDataBrowserController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        UnoDataBrowserView* getBrowserView() const { return static_cast< UnoDataBrowserView*>(getView()); }
        // late construction
        virtual sal_Bool Construct(Window* pParent);

        // UNO
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::util::XModifyListener
        virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // XModule
        virtual void SAL_CALL setIdentifier( const ::rtl::OUString& Identifier ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getIdentifier(  ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::awt::XFocusListener
        virtual void SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing();

        // ::com::sun::star::frame::XFrameActionListener
        virtual void        SAL_CALL frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::sdb::XSQLErrorListener
        virtual void        SAL_CALL errorOccured(const ::com::sun::star::sdb::SQLErrorEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::form::XDatabaseParameterListener
        virtual sal_Bool    SAL_CALL approveParameter(const ::com::sun::star::form::DatabaseParameterEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::form::XConfirmDeleteListener
        virtual sal_Bool    SAL_CALL confirmDelete(const ::com::sun::star::sdb::RowChangeEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::form::XLoadListener
        virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::form::XResetListener
        virtual sal_Bool SAL_CALL approveReset(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL resetted(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );

        // SbaGridListener
        virtual void RowChanged();
        virtual void ColumnChanged();
        virtual void SelectionChanged();
        virtual void CellActivated();
        virtual void CellDeactivated();
        virtual void BeforeDrop();
        virtual void AfterDrop();

    public:

    protected:
        virtual ~SbaXDataBrowserController();

        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        virtual void    startFrameListening( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame );
        virtual void    stopFrameListening( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  CreateForm();
            // our default implementation simply instantiates a stardiv.one.form.component.Form service
            // (probably this needs not to be overloaded, but you may return anything you want as long as it
            // supports the ::com::sun::star::form::DatabaseForm service. For instance you may want to create an adapter here which
            // is synchronized with a foreign ::com::sun::star::form::DatabaseForm you got elsewhere)
        virtual sal_Bool InitializeForm(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& i_formProperties ) = 0;
            // called immediately after a successfull CreateForm
            // do any initialization (data source etc.) here. the form should be fully functional after that.
            // return sal_False if you didn't succeed (don't throw exceptions, they won't be caught)

        virtual sal_Bool InitializeGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);


        virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  CreateGridModel();
            // our default implementation simply instantiates a stardiv.one.form.component.Grid service
            // you most probably don't want to override this behaviuor

        // the default implementation of disposing distributes the events to the following disposingXXX functions
        virtual void disposingGridControl(const ::com::sun::star::lang::EventObject& Source);   // calls removeControlListeners
        virtual void disposingGridModel(const ::com::sun::star::lang::EventObject& Source);     // calls removeModelListeners
        virtual void disposingFormModel(const ::com::sun::star::lang::EventObject& Source);
        virtual void disposingColumnModel(const ::com::sun::star::lang::EventObject& Source);

        // want to be a listener to the grid control ? use this !
        virtual void addControlListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > & _xGridControl);
        virtual void removeControlListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > & _xGridControl);

        // want to be a listener to the grid model ? use this !
        virtual void addModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);
        virtual void removeModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);

        // want to be a listener grid columns ? use this !
        virtual void AddColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);
        virtual void RemoveColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);


            // call after "major changes" (e.g. the completion of the async load).
            // invalidates all toolbox slots and all supported features.

        virtual sal_Bool LoadForm();
            // load the form
            // the default implementation does an direct load or starts a load thread, depending on the multithread capabilities
            // of the data source.
            // the default implementation also calls LoadFinished after a syncronous load, so be sure to do the same if you override
            // this metod and don't call the base class' method

        virtual void LoadFinished(sal_Bool bWasSynch);
            // called if the loading (the _complete_ loading process) is done (no matter if synchron or asynchron).

        virtual void criticalFail();
            // called whenever a reload operation on the rowset failed
            // (a "operation" is not only a simple reload: If the user sets a filter, an reloading the form
            // after setting this filter fails, the filter is reset and the form is reloaded, again. Only the
            // whole process (_both_ XLoadable::reload calls _together_) form the "reload operation"

        // --------------------

            // empty the frame where our view resides
        virtual sal_Bool CommitCurrent();
            // commit the current column (i.e. cell)
        virtual sal_Bool SaveModified(sal_Bool bAskFor = sal_True);
            // save the modified record

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   getBoundField(sal_uInt16 nViewPos = (sal_uInt16)-1) const;
            // a PropertySet corresponding to the cursor field a column is bound to
            // if nViewPos is (sal_uInt16)-1 (the default) then the field for the current column will be retrieved

        void enterFormAction();
        void leaveFormAction();

        // init the formatter if form changes
        void initFormatter();

        /// loads or reloads the form
        virtual sal_Bool reloadForm(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >& _rxLoadable);

        virtual sal_Bool    preReloadForm(){ return sal_False; }
        virtual void        postReloadForm(){}

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >
                            createParser_nothrow();

    private:
        void setCurrentModified( sal_Bool _bSet );

        // execute the filter or sort slot
        void ExecuteFilterSortCrit(sal_Bool bFilter);

        // execute the search slot
        void        ExecuteSearch();

        void        initializeParser() const; // changes the mutable member m_xParser
        void        applyParserFilter(const ::rtl::OUString& _rOldFilter, sal_Bool _bOldFilterApplied,const ::rtl::OUString& _sOldHaving,const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >& _xParser);
        void        applyParserOrder(const ::rtl::OUString& _rOldOrder,const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >& _xParser);

        sal_Int16   getCurrentColumnPosition();
        void        setCurrentColumnPosition( sal_Int16 _nPos );
        void        addColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);

        void        impl_checkForCannotSelectUnfiltered( const ::dbtools::SQLExceptionInfo& _rError );

        // time to check the CUT/COPY/PASTE-slot-states
        DECL_LINK( OnInvalidateClipboard, AutoTimer* );
        DECL_LINK( OnClipboardChanged, void* );

        // search callbacks
        DECL_LINK(OnSearchContextRequest, FmSearchContext*);
        DECL_LINK(OnFoundData, FmFoundRecordInformation*);
        DECL_LINK(OnCanceledNotFound, FmFoundRecordInformation*);

        DECL_LINK(OnAsyncGetCellFocus, void*);

        DECL_LINK( OnAsyncDisplayError, void* );
    };
}

#endif // _SBA_BWRCTRLR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
