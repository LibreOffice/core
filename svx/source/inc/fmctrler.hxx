/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmctrler.hxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:24:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_FMCTRLER_HXX
#define _SVX_FMCTRLER_HXX

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLERRORBROADCASTER_HPP_
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLERRORLISTENER_HPP_
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XROWSETAPPROVEBROADCASTER_HPP_
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XROWSETAPPROVELISTENER_HPP_
#include <com/sun/star/sdb/XRowSetApproveListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESETLISTENER_HPP_
#include <com/sun/star/form/XResetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_CONTAINEREVENT_HPP_
#include <com/sun/star/container/ContainerEvent.hpp>
#endif
#include <com/sun/star/container/XEnumerationAccess.hpp>
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCONFIRMDELETEBROADCASTER_HPP_
#include <com/sun/star/form/XConfirmDeleteBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_DATABASEPARAMETEREVENT_HPP_
#include <com/sun/star/form/DatabaseParameterEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_DATABASEDELETEEVENT_HPP_
#include <com/sun/star/form/DatabaseDeleteEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XDATABASEPARAMETERLISTENER_HPP_
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADLISTENER_HPP_
#include <com/sun/star/form/XLoadListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCONFIRMDELETELISTENER_HPP_
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_ERROREVENT_HPP_
#include <com/sun/star/form/ErrorEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XDATABASEPARAMETERBROADCASTER2_HPP_
#include <com/sun/star/form/XDatabaseParameterBroadcaster2.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLERLISTENER_HPP_
#include <com/sun/star/form/XFormControllerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODESELECTOR_HPP_
#include <com/sun/star/util/XModeSelector.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLISTENER_HPP_
#include <com/sun/star/awt/XTextListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XITEMLISTENER_HPP_
#include <com/sun/star/awt/XItemListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLER_HPP_
#include <com/sun/star/awt/XTabController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_VALIDATION_XFORMCOMPONENTVALIDITYLISTENER_HPP_
#include <com/sun/star/form/validation/XFormComponentValidityListener.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef SVX_SQLPARSERCLIENT_HXX
#include "sqlparserclient.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE12_HXX_
#include <cppuhelper/implbase12.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE12_HXX_
#include <cppuhelper/compbase12.hxx>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef SVX_FORMCONTROLLING_HXX
#include "formcontrolling.hxx"
#endif

struct FmXTextComponentLess : public ::std::binary_function< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent> , sal_Bool>
{
    sal_Bool operator() (const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >& x, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >& y) const
    {
        return reinterpret_cast<sal_Int64>(x.get()) < reinterpret_cast<sal_Int64>(y.get());
    }
};

typedef ::std::map< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >, FmXTextComponentLess> FmFilterControls;
typedef ::std::map< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >, ::rtl::OUString, FmXTextComponentLess> FmFilterRow;
typedef ::std::vector< FmFilterRow > FmFilterRows;
typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > > FmFormControllers;

struct FmFieldInfo;
class FmFormView;
class Window;

namespace svxform
{
    class ControlBorderManager;
}

typedef ::cppu::WeakAggComponentImplHelper12<   ::com::sun::star::form::XFormController
                                            ,   ::com::sun::star::container::XChild
                                            ,   ::com::sun::star::container::XIndexAccess
                                            ,   ::com::sun::star::container::XEnumerationAccess
                                            ,   ::com::sun::star::awt::XFocusListener
                                            ,   ::com::sun::star::form::XLoadListener
                                            ,   ::com::sun::star::beans::XPropertyChangeListener
                                            ,   ::com::sun::star::awt::XTextListener
                                            ,   ::com::sun::star::awt::XItemListener
                                            ,   ::com::sun::star::container::XContainerListener
                                            ,   ::com::sun::star::util::XModifyListener
                                            ,   ::com::sun::star::util::XModifyBroadcaster
                                            >   FmXFormController_BASE1;

typedef ::cppu::ImplHelper12<   ::com::sun::star::util::XModeSelector
                            ,   ::com::sun::star::form::XConfirmDeleteListener
                            ,   ::com::sun::star::form::XConfirmDeleteBroadcaster
                            ,   ::com::sun::star::sdb::XSQLErrorListener
                            ,   ::com::sun::star::sdb::XSQLErrorBroadcaster
                            ,   ::com::sun::star::sdbc::XRowSetListener
                            ,   ::com::sun::star::sdb::XRowSetApproveListener
                            ,   ::com::sun::star::sdb::XRowSetApproveBroadcaster
                            ,   ::com::sun::star::form::XDatabaseParameterListener
                            ,   ::com::sun::star::form::XDatabaseParameterBroadcaster
                            ,   ::com::sun::star::lang::XServiceInfo
                            ,   ::com::sun::star::form::XResetListener
                            >   FmXFormController_BASE2;

