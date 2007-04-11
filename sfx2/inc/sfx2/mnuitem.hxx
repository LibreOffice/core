/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mnuitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:23:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXMNUITEM_HXX
#define _SFXMNUITEM_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

class SfxVirtualMenu;
class SfxBindings;
class SfxModule;
class Menu;
class PopupMenu;
class SfxUnoMenuControl;
class SfxUnoControllerItem;
struct SfxMenuCtrlFactory;

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
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

    static SfxMenuControl*  CreateControl( USHORT nId, Menu &, SfxBindings & );
    static SfxUnoMenuControl*
                            CreateControl( const String&, USHORT, Menu&, SfxBindings&, SfxVirtualMenu* );
    static BOOL             IsSpecialControl( USHORT nId, SfxModule* );
    static void             RegisterMenuControl(SfxModule*, SfxMenuCtrlFactory*);

};

class SfxUnoMenuControl : public SfxMenuControl
{
    SfxUnoControllerItem*   pUnoCtrl;
public:
                            SfxUnoMenuControl( const String&, USHORT nId, Menu&,
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

//#if 0 // _SOLAR__PRIVATE

class SfxAppMenuControl_Impl : public SfxMenuControl
{
    PopupMenu*  pMenu;
    BOOL        m_bWasHiContrastMode;
    BOOL        m_bShowMenuImages;

protected:
    DECL_LINK( Activate, Menu * ); // Needed to support high contrast images

public:
    SFX_DECL_MENU_CONTROL();
    SfxAppMenuControl_Impl( USHORT nPos, Menu& rMenu, SfxBindings& rBindings );
    ~SfxAppMenuControl_Impl();
};

//#endif

#endif
