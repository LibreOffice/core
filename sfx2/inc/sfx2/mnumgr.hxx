/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SFXMNUMGR_HXX
#define _SFXMNUMGR_HXX

#include <stdarg.h>

#ifndef _MENU_HXX //autogen //wg. MENU_APPEND !!!!
#include <vcl/menu.hxx>
#endif
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

    SfxVirtualMenu*         pMenu;          // das eigentliche Menu
    SfxVirtualMenu*         pOldMenu;       // only while reconfiguring
    sal_Bool                    bMenuBar;       // Popup oder MenuBar
    SfxBindings*            pBindings;
    ResMgr*                 pResMgr;
    sal_uInt32              nType;
    sal_Bool                    bAddClipboardFuncs : 1;

    void                    Construct( SfxVirtualMenu& rMenu );

protected:
                            SfxMenuManager( Menu*, SfxBindings& );
                            SfxMenuManager( const ResId&, SfxBindings& );
                            ~SfxMenuManager();
    sal_uInt16                  GetItemPos( sal_uInt16 nId );
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
    void                    SetPopupMenu( sal_uInt16 nId, PopupMenu *pMenu );

//#if 0 // _SOLAR__PRIVATE
    void            Construct_Impl( Menu* pMenu, sal_Bool bWithHelp );
//#endif
};

//--------------------------------------------------------------------

class SAL_DLLPUBLIC_EXPORT SfxPopupMenuManager : public SfxMenuManager
{
private:
    DECL_LINK( SelectHdl, void * );
    Menu*               pSVMenu;

    // when #i107205 gets fixed this one should be superfluous.
    // But right now we want to avoid the memory leak that would otherwise occur,
    // if we don't delete the pointer that got created in SfxPopupMenuManager::Popup
    static PopupMenu *  pStaticThesSubMenu;

    // only declared, but not defined: don't allow copying
    SfxPopupMenuManager( const SfxPopupMenuManager& );
    SfxPopupMenuManager& operator=( const SfxPopupMenuManager& );

public:
                        SfxPopupMenuManager( const ResId&, SfxBindings& );
                        SfxPopupMenuManager( PopupMenu*, SfxBindings& );
                        ~SfxPopupMenuManager();
    static void         ExecutePopup( const ResId&, SfxViewFrame* pViewFrame, const Point& rPoint, Window* pWindow );
    // @deprecated!!
    // Don't use this method any longer. The whole class will be removed in the future.
    // Changing code which relies on Popup would need much more effort.
    // Please contact cd@openoffice.org if you have questions or need help
    static SfxPopupMenuManager* Popup( const ResId& rResId, SfxViewFrame* pFrame,const Point& rPoint, Window* pWindow );

    sal_uInt16              Execute( const Point& rPos, Window *pWindow );
    sal_uInt16              Execute( const Point& rPoint, Window* pWindow, va_list pArgs, const SfxPoolItem *pArg1 );
    sal_uInt16              Execute( const Point& rPoint, Window* pWindow, const SfxPoolItem *pArg1 ... );

    // @deprecated (start)!!
    // Don't use these methods any longer. The whole class will be removed in the future.
    // Changing code which relies on these methods would need much more effort!
    // Please contact cd@openoffice.org if you have questions or need help
    void                StartInsert();
    void                EndInsert();
    void                CheckItem( sal_uInt16, sal_Bool );
    void                RemoveItem( sal_uInt16 );
    void                InsertItem( sal_uInt16, const String&, MenuItemBits, const rtl::OString& rHelpId,
                                sal_uInt16 nPos = MENU_APPEND );
    void                InsertSeparator( sal_uInt16 nPos = MENU_APPEND );
    // @deprecated (end)

    void                RemoveDisabledEntries();
    void                AddClipboardFunctions();
    Menu*                   GetSVMenu();
};

#endif // #ifndef _SFXMNUMGR_HXX