typedef ::cppu::ImplHelper6<    ::com::sun::star::lang::XUnoTunnel
                           ,    ::com::sun::star::frame::XDispatch
                           ,    ::com::sun::star::awt::XMouseListener
                           ,    ::com::sun::star::form::validation::XFormComponentValidityListener
                           ,    ::com::sun::star::task::XInteractionHandler
                           ,    ::com::sun::star::lang::XInitialization
                           >    FmXFormController_BASE3;

//==================================================================
// FmXFormController
//==================================================================
class SAL_DLLPRIVATE FmXFormController  :public ::comphelper::OBaseMutex
                                        ,public FmXFormController_BASE1
                                        ,public FmXFormController_BASE2
                                        ,public FmXFormController_BASE3
                                        ,public ::cppu::OPropertySetHelper
                                        ,public FmDispatchInterceptor
                                        ,public ::comphelper::OAggregationArrayUsageHelper< FmXFormController >
                                        ,public ::svxform::OSQLParserClient
                                        ,public ::svx::IControllerFeatureInvalidation
{
    typedef ::std::map  <   sal_Int32,
                            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
                        >   DispatcherContainer;

    friend class FmXPageViewWinRec;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation>              m_xAggregate;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController>            m_xTabController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>                  m_xActiveControl, m_xCurrentControl;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>        m_xModelAsIndex;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager>  m_xModelAsManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>                m_xParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
    // Composer used for checking filter conditions
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >   m_xComposer;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >         m_xInteractionHandler;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> >   m_aControls;
    ::cppu::OInterfaceContainerHelper
                                m_aActivateListeners,
                                m_aModifyListeners,
                                m_aErrorListeners,
                                m_aDeleteListeners,
                                m_aRowSetApproveListeners,
                                m_aParameterListeners;

    FmFormControllers           m_aChilds;
    FmFilterControls            m_aFilterControls;
    FmFilterRows                m_aFilters;

    Timer                       m_aTabActivationTimer;
    Timer                       m_aFeatureInvalidationTimer;

    FmFormView*                 m_pView;
    Window*                     m_pWindow;
    ::svxform::ControlBorderManager*
                                m_pControlBorderManager;

    ::svx::ControllerFeatures   m_aControllerFeatures;
    DispatcherContainer         m_aFeatureDispatchers;
    ::std::set< sal_Int32 >     m_aInvalidFeatures;     // for asynchronous feature invalidation

    ::rtl::OUString             m_aMode;

    ULONG                       m_nLoadEvent;
    ULONG                       m_nToggleEvent;

    sal_Int32                   m_nCurrentFilterPosition;   // current level for filtering (or-criteria)

    sal_Bool                    m_bCurrentRecordModified : 1;
    sal_Bool                    m_bCurrentRecordNew : 1;
    sal_Bool                    m_bLocked           : 1;
    sal_Bool                    m_bDBConnection  : 1;   // Focuslistener nur fuer Datenbankformulare
    sal_Bool                    m_bCycle             : 1;
    sal_Bool                    m_bCanInsert         : 1;
    sal_Bool                    m_bCanUpdate         : 1;
    sal_Bool                    m_bCommitLock    : 1;   // lock the committing of controls see focusGained
    sal_Bool                    m_bModified      : 1;   // ist der Inhalt eines Controls modifiziert ?
    sal_Bool                    m_bControlsSorted : 1;
    sal_Bool                    m_bFiltering : 1;
    sal_Bool                    m_bAttachEvents : 1;
    sal_Bool                    m_bDetachEvents : 1;
    sal_Bool                    m_bAttemptedHandlerCreation : 1;

    // as we want to intercept dispatches of _all_ controls we're responsible for, and an object implementing
    // the ::com::sun::star::frame::XDispatchProviderInterceptor interface can intercept only _one_ objects dispatches, we need a helper class
    DECLARE_STL_VECTOR(FmXDispatchInterceptorImpl*, Interceptors);
    Interceptors    m_aControlDispatchInterceptors;

public:
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >&
        getInteractionHandler() const
    {
        const_cast< FmXFormController* >( this )->ensureInteractionHandler();
        return m_xInteractionHandler;
    }

public:
    FmXFormController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & _rxORB,
                      FmFormView* _pView = NULL, Window* _pWindow = NULL );
    ~FmXFormController();

    // UNO Anbindung
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
            { return FmXFormController_BASE1::queryInterface( type ); }
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    static FmXFormController* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent );

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

    using OPropertySetHelper::getFastPropertyValue;

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

