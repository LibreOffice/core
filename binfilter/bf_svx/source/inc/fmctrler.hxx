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
#ifndef _SVX_FMCTRLER_HXX
#define _SVX_FMCTRLER_HXX

#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#include <com/sun/star/form/XResetListener.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/form/XConfirmDeleteBroadcaster.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/form/XDatabaseParameterBroadcaster.hpp>
#include <com/sun/star/form/XFormController.hpp>
#include <com/sun/star/util/XModeSelector.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>

#include <vcl/timer.hxx>

#include "fmtools.hxx"
#include "sqlparserclient.hxx"

#include <cppuhelper/compbase12.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/broadcasthelper.hxx>
class Window;
namespace binfilter {

struct FmXTextComponentLess : public binary_function< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent> , sal_Bool>
{
    sal_Bool operator() (const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >& x, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >& y) const
    {
        return reinterpret_cast<sal_Int64>(x.get()) < reinterpret_cast<sal_Int64>(y.get());
    }
};

typedef map< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >, FmXTextComponentLess> FmFilterControls;
typedef map< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >, ::rtl::OUString, FmXTextComponentLess> FmFilterRow;
typedef vector< FmFilterRow > FmFilterRows;
typedef vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > > FmFormControllers;

struct FmFieldInfo;
class FmXFormController;
class FmFormView;

typedef ::cppu::WeakAggComponentImplHelper12<	::com::sun::star::form::XFormController
                                            ,::com::sun::star::container::XChild
                                            ,::com::sun::star::container::XIndexAccess		//  access of child contoller
                                            ,::com::sun::star::container::XEnumerationAccess
                                            ,::com::sun::star::awt::XFocusListener
                                            ,::com::sun::star::form::XLoadListener
                                            ,::com::sun::star::beans::XPropertyChangeListener
                                            ,::com::sun::star::awt::XTextListener
                                            ,::com::sun::star::awt::XItemListener
                                            ,::com::sun::star::container::XContainerListener
                                            ,::com::sun::star::util::XModifyListener
                                            ,::com::sun::star::util::XModifyBroadcaster> FmXFormController_BASE1;

typedef ::cppu::ImplHelper12<				::com::sun::star::util::XModeSelector
                                            ,::com::sun::star::form::XConfirmDeleteListener
                                            ,::com::sun::star::form::XConfirmDeleteBroadcaster
                                            ,::com::sun::star::sdb::XSQLErrorListener
                                            ,::com::sun::star::sdb::XSQLErrorBroadcaster
                                            ,::com::sun::star::sdbc::XRowSetListener
                                            ,::com::sun::star::sdb::XRowSetApproveListener
                                            ,::com::sun::star::sdb::XRowSetApproveBroadcaster
                                            ,::com::sun::star::form::XDatabaseParameterListener
                                            ,::com::sun::star::form::XDatabaseParameterBroadcaster
                                            ,::com::sun::star::lang::XServiceInfo
                                            ,::com::sun::star::form::XResetListener> FmXFormController_BASE2;
typedef ::cppu::ImplHelper2					<	::com::sun::star::lang::XUnoTunnel
                                            ,	::com::sun::star::frame::XDispatch
                                            >	FmXFormController_BASE3;

//==================================================================
// FmXFormController
//==================================================================
class FmXFormController		: public ::comphelper::OBaseMutex
                            ,public FmXFormController_BASE1
                            ,public FmXFormController_BASE2
                            ,public FmXFormController_BASE3
                            ,public ::cppu::OPropertySetHelper
                            ,public FmDispatchInterceptor
                            ,public ::comphelper::OAggregationArrayUsageHelper< FmXFormController>
                            ,public ::binfilter::svxform::OSQLParserClient//STRIP008 							,public ::svxform::OSQLParserClient
{
    friend class FmXPageViewWinRec;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation>				m_xAggregate;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController>			m_xTabController, m_xNavigationController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>					m_xActiveControl, m_xCurrentControl;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>		m_xModelAsIndex;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager>	m_xModelAsManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>				m_xParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	m_xORB;
    // Composer used for checking filter conditions
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer>	m_xComposer;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> >	m_aControls;
    ::cppu::OInterfaceContainerHelper
                                m_aActivateListeners,
                                m_aModifyListeners,
                                m_aErrorListeners,
                                m_aDeleteListeners,
                                m_aRowSetApproveListeners,
                                m_aParameterListeners;

    FmFormControllers			m_aChilds;
    FmFilterControls			m_aFilterControls;
    FmFilterRows				m_aFilters;
    ::binfilter::form::OImplementationIdsRef	m_aHoldImplIdHelper;//STRIP008 ::form::OImplementationIdsRef	m_aHoldImplIdHelper;

    Timer						m_aInsertTimer;

    FmFormView*					m_pView;
    Window*						m_pWindow;

    ::rtl::OUString				m_aMode;

    sal_uInt32 					m_nLoadEvent;
    sal_uInt32					m_nUpdateDispatcherEvent;
    sal_uInt32					m_nToggleEvent;

    sal_Int32					m_nCurrentFilterPosition;	// current level for filtering (or-criteria)
    ::rtl::OUString					m_sDispatchPrefix;

    sal_Bool					m_bCurrentRecordModified : 1;
    sal_Bool					m_bCurrentRecordNew : 1;
    sal_Bool					m_bLocked			: 1;
    sal_Bool					m_bDBConnection	 : 1;	// Focuslistener nur fuer Datenbankformulare
    sal_Bool					m_bCycle			 : 1;
    sal_Bool					m_bCanInsert		 : 1;
    sal_Bool					m_bCanUpdate		 : 1;
    sal_Bool					m_bCommitLock	 : 1;	// lock the committing of controls see focusGained
    sal_Bool					m_bModified		 : 1;	// ist der Inhalt eines Controls modifiziert ?
    sal_Bool					m_bControlsSorted : 1;
    sal_Bool					m_bFiltering : 1;
    sal_Bool					m_bAttachEvents : 1;
    sal_Bool					m_bDetachEvents : 1;

    // as we want to intercept dispatches of _all_ controls we're responsible for, and an object implementing
    // the ::com::sun::star::frame::XDispatchProviderInterceptor interface can intercept only _one_ objects dispatches, we need a helper class
    DECLARE_STL_VECTOR(FmXDispatchInterceptorImpl*, Interceptors);
    Interceptors	m_aControlDispatchInterceptors;

public:
    FmXFormController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & _rxORB,
                      FmFormView* _pView = NULL, Window* _pWindow = NULL, const UniString& _sDispatchPrefix = UniString());
    ~FmXFormController();

