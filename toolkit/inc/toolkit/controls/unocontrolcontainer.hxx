/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unocontrolcontainer.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:28:12 $
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
#ifndef _COM_SUN_STAR_CONTAINER_XIDENTIFIERCONTAINER_HPP_
#include <com/sun/star/container/XIdentifierContainer.hpp>
#endif

#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>

#include <cppuhelper/implbase4.hxx>

class UnoControlHolderList;

//  ----------------------------------------------------
//  class UnoControlContainer
//  ----------------------------------------------------
typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   ::com::sun::star::awt::XUnoControlContainer
                                            ,   ::com::sun::star::awt::XControlContainer
                                            ,   ::com::sun::star::container::XContainer
                                            ,   ::com::sun::star::container::XIdentifierContainer
                                            >   UnoControlContainer_Base;

class UnoControlContainer : public UnoControlContainer_Base
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


    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XIdentifierContainer
    virtual ::sal_Int32 SAL_CALL insert( const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XIdentifierReplace
    virtual void SAL_CALL removeByIdentifier( ::sal_Int32 Identifier ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL replaceByIdentifer( ::sal_Int32 Identifier, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XIdentifierAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByIdentifier( ::sal_Int32 Identifierr ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL getIdentifiers(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

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

private:
    /** adds the control to the container, does necessary notifications, and the like
        @param _rxControl
            the control to add. Must not be <NULL/>
        @param _pName
            Pointer to a name for the control. Might be <NULL/>, in this case an auotmatic name is generated
        @return
            the ID of the newly added control
    */
    sal_Int32 impl_addControl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl,
        const ::rtl::OUString* _pName = NULL
    );

    /** removes the given control from the container, including necessary notifications and the like
        @param  _nId
            the ID of the control to remove
        @param  _rxControl
            the control itself. Must be the one which is stored under the given ID. This parameter could also be
            obtained inside the method, but callers usually have obtained it, anyway.
        @param  _pNameAccessor
            the name which the control was registered for. Might be <NULL/>, in this case
            container event broadcasts use the ID as accessor.
    */
    void      impl_removeControl(
        sal_Int32 _nId,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl,
        const ::rtl::OUString* _pNameAccessor
    );

    /** ensures that the given control has a peer, if necessary and possible
        @param _rxControl
            an ->XControl which has just been inserted into the container. Must not be <NULL/>.
        @precond
            our mutex is locked
    */
    void    impl_createControlPeerIfNecessary(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl
    );
};



#endif // _TOOLKIT_CONTROLS_UNOCONTROLCONTAINER_HXX_

