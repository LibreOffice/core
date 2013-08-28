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
#ifndef SFXVIRTMENU_HXX
#define SFXVIRTMENU_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#include <sfx2/mnuitem.hxx>

class SfxBindings;
class SfxMenuImageControl_Impl;

typedef ::boost::ptr_vector<SfxMenuControl> SfxMenuCtrlArr_Impl;

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
    sal_Bool            bResCtor : 1; // SV Menu created from resource
    sal_Bool            bOLE : 1;     // InPlaceMenu
    sal_Bool            bHelpInitialized : 1;
    sal_Bool            bIsActive : 1;
    sal_Bool            bControllersUnBound : 1;
    sal_Bool            bIsAddonPopupMenu : 1;

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
    SfxVirtualMenu( sal_uInt16 nOwnId, SfxVirtualMenu* pParent, Menu& rMenu, sal_Bool bWithHelp,
                        SfxBindings &rBind, sal_Bool bOLEServer=sal_False, sal_Bool bRes=sal_False, sal_Bool bIsAddonMenu=sal_False );

    void            CreateFromSVMenu();
    DECL_LINK( Highlight, void * );
    DECL_LINK( Activate, Menu * );
    DECL_LINK( Deactivate, Menu * );
    DECL_LINK( SettingsChanged, void* );

    // Used for runtime popup menus
    void            UpdateImages( Menu* pMenu );
    void            RemoveMenuImages( Menu* pMenu );
    void            InsertAddOnsMenuItem( Menu* pMenu );

public:
                    ~SfxVirtualMenu();
                    SfxVirtualMenu( Menu *pStarViewMenu, sal_Bool bWithHelp,
                        SfxBindings &rBind, sal_Bool bOLEServer=sal_False, sal_Bool bRes=sal_False, sal_Bool bIsAddonMenu=sal_False );
    void            CheckItem( sal_uInt16 nItemId, sal_Bool bCheck );
    void            EnableItem( sal_uInt16 nItemId, sal_Bool bEnable );
    void            SetItemText( sal_uInt16 nItemId, const OUString& rText );

    sal_uInt16          GetItemCount() const;
    Menu*           GetSVMenu() const;
    SfxMenuControl& operator[]( sal_uInt16 nPos ) const;

    sal_uInt16          GetItemId( sal_uInt16 nPos ) const;

    SfxVirtualMenu* GetParentMenu() const { return pParent; }
    void            SetParentMenu( SfxVirtualMenu* pNewParent )
                    { pParent = pNewParent; }

    void            SetPopupMenu( sal_uInt16 nId, PopupMenu *pMenu );
    sal_Bool            IsFromResource() const
                    { return bResCtor; }
    void            InitPopup(sal_uInt16 nPos, sal_Bool bOLE = sal_True);
    void            InitializeHelp();
    void            SetResMgr(ResMgr* pMgr)  {pResMgr = pMgr; }
    ResMgr*         GetResMgr() { return pResMgr; }

    DECL_LINK( Select, Menu * );
};

//--------------------------------------------------------------------

// return the number of virtual items in this menu

inline sal_uInt16 SfxVirtualMenu::GetItemCount() const
{
    return nCount;
}
//--------------------------------------------------------------------


inline SfxMenuControl& SfxVirtualMenu::operator[]( sal_uInt16 nPos ) const
{
    return *(pItems+nPos);
}
//--------------------------------------------------------------------

// returns the item id at position nPos in the menu (or 0 if sep.)

inline sal_uInt16 SfxVirtualMenu::GetItemId( sal_uInt16 nPos ) const
{
    return pItems ? pItems[nPos].GetId() : 0;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