    // UNO Anbindung
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
            { return FmXFormController_BASE1::queryInterface( type ); }
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException)
    {
        return ::binfilter::form::OImplementationIds::getImplementationId(getTypes());//STRIP008 		return ::form::OImplementationIds::getImplementationId(getTypes());
    }
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

// XDispatch
    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& _rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArgs ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL getParent(void) throw( ::com::sun::star::uno::RuntimeException ) {return m_xParent;}
    virtual void SAL_CALL setParent(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Parent) throw( ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException )
    {m_xParent = Parent;}

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

// OComponentHelper
    virtual void SAL_CALL disposing();

// OPropertySetHelper
    virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue,
                                            sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw( ::com::sun::star::lang::IllegalArgumentException );

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw( ::com::sun::star::uno::Exception );
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

// XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasElements(void) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration> SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );

// XModeSelector
    virtual void SAL_CALL setMode(const ::rtl::OUString& Mode) throw( ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getMode(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedModes(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsMode(const ::rtl::OUString& Mode) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 Index) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

// XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener>& l) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener>& l) throw( ::com::sun::star::uno::RuntimeException );

// XFocusListener
    virtual void SAL_CALL focusGained(const  ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL focusLost(const  ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::beans::XPropertyChangeListener -> aenderung der stati
    virtual void SAL_CALL propertyChange(const  ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException );

// XTextListener		   -> modify setzen
    virtual void SAL_CALL textChanged(const  ::com::sun::star::awt::TextEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// XItemListener			-> modify setzen
    virtual void SAL_CALL itemStateChanged(const  ::com::sun::star::awt::ItemEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// XModifyListener   -> modify setzen
    virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// XFormController
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> SAL_CALL getCurrentControl(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addActivateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener>& l) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeActivateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener>& l) throw( ::com::sun::star::uno::RuntimeException );

// XTabController
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> > SAL_CALL getControls(void) throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel>& Model) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel> SAL_CALL getModel() throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer>& Container) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> SAL_CALL getContainer() throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL autoTabOrder() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL activateTabOrder() throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL activateFirst() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL activateLast() throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const ::com::sun::star::lang::EventObject& event) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL rowChanged(const ::com::sun::star::lang::EventObject& event) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL rowSetChanged(const ::com::sun::star::lang::EventObject& event) throw( ::com::sun::star::uno::RuntimeException );

// XRowSetApproveListener
    virtual sal_Bool SAL_CALL approveCursorMove(const ::com::sun::star::lang::EventObject& event) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL approveRowChange(const  ::com::sun::star::sdb::RowChangeEvent& event) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL approveRowSetChange(const ::com::sun::star::lang::EventObject& event) throw( ::com::sun::star::uno::RuntimeException );

// XRowSetApproveBroadcaster
    virtual void SAL_CALL addRowSetApproveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener>& listener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeRowSetApproveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener>& listener) throw( ::com::sun::star::uno::RuntimeException );

// XSQLErrorBroadcaster
    virtual void SAL_CALL errorOccured(const ::com::sun::star::sdb::SQLErrorEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

// XSQLErrorListener
    virtual void SAL_CALL addSQLErrorListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener>& _rListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeSQLErrorListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener>& _rListener) throw( ::com::sun::star::uno::RuntimeException );

// XDatabaseParameterBroadcaster
    virtual void SAL_CALL addParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& aListener) throw( ::com::sun::star::uno::RuntimeException );