// XMouseListener
    virtual void SAL_CALL mousePressed( const ::com::sun::star::awt::MouseEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseReleased( const ::com::sun::star::awt::MouseEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseEntered( const ::com::sun::star::awt::MouseEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseExited( const ::com::sun::star::awt::MouseEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

// XFormComponentValidityListener
    virtual void SAL_CALL componentValidityChanged( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

// XInteractionHandler
    virtual void SAL_CALL handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& Request ) throw (::com::sun::star::uno::RuntimeException);

// XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener -> aenderung der stati
    virtual void SAL_CALL propertyChange(const  ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException );

// XTextListener           -> modify setzen
    virtual void SAL_CALL textChanged(const  ::com::sun::star::awt::TextEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// XItemListener            -> modify setzen
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

// com::sun::star::sdbc::XRowSetListener
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

// XDatabaseParameterBroadcaster2
    virtual void SAL_CALL addDatabaseParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeDatabaseParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& aListener) throw( ::com::sun::star::uno::RuntimeException );

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
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >  SAL_CALL getSupportedServiceNames(void) throw(::com::sun::star::uno::RuntimeException);

// XResetListener
    virtual sal_Bool SAL_CALL approveReset(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL resetted(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// method for registration
    static  ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static(void);

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

    void setCurrentFilterPosition(sal_Int32 nPos);
    sal_Int32 getCurrentFilterPosition() const {return m_nCurrentFilterPosition;}

protected:
    // FmDispatchInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>
    interceptedQueryDispatch(sal_uInt16 _nId,const ::com::sun::star::util::URL& aURL,
                            const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags)
                            throw( ::com::sun::star::uno::RuntimeException );

    // IControllerFeatureInvalidation
    virtual void invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures );

    virtual ::osl::Mutex* getInterceptorMutex() { return &m_aMutex; }

    /// update all our dispatchers
    void    updateAllDispatchers() const;

    /** disposes all dispatchers in m_aFeatureDispatchers, empties m_aFeatureDispatchers,
        and disposes m_aControllerFeatures
    */
    void    disposeAllFeaturesAndDispatchers() SAL_THROW(());

    void startFiltering();
    void stopFiltering();
    void setFilter(::std::vector<FmFieldInfo>&);
    void startListening();
    void stopListening();

    /** ensures that we have an interaction handler, if possible

        If an interaction handler was provided at creation time (<member>initialize</member>), this
        one will be used. Else, an attempt is made to create an <type scope="com::sun::star::sdb">InteractionHandler</type>
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
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxExistentControl,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxNewControl
    );

    // we're listening at all bound controls for modifications
    void startControlModifyListening(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);
    void stopControlModifyListening(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);

    void setLocks();
    void setControlLock(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);
    void addToEventAttacher(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);
    void removeFromEventAttacher(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);
    void toggleAutoFields(sal_Bool bAutoFields);
    void unload() throw( ::com::sun::star::uno::RuntimeException );
    void removeBoundFieldListener();

    void startFormListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm, sal_Bool _bPropertiesOnly  );
    void stopFormListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm, sal_Bool _bPropertiesOnly );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> findControl( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> >& rCtrls, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& rxCtrlModel, sal_Bool _bRemove, sal_Bool _bOverWrite ) const;

    void insertControl(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);
    void removeControl(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl);

    /// called when a new control is to be handled by the controller
    void implControlInserted( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& _rxControl, bool _bAddToEventAttacher );
    /// called when a control is not to be handled by the controller anymore
    void implControlRemoved( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& _rxControl, bool _bRemoveFromEventAttacher );

    void onModify( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl );
    void onActivate();

    sal_Bool isLocked() const {return m_bLocked;}
    sal_Bool determineLockState() const;

    Window* getDialogParentWindow();
        // returns m_pWindow or - if m_pWindow is NULL - the window of the currently set container

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor>    createInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>& _xInterception);
        // create a new interceptor, register it on the given object
    void                            deleteInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>& _xInterception);
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
                ::rtl::OUString& /* [out] */ _rFirstInvalidityExplanation,
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& /* [out] */ _rxFirstInvalidModel
            ) SAL_THROW(());

    /** locates the control which belongs to a given model
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
            locateControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel ) SAL_THROW(());

    // in filter mode we do not listen for changes
    sal_Bool isListeningForChanges() const {return m_bDBConnection && !m_bFiltering && !isLocked();}
    void addChild(FmXFormController* pChild);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> isInList(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>& xPeer) const;

    DECL_LINK( OnActivateTabOrder, void* );
    DECL_LINK( OnInvalidateFeatures, void* );
    DECL_LINK( OnLoad, void* );
    DECL_LINK( OnToggleAutoFields, void* );
};


#endif  // _SVX_FMCTRLER_HXX

