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
#ifndef _SFXMNUITEM_HXX
#define _SFXMNUITEM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

class SfxVirtualMenu;
class SfxBindings;
class SfxModule;
class Menu;
class PopupMenu;
class SfxUnoMenuControl;
class SfxUnoControllerItem;
struct SfxMenuCtrlFactory;
#include <tools/string.hxx>
#include <sfx2/ctrlitem.hxx>

class SFX2_DLLPUBLIC SfxMenuControl: public SfxControllerItem
{
//friend SvStream& operator<<( SvStream& rStream, const SfxMenuControl& rItem );
//friend SvStream& operator>>( SvStream& rStream, SfxMenuControl& rItem );

    String                  aTitle;
    String                  aHelpText;
    SfxVirtualMenu*         pOwnMenu;
    SfxVirtualMenu*         pSubMenu;
    BOOL                    b_ShowStrings;
    BOOL                    b_UnusedDummy;

public:
                            SfxMenuControl();
                            SfxMenuControl( BOOL bShowStrings );
                            SfxMenuControl( USHORT, SfxBindings&);

    static SfxMenuControl*  CreateImpl( USHORT nId, Menu &rMenu, SfxBindings &rBindings );
    static void             RegisterControl( USHORT nSlotId = 0, SfxModule *pMod=NULL );

                            ~SfxMenuControl();

        using SfxControllerItem::Bind;
    void                    Bind( SfxVirtualMenu* pOwnMenu, USHORT nId,
                                  const String& rTitle, const String &rHelpText,
                                  SfxBindings & );
    void                    Bind( SfxVirtualMenu* pOwnMenu, USHORT nId,
                                  SfxVirtualMenu& rSubMenu,
                                  const String& rTitle, const String &rHelpText,
                                  SfxBindings & );

//  SvStream &              Load(SvStream &, SfxBindings*);
//  SvStream &              Store(SvStream &);

    String                  GetTitle() const;
    SfxVirtualMenu*         GetPopupMenu() const;
    virtual PopupMenu*      GetPopup() const;
    void                    SetOwnMenu( SfxVirtualMenu* pMenu );
    void                    RemovePopup();

    const String&           GetHelpText() const { return aHelpText; }
    void                    SetHelpText(const String &rStr) { aHelpText  = rStr; }

    virtual void            StateChanged( USHORT nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );

    static SfxMenuControl*    CreateControl( USHORT nId, Menu &, SfxBindings & );
    static SfxUnoMenuControl* CreateControl( const String&, USHORT, Menu&, SfxBindings&, SfxVirtualMenu* );
    static SfxUnoMenuControl* CreateControl( const String&, USHORT, Menu&, const String& sItemText, const String& sHelpText, SfxBindings&, SfxVirtualMenu* );
    static BOOL             IsSpecialControl( USHORT nId, SfxModule* );
    static void             RegisterMenuControl(SfxModule*, SfxMenuCtrlFactory*);

};

class SfxUnoMenuControl : public SfxMenuControl
{
    SfxUnoControllerItem*   pUnoCtrl;
public:
                            SfxUnoMenuControl( const String&, USHORT nId, Menu&,
                                                SfxBindings&, SfxVirtualMenu* );
                            SfxUnoMenuControl( const String&, USHORT nId, Menu&,
                                               const String&, const String&,
                                                SfxBindings&, SfxVirtualMenu* );
                            ~SfxUnoMenuControl();
    void                    Select();
};

//--------------------------------------------------------------------

typedef SfxMenuControl* (*SfxMenuControlCtor)( USHORT nId, Menu &, SfxBindings & );

struct SfxMenuCtrlFactory
{
    SfxMenuControlCtor  pCtor;
    TypeId              nTypeId;
    USHORT              nSlotId;

    SfxMenuCtrlFactory( SfxMenuControlCtor pTheCtor,
            TypeId nTheTypeId, USHORT nTheSlotId ):
        pCtor(pTheCtor),
        nTypeId(nTheTypeId),
        nSlotId(nTheSlotId)
    {}
};

//

inline String SfxMenuControl::GetTitle() const
{
    return aTitle;
}
//--------------------------------------------------------------------

//

inline SfxVirtualMenu* SfxMenuControl::GetPopupMenu() const
{
    return pSubMenu;
}
//--------------------------------------------------------------------

#define SFX_DECL_MENU_CONTROL() \
        static SfxMenuControl* CreateImpl( USHORT nId, Menu &rMenu, SfxBindings &rBindings ); \
        static void RegisterControl(USHORT nSlotId = 0, SfxModule *pMod=NULL)

#define SFX_IMPL_MENU_CONTROL(Class, nItemClass) \
        SfxMenuControl* __EXPORT Class::CreateImpl( USHORT nId, Menu &rMenu, SfxBindings &rBindings ) \
               { return new Class(nId, rMenu, rBindings); } \
        void Class::RegisterControl(USHORT nSlotId, SfxModule *pMod) \
               { SfxMenuControl::RegisterMenuControl( pMod, new SfxMenuCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }


class SfxAppMenuControl_Impl : public SfxMenuControl
{
    PopupMenu*  pMenu;
    ULONG       m_nSymbolsStyle;
    BOOL        m_bShowMenuImages;

protected:
    DECL_LINK( Activate, Menu * ); // Needed to support high contrast images

public:
    SFX_DECL_MENU_CONTROL();
    SfxAppMenuControl_Impl( USHORT nPos, Menu& rMenu, SfxBindings& rBindings );
    ~SfxAppMenuControl_Impl();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
