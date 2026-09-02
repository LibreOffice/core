/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <config_options.h>
#include <toolkit/dllapi.h>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include <cppuhelper/weak.hxx>
#include <mutex>

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
                                    public ::cppu::OWeakObject
{
private:
    std::mutex              maMutex;
    VclPtr<Menu>            mpMenu;
    MenuListenerMultiplexer maMenuListeners;
    PopupMenuRefList        maPopupMenuRefs;
    sal_Int16               mnDefaultItem;

protected:

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
    cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override;
    void acquire() noexcept override  { OWeakObject::acquire(); }
    void release() noexcept override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
    cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    // css::awt::XMenu
    virtual void addMenuListener( const css::uno::Reference< css::awt::XMenuListener >& xListener ) override;
    virtual void removeMenuListener( const css::uno::Reference< css::awt::XMenuListener >& xListener ) override;
    virtual void insertItem( sal_Int16 nItemId, const OUString& aText, sal_Int16 nItemStyle, sal_Int16 nPos ) override;
    virtual void removeItem( sal_Int16 nPos, sal_Int16 nCount ) override;
    virtual void clear( ) override;
    virtual sal_Int16 getItemCount(  ) override;
    virtual sal_Int16 getItemId( sal_Int16 nPos ) override;
    virtual sal_Int16 getItemPos( sal_Int16 nId ) override;
    virtual css::awt::MenuItemType getItemType( ::sal_Int16 nItemPos ) override;
    virtual void enableItem( sal_Int16 nItemId, bool bEnable ) override;
    virtual bool isItemEnabled( sal_Int16 nItemId ) override;
    virtual void hideDisabledEntries( bool bHide ) override;
    virtual void enableAutoMnemonics( bool bEnable ) override;
    virtual void setItemText( sal_Int16 nItemId, const OUString& aText ) override;
    virtual OUString getItemText( sal_Int16 nItemId ) override;
    virtual void setCommand( sal_Int16 nItemId, const OUString& aCommand ) override;
    virtual OUString getCommand( sal_Int16 nItemId ) override;
    virtual void setHelpCommand( sal_Int16 nItemId, const OUString& aHelp ) override;
    virtual OUString getHelpCommand( sal_Int16 nItemId ) override;
    virtual void setHelpText( ::sal_Int16 nItemId, const OUString& sHelpText ) override;
    virtual OUString getHelpText( ::sal_Int16 nItemId ) override;
    virtual void setTipHelpText( ::sal_Int16 nItemId, const OUString& sTipHelpText ) override;
    virtual OUString getTipHelpText( ::sal_Int16 nItemId ) override;

    virtual bool isPopupMenu(  ) override;
    virtual void setPopupMenu( sal_Int16 nItemId, const css::uno::Reference< css::awt::XPopupMenu >& aPopupMenu ) override;
    virtual css::uno::Reference< css::awt::XPopupMenu > getPopupMenu( sal_Int16 nItemId ) override;

    // css::awt::XPopupMenu
    virtual void insertSeparator( sal_Int16 nPos ) override;
    virtual void setDefaultItem( sal_Int16 nItemId ) override;
    virtual sal_Int16 getDefaultItem(  ) override;
    virtual void checkItem( sal_Int16 nItemId, bool bCheck ) override;
    virtual bool isItemChecked( sal_Int16 nItemId ) override;
    virtual ::sal_Int16 execute( const css::uno::Reference< css::awt::XWindowPeer >& Parent, const css::awt::Rectangle& Position, ::sal_Int16 Direction ) override;
    virtual bool isInExecute(  ) override;
    virtual void endExecute(  ) override;
    virtual void setAcceleratorKeyEvent( ::sal_Int16 nItemId, const css::awt::KeyEvent& aKeyEvent ) override;
    virtual css::awt::KeyEvent getAcceleratorKeyEvent( ::sal_Int16 nItemId ) override;
    virtual void setItemImage( ::sal_Int16 nItemId, const css::uno::Reference< css::graphic::XGraphic >& xGraphic, bool bScale ) override;
    virtual css::uno::Reference< css::graphic::XGraphic > getItemImage( ::sal_Int16 nItemId ) override;

    // css::lang::XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;
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

    ~VCLXPopupMenu() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
