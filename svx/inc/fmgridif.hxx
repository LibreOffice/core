/*************************************************************************
 *
 *  $RCSfile: fmgridif.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-20 14:12:24 $
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
#ifndef _SVX_FMGRIDIF_HXX
#define _SVX_FMGRIDIF_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XROWSETSUPPLIER_HPP_
#include <com/sun/star/sdb/XRowSetSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCOMPONENT_HPP_
#include <com/sun/star/form/XBoundComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADLISTENER_HPP_
#include <com/sun/star/form/XLoadListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRID_HPP_
#include <com/sun/star/form/XGrid.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDPEER_HPP_
#include <com/sun/star/form/XGridPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDFIELDDATASUPPLIER_HPP_
#include <com/sun/star/form/XGridFieldDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
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

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#include <toolkit/controls/unocontrol.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

class DbGridColumn;

class OWeakSubObject : public ::cppu::OWeakObject
{
protected:
    ::cppu::OWeakObject&    m_rParent;

public:
    OWeakSubObject(::cppu::OWeakObject& rParent) : m_rParent(rParent) { }

    virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException) { m_rParent.acquire(); }
    virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException) { m_rParent.release(); }
};

//==================================================================
// FmXModifyMultiplexer
//==================================================================
class FmXModifyMultiplexer  :public OWeakSubObject
                            ,public ::cppu::OInterfaceContainerHelper
                            ,public ::com::sun::star::util::XModifyListener
{
public:
    FmXModifyMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXModifyMultiplexer,OWeakSubObject);
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& Source);

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};

//==================================================================
// FmXUpdateMultiplexer
//==================================================================
class FmXUpdateMultiplexer : public OWeakSubObject,
                             public ::cppu::OInterfaceContainerHelper,
                             public ::com::sun::star::form::XUpdateListener
{
public:
    FmXUpdateMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXUpdateMultiplexer,OWeakSubObject);

    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XUpdateListener
    virtual sal_Bool SAL_CALL approveUpdate(const ::com::sun::star::lang::EventObject &);
    virtual void SAL_CALL updated(const ::com::sun::star::lang::EventObject &);

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};

//==================================================================
// FmXContainerMultiplexer
//==================================================================
class FmXContainerMultiplexer : public OWeakSubObject,
                                public ::cppu::OInterfaceContainerHelper,
                                public ::com::sun::star::container::XContainerListener
{
public:
    FmXContainerMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
    DECLARE_UNO3_DEFAULTS(FmXContainerMultiplexer,OWeakSubObject);
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event);
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event);
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event);

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};

//==================================================================
// FmXGridControl
//==================================================================
class FmXGridPeer;
class FmXGridControl    :public UnoControl
                        ,public ::com::sun::star::form::XBoundComponent
                        ,public ::com::sun::star::form::XGrid
                        ,public ::com::sun::star::util::XModifyBroadcaster
                        ,public ::com::sun::star::form::XGridFieldDataSupplier
                        ,public ::com::sun::star::container::XIndexAccess
                        ,public ::com::sun::star::container::XEnumerationAccess
                        ,public ::com::sun::star::util::XModeSelector
                        ,public ::com::sun::star::container::XContainer
                        ,public ::com::sun::star::frame::XDispatchProvider
                        ,public ::com::sun::star::frame::XDispatchProviderInterception
{
    FmXModifyMultiplexer    m_aModifyListeners;
    FmXUpdateMultiplexer    m_aUpdateListeners;
    FmXContainerMultiplexer m_aContainerListeners;

protected:
    sal_uInt16  m_nPeerCreationLevel;
    sal_Bool    m_bInDraw;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;

public:
    FmXGridControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
    virtual ~FmXGridControl();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(FmXGridControl, UnoControl);
    virtual ::com::sun::star::uno::Any  SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw();
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw();
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::awt::XControl
    virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model);
    virtual void SAL_CALL setDesignMode(sal_Bool bOn);

// ::com::sun::star::awt::XView
    virtual void SAL_CALL draw( long x, long y );

// ::com::sun::star::form::XBoundComponent
    virtual void SAL_CALL addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l);
    virtual void SAL_CALL removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l);
    virtual sal_Bool SAL_CALL commit();

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XGrid
    virtual sal_Int16 SAL_CALL getCurrentColumnPosition();
    virtual void SAL_CALL setCurrentColumnPosition(sal_Int16 nPos);

// UnoControl
    virtual ::rtl::OUString GetComponentServiceName();

// ::com::sun::star::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l);
    virtual void SAL_CALL removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l);

// ::com::sun::star::form::XGridFieldDataSupplier
    virtual ::com::sun::star::uno::Sequence< sal_Bool > SAL_CALL queryFieldDataType( const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL queryFieldData( sal_Int32 nRow, const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XModeSelector
    virtual void SAL_CALL setMode(const ::rtl::OUString& Mode) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getMode() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedModes() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsMode(const ::rtl::OUString& Mode) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::frame::XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  > SAL_CALL queryDispatches(const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::frame::XDispatchProviderInterception
    virtual void SAL_CALL registerDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& xInterceptor) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL releaseDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& xInterceptor) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual FmXGridPeer*    imp_CreatePeer(Window* pParent);
        // ImplCreatePeer waere besser ;) geht aber nicht, da dann nicht exportiert

};

//==================================================================
// FmXGridPeer -> Peer fuers Gridcontrol
//==================================================================
class FmGridControl;
class FmXGridPeer   :public VCLXWindow
                    ,public ::com::sun::star::form::XGridPeer
                    ,public ::com::sun::star::form::XBoundComponent
                    ,public ::com::sun::star::form::XGrid
                    ,public ::com::sun::star::sdb::XRowSetSupplier
                    ,public ::com::sun::star::util::XModifyBroadcaster
                    ,public ::com::sun::star::beans::XPropertyChangeListener
                    ,public ::com::sun::star::container::XContainerListener
                    ,public ::com::sun::star::sdbc::XRowSetListener
                    ,public ::com::sun::star::form::XLoadListener
                    ,public ::com::sun::star::view::XSelectionChangeListener
                    ,public ::com::sun::star::form::XGridFieldDataSupplier
                    ,public ::com::sun::star::container::XIndexAccess
                    ,public ::com::sun::star::container::XEnumerationAccess
                    ,public ::com::sun::star::util::XModeSelector
                    ,public ::com::sun::star::container::XContainer
                    ,public ::com::sun::star::frame::XStatusListener
                    ,public ::com::sun::star::frame::XDispatchProvider
                    ,public ::com::sun::star::frame::XDispatchProviderInterception
                    ,public ::com::sun::star::form::XResetListener
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >    m_xColumns;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >                 m_xCursor;
    ::cppu::OInterfaceContainerHelper       m_aModifyListeners,
                                            m_aUpdateListeners,
                                            m_aContainerListeners;
    ::rtl::OUString         m_aMode;
    sal_Int32               m_nCursorListening;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >   m_xFirstDispatchInterceptor;

    sal_Bool                                m_bInterceptingDispatch;

    sal_Bool*                               m_pStateCache;
        // one bool for each supported url
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > *                        m_pDispatchers;
        // one dispatcher for each supported url
        // (I would like to have a vector here but including the stl in an exported file seems
        // very risky to me ....)

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;
    ::osl::Mutex                                                                        m_aMutex;

public:
    FmXGridPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
    ~FmXGridPeer();

    // spaeter Constructor, immer nach dem realen Constructor zu rufen !
    void Create(Window* pParent, WinBits nStyle);

// UNO Anbindung
    DECLARE_UNO3_DEFAULTS(FmXGridPeer, VCLXWindow);
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   getUnoTunnelImplementationId() throw();
    static FmXGridPeer*                                         getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XGridPeer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumns( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& aColumns ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XBoundComponent
    virtual void SAL_CALL addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l);
    virtual void SAL_CALL removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l);
    virtual sal_Bool SAL_CALL commit();

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

// ::com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rowChanged(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rowSetChanged(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event);
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event);
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event);

// VCLXWindow
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XGrid
    virtual sal_Int16 SAL_CALL getCurrentColumnPosition();
    virtual void SAL_CALL setCurrentColumnPosition(sal_Int16 nPos);

// ::com::sun::star::sdb::XRowSetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  SAL_CALL getRowSet() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowSet(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xDataSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l);
    virtual void SAL_CALL removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l);

// UnoControl
    virtual void SAL_CALL SAL_CALL setDesignMode(sal_Bool bOn);
    virtual sal_Bool SAL_CALL isDesignMode();

// ::com::sun::star::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged(const ::com::sun::star::lang::EventObject& aEvent);

    void CellModified();

// PropertyListening
    void updateGrid(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rDatabaseCursor);
    void startCursorListening();
    void stopCursorListening();

// ::com::sun::star::form::XGridFieldDataSupplier
    virtual ::com::sun::star::uno::Sequence< sal_Bool > SAL_CALL queryFieldDataType( const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL queryFieldData( sal_Int32 nRow, const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XModeSelector
    virtual void SAL_CALL setMode(const ::rtl::OUString& Mode) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getMode() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedModes() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsMode(const ::rtl::OUString& Mode) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw(::com::sun::star::uno::RuntimeException);

    void columnVisible(DbGridColumn* pColumn);
    void columnHidden(DbGridColumn* pColumn);

// ::com::sun::star::awt::XView
    virtual void SAL_CALL draw( long x, long y );

// ::com::sun::star::frame::XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  > SAL_CALL queryDispatches(const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::frame::XDispatchProviderInterception
    virtual void SAL_CALL registerDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& xInterceptor) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL releaseDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& xInterceptor) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::frame::XStatusListener
    virtual void SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XResetListener
    virtual sal_Bool SAL_CALL approveReset(const ::com::sun::star::lang::EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetted(const ::com::sun::star::lang::EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual FmGridControl*  imp_CreateControl(Window* pParent, WinBits nStyle);

    static ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>&       getSupportedURLs();
    static ::com::sun::star::uno::Sequence<sal_uInt16>& getSupportedGridSlots();
    void    ConnectToDispatcher();
    void    DisConnectFromDispatcher();
    void    UpdateDispatches(); // will connect if not already connected and just update else

    /** If a derived class wants to listen at some column properties, it doesn't have
        to overload all methods affecting columns (setColumns, elementInserted, elementRemoved ...)
        Instead it may use addColumnListeners and removeColumnListeners which are called in all
        the cases.
    */
    virtual void addColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xCol);
    virtual void removeColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xCol);

    DECL_LINK(OnQueryGridSlotState, void*);
    DECL_LINK(OnExecuteGridSlot, void*);
};



#endif // _SVX_FMGRID_HXX

