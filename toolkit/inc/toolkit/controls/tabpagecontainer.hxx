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

#ifndef TOOLKIT_TABPAGE_CONTAINER_HXX
#define TOOLKIT_TABPAGE_CONTAINER_HXX

#include <com/sun/star/awt/tab/XTabPageContainer.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerModel.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerListener.hpp>
#include <com/sun/star/awt/tab/XTabPage.hpp>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//  ------------------------------------------------------------------
//  class ::com::sun::star::awt::tab::UnoControlTabPageContainerModel
//  ------------------------------------------------------------------
class UnoControlTabPageContainerModel : public com::sun::star::awt::tab::XTabPageContainerModel
                                        ,public UnoControlModel
{
private:
    std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > > m_aTabPageVector;
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   getPropertySetInfo() const;
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

public:
    UnoControlTabPageContainerModel();
    UnoControlTabPageContainerModel( const UnoControlTabPageContainerModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlTabPageContainerModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainerModel, UnoControlModel, szServiceName_UnoControlTabPageContainerModel )
    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
             throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( sal_Int32 Index )
             throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

         // XIndexReplace
     virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
             throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

         // XIndexAccess
     virtual sal_Int32 SAL_CALL getCount()
             throw (::com::sun::star::uno::RuntimeException);

     virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
             throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

         // XElementAccess
     virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
             throw (::com::sun::star::uno::RuntimeException);
     //{
         //return ::getCppuType((com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >*)0);
    //}

    virtual sal_Bool SAL_CALL hasElements()
             throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
};
// ===================================================================
// = UnoControlTabPageContainer
// ===================================================================
class UnoControlTabPageContainer : public ControlContainerBase,
                                public ::com::sun::star::awt::tab::XTabPageContainer
{
public:
    UnoControlTabPageContainer();
    ::rtl::OUString             GetComponentServiceName();

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) ;//{ return UnoControlContainer::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
    // ::com::sun::star::awt::grid::XTabPageContainer
    virtual ::sal_Int16 SAL_CALL getActiveTabPageID() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTabPageCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isTabPageActive( ::sal_Int16 tabPageIndex ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL getTabPage( ::sal_Int16 tabPageIndex ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL getTabPageByID( ::sal_Int16 tabPageID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addTabPageListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTabPageListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException);

     // ::com::sun::star::awt::XControlContainer
    virtual void SAL_CALL setStatusText( const ::rtl::OUString& StatusText ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > SAL_CALL getControls(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > SAL_CALL getControl( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addControl( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw (::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainer, UnoControlBase, szServiceName_UnoControlTabPageContainer )

    using UnoControl::getPeer;
private:
    TabPageListenerMultiplexer  m_aTabPageListeners;
    sal_Int16 m_nActiveTabPageId;
};

#endif // _TOOLKIT_TABPAGE_CONTAINER_HXX
