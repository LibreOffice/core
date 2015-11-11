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
#ifndef INCLUDED_SFX2_SOURCE_INC_VIRTMENU_HXX
#define INCLUDED_SFX2_SOURCE_INC_VIRTMENU_HXX

#include <vector>
#include <memory>

#include <sfx2/mnuitem.hxx>

class SfxBindings;
class SfxMenuImageControl_Impl;

typedef std::vector<std::unique_ptr<SfxMenuControl> > SfxMenuCtrlArr_Impl;

class SAL_DLLPUBLIC_EXPORT SfxVirtualMenu
{
friend class SfxMenuControl;

private:
    Menu*           pSVMenu;
    SfxVirtualMenu* pParent;
    SfxMenuCtrlArr_Impl* pAppCtrl;
    SfxMenuControl* pItems;
    SfxMenuImageControl_Impl* pImageControl;
    SfxBindings*    pBindings;
    ResMgr*         pResMgr;
    PopupMenu*      pWindowMenu;
    PopupMenu*      pPickMenu;
    PopupMenu*      pAddonsMenu;
    sal_uInt16          nVisibleItems;
    sal_uInt16          nId;
    sal_uInt16          nCount;
    sal_uInt16          nLocks;
    bool            bResCtor : 1; // SV Menu created from resource
    bool            bOLE : 1;     // InPlaceMenu
    bool            bHelpInitialized : 1;
    bool            bIsActive : 1;
    bool            bControllersUnBound : 1;
    bool            bIsAddonPopupMenu : 1;

private:
    void            Construct_Impl();
    bool            Bind_Impl( Menu *pMenu );
    inline SfxMenuCtrlArr_Impl& GetAppCtrl_Impl()
    {
        return pAppCtrl? *pAppCtrl: *(pAppCtrl = new SfxMenuCtrlArr_Impl) ;
    }
    void            UnbindControllers();
    void            BindControllers();

protected:
    SfxVirtualMenu( sal_uInt16 nOwnId, SfxVirtualMenu* pParent, Menu& rMenu, bool bWithHelp,
                        SfxBindings &rBind, bool bOLEServer=false, bool bRes=false, bool bIsAddonMenu=false );

    void            CreateFromSVMenu();
    DECL_STATIC_LINK_TYPED( SfxVirtualMenu, Highlight, Menu *, bool );
    DECL_LINK_TYPED( Activate, Menu *, bool );
    DECL_LINK_TYPED( Deactivate, Menu *, bool );
    DECL_LINK_TYPED( SettingsChanged, LinkParamNone*, void );

    // Used for runtime popup menus
    void            UpdateImages( Menu* pMenu );
    void            RemoveMenuImages( Menu* pMenu );
    void            InsertAddOnsMenuItem( Menu* pMenu );

public:
                    ~SfxVirtualMenu();
                    SfxVirtualMenu( Menu *pStarViewMenu, bool bWithHelp,
                        SfxBindings &rBind, bool bOLEServer=false, bool bRes=false, bool bIsAddonMenu=false );
    void            CheckItem( sal_uInt16 nItemId, bool bCheck );
    void            EnableItem( sal_uInt16 nItemId, bool bEnable );
    void            SetItemText( sal_uInt16 nItemId, const OUString& rText );

    Menu*           GetSVMenu() const { return pSVMenu;}
    SfxMenuControl& operator[]( sal_uInt16 nPos ) const;

    void            SetPopupMenu( sal_uInt16 nId, PopupMenu *pMenu );
    void            InitPopup(sal_uInt16 nPos, bool bOLE = true);
    void            InitializeHelp();

    DECL_LINK_TYPED( Select, Menu*, bool );
};



inline SfxMenuControl& SfxVirtualMenu::operator[]( sal_uInt16 nPos ) const
{
    return *(pItems+nPos);
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
