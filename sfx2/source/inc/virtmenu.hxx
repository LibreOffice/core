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
    USHORT          nVisibleItems;
    USHORT          nId;
    USHORT          nCount;
    USHORT          nLocks;
    BOOL            bResCtor : 1; // SV Menu created from resource
    BOOL            bOLE : 1;     // InPlaceMenu
    BOOL            bHelpInitialized : 1;
    BOOL            bIsActive : 1;
    BOOL            bControllersUnBound : 1;
    BOOL            bIsAddonPopupMenu : 1;

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
    SfxVirtualMenu( USHORT nOwnId, SfxVirtualMenu* pParent, Menu& rMenu, BOOL bWithHelp,
                        SfxBindings &rBind, BOOL bOLEServer=FALSE, BOOL bRes=FALSE, BOOL bIsAddonMenu=FALSE );

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
                    SfxVirtualMenu( Menu *pStarViewMenu, BOOL bWithHelp,
                        SfxBindings &rBind, BOOL bOLEServer=FALSE, BOOL bRes=FALSE, BOOL bIsAddonMenu=FALSE );
    void            CheckItem( USHORT nItemId, BOOL bCheck );
    void            EnableItem( USHORT nItemId, BOOL bEnable );
    void            SetItemText( USHORT nItemId, const String& rText );

    USHORT          GetItemPos( USHORT nItemId ) const;

    USHORT          GetItemCount() const;
    Menu*           GetSVMenu() const;
    SfxMenuControl& operator[]( USHORT nPos ) const;

    USHORT          GetItemId( USHORT nPos ) const;
    SfxVirtualMenu* GetPopupMenu( USHORT nId ) const;
    String          GetItemText( USHORT nId ) const;

    //void            InvalidateKeyCodes();

    SfxVirtualMenu* GetParentMenu() const { return pParent; }
    void            SetParentMenu( SfxVirtualMenu* pNewParent )
                    { pParent = pNewParent; }

    void            SetPopupMenu( USHORT nId, PopupMenu *pMenu );
    BOOL            IsFromResource() const
                    { return bResCtor; }
    void            InitPopup(USHORT nPos, BOOL bOLE = TRUE);
    void            InitializeHelp();
    void            SetResMgr(ResMgr* pMgr)  {pResMgr = pMgr; }
    ResMgr*         GetResMgr() { return pResMgr; }
    void            SetHelpIds( ResMgr* );
    void            UpdateImages();

    DECL_LINK( Select, Menu * );
};

//--------------------------------------------------------------------

// return the number of virtual items in this menu

inline USHORT SfxVirtualMenu::GetItemCount() const
{
    return nCount;
}
//--------------------------------------------------------------------

//

inline SfxMenuControl& SfxVirtualMenu::operator[]( USHORT nPos ) const
{
    return *(pItems+nPos);
}
//--------------------------------------------------------------------

// returns the item id at position nPos in the menu (or 0 if sep.)

inline USHORT SfxVirtualMenu::GetItemId( USHORT nPos ) const
{
    return pItems ? pItems[nPos].GetId() : 0;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
