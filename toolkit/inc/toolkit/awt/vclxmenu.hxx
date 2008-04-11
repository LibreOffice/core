/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxmenu.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _TOOLKIT_AWT_VCLXMENU_HXX_
#define _TOOLKIT_AWT_VCLXMENU_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/awt/XMenuExtended.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <tools/list.hxx>
#include <tools/link.hxx>

#include <toolkit/helper/listenermultiplexer.hxx>

class Menu;
class MenuBar;
class VclSimpleEvent;

DECLARE_LIST( PopupMenuRefList, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >* )

//  ----------------------------------------------------
//  class VCLXMenu
//  ----------------------------------------------------

class TOOLKIT_DLLPUBLIC VCLXMenu :  public ::com::sun::star::awt::XMenuBar,
                    public ::com::sun::star::awt::XPopupMenu,
                    public ::com::sun::star::awt::XMenuExtended,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::cppu::OWeakObject
{
private:
    ::osl::Mutex            maMutex;
    Menu*                   mpMenu;
    MenuListenerMultiplexer maMenuListeners;
    PopupMenuRefList        maPopupMenueRefs;

protected:
    ::osl::Mutex&           GetMutex() { return maMutex; }

    DECL_LINK(      MenuEventListener, VclSimpleEvent* );

    void            ImplCreateMenu( BOOL bPopup );

public:
                    VCLXMenu();
                    VCLXMenu( Menu* pMenu );
                    ~VCLXMenu();


    Menu*           GetMenu() const { return mpMenu; }
    BOOL            IsPopupMenu() const;

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXMenu*                                            GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XMenu
    void SAL_CALL addMenuListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeMenuListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL insertItem( sal_Int16 nItemId, const ::rtl::OUString& aText, sal_Int16 nItemStyle, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeItem( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getItemCount(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getItemId( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getItemPos( sal_Int16 nId ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL enableItem( sal_Int16 nItemId, sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isItemEnabled( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setItemText( sal_Int16 nItemId, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getItemText( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPopupMenu( sal_Int16 nItemId, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& aPopupMenu ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > SAL_CALL getPopupMenu( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XPopupMenu
    void SAL_CALL insertSeparator( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDefaultItem( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getDefaultItem(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL checkItem( sal_Int16 nItemId, sal_Bool bCheck ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isItemChecked( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent, const ::com::sun::star::awt::Rectangle& Area, sal_Int16 Direction ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XMenuBar

    // ::com::sun::star::awt::XMenuExtended
    virtual void SAL_CALL setCommand( sal_Int16 nItemId, const ::rtl::OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCommand( sal_Int16 nItemId ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHelpCommand( sal_Int16 nItemId, const ::rtl::OUString& aHelp ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getHelpCommand( sal_Int16 nItemId ) throw (::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXMenuBar
//  ----------------------------------------------------
class TOOLKIT_DLLPUBLIC VCLXMenuBar : public VCLXMenu
{
public:
        VCLXMenuBar();
        VCLXMenuBar( MenuBar* pMenuBar );
};

//  ----------------------------------------------------
//  class VCLXPopupMenu
//  ----------------------------------------------------
class TOOLKIT_DLLPUBLIC VCLXPopupMenu : public VCLXMenu
{
public:
        VCLXPopupMenu();
};

#endif // _TOOLKIT_AWT_VCLXMENU_HXX_
