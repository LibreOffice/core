/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unocontrolcontainer.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:52:15 $
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

#ifndef _TOOLKIT_CONTROLS_UNOCONTROLCONTAINER_HXX_
#define _TOOLKIT_CONTROLS_UNOCONTROLCONTAINER_HXX_


#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XUNOCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>


class UnoControlHolderList;

//  ----------------------------------------------------
//  class UnoControlContainer
//  ----------------------------------------------------
class UnoControlContainer : public ::com::sun::star::awt::XUnoControlContainer,
                            public ::com::sun::star::awt::XControlContainer,
                            public ::com::sun::star::container::XContainer,
                            public UnoControlBase
{
private:
    UnoControlHolderList*                   mpControls;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController > >    maTabControllers;
    ContainerListenerMultiplexer            maCListeners;

protected:
    void                                    ImplActivateTabControllers();

public:
                UnoControlContainer();
                UnoControlContainer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xPeer );
                ~UnoControlContainer();


    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControl::queryInterface(rType); }
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControlContainer
    void SAL_CALL setStatusText( const ::rtl::OUString& StatusText ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > SAL_CALL getControls(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > SAL_CALL getControl( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addControl( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XUnoControlContainer
    void SAL_CALL setTabControllers( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController > >& TabControllers ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController > > SAL_CALL getTabControllers(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addTabController( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >& TabController ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTabController( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >& TabController ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindow
    void SAL_CALL setVisible( sal_Bool Visible ) throw(::com::sun::star::uno::RuntimeException);

    DECLIMPL_SERVICEINFO_DERIVED( UnoControlContainer, UnoControlBase, szServiceName2_UnoControlContainer )


protected:
    virtual void removingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
    virtual void addingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
};



#endif // _TOOLKIT_CONTROLS_UNOCONTROLCONTAINER_HXX_

