/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxmenu.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-30 09:55:56 $
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

#ifndef _TOOLKIT_AWT_VCLXMENU_HXX_
#define _TOOLKIT_AWT_VCLXMENU_HXX_

#ifndef TOOLKIT_DLLAPI_H
#include <toolkit/dllapi.h>
#endif

#ifndef _COM_SUN_STAR_AWT_XMENUBAR_HPP_
#include <com/sun/star/awt/XMenuBar.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPOPUPMENU_HPP_
#include <com/sun/star/awt/XPopupMenu.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMENUEXTENDED_HPP_
#include <com/sun/star/awt/XMenuExtended.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <tools/list.hxx>
#include <tools/link.hxx>

#include <toolkit/helper/listenermultiplexer.hxx>

class Menu;
class MenuBar;
class VclSimpleEvent;

DECLARE_LIST( PopupMenuRefList, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >* );

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
