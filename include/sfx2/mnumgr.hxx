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
#ifndef _SFXMNUMGR_HXX
#define _SFXMNUMGR_HXX

#include <vcl/menu.hxx>
#include <tools/wintypes.hxx>
#include <tools/link.hxx>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <tools/stream.hxx>
#include <tools/resid.hxx>
#include <sfx2/sfx.hrc>

class SfxVirtualMenu;
class SfxPoolItem;
class SfxBindings;
class ResMgr;
class Menu;
class PopupMenu;
class SfxBindings;
class ResId;
class Point;
class SfxObjectShell;
class SfxModule;
class SfxViewFrame;

class SfxMenuManager
{
friend class SfxPopupMenuManager;

    SfxVirtualMenu*  pMenu;     // the actual Menu
    SfxVirtualMenu*         pOldMenu;       // only while reconfiguring
    sal_Bool         bMenuBar;  // Popup or MenuBar
    SfxBindings*            pBindings;
    ResMgr*                 pResMgr;
    sal_uInt32              nType;
    sal_Bool                    bAddClipboardFuncs : 1;

    void                    Construct( SfxVirtualMenu& rMenu );

protected:
                            SfxMenuManager( Menu*, SfxBindings& );
                            ~SfxMenuManager();
    sal_uInt16                  GetItemPos( sal_uInt16 nId );
    sal_uInt32              GetType() { return nType; }
public:

    DECL_LINK( Select, Menu* );

    SfxVirtualMenu*         GetMenu() const
                            { return pMenu; }

    SfxBindings&            GetBindings() { return *pBindings; }
    const SfxBindings&      GetBindings() const { return *pBindings; }
    void                    SetResMgr(ResMgr* pMgr)  {pResMgr = pMgr; }
    ResMgr*                 GetResMgr() const { return pResMgr; }
    void                    SetPopupMenu( sal_uInt16 nId, PopupMenu *pMenu );
};



class SAL_DLLPUBLIC_EXPORT SfxPopupMenuManager : public SfxMenuManager
{
private:
    Menu*               pSVMenu;

    // when #i107205 gets fixed this one should be superfluous.
    // But right now we want to avoid the memory leak that would otherwise occur,
    // if we don't delete the pointer that got created in SfxPopupMenuManager::Popup
    static PopupMenu *  pStaticThesSubMenu;

    // only declared, but not defined: don't allow copying
    SfxPopupMenuManager( const SfxPopupMenuManager& );
    SfxPopupMenuManager& operator=( const SfxPopupMenuManager& );

public:
                        SfxPopupMenuManager( PopupMenu*, SfxBindings& );
                        ~SfxPopupMenuManager();
    static void         ExecutePopup( const ResId&, SfxViewFrame* pViewFrame, const Point& rPoint, Window* pWindow );
    // @deprecated!!
    // Don't use this method any longer. The whole class will be removed in the future.
    // Changing code which relies on Popup would need much more effort.
    static SfxPopupMenuManager* Popup( const ResId& rResId, SfxViewFrame* pFrame,const Point& rPoint, Window* pWindow );

    sal_uInt16              Execute( const Point& rPos, Window *pWindow );

    void                RemoveDisabledEntries();
    Menu*                   GetSVMenu();
};

#endif // #ifndef _SFXMNUMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
