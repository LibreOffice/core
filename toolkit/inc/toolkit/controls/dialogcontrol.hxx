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

#ifndef TOOLKIT_DIALOG_CONTROL_HXX
#define TOOLKIT_DIALOG_CONTROL_HXX

#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include "toolkit/helper/servicenames.hxx"
#include "toolkit/helper/macros.hxx"
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase3.hxx>
#include <list>

//  ----------------------------------------------------
//  class UnoControlDialogModel
//  ----------------------------------------------------

class UnoControlDialogModel :   public ControlModelContainerBase
{
protected:
    ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();
public:
                        UnoControlDialogModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
                        UnoControlDialogModel( const UnoControlDialogModel& rModel );
                        ~UnoControlDialogModel();

    UnoControlModel*    Clone() const;
    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlDialogModel, ControlModelContainerBase, szServiceName2_UnoControlDialogModel )

};

typedef ::cppu::AggImplInheritanceHelper3   <   ControlContainerBase
                                            ,   ::com::sun::star::awt::XTopWindow
                                            ,   ::com::sun::star::awt::XDialog2
                                            ,   ::com::sun::star::awt::XWindowListener
                                            >   UnoDialogControl_Base;
class UnoDialogControl : public UnoDialogControl_Base
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >         mxMenuBar;
    TopWindowListenerMultiplexer                                                maTopWindowListeners;
    bool                                                                        mbWindowListener;

public:

                                UnoDialogControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
                                ~UnoDialogControl();
    ::rtl::OUString             GetComponentServiceName();

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL toFront(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL toBack(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& xMenu ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindowListener
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDialog2
    virtual void SAL_CALL endDialog( ::sal_Int32 Result ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHelpId( const rtl::OUString& Id ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDialog
    void SAL_CALL setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getTitle() throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL endExecute() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException);

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoDialogControl, szServiceName2_UnoControlDialog )

protected:
   // virtual void ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException);
    virtual void PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );
    virtual void ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException);
protected:
};

class UnoMultiPageModel : public ControlModelContainerBase
{
public:
    UnoMultiPageModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ~UnoMultiPageModel();
    UnoMultiPageModel( const UnoMultiPageModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageModel, ControlModelContainerBase, szServiceName_UnoMultiPageModel )

    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);
    // XNamedContainer
    void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException);
protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

};

class UnoMultiPageControl :  public ControlContainerBase
                            ,public ::com::sun::star::awt::XSimpleTabController
                            ,public ::com::sun::star::awt::XTabListener
{
    TabListenerMultiplexer maTabListeners;
    void bindPage( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
public:
    UnoMultiPageControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ~UnoMultiPageControl();
    ::rtl::OUString     GetComponentServiceName();

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageControl, ControlContainerBase, szServiceName_UnoMultiPageControl )
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return ControlContainerBase::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    // com::sun::star::awt::XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getActiveTabID() throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    // XTabListener
    virtual void SAL_CALL inserted( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removed( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changed( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL activated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deactivated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& evt ) throw (::com::sun::star::uno::RuntimeException);
    // XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual void    impl_createControlPeerIfNecessary(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl
    );

};


class UnoPageModel : public ControlModelContainerBase
{
public:
    UnoPageModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ~UnoPageModel();
    UnoPageModel( const UnoPageModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoPageModel, ControlModelContainerBase, szServiceName_UnoPageModel )

    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException);
protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

};

class UnoPageControl :  public ControlContainerBase
{
public:
    UnoPageControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ~UnoPageControl();
    ::rtl::OUString     GetComponentServiceName();


    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoPageControl, ControlContainerBase, szServiceName_UnoPageControl )
};

class UnoFrameModel : public ControlModelContainerBase
{
public:
    UnoFrameModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ~UnoFrameModel();
    UnoFrameModel( const UnoFrameModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoFrameModel, ControlModelContainerBase, szServiceName_UnoFrameModel )

    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);

protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();
};

class UnoFrameControl :  public ControlContainerBase
{
protected:
    virtual void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );
public:
    UnoFrameControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ~UnoFrameControl();
    ::rtl::OUString     GetComponentServiceName();

// ::com::sun::star::lang::XServiceInfo
DECLIMPL_SERVICEINFO_DERIVED( UnoFrameControl, ControlContainerBase, szServiceName_UnoPageControl )
};

#endif // TOOLKIT_DIALOG_CONTROL_HXX
