/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mnumgr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:24:03 $
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
#ifndef _SFXMNUMGR_HXX
#define _SFXMNUMGR_HXX

#include <stdarg.h>

#ifndef _MENU_HXX //autogen //wg. MENU_APPEND !!!!
#include <vcl/menu.hxx>
#endif
#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>
#endif
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_VERBDESCRIPTOR_HPP_
#include <com/sun/star/embed/VerbDescriptor.hpp>
#endif
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

    SfxVirtualMenu*         pMenu;          // das eigentliche Menu
    SfxVirtualMenu*         pOldMenu;       // only while reconfiguring
    BOOL                    bMenuBar;       // Popup oder MenuBar
    SfxBindings*            pBindings;
    ResMgr*                 pResMgr;
    sal_uInt32              nType;
    BOOL                    bAddClipboardFuncs : 1;

    void                    Construct( SfxVirtualMenu& rMenu );

protected:
                            SfxMenuManager( Menu*, SfxBindings& );
                            SfxMenuManager( const ResId&, SfxBindings& );
                            ~SfxMenuManager();
    USHORT                  GetItemPos( USHORT nId );
    sal_uInt32              GetType() { return nType; }
public:

            void            UseDefault();

    DECL_LINK( Select, Menu* );

    SfxVirtualMenu*         GetMenu() const
                            { return pMenu; }

    SfxBindings&            GetBindings() { return *pBindings; }
    const SfxBindings&      GetBindings() const { return *pBindings; }
    void                    SetResMgr(ResMgr* pMgr)  {pResMgr = pMgr; }
    ResMgr*                 GetResMgr() const { return pResMgr; }
    void                    SetPopupMenu( USHORT nId, PopupMenu *pMenu );

//#if 0 // _SOLAR__PRIVATE
    void            Construct_Impl( Menu* pMenu, BOOL bWithHelp );
//#endif
};

//--------------------------------------------------------------------

class SfxPopupMenuManager : public SfxMenuManager
{
private:
    DECL_LINK( SelectHdl, void * );
    Menu*               pSVMenu;

public:
                        SfxPopupMenuManager( const ResId&, SfxBindings& );
                        SfxPopupMenuManager( PopupMenu*, SfxBindings& );
    static void         ExecutePopup( const ResId&, SfxViewFrame* pViewFrame, const Point& rPoint, Window* pWindow );

    USHORT              Execute( const Point& rPos, Window *pWindow );
    USHORT              Execute( const Point& rPoint, Window* pWindow, va_list pArgs, const SfxPoolItem *pArg1 );
    USHORT              Execute( const Point& rPoint, Window* pWindow, const SfxPoolItem *pArg1 ... );

    void                StartInsert();
    void                EndInsert();
    void                CheckItem( USHORT, BOOL );
    void                RemoveItem( USHORT );
    void                InsertItem( USHORT, const String&, MenuItemBits,
                                USHORT nPos = MENU_APPEND );
    void                InsertSeparator( USHORT nPos = MENU_APPEND );
    void                RemoveDisabledEntries();
    void                AddClipboardFunctions();
};

#endif // #ifndef _SFXMNUMGR_HXX

