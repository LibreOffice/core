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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXMENU_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXMENU_HXX

#include <config_options.h>
#include <toolkit/dllapi.h>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

#include <vector>

class Menu;
class MenuBar;
class PopupMenu;
class VclMenuEvent;

typedef ::std::vector<
    css::uno::Reference< css::awt::XPopupMenu >
> PopupMenuRefList;

typedef void (*MenuUserDataReleaseFunction)(void*);

class TOOLKIT_DLLPUBLIC VCLXMenu :  public css::awt::XMenuBar,
                                    public css::awt::XPopupMenu,
                                    public css::lang::XServiceInfo,
                                    public css::lang::XTypeProvider,
                                    public css::lang::XUnoTunnel,
                                    public ::cppu::OWeakObject
{
private:
    ::osl::Mutex            maMutex;
    VclPtr<Menu>            mpMenu;
    MenuListenerMultiplexer maMenuListeners;
    PopupMenuRefList        maPopupMenuRefs;
    sal_Int16               mnDefaultItem;

protected:
    ::osl::Mutex&           GetMutex() { return maMutex; }

    DECL_DLLPRIVATE_LINK( MenuEventListener, VclMenuEvent&, void );

    void ImplCreateMenu( bool bPopup );
    void ImplAddListener();

public:
    VCLXMenu();
    VCLXMenu( Menu* pMenu );
    virtual ~VCLXMenu() override;


    Menu*    GetMenu() const { return mpMenu; }
    bool IsPopupMenu() const;
    void setUserValue(sal_uInt16 nItemId, void* nUserValue, MenuUserDataReleaseFunction aFunc);
    void* getUserValue(sal_uInt16 nItemId);

    // css::uno::XInterface
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XUnoTunnel
    UNO3_GETIMPLEMENTATION_DECL(VCLXMenu)

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // css::awt::XMenu
    virtual void SAL_CALL addMenuListener( const css::uno::Reference< css::awt::XMenuListener >& xListener ) override;
    virtual void SAL_CALL removeMenuListener( const css::uno::Reference< css::awt::XMenuListener >& xListener ) override;
    virtual void SAL_CALL insertItem( sal_Int16 nItemId, const OUString& aText, sal_Int16 nItemStyle, sal_Int16 nPos ) override;
    virtual void SAL_CALL removeItem( sal_Int16 nPos, sal_Int16 nCount ) override;
    virtual void SAL_CALL clear( ) override;
    virtual sal_Int16 SAL_CALL getItemCount(  ) override;
    virtual sal_Int16 SAL_CALL getItemId( sal_Int16 nPos ) override;
    virtual sal_Int16 SAL_CALL getItemPos( sal_Int16 nId ) override;
    virtual css::awt::MenuItemType SAL_CALL getItemType( ::sal_Int16 nItemPos ) override;
    virtual void SAL_CALL enableItem( sal_Int16 nItemId, sal_Bool bEnable ) override;
    virtual sal_Bool SAL_CALL isItemEnabled( sal_Int16 nItemId ) override;
    virtual void SAL_CALL hideDisabledEntries( sal_Bool bHide ) override;
    virtual void SAL_CALL enableAutoMnemonics( sal_Bool bEnable ) override;
    virtual void SAL_CALL setItemText( sal_Int16 nItemId, const OUString& aText ) override;
    virtual OUString SAL_CALL getItemText( sal_Int16 nItemId ) override;
    virtual void SAL_CALL setCommand( sal_Int16 nItemId, const OUString& aCommand ) override;
    virtual OUString SAL_CALL getCommand( sal_Int16 nItemId ) override;
    virtual void SAL_CALL setHelpCommand( sal_Int16 nItemId, const OUString& aHelp ) override;
    virtual OUString SAL_CALL getHelpCommand( sal_Int16 nItemId ) override;
    virtual void SAL_CALL setHelpText( ::sal_Int16 nItemId, const OUString& sHelpText ) override;
    virtual OUString SAL_CALL getHelpText( ::sal_Int16 nItemId ) override;
    virtual void SAL_CALL setTipHelpText( ::sal_Int16 nItemId, const OUString& sTipHelpText ) override;
    virtual OUString SAL_CALL getTipHelpText( ::sal_Int16 nItemId ) override;

    virtual sal_Bool SAL_CALL isPopupMenu(  ) override;
    virtual void SAL_CALL setPopupMenu( sal_Int16 nItemId, const css::uno::Reference< css::awt::XPopupMenu >& aPopupMenu ) override;
    virtual css::uno::Reference< css::awt::XPopupMenu > SAL_CALL getPopupMenu( sal_Int16 nItemId ) override;

    // css::awt::XPopupMenu
    virtual void SAL_CALL insertSeparator( sal_Int16 nPos ) override;
    virtual void SAL_CALL setDefaultItem( sal_Int16 nItemId ) override;
    virtual sal_Int16 SAL_CALL getDefaultItem(  ) override;
    virtual void SAL_CALL checkItem( sal_Int16 nItemId, sal_Bool bCheck ) override;
    virtual sal_Bool SAL_CALL isItemChecked( sal_Int16 nItemId ) override;
    virtual ::sal_Int16 SAL_CALL execute( const css::uno::Reference< css::awt::XWindowPeer >& Parent, const css::awt::Rectangle& Position, ::sal_Int16 Direction ) override;
    virtual sal_Bool SAL_CALL isInExecute(  ) override;
    virtual void SAL_CALL endExecute(  ) override;
    virtual void SAL_CALL setAcceleratorKeyEvent( ::sal_Int16 nItemId, const css::awt::KeyEvent& aKeyEvent ) override;
    virtual css::awt::KeyEvent SAL_CALL getAcceleratorKeyEvent( ::sal_Int16 nItemId ) override;
    virtual void SAL_CALL setItemImage( ::sal_Int16 nItemId, const css::uno::Reference< css::graphic::XGraphic >& xGraphic, sal_Bool bScale ) override;
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL getItemImage( ::sal_Int16 nItemId ) override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

class UNLESS_MERGELIBS(TOOLKIT_DLLPUBLIC) VCLXMenuBar final : public VCLXMenu
{
public:
    VCLXMenuBar();
    VCLXMenuBar( MenuBar* pMenuBar );
};

class TOOLKIT_DLLPUBLIC VCLXPopupMenu final : public VCLXMenu
{
public:
    VCLXPopupMenu();
    VCLXPopupMenu( PopupMenu* pPopMenu );
};

#endif // INCLUDED_TOOLKIT_AWT_VCLXMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
