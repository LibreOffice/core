/*************************************************************************
 *
 *  $RCSfile: vclxmenu.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:07 $
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

#ifndef _TOOLKIT_AWT_VCLXMENU_HXX_
#define _TOOLKIT_AWT_VCLXMENU_HXX_


#ifndef _COM_SUN_STAR_AWT_XMENUBAR_HPP_
#include <com/sun/star/awt/XMenuBar.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPOPUPMENU_HPP_
#include <com/sun/star/awt/XPopupMenu.hpp>
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

DECLARE_LIST( PopupMenuRefList, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >* );

//  ----------------------------------------------------
//  class VCLXMenu
//  ----------------------------------------------------

class VCLXMenu :    public ::com::sun::star::awt::XMenuBar,
                    public ::com::sun::star::awt::XPopupMenu,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::cppu::OWeakObject
{
private:
    ::osl::Mutex            maMutex;
    Menu*                   mpMenu;
    BOOL                    mbPopup;
    MenuListenerMultiplexer maMenuListeners;
    PopupMenuRefList        maPopupMenueRefs;

protected:
    ::osl::Mutex&           GetMutex() { return maMutex; }

    DECL_LINK(      SelectHdl, Menu* );
    DECL_LINK(      HighlightHdl, Menu* );
    DECL_LINK(      ActivateHdl, Menu* );
    DECL_LINK(      DeactivateHdl, Menu* );

    void            ImplCreateMenu( BOOL bPopup );

public:
                    VCLXMenu();
                    ~VCLXMenu();


    Menu*           GetMenu() const { return mpMenu; }
    BOOL            IsPopupMenu() const { return mbPopup; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

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

};

//  ----------------------------------------------------
//  class VCLXMenuBar
//  ----------------------------------------------------
class VCLXMenuBar : public VCLXMenu
{
public:
        VCLXMenuBar() { ImplCreateMenu( FALSE ); }
};

//  ----------------------------------------------------
//  class VCLXPopupMenu
//  ----------------------------------------------------
class VCLXPopupMenu : public VCLXMenu
{
public:
        VCLXPopupMenu() { ImplCreateMenu( TRUE ); }
};




#endif // _TOOLKIT_AWT_VCLXMENU_HXX_