// XDatabaseParameterListener
    virtual sal_Bool SAL_CALL approveParameter(const ::com::sun::star::form::DatabaseParameterEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

// XConfirmDeleteBroadcaster
    virtual void SAL_CALL addConfirmDeleteListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XConfirmDeleteListener>& aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeConfirmDeleteListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XConfirmDeleteListener>& aListener) throw( ::com::sun::star::uno::RuntimeException );

// XConfirmDeleteListener
    virtual sal_Bool SAL_CALL confirmDelete(const  ::com::sun::star::sdb::RowChangeEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

// XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString	SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >	SAL_CALL getSupportedServiceNames(void) throw(::com::sun::star::uno::RuntimeException);

// XResetListener
    virtual sal_Bool SAL_CALL approveReset(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL resetted(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// method for registration
    static  ::com::sun::star::uno::Sequence< ::rtl::OUString >	static_getSupportedServiceNames(void);

    // comphelper::OPropertyArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;

// access to the controls for filtering
    const FmFilterControls& getFilterControls() const {return m_aFilterControls;}

// access to the current filter rows
    const FmFilterRows& getFilterRows() const {return m_aFilters;}
    FmFilterRows& getFilterRows() {return m_aFilters;}

    // just decr. the positions no notifications for the view
    void decrementCurrentFilterPosition()
    {
        DBG_ASSERT(m_nCurrentFilterPosition, "Invalid Position");
        --m_nCurrentFilterPosition;
    }

    sal_Int32 getCurrentFilterPosition() const {return m_nCurrentFilterPosition;}

protected:
    // FmDispatchInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>
    interceptedQueryDispatch(sal_uInt16 _nId,const ::com::sun::star::util::URL& aURL,
                            const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags)
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual ::osl::Mutex* getInterceptorMutex() { return &m_aMutex; }

    void stopFiltering();
    void addToEventAttacher(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);
    void removeFromEventAttacher(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);
    void removeBoundFieldListener();

    void startFormListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm, sal_Bool _bPropertiesOnly  );
    void stopFormListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm, sal_Bool _bPropertiesOnly );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> findControl(::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> >& rCtrls, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& rxCtrlModel ,sal_Bool _bRemove = sal_True,sal_Bool _bOverWrite=sal_True) const;



    sal_Bool isLocked() const {return m_bLocked;}

        // returns m_pWindow or - if m_pWindow is NULL - the window of the currently set container

        // create a new interceptor, register it on the given object
        // if createInterceptor was called for the given object the according interceptor will be removed
        // from the objects interceptor chain and released

    // in filter mode we do not listen for changes
    sal_Bool isListeningForChanges() const {return m_bDBConnection && !m_bFiltering && !isLocked();}

    DECL_LINK( OnTimeout, void* );
    DECL_LINK( OnLoad, void* );
    DECL_LINK( OnToggleAutoFields, void* );
    DECL_LINK( OnUpdateDispatchers, void* );
};


}//end of namespace binfilter
#endif	// _SVX_FMCTRLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
