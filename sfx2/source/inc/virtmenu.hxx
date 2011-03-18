/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SFXVIRTMENU_HXX
#define _SFXVIRTMENU_HXX


#include <svl/svarray.hxx>
#include <sfx2/mnuitem.hxx>
#include "mnucfga.hxx"

class SfxBindings;
class Timer;
class SfxMenuImageControl_Impl;

SV_DECL_PTRARR_DEL( SfxMenuCtrlArr_Impl, SfxMenuControl*, 2, 2 )

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
    Timer*          pAutoDeactivate; // Hack for QAP-Bug
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
    DECL_LINK( Highlight, Menu * );
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
    void            SetItemText( sal_uInt16 nItemId, const String& rText );

    sal_uInt16          GetItemPos( sal_uInt16 nItemId ) const;

    sal_uInt16          GetItemCount() const;
    Menu*           GetSVMenu() const;
    SfxMenuControl& operator[]( sal_uInt16 nPos ) const;

    sal_uInt16          GetItemId( sal_uInt16 nPos ) const;
    SfxVirtualMenu* GetPopupMenu( sal_uInt16 nId ) const;
    String          GetItemText( sal_uInt16 nId ) const;

    //void            InvalidateKeyCodes();

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
    void            SetHelpIds( ResMgr* );
    void            UpdateImages();

    DECL_LINK( Select, Menu * );
};

//--------------------------------------------------------------------

// return the number of virtual items in this menu

inline sal_uInt16 SfxVirtualMenu::GetItemCount() const
{
    return nCount;
}
//--------------------------------------------------------------------

//

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
