/*************************************************************************
 *
 *  $RCSfile: virtmenu.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SFXVIRTMENU_HXX
#define _SFXVIRTMENU_HXX


#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _SFXMNUITEM_HXX
#include "mnuitem.hxx"
#endif

#ifndef _SFXMNUCFGA_HXX
#include "mnucfga.hxx"
#endif

class SfxBindings;
class Timer;

SV_DECL_PTRARR_DEL( SfxMenuCtrlArr_Impl, SfxMenuControl*, 2, 2 );

class SfxVirtualMenu
{
friend class SfxMenuControl;

private:
    Menu*           pSVMenu;
    SfxVirtualMenu* pParent;
    SfxMenuCtrlArr_Impl* pAppCtrl;
    SfxMenuControl* pItems;
    SfxBindings*    pBindings;
    ResMgr*         pResMgr;
    PopupMenu*      pWindowMenu;
    PopupMenu*      pPickMenu;
    Timer*          pAutoDeactivate; // Hack fuer QAP-Bug
    USHORT          nVisibleItems;
    USHORT          nId;
    USHORT          nCount;
    USHORT          nLocks;
    BOOL            bResCtor : 1; // SV Menu created from resource
    BOOL            bOLE : 1;     // InPlaceMenu
    BOOL            bHelpInitialized : 1;
    BOOL            bIsActive : 1;
    BOOL            bControllersUnBound : 1;
    BOOL            bRemoveDisabledEntries: 1;

private:
    void            Construct_Impl();
    FASTBOOL        Bind_Impl( Menu *pMenu );
    inline SfxMenuCtrlArr_Impl& GetAppCtrl_Impl()
    {
        return pAppCtrl? *pAppCtrl: *(pAppCtrl = new SfxMenuCtrlArr_Impl) ;
    }
    void            UnbindControllers();
    void            BindControllers();

protected:
    SfxVirtualMenu( USHORT nOwnId, SfxVirtualMenu* pParent, Menu& rMenu, BOOL bWithHelp,
                        SfxBindings &rBind, BOOL bOLEServer=FALSE, BOOL bRes=FALSE );

    void            CreateFromSVMenu();
    DECL_LINK( Highlight, Menu * );
    DECL_LINK( Activate, Menu * );
    DECL_LINK( Deactivate, Menu * );
    DECL_LINK( Select, Menu * );

public:
                    ~SfxVirtualMenu();
                    SfxVirtualMenu( Menu *pStarViewMenu, BOOL bWithHelp,
                        SfxBindings &rBind, BOOL bOLEServer=FALSE, BOOL bRes=FALSE );
    void            CheckItem( USHORT nItemId, BOOL bCheck );
    void            EnableItem( USHORT nItemId, BOOL bEnable );
    void            SetItemText( USHORT nItemId, const String& rText );

    USHORT          GetItemPos( USHORT nItemId ) const;

    USHORT          GetItemCount() const;
    Menu*           GetSVMenu() const;
    SfxMenuControl& operator[]( USHORT nPos ) const;
    USHORT          GetItemId( USHORT nPos ) const;
    SfxVirtualMenu* GetPopupMenu( USHORT nId ) const;
    SfxVirtualMenu* GetParentMenu() const { return pParent; }
    void            SetParentMenu( SfxVirtualMenu* pNewParent )
                    { pParent = pNewParent; }
    String          GetItemText( USHORT nId ) const;
    String          GetItemHelpText( USHORT nId ) const;
    void            InvalidateKeyCodes();

    void            SetPopupMenu( USHORT nId, PopupMenu *pMenu );
    BOOL            IsFromResource() const
                    { return bResCtor; }
    void            InitPopup(USHORT nPos, BOOL bOLE = TRUE);
    void            InitializeHelp();
    void            SetResMgr(ResMgr* pMgr)  {pResMgr = pMgr; }
    ResMgr*         GetResMgr() { return pResMgr; }
    void            SetHelpIds( ResMgr* );
    void            RemoveDisabledEntries();
    void            SetRemoveDisabledEntries();
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
