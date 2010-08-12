//  ----------------------------------------------------
//  class VCLXDialog
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
#ifndef _TOOLKIT_AWT_VCLXTABPAGECONTAINER_HXX_
#define _TOOLKIT_AWT_VCLXTABPAGECONTAINER_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include "toolkit/awt/vclxwindow.hxx"
//#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/awt/tab/XTabPageContainer.hpp>
#include <toolkit/helper/listenermultiplexer.hxx>

//  ----------------------------------------------------
class VCLXTabPageContainer : public ::com::sun::star::awt::tab::XTabPageContainer,
                         //public ::com::sun::star::awt::tab::XTabPageModel,
                         public VCLXWindow
{
public:
    VCLXTabPageContainer();
    ~VCLXTabPageContainer();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDevice,
    ::com::sun::star::awt::DeviceInfo SAL_CALL getInfo() throw(::com::sun::star::uno::RuntimeException);

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

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) { return ImplGetPropertyIds( aIds ); }

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
private:
    sal_Int16 m_nActiveTabPageId;
    TabPageListenerMultiplexer m_aTabPageListeners;
};
#endif // _TOOLKIT_AWT_VCLXTABPAGEMODEL_HXX_
