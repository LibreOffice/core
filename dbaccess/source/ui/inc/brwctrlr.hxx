/*************************************************************************
 *
 *  $RCSfile: brwctrlr.hxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:55:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBA_BWRCTRLR_HXX
#define _SBA_BWRCTRLR_HXX

#ifndef _SBX_BRWVIEW_HXX
#include "brwview.hxx"
#endif
#ifndef _SBA_GRID_HXX
#include "sbagrid.hxx"
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XSQLERRORLISTENER_HPP_
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESETLISTENER_HPP_
#include <com/sun/star/form/XResetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XDATABASEPARAMETERLISTENER_HPP_
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCONFIRMDELETELISTENER_HPP_
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif
#ifndef _SFXCANCEL_HXX
#include <svtools/cancel.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE8_HXX_
#include <cppuhelper/implbase8.hxx>
#endif
#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef _CLIPLISTENER_HXX
#include <svtools/cliplistener.hxx>
#endif

class ResMgr;
struct FmFoundRecordInformation;
struct FmSearchContext;

namespace dbaui
{

    // =========================================================================

    typedef ::cppu::ImplHelper8 <   ::com::sun::star::sdb::XSQLErrorListener
                                ,   ::com::sun::star::form::XDatabaseParameterListener
                                ,   ::com::sun::star::form::XConfirmDeleteListener
                                ,   ::com::sun::star::form::XLoadListener
                                ,   ::com::sun::star::form::XResetListener
                                ,   ::com::sun::star::awt::XFocusListener
                                ,   ::com::sun::star::container::XContainerListener
                                ,   ::com::sun::star::beans::XPropertyChangeListener
                                >   SbaXDataBrowserController_Base;

    class SbaXDataBrowserController
                    :public OGenericUnoController
                    ,public SbaXDataBrowserController_Base
                    ,public SbaGridListener
    {
    // ==========
    // attributes
    private:
        // for implementing the XFormController
        class FormControllerImpl;
        friend class FormControllerImpl;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >             m_xRowSet;      // our rowset
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >   m_xColumnsSupplier; // queried from the rowset member
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >           m_xLoadable;        // queried from the rowset member as well
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >      m_xGridModel;   // the model of our grid
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >         m_xFormControllerImpl;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >    m_xParser;      // for sorting 'n filtering

        AutoTimer               m_aInvalidateClipboard;             // for testing the state of the CUT/COPY/PASTE-slots

        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        TransferableClipboardListener*
                                m_pClipbordNotifier;    // notifier for changes in the clipboard

        ::osl::Mutex            m_aAsyncLoadSafety;     // for multi-thread access to our members

        OAsyncronousLink        m_aAsyncGetCellFocus;

        String                  m_sLoadStopperCaption;
            // If the data source is loaded asynchronously a stopper is created (which will result in activation of
            // the office's red button). This string is used as caption for the stopper.
        String                  m_sStateSaveRecord;
        String                  m_sStateUndoRecord;

        // members for asynchronous load operations
        ::vos::OThread*         m_pLoadThread;          // the thread wherein the form is loaded
        FormControllerImpl*     m_pFormControllerImpl;  // implementing the XFormController

        sal_uInt32              m_nPendingLoadFinished;         // the event used to tell ourself that the load is finished
        sal_uInt16              m_nFormActionNestingLevel;      // see enter-/leaveFormAction

        sal_Bool                m_bLoadCanceled : 1;            // the load was canceled somehow
        sal_Bool                m_bClosingKillOpen : 1;         // are we killing the load thread because we are to be suspended ?
        sal_Bool                m_bErrorOccured : 1;            // see enter-/leaveFormAction

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
        void        setLoadingStarted()     { m_bLoadCanceled = sal_False; }
        void        setLoadingCancelled()   { m_bLoadCanceled = sal_True; }

        const TransferableDataHelper&
            getViewClipboard() const { return m_aSystemClipboard; }

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >    getParser() const { return m_xParser; }

    public:
        SbaXDataBrowserController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        UnoDataBrowserView* getBrowserView() const { return static_cast< UnoDataBrowserView*>(m_pView); }
        // late construction
        virtual sal_Bool Construct(Window* pParent);

        // UNO
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

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

        // ::com::sun::star::awt::XFocusListener
        virtual void SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XController
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException );
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

    protected:
        virtual ~SbaXDataBrowserController();
        // --------------------
        // our own overridables
        // all the features which should be handled by this class
        virtual void            AddSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  CreateForm();
            // our default implementation simply instantiates a stardiv.one.form.component.Form service
            // (probably this needs not to be overloaded, but you may return anything you want as long as it
            // supports the ::com::sun::star::form::DatabaseForm service. For instance you may want to create an adapter here which
            // is synchronized with a foreign ::com::sun::star::form::DatabaseForm you got elsewhere)
        virtual sal_Bool InitializeForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & xForm) = 0;
            // called immediately after a successfull CreateForm
            // do any initialization (data source etc.) here. the form should be fully functional after that.
            // return sal_False if you didn't succeed (don't throw exceptions, they won't be catched)


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
//      virtual sal_uInt16 SaveData(sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False);
            // save all data if modified, maybe ask the user

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   getBoundField(sal_uInt16 nViewPos = (sal_uInt16)-1) const;
            // a PropertySet corresponding to the cursor field a column is bound to
            // if nViewPos is (sal_uInt16)-1 (the default) then the field for the current column will be retrieved

        sal_Bool PendingLoad() const { return m_pLoadThread != NULL; }
            // is there an asyncronous load operation in progress ?

        void enterFormAction();
        void leaveFormAction();
        bool errorOccured() const { return m_bErrorOccured; }
            // As many form actions don't throw an exception but call their error handler instead we don't have
            // a chance to recognize errors by exception catching.
            // So for error recognition the above methods may be used.
        // init the formatter if form changes
        void initFormatter();

        /// loads or reloads the form
        sal_Bool reloadForm(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >& _rxLoadable);

    private:
        void setCurrentModified( sal_Bool _bSet );

        // execute the filter or sort slot
        void ExecuteFilterSortCrit(sal_Bool bFilter);

        // execute the search slot
        void        ExecuteSearch();

        void        applyParserFilter(const ::rtl::OUString& _rOldFilter, sal_Bool _bOldFilterApplied);
        void        applyParserOrder(const ::rtl::OUString& _rOldOrder);

        sal_uInt16  getCurrentColumnPosition();
        void        setCurrentColumnPosition(sal_uInt16 _nPos);
        void        deleteView();

        // time to check the CUT/COPY/PASTE-slot-states
        DECL_LINK( OnInvalidateClipboard, void* );
        DECL_LINK( OnClipboardChanged, void* );

        // search callbacks
        DECL_LINK(OnSearchContextRequest, FmSearchContext*);
        DECL_LINK(OnFoundData, FmFoundRecordInformation*);
        DECL_LINK(OnCanceledNotFound, FmFoundRecordInformation*);

        // callbacks for the completed loading process
        DECL_LINK(OnOpenFinished, void*);
        DECL_LINK(OnOpenFinishedMainThread, void*);
            // OnOpenFinsihed is called in a foreign thread (the one which does the loading) so it simply posts the
            // OnOpenFinishedMainThread-link (which will be called in the main thread, then) as user event.
            // (the alternative would be to lock the SolarMutex in OnOpenFinished to avoid problems with the needed updates,
            // but playing with this mutex seems very hazardous to me ....)
        DECL_LINK(OnAsyncGetCellFocus, void*);
    };

    //==================================================================
    // LoadFormThread - a thread for asynchronously loading a form
    //==================================================================
    class LoadFormThread : public ::vos::OThread
    {
        ::osl::Mutex    m_aAccessSafety;        // for securing the multi-thread access
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >                 m_xRowSet;          // the data source to be loaded

        Link                    m_aTerminationHandler;  // the handler to be called upon termination
        sal_Bool                    m_bCanceled;            // StopIt has been called ?
        String                  m_sStopperCaption;      // the caption for the ThreadStopper

        // a ThreadStopper will be instantiated so that the open can be canceled via the UI
        class ThreadStopper : protected SfxCancellable
        {
            LoadFormThread* m_pOwner;

        public:
            ThreadStopper(LoadFormThread* pOwner, const String& rTitle);
            virtual ~ThreadStopper() { }

            virtual void    Cancel();

            virtual void    OwnerTerminated();
            // Normally the Owner (a LoadFormThread) would delete the stopper when terminated.
            // Unfortunally the application doesn't remove the 'red light' when a SfxCancellable is deleted
            // if it (the app) can't acquire the solar mutex. The deletion is IGNORED then. So we have to make
            // sure that a) the stopper is deleted from inside the main thread (where the solar mutex is locked)
            // and b) that in the time between the termination of the thread and the deletion of the stopper
            // the latter doesn't access the former.
            // The OwnerTerminated cares for both aspects.
            // SO DON'T DELETE THE STOPPER EXPLICITLY !

        protected:
            // HACK HACK HACK HACK HACK : this should be private, but MSVC doesn't accept the LINK-macro then ....
            DECL_LINK(OnDeleteInMainThread, ThreadStopper*);
        };
        friend class LoadFormThread::ThreadStopper;

    public:
        LoadFormThread(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & _xRowSet, const String& _rStopperCaption) : m_xRowSet(_xRowSet), m_sStopperCaption(_rStopperCaption) { }

        virtual void SAL_CALL run();
        virtual void SAL_CALL onTerminated();

        void SetTerminationHdl(const Link& aTermHdl) { m_aTerminationHandler = aTermHdl; }
            // the handler will be called synchronously (the parameter is a pointer to the thread)
            // if no termination handler is set, the thread disposes the data source and deletes
            // itself upon termination

        // cancels the process. to be called from another thread (of course ;)
        void StopIt();

        // ask if the load canceled
        sal_Bool WasCanceled() const { return m_bCanceled; }
    };
}

#endif // _SBA_BWRCTRLR_HXX

