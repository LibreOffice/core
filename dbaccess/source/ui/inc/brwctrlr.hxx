/*************************************************************************
 *
 *  $RCSfile: brwctrlr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:44:55 $
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
#ifndef _COM_SUN_STAR_SDBC_XSQLERRORLISTENER_HPP_
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESETLISTENER_HPP_
#include <com/sun/star/form/XResetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
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
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_ //autogen
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX_ //autogen
#include <vos/thread.hxx>
#endif
#ifndef _SFXCANCEL_HXX //autogen
#include <svtools/cancel.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE12_HXX_
#include <cppuhelper/implbase12.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif


class ToolBox;
class ResMgr;
struct FmFoundRecordInformation;
struct FmSearchContext;

namespace dbaui
{
    // =========================================================================
    // a helper for multi-threaded handling of async events
    // -------------------------------------------------------------------------
    /** handles asynchronous links which may be called in multi-threaded environments
        If you use an instance of this class as member of your own class, it will handle
        several crucial points for you (for instance the case that somebody posts the
        event while another thread tries to delete this event in the _destructor_ of the
        class).
    */
    class OAsyncronousLink
    {
        Link                    m_aHandler;

    protected:
        NAMESPACE_VOS(OMutex)*  m_pEventSafety;
        NAMESPACE_VOS(OMutex)*  m_pDestructionSafety;
        BOOL                    m_bOwnMutexes;
        ULONG                   m_nEventId;

    public:
        /** constructs the object
            @param      _rHandler           The link to be called asyncronously
            @param      _pEventSafety       A pointer to a mutex to be used for event safety. See below.
            @param      _pDestructionSafety A pointer to a mutex to be used for destruction safety. See below.

            For a really safe behaviour two mutexes are required. In case your class needs more than one
            OAsyncronousLink instance, you may want to share these mutexes (and hold them as members of
            your class). Thus, if _pEventSafety and _pDestructionSafety are both non-NULL, they will be
            used for the thread-safety of the link. Otherwise the instance will create it's own mutexes.
            <BR>
            If you use this "mutex feature" be aware of the lifetime of all involved objects : the mutexes
            you specify upon construction should live (at least) as long as the OAsyncronousLink object does.
        */
        OAsyncronousLink(const Link& _rHandler, NAMESPACE_VOS(OMutex)* _pEventSafety = NULL, NAMESPACE_VOS(OMutex)* _pDestructionSafety = NULL);
        virtual ~OAsyncronousLink();

        void Call(void* _pArgument = NULL);
        void CancelCall();

    protected:
        DECL_LINK(OnAsyncCall, void*);
    };
    // a smaller version of the FeatureEvent to be broadcasted by an ::com::sun::star::frame::XController
    // -------------------------------------------------------------------------
    struct FeatureState
    {
        ::com::sun::star::uno::Any  aState;
        sal_Bool                    bEnabled;
        sal_Bool                    bRequery;

        FeatureState() : bEnabled(sal_False), bRequery(sal_False) { }
    };

    // =========================================================================

    typedef ::cppu::WeakImplHelper12< ::com::sun::star::frame::XController
                                    , ::com::sun::star::frame::XDispatch
                                    , ::com::sun::star::frame::XDispatchProviderInterceptor
                                    , ::com::sun::star::beans::XPropertyChangeListener
                                    , ::com::sun::star::util::XModifyListener
                                    , ::com::sun::star::container::XContainerListener
                                    , ::com::sun::star::frame::XFrameActionListener
                                    , ::com::sun::star::sdb::XSQLErrorListener
                                    , ::com::sun::star::form::XDatabaseParameterListener
                                    , ::com::sun::star::form::XConfirmDeleteListener
                                    , ::com::sun::star::form::XLoadListener
                                    , ::com::sun::star::form::XResetListener > SbaXDataBrowserController_Base;

    typedef ::cppu::ImplHelper2< ::com::sun::star::lang::XInitialization
                                ,::com::sun::star::lang::XServiceInfo > SbaXDataBrowserController_Base2;

    class SbaXDataBrowserController
                    :public SbaXDataBrowserController_Base
                    ,public ::comphelper::OPropertyContainer
                    ,public SbaGridListener
                    ,public ::comphelper::OPropertyArrayUsageHelper < SbaXDataBrowserController>
                    ,public dbaui::OModuleClient
                    ,public SbaXDataBrowserController_Base2
    {
    // ==========
    // attributes
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >             m_xRowSet;  // our rowset
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >      m_xGridModel;   // the model of our grid
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier

        ::osl::Mutex            m_aAsyncLoadSafety;     // for multi-thread access to our members
        UnoDataBrowserView*     m_pContent;             // our (VCL) "main window"
        // members for asynchronous load operations
        ::vos::OThread*         m_pLoadThread;          // the thread wherein the form is loaded
        sal_uInt32              m_nPendingLoadFinished; // the event used to tell ourself that the load is finished
        sal_Bool                m_bClosingKillOpen;     // are we killing the load thread because we are to be suspended ?

        OAsyncronousLink        m_aAsynClose;
        OAsyncronousLink        m_aAsyncGetCellFocus;
        OAsyncronousLink        m_aAsyncInvalidateAll;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >        m_xParser;              // for sorting 'n filtering
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >      m_xSlaveDispatcher;     // for intercepting dispatches of the grid control
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >      m_xMasterDispatcher;    // dito

        // ------------------
        struct DispatchTarget
        {
            ::com::sun::star::util::URL                 aURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >    xListener;

            DispatchTarget() { }
            DispatchTarget(const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  xRef) : aURL(rURL), xListener(xRef) { }
        };

        // all our listeners where we dispatch status changes
        DECLARE_STL_VECTOR(DispatchTarget,Dispatch);
        Dispatch    m_arrStatusListener;

        String      m_sStateSaveRecord;
        String      m_sStateUndoRecord;

        sal_uInt16  m_nFormActionNestingLevel;  // see enter-/leaveFormAction
        bool        m_bErrorOccured;            // dito

        ::cppu::OBroadcastHelper    m_aPropertyBroadcastHelper;

    protected:
        DECLARE_STL_MAP(::rtl::OUString,sal_Int32, comphelper::UStringMixEqual, SupportedFeatures);
        SupportedFeatures                                                                   m_aSupportedFeatures;
        ::osl::Mutex                                                                        m_aPropertyMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 m_xCurrentFrame;        // the frame we're residing in

        ::cppu::OInterfaceContainerHelper       m_aDisposeListeners;    // clear enough

        sal_Bool                    m_bCurrentlyModified : 1;
            // the RecordStatus of the form is insufficient for deciding whether the current record is modified
            // because it doesn't reflect the state of the current cell
        sal_Bool                    m_bLoadCanceled : 1;        // the load was canceled somehow
        sal_Bool                    m_bFrameUiActive : 1;       // updated in ::frameAction
        sal_Bool                    m_bSuspending : 1;          // we're within suspend

        String                  m_sLoadStopperCaption;
            // If the data source is loaded asynchronously a stopper is created (which will result in activation of
            // the office's red button). This string is used as caption for the stopper.
        AutoTimer   m_aInvalidateClipboard;             // for testing the state of the CUT/COPY/PASTE-slots


        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >         m_xUrlTransformer;      // needed sometimes

        DECLARE_STL_MAP(sal_uInt16,FeatureState,::std::less<sal_uInt16> ,StateCache);
        StateCache              m_aStateCache;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xMultiServiceFacatory;

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
    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >             getRowSet()         const   { return m_xRowSet; }
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >      getFormComponent()  const   { return m_xGridModel; }
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >        getControlModel()   const   { return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > (m_xGridModel, ::com::sun::star::uno::UNO_QUERY); }
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    getNumberFormatter()const   { return m_xFormatter; }

        sal_Bool    isValid() const         { return m_xRowSet.is() && m_xGridModel.is(); }
        sal_Bool    isValidCursor() const;  // checks the ::com::sun::star::data::XDatabaseCursor-interface of m_xRowSet

        UnoDataBrowserView* getContent() const { return m_pContent; }

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >    getParser() const { return m_xParser; }

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    public:
        SbaXDataBrowserController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        // late construction
        virtual sal_Bool Construct(Window* pParent);

        // UNO
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

        // ::com::sun::star::util::XModifyListener
        virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XController
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL attachModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel) throw( ::com::sun::star::uno::RuntimeException ){ return sal_False; };
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Any SAL_CALL getViewData(void) throw( ::com::sun::star::uno::RuntimeException ){ return ::com::sun::star::uno::Any(); };
        virtual void SAL_CALL restoreViewData(const ::com::sun::star::uno::Any& Data) throw( ::com::sun::star::uno::RuntimeException ){};
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  SAL_CALL getModel(void) throw( ::com::sun::star::uno::RuntimeException ){ return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > (); };
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  SAL_CALL getFrame(void) throw( ::com::sun::star::uno::RuntimeException ){ return m_xCurrentFrame; };

        // ::com::sun::star::frame::XDispatch
        virtual void        SAL_CALL dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);
        virtual void        SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL);
        virtual void        SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL);

        // ::com::sun::star::frame::XDispatchProviderInterceptor
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  SAL_CALL getSlaveDispatchProvider(void) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL setSlaveDispatchProvider(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & _xNewProvider) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  SAL_CALL getMasterDispatchProvider(void) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL setMasterDispatchProvider(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & _xNewProvider) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  > SAL_CALL queryDispatches(const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL dispose();
        virtual void        SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener);
        virtual void        SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener);

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
        // lang::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException) = 0;
        virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException) = 0;

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
        {
            return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
        }

        // SbaGridListener
        virtual void RowChanged()       { /* we're not interested in */ };
        virtual void ColumnChanged();
        virtual void SelectionChanged();
        virtual void CellActivated();
        virtual void CellDeactivated();

    protected:
        virtual ~SbaXDataBrowserController();
        // --------------------
        // our own overridables

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
        virtual sal_Bool InitializeGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid) = 0;
            // called immediately after a successfull CreateGridModel
            // do any initialization (data source etc.) here. the grid model should be fully functional after that.
            // return sal_False if you didn't succeed (don't throw exceptions, they won't be catched)
            //
            // the form returned by getDataSource() is already valid so you may want to use it ...

        virtual ToolBox* CreateToolBox(Window* pParent) { return NULL; }
            // If you want a toolbar to be displayed at the top of the grid overload this function.
            // The ToolBox returned here has to be fully initialized (you get no further chance ;)

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

        // all the features which should be handled by this class
        virtual void            AddSupportedFeatures();
            // the features have to be sorted alphabetically !
            // (and please keep in mind that a _reference_ to a sequence is returned !)

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId);
        // execute a feature
        virtual void            Execute(sal_uInt16 nId);

        virtual void InvalidateFeature(const ::rtl::OUString& rURLPath, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > (), sal_Bool _bForceBroadcast = sal_False);
            // if xListener is NULL the change will be forwarded to all listeners to the given ::com::sun::star::util::URL
            // if _bForceBroadcast is sal_True, the current feature state is broadcasted no matter if it is the same as the cached state
        virtual void InvalidateFeature(sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > (), sal_Bool _bForceBroadcast = sal_False);
            // if there is an ::com::sun::star::util::URL translation for the id ('handle') the preceding InvalidateFeature is used.
            // if there is a toolbar slot with the given id it is updated (the new state is determined via GetState)
            // if _bForceBroadcast is sal_True, the current feature state is broadcasted no matter if it is the same as the cached state
        virtual void InvalidateAll(sal_Bool bAsync = sal_False);
            // call after "major changes" (e.g. the completion of the async load).
            // invalidates all toolbox slots and all supported features.

        virtual sal_Bool LoadForm();
            // load the form
            // the default implementation does an direct load or starts a load thread, depending on the multithread capabilities
            // of the data source.
            // the default implementation also calls FormLoaded after a syncronous load, so be sure to do the same if you override
            // this metod and don't call the base class' method

        virtual void FormLoaded(sal_Bool bWasSynch);
            // called if the loading is done (no matter if synchron or asynchron).

        // --------------------
        virtual void EmptyWindow();
            // empty the frame where our view resides
        virtual sal_Bool CommitCurrent();
            // commit the current column (i.e. cell)
        virtual sal_Bool SaveModified(sal_Bool bCommit = sal_True);
            // save the modified record
        virtual sal_uInt16 SaveData(sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False);
            // save all data if modified, eventually ask the user

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   getBoundField(sal_uInt16 nViewPos = (sal_uInt16)-1) const;
            // a PropertySet corresponding to the cursor field a column is bound to
            // if nViewPos is (sal_uInt16)-1 (the default) then the field for the current column will be retrieved

        sal_Bool PendingLoad() { return m_pLoadThread != NULL; }
            // is there an asyncronous load operation in progress ?

        void enterFormAction();
        void leaveFormAction();
        bool errorOccured() const { return m_bErrorOccured; }
            // As many form actions don't throw an exception but call their error handler instead we don't have
            // a chance to recognize errors by exception catching.
            // So for error recognition the above methods may be used.
        // init the formatter if form changes
        void initFormatter();

    private:
        // invalidate features - implementation
        sal_Bool ImplInvalidateTBItem(sal_uInt16 nId, const FeatureState& rState);
        void ImplBroadcastFeatureState(const ::rtl::OUString& _rFeature, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener, sal_Bool _bIgnoreCache);

        // execute the filter or sort slot
        void ExecuteFilterSortCrit(sal_Bool bFilter);

        // execute the search slot
        void ExecuteSearch();

        // click a TB slot
        DECL_LINK(OnToolBoxSelected, ToolBox*);

        // time to check the CUT/COPY/PASTE-slot-states
        DECL_LINK(OnInvalidateClipboard, void*);

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
        DECL_LINK(OnAsyncClose, void*);
        DECL_LINK(OnAsyncGetCellFocus, void*);
        DECL_LINK(OnAsyncInvalidateAll, void*);
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

