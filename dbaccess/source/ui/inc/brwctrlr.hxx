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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_BRWCTRLR_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_BRWCTRLR_HXX

#include <dbaccess/genericcontroller.hxx>
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
#include <cppuhelper/implbase.hxx>
#include <svtools/cliplistener.hxx>

struct FmFoundRecordInformation;
struct FmSearchContext;

namespace dbtools
{
    class SQLExceptionInfo;
}

namespace dbaui
{

    typedef ::cppu::ImplInheritanceHelper  <   OGenericUnoController
                                            ,   css::sdb::XSQLErrorListener
                                            ,   css::form::XDatabaseParameterListener
                                            ,   css::form::XConfirmDeleteListener
                                            ,   css::form::XLoadListener
                                            ,   css::form::XResetListener
                                            ,   css::awt::XFocusListener
                                            ,   css::container::XContainerListener
                                            ,   css::beans::XPropertyChangeListener
                                            ,   css::frame::XModule
                                            >   SbaXDataBrowserController_Base;

    class SbaXDataBrowserController :public SbaXDataBrowserController_Base
                                    ,public SbaGridListener
    {
    // attributes
    private:
        // for implementing the XFormController
        class FormControllerImpl;
        friend class FormControllerImpl;
        OModuleClient                                         m_aModuleClient;

        css::uno::Reference< css::sdbc::XRowSet >             m_xRowSet;      // our rowset
        css::uno::Reference< css::sdbcx::XColumnsSupplier >   m_xColumnsSupplier; // queried from the rowset member
        css::uno::Reference< css::form::XLoadable >           m_xLoadable;        // queried from the rowset member as well
        css::uno::Reference< css::form::XFormComponent >      m_xGridModel;   // the model of our grid
        css::uno::Reference< css::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        css::uno::Reference< css::uno::XAggregation >         m_xFormControllerImpl;
        mutable css::uno::Reference< css::sdb::XSingleSelectQueryComposer >
                                                              m_xParser;      // for sorting 'n filtering

        sal_Int32               m_nRowSetPrivileges;    // cached Privileges property of m_xRowSet

        AutoTimer               m_aInvalidateClipboard;             // for testing the state of the CUT/COPY/PASTE-slots

        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        TransferableClipboardListener*
                                m_pClipbordNotifier;    // notifier for changes in the clipboard

        OAsynchronousLink       m_aAsyncGetCellFocus;
        OAsynchronousLink       m_aAsyncDisplayError;
        ::dbtools::SQLExceptionInfo m_aCurrentError;

        OUString                m_sStateSaveRecord;
        OUString                m_sStateUndoRecord;
        OUString                m_sModuleIdentifier;

        // members for asynchronous load operations
        FormControllerImpl*     m_pFormControllerImpl;  // implementing the XFormController

        sal_uInt16              m_nFormActionNestingLevel;      // see enter-/leaveFormAction

        bool                    m_bLoadCanceled : 1;            // the load was canceled somehow
        bool                    m_bCannotSelectUnfiltered : 1;  // received an DATA_CANNOT_SELECT_UNFILTERED error

    protected:
        class FormErrorHelper
        {
            SbaXDataBrowserController*  m_pOwner;
        public:
            FormErrorHelper(SbaXDataBrowserController* pOwner) : m_pOwner(pOwner) { m_pOwner->enterFormAction(); }
            virtual ~FormErrorHelper() { m_pOwner->leaveFormAction(); }
        };
        friend class FormErrorHelper;

    // attribute access
    protected:
        css::uno::Reference< css::sdbc::XRowSet >             getRowSet()         const   { return m_xRowSet; }
        css::uno::Reference< css::sdbcx::XColumnsSupplier >   getColumnsSupplier()const   { return m_xColumnsSupplier; }
        css::uno::Reference< css::form::XLoadable >           getLoadable()       const   { return m_xLoadable; }

        css::uno::Reference< css::form::XFormComponent >      getFormComponent()  const   { return m_xGridModel; }
        css::uno::Reference< css::awt::XControlModel >        getControlModel()   const   { return css::uno::Reference< css::awt::XControlModel > (m_xGridModel, css::uno::UNO_QUERY); }
        css::uno::Reference< css::util::XNumberFormatter >    getNumberFormatter()const   { return m_xFormatter; }

        bool    isValid() const         { return m_xRowSet.is() && m_xGridModel.is(); }
        bool    isValidCursor() const;  // checks the css::data::XDatabaseCursor-interface of m_xRowSet
        bool    isLoaded() const;
        bool    loadingCancelled() const { return m_bLoadCanceled; }
        void    onStartLoading( const css::uno::Reference< css::form::XLoadable >& _rxLoadable );
        void    setLoadingCancelled()   { m_bLoadCanceled = true; }

    public:
        SbaXDataBrowserController(const css::uno::Reference< css::uno::XComponentContext >& _rM);

        UnoDataBrowserView* getBrowserView() const { return static_cast< UnoDataBrowserView*>(getView()); }
        // late construction
        virtual bool Construct(vcl::Window* pParent) SAL_OVERRIDE;

        // UNO
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::util::XModifyListener
        virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::container::XContainerListener
        virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XModule
        virtual void SAL_CALL setIdentifier( const OUString& Identifier ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getIdentifier(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // css::awt::XFocusListener
        virtual void SAL_CALL focusGained(const css::awt::FocusEvent& e) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL focusLost(const css::awt::FocusEvent& e) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::lang::XComponent
        virtual void        SAL_CALL disposing() SAL_OVERRIDE;

        // css::frame::XFrameActionListener
        virtual void        SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::sdb::XSQLErrorListener
        virtual void        SAL_CALL errorOccured(const css::sdb::SQLErrorEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::form::XDatabaseParameterListener
        virtual sal_Bool    SAL_CALL approveParameter(const css::form::DatabaseParameterEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::form::XConfirmDeleteListener
        virtual sal_Bool    SAL_CALL confirmDelete(const css::sdb::RowChangeEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::form::XLoadListener
        virtual void SAL_CALL loaded(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL unloading(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL unloaded(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL reloading(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL reloaded(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // css::form::XResetListener
        virtual sal_Bool SAL_CALL approveReset(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL resetted(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // SbaGridListener
        virtual void RowChanged() SAL_OVERRIDE;
        virtual void ColumnChanged() SAL_OVERRIDE;
        virtual void SelectionChanged() SAL_OVERRIDE;
        virtual void CellActivated() SAL_OVERRIDE;
        virtual void CellDeactivated() SAL_OVERRIDE;
        virtual void BeforeDrop() SAL_OVERRIDE;
        virtual void AfterDrop() SAL_OVERRIDE;

    public:

    protected:
        virtual ~SbaXDataBrowserController();

        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures() SAL_OVERRIDE;
        // state of a feature. 'feature' may be the handle of a css::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const SAL_OVERRIDE;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) SAL_OVERRIDE;

        virtual void    startFrameListening( const css::uno::Reference< css::frame::XFrame >& _rxFrame ) SAL_OVERRIDE;
        virtual void    stopFrameListening( const css::uno::Reference< css::frame::XFrame >& _rxFrame ) SAL_OVERRIDE;

        virtual css::uno::Reference< css::sdbc::XRowSet >  CreateForm();
            // our default implementation simply instantiates a stardiv.one.form.component.Form service
            // (probably this needs not to be overridden, but you may return anything you want as long as it
            // supports the css::form::DatabaseForm service. For instance you may want to create an adapter here which
            // is synchronized with a foreign css::form::DatabaseForm you got elsewhere)
        virtual bool InitializeForm(
            const css::uno::Reference< css::beans::XPropertySet >& i_formProperties ) = 0;
            // called immediately after a successful CreateForm
            // do any initialization (data source etc.) here. the form should be fully functional after that.
            // return sal_False if you didn't succeed (don't throw exceptions, they won't be caught)

        virtual bool InitializeGridModel(const css::uno::Reference< css::form::XFormComponent > & xGrid);

        css::uno::Reference< css::form::XFormComponent >  CreateGridModel();
            // our default implementation simply instantiates a stardiv.one.form.component.Grid service
            // you most probably don't want to override this behavior

        // the default implementation of disposing distributes the events to the following disposingXXX functions
        void disposingGridControl(const css::lang::EventObject& Source);   // calls removeControlListeners
        void disposingGridModel(const css::lang::EventObject& Source);     // calls removeModelListeners
        void disposingFormModel(const css::lang::EventObject& Source);
        void disposingColumnModel(const css::lang::EventObject& Source);

        // want to be a listener to the grid control ? use this !
        void addControlListeners(const css::uno::Reference< css::awt::XControl > & _xGridControl);
        void removeControlListeners(const css::uno::Reference< css::awt::XControl > & _xGridControl);

        // want to be a listener to the grid model ? use this !
        virtual void addModelListeners(const css::uno::Reference< css::awt::XControlModel > & _xGridControlModel);
        virtual void removeModelListeners(const css::uno::Reference< css::awt::XControlModel > & _xGridControlModel);

        // want to be a listener grid columns ? use this !
        virtual void AddColumnListener(const css::uno::Reference< css::beans::XPropertySet > & xCol);
        virtual void RemoveColumnListener(const css::uno::Reference< css::beans::XPropertySet > & xCol);

            // call after "major changes" (e.g. the completion of the async load).
            // invalidates all toolbox slots and all supported features.

        virtual bool LoadForm();
            // load the form
            // the default implementation does an direct load or starts a load thread, depending on the multithread capabilities
            // of the data source.
            // the default implementation also calls LoadFinished after a synchronous load, so be sure to do the same if you override
            // this method and don't call the base class' method

        virtual void LoadFinished(bool bWasSynch);
            // called if the loading (the _complete_ loading process) is done (no matter if synchron or asynchron).

        virtual void criticalFail();
            // called whenever a reload operation on the rowset failed
            // (a "operation" is not only a simple reload: If the user sets a filter, an reloading the form
            // after setting this filter fails, the filter is reset and the form is reloaded, again. Only the
            // whole process (_both_ XLoadable::reload calls _together_) form the "reload operation"

            // empty the frame where our view resides
        bool CommitCurrent();
            // commit the current column (i.e. cell)
        bool SaveModified(bool bAskFor = true);
            // save the modified record

        css::uno::Reference< css::beans::XPropertySet >   getBoundField(sal_uInt16 nViewPos = (sal_uInt16)-1) const;
            // a PropertySet corresponding to the cursor field a column is bound to
            // if nViewPos is (sal_uInt16)-1 (the default) then the field for the current column will be retrieved

        void enterFormAction();
        void leaveFormAction();

        // init the formatter if form changes
        void initFormatter();

        /// loads or reloads the form
        bool reloadForm(const css::uno::Reference< css::form::XLoadable >& _rxLoadable);

        virtual bool    preReloadForm(){ return false; }
        virtual void        postReloadForm(){}

        css::uno::Reference< css::sdb::XSingleSelectQueryComposer >
                            createParser_nothrow();

    private:
        void setCurrentModified( bool _bSet );

        // execute the filter or sort slot
        void ExecuteFilterSortCrit(bool bFilter);

        // execute the search slot
        void        ExecuteSearch();

        void        initializeParser() const; // changes the mutable member m_xParser
        void        applyParserFilter(const OUString& _rOldFilter, bool _bOldFilterApplied,const ::OUString& _sOldHaving,const css::uno::Reference< css::sdb::XSingleSelectQueryComposer >& _xParser);
        void        applyParserOrder(const OUString& _rOldOrder,const css::uno::Reference< css::sdb::XSingleSelectQueryComposer >& _xParser);

        sal_Int16   getCurrentColumnPosition();
        void        setCurrentColumnPosition( sal_Int16 _nPos );
        void        addColumnListeners(const css::uno::Reference< css::awt::XControlModel > & _xGridControlModel);

        void        impl_checkForCannotSelectUnfiltered( const ::dbtools::SQLExceptionInfo& _rError );

        // time to check the CUT/COPY/PASTE-slot-states
        DECL_LINK_TYPED( OnInvalidateClipboard, Timer*, void );
        DECL_LINK_TYPED( OnClipboardChanged, TransferableDataHelper*, void );

        // search callbacks
        DECL_LINK_TYPED(OnSearchContextRequest, FmSearchContext&, sal_uInt32);
        DECL_LINK(OnFoundData, FmFoundRecordInformation*);
        DECL_LINK(OnCanceledNotFound, FmFoundRecordInformation*);

        DECL_LINK_TYPED( OnAsyncGetCellFocus, void*, void );
        DECL_LINK_TYPED( OnAsyncDisplayError, void*, void );
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_BRWCTRLR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
