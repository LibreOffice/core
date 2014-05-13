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
#ifndef INCLUDED_SFX2_MNUITEM_HXX
#define INCLUDED_SFX2_MNUITEM_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URL.hpp>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/dllapi.h>

class SfxVirtualMenu;
class SfxBindings;
class SfxModule;
class Menu;
class PopupMenu;
class SfxUnoMenuControl;
class SfxUnoControllerItem;
struct SfxMenuCtrlFactory;

class SFX2_DLLPUBLIC SfxMenuControl: public SfxControllerItem
{
    OUString                aTitle;
    SfxVirtualMenu*         pOwnMenu;
    SfxVirtualMenu*         pSubMenu;
    bool                    b_ShowStrings;

public:
                            SfxMenuControl();
                            SfxMenuControl( bool bShowStrings );
                            SfxMenuControl( sal_uInt16, SfxBindings&);

    static SfxMenuControl*  CreateImpl( sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings );
    static void             RegisterControl( sal_uInt16 nSlotId = 0, SfxModule *pMod=NULL );

                    virtual ~SfxMenuControl();

    using SfxControllerItem::Bind;
    void                    Bind( SfxVirtualMenu* pOwnMenu, sal_uInt16 nId,
                                  const OUString& rTitle, SfxBindings& rBindings );
    void                    Bind( SfxVirtualMenu* pOwnMenu, sal_uInt16 nId,
                                  SfxVirtualMenu& rSubMenu,
                                  const OUString& rTitle, SfxBindings& rBindings );

    OUString                GetTitle() const;
    SfxVirtualMenu*         GetPopupMenu() const;
    virtual PopupMenu*      GetPopup() const;

    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                          const SfxPoolItem* pState ) SAL_OVERRIDE;

    static SfxMenuControl*    CreateControl( sal_uInt16 nId, Menu &, SfxBindings & );
    static SfxUnoMenuControl* CreateControl( const OUString&, sal_uInt16, Menu&, const OUString& sItemText, SfxBindings&, SfxVirtualMenu* );
    static void               RegisterMenuControl( SfxModule*, SfxMenuCtrlFactory* );

};

class SfxUnoMenuControl : public SfxMenuControl
{
    SfxUnoControllerItem*   pUnoCtrl;
public:
                            SfxUnoMenuControl( const OUString&, sal_uInt16 nId, Menu&,
                                               const OUString&,
                                                SfxBindings&, SfxVirtualMenu* );
                            virtual ~SfxUnoMenuControl();
};

typedef SfxMenuControl* (*SfxMenuControlCtor)( sal_uInt16 nId, Menu &, SfxBindings & );

struct SfxMenuCtrlFactory
{
    SfxMenuControlCtor  pCtor;
    TypeId              nTypeId;
    sal_uInt16              nSlotId;

    SfxMenuCtrlFactory( SfxMenuControlCtor pTheCtor,
            TypeId nTheTypeId, sal_uInt16 nTheSlotId ):
        pCtor(pTheCtor),
        nTypeId(nTheTypeId),
        nSlotId(nTheSlotId)
    {}
};

inline OUString SfxMenuControl::GetTitle() const
{
    return aTitle;
}

inline SfxVirtualMenu* SfxMenuControl::GetPopupMenu() const
{
    return pSubMenu;
}

#define SFX_DECL_MENU_CONTROL() \
        static SfxMenuControl* CreateImpl( sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings ); \
        static void RegisterControl(sal_uInt16 nSlotId = 0, SfxModule *pMod=NULL)

#define SFX_IMPL_MENU_CONTROL(Class, nItemClass) \
        SfxMenuControl* Class::CreateImpl( sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings ) \
               { return new Class(nId, rMenu, rBindings); } \
        void Class::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod) \
               { SfxMenuControl::RegisterMenuControl( pMod, new SfxMenuCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }

class SfxAppMenuControl_Impl : public SfxMenuControl
{
    PopupMenu*  pMenu;
    OUString    m_sIconTheme;
    bool        m_bShowMenuImages;

protected:
    DECL_LINK( Activate, Menu * ); // Needed to support high contrast images

public:
    SFX_DECL_MENU_CONTROL();
    SfxAppMenuControl_Impl( sal_uInt16 nPos, Menu& rMenu, SfxBindings& rBindings );
    virtual ~SfxAppMenuControl_Impl();

    struct ExecuteInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
        ::com::sun::star::util::URL                                                aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
    };

    DECL_STATIC_LINK( SfxAppMenuControl_Impl, ExecuteHdl_Impl, ExecuteInfo* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
