/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _TOOLKIT_AWT_VCLXMENU_HXX_
#define _TOOLKIT_AWT_VCLXMENU_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XMenuBarExtended.hpp>
#include <com/sun/star/awt/XPopupMenuExtended.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <tools/link.hxx>

#include <toolkit/helper/listenermultiplexer.hxx>
#include <vector>

class Menu;
class MenuBar;
class VclSimpleEvent;
class PopupMenu;

typedef ::std::vector<
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >*
> PopupMenuRefList;

//  ----------------------------------------------------
//  class VCLXMenu
//  ----------------------------------------------------

class TOOLKIT_DLLPUBLIC VCLXMenu :  public ::com::sun::star::awt::XMenuBarExtended,
                                    public ::com::sun::star::awt::XPopupMenuExtended,
                                    public ::com::sun::star::lang::XServiceInfo,
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

    void            ImplCreateMenu( sal_Bool bPopup );

public:
                    VCLXMenu();
                    VCLXMenu( Menu* pMenu );
                    ~VCLXMenu();


    Menu*           GetMenu() const { return mpMenu; }
    sal_Bool            IsPopupMenu() const;

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

    // ========================================================================
    // ========================================================================
    // ========================================================================

    // XMenuExtended2 Methods
    virtual ::sal_Bool SAL_CALL isPopupMenu(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clear(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::MenuItemType SAL_CALL getItemType( ::sal_Int16 nItemPos ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL hideDisabledEntries( ::sal_Bool bHide ) throw (::com::sun::star::uno::RuntimeException);

    // XMenuBarExtended Methods

    // XPopupMenuExtended Methods
    virtual ::sal_Bool SAL_CALL isInExecute(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endExecute(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLogo( const ::com::sun::star::awt::MenuLogo& aMenuLogo ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::MenuLogo SAL_CALL getLogo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL enableAutoMnemonics( ::sal_Bool bEnable ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAcceleratorKeyEvent( ::sal_Int16 nItemId, const ::com::sun::star::awt::KeyEvent& aKeyEvent ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::KeyEvent SAL_CALL getAcceleratorKeyEvent( ::sal_Int16 nItemId ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHelpText( ::sal_Int16 nItemId, const ::rtl::OUString& sHelpText ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getHelpText( ::sal_Int16 nItemId ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTipHelpText( ::sal_Int16 nItemId, const ::rtl::OUString& sTipHelpText ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTipHelpText( ::sal_Int16 nItemId ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setItemImage( ::sal_Int16 nItemId, const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& xGraphic, ::sal_Bool bScale ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL getItemImage( ::sal_Int16 nItemId ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setItemImageAngle( ::sal_Int16 nItemId, ::sal_Int32 nAngle ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getItemImageAngle( ::sal_Int16 nItemId ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setItemImageMirrorMode( ::sal_Int16 nItemId, ::sal_Bool bMirror ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isItemImageInMirrorMode( ::sal_Int16 nItemId ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

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
        VCLXPopupMenu( PopupMenu* pPopMenu );
};

#endif // _TOOLKIT_AWT_VCLXMENU_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
